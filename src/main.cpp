#include <fstream>
#include <iostream>
#include "Server.h"
#include "Scheduler.h"
#include "TemperatureSensor.h"
#include "HumiditySensor.h"
#include "SoundSensor.h"
#include "LightSensor.h"

int main() {
    try {
        Server server("Central-IOT");
        // Seuils choisis pour la demo, ce ne sont pas des normes officielles.
        server.setAlertThresholds("Temperature", std::nullopt, 25.0, "degC");
        server.setAlertThresholds("Humidity", 45.0, std::nullopt, "%");
        server.setAlertThresholds("Sound", std::nullopt, 70.0, "dB");
        server.fileWrite("server.log", "Demarrage du serveur Central-IOT");
        TemperatureSensor temperature(&server);
        // On choisit un bureau chaud, sec et bruyant pour voir les alertes.
        TemperatureSensor temperatureBureau2(&server, 26.0f, 28.0f, 3);
        SoundSensor sound(&server, 72, 82);
        HumiditySensor humidity(&server, 35.0f, 45.0f);
        server.consoleWrite("Scenario de demonstration : bureau 2 chaud, air sec et bruit eleve.");
        LightSensor light(&server);
        Scheduler scheduler;
        scheduler.addSensor(&temperature);
        scheduler.addSensor(&temperatureBureau2);
        scheduler.addSensor(&sound);
        scheduler.addSensor(&humidity);
        scheduler.addSensor(&light);
        // On utilise << pour afficher les objets, puis les ecrire dans un fichier.
        std::cout << server << '\n' << scheduler << '\n' << temperature << '\n'
                  << temperatureBureau2 << '\n' << sound << '\n' << humidity << '\n' << light << '\n';
        std::ofstream inventory(server.getSessionDirectory() / "inventaire.log");
        inventory << server << '\n' << scheduler << '\n' << temperature << '\n'
                  << temperatureBureau2 << '\n' << sound << '\n' << humidity << '\n' << light << '\n';
        inventory.close();
        if (!inventory) throw std::runtime_error("Ecriture inventaire impossible");
        scheduler.simulation(6);
        server.printStatistics();
        std::cout << "Journaux : " << server.getSessionDirectory().string() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Erreur : " << error.what() << '\n';
        return 1;
    }
    return 0;
}
