#include "Server.h"
#include "Scheduler.h"
#include "TemperatureSensor.h"
#include "HumiditySensor.h"
#include "SoundSensor.h"
#include "LightSensor.h"
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class F> void rejects(F action) {
    bool caught = false;
    try { action(); } catch (const std::invalid_argument&) { caught = true; }
    check(caught, "Une entree invalide a ete acceptee");
}
struct Counter : Sensor {
    int calls = 0;
    explicit Counter(int period = 1) : Sensor("Counter", nullptr, period) {}
    void execute() override { ++calls; }
};
int main() {
    try {
        static_assert(std::is_same_v<decltype(SoundSensor().getValue()), int>);
        TemperatureSensor defaults;
        check(defaults.getPeriod() == 2, "Periode de temperature par defaut");
        TemperatureSensor copy(defaults);
        check(copy.getId() != defaults.getId(), "Identifiant de copie duplique");
        int identity = copy.getId();
        copy = defaults;
        check(copy.getId() == identity, "L'affectation change l'identite");
        copy = copy;
        check(copy.getId() == identity, "Auto-affectation");
        rejects([&] { TemperatureSensor duplicate(defaults.getId(), nullptr); });
        rejects([] { LightSensor invalid(0, nullptr); });
        LightSensor explicitId(1000, nullptr);
        rejects([] { LightSensor duplicate(1000, nullptr); });
        rejects([] { TemperatureSensor invalid(nullptr, 26, 12); });
        rejects([] { TemperatureSensor invalid(nullptr, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max()); });
        rejects([] { HumiditySensor invalid(nullptr, std::numeric_limits<float>::quiet_NaN(), 70); });
        rejects([] { SoundSensor invalid(nullptr, 85, 30); });
        rejects([] { LightSensor invalid(nullptr, 0); });
        rejects([&] { defaults.setPeriod(-1); });
        HumiditySensor randomHumidity;
        SoundSensor randomSound;
        for (int i = 0; i < 100; ++i) {
            const auto previousTemperature = defaults.getValue();
            const auto previousHumidity = randomHumidity.getValue();
            const auto previousSound = randomSound.getValue();
            defaults.execute(); randomHumidity.execute(); randomSound.execute();
            check(std::abs(defaults.getValue() - previousTemperature) <= 0.5001f, "Saut de temperature");
            check(std::abs(randomHumidity.getValue() - previousHumidity) <= 2.0001f, "Saut d'humidite");
            check(std::abs(randomSound.getValue() - previousSound) <= 5, "Saut sonore");
            check(defaults.getValue() >= 12 && defaults.getValue() <= 26, "Plage temperature");
            check(randomHumidity.getValue() >= 40 && randomHumidity.getValue() <= 70, "Plage humidite");
            check(randomSound.getValue() >= 30 && randomSound.getValue() <= 85, "Plage sonore");
        }

        Counter counter(2);
        Scheduler scheduler;
        scheduler.addSensor(nullptr);
        scheduler.addSensor(&counter);
        scheduler.addSensor(&counter);
        scheduler.simulation(0);
        check(counter.calls == 0, "Zero tick");
        rejects([&] { scheduler.simulation(-1); });
        scheduler.simulation(6);
        check(counter.calls == 3, "Periode ou ajout en double");
        Scheduler shared(scheduler);
        shared.simulation(2);
        check(counter.calls == 4, "La copie ne partage pas les capteurs");
        counter.setPeriod(3);
        scheduler.simulation(2);
        check(counter.calls == 4, "Reinitialisation de periode");
        scheduler.simulation(1);
        check(counter.calls == 5, "Nouvelle periode");

        Server server("Test", "test-logs");
        Server second("Test", "test-logs");
        check(server.getSessionDirectory() != second.getSessionDirectory(), "Sessions confondues");
        TemperatureSensor a(&server, 20, 20, 2), b(&server, 24, 24, 3);
        SoundSensor sound(&server, 40, 40);
        HumiditySensor humidity(&server, 50, 50);
        LightSensor light(&server);
        Scheduler simulation;
        for (Sensor* sensor : {static_cast<Sensor*>(&a), static_cast<Sensor*>(&b),
             static_cast<Sensor*>(&sound), static_cast<Sensor*>(&humidity), static_cast<Sensor*>(&light)})
            simulation.addSensor(sensor);
        simulation.simulation(6);
        const auto& stats = server.getStatistics();
        check(stats.at("Temperature").count == 5 && stats.at("Temperature").sum == 108, "Agregation multi-capteurs");
        check(stats.at("Temperature").minVal == 20 && stats.at("Temperature").maxVal == 24, "Min/max");
        check(stats.at("Sound").count == 6 && stats.at("Humidity").count == 2 && stats.at("Light").count == 6, "Frequences");
        check(stats.at("Light").sum >= 0 && stats.at("Light").sum <= 6, "Valeurs booleennes");
        Server serverCopy(server);
        serverCopy.receive(999, "Custom", 2, 1, "u");
        check(server.getStatistics().count("Custom") == 0, "Statistiques de copie non independantes");
        check(serverCopy.getSessionDirectory() == server.getSessionDirectory(), "Destination de copie");
        Server assigned("Assigned", "test-logs");
        assigned = server;
        check(assigned.getStatistics().at("Sound").count == 6, "Affectation serveur");
        rejects([&] { server.receive(999, "../bad", 1, 1, "u"); });
        rejects([&] { server.receive(999, "Sound", 1, 1, "%"); });

        std::ifstream csv(server.getSessionDirectory() / "log_Temperature.csv");
        std::string line;
        std::getline(csv, line);
        check(line == "Tick;SensorID;Type;Valeur;Unite", "Entete CSV");
        std::getline(csv, line);
        check(line.rfind("2;", 0) == 0, "Tick de premiere mesure");
        int rows = 1;
        while (std::getline(csv, line)) ++rows;
        check(rows == 5, "Nombre de lignes CSV");
        server.fileWrite("server.log", "Demarrage");
        std::ifstream log(server.getSessionDirectory() / "server.log");
        std::getline(log, line);
        check(line == "Demarrage", "Entete CSV dans journal texte");
        std::ostringstream stream;
        stream << server << a << sound << humidity << light << simulation;
        check(!stream.str().empty(), "Operateurs de flux");
        std::ofstream inventory(server.getSessionDirectory() / "inventaire.log");
        inventory << a << '\n'; inventory.close();
        check(static_cast<bool>(inventory), "Redirection vers fichier");

        // Un dossier a la place du fichier permet de tester une erreur d'ecriture.
        std::filesystem::create_directory(second.getSessionDirectory() / "log_Blocked.csv");
        bool ioFailure = false;
        try { second.receive(999, "Blocked", 1, 1, "u"); }
        catch (const std::runtime_error&) { ioFailure = true; }
        check(ioFailure && second.getStatistics().empty(), "Erreur disque non propagee");

        // On teste les alertes au-dessus et en dessous des seuils, puis les cas limites.
        Server alarms("Alerts", "test-logs");
        alarms.setAlertThresholds("Temperature", std::nullopt, 25.0, "degC");
        alarms.setAlertThresholds("Humidity", 45.0, std::nullopt, "%");
        alarms.setAlertThresholds("Sound", std::nullopt, 70.0, "dB");
        alarms.receive(1, "Temperature", 25.0f, 1, "degC");
        alarms.receive(2, "Humidity", 45.0f, 1, "%");
        alarms.receive(3, "Sound", 70, 1, "dB");
        check(alarms.getAlertCount() == 0, "Egalite au seuil");
        alarms.receive(1, "Temperature", 26.0f, 2, "degC");
        alarms.receive(1, "Temperature", 26.5f, 3, "degC");
        alarms.receive(2, "Humidity", 44.0f, 2, "%");
        alarms.receive(3, "Sound", 71, 2, "dB");
        check(alarms.getAlertCount() == 4, "Alertes de seuil haut/bas ou repetition");
        alarms.receive(1, "Temperature", 24.0f, 4, "degC");
        check(alarms.getAlertCount() == 4, "Alerte en zone normale");
        std::ifstream alertsFile(alarms.getSessionDirectory() / "alerts.csv");
        std::getline(alertsFile, line);
        check(line == "Tick;SensorID;Type;Valeur;Unite;Direction;Seuil", "Entete des alertes");
        std::getline(alertsFile, line);
        check(line == "2;1;Temperature;26;degC;HAUT;25", "Contenu d'alerte haute");
        std::getline(alertsFile, line);
        std::getline(alertsFile, line);
        check(line == "2;2;Humidity;44;%;BAS;45", "Contenu d'alerte basse");
        int alertRows = 3;
        while (std::getline(alertsFile, line)) ++alertRows;
        check(alertRows == 4, "Nombre d'alertes sauvegardees");
        rejects([&] { alarms.setAlertThresholds("X", 10, 5, "u"); });
        rejects([&] { alarms.setAlertThresholds("X", std::nullopt, std::nullopt, "u"); });
        rejects([&] { alarms.setAlertThresholds("X", std::numeric_limits<double>::infinity(), std::nullopt, "u"); });
        rejects([&] { alarms.setAlertThresholds("Sound", std::nullopt, 80, "%"); });
        rejects([&] { alarms.receive(1, "Temperature", 30.0f, 5, "F"); });
        Server alarmCopy(alarms);
        alarmCopy.receive(3, "Sound", 80, 3, "dB");
        check(alarmCopy.getAlertCount() == 5 && alarms.getAlertCount() == 4, "Copie des seuils et compteurs");
        Server both("BothLimits", "test-logs");
        both.setAlertThresholds("Custom", 10, 20, "u");
        both.receive(1, "Custom", 9, 1, "u");
        both.receive(1, "Custom", 21, 2, "u");
        both.receive(1, "Custom", 15, 3, "u");
        check(both.getAlertCount() == 2, "Double borne");
        Server failedAlert("FailedAlert", "test-logs");
        failedAlert.setAlertThresholds("Sound", std::nullopt, 70, "dB");
        std::filesystem::create_directory(failedAlert.getSessionDirectory() / "alerts.csv");
        ioFailure = false;
        try { failedAlert.receive(1, "Sound", 80, 1, "dB"); }
        catch (const std::runtime_error&) { ioFailure = true; }
        check(ioFailure && failedAlert.getAlertCount() == 0 && failedAlert.getStatistics().empty(), "Erreur d'ecriture des alertes");
        std::cout << "Tous les contrats du simulateur sont verifies.\n";
    } catch (const std::exception& error) {
        std::cerr << "ECHEC : " << error.what() << '\n'; return 1;
    }
}
