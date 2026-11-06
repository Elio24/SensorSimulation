#include "Server.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

Server::Server() : Server("DefaultServer") {}
Server::Server(const std::string& name, const std::filesystem::path& output) : name(name) {
    std::filesystem::create_directories(output);
    const auto stamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    for (unsigned int n = 0; ; ++n) {
        sessionDirectory = output / ("simulation_" + std::to_string(stamp) + "_" + std::to_string(n));
        if (std::filesystem::create_directory(sessionDirectory)) break;
    }
}
Server::Server(const Server& other) = default;
Server& Server::operator=(const Server& other) = default;
Server::~Server() = default;

void Server::consoleWrite(const std::string& data) const {
    std::cout << "[Server " << name << "] " << data << '\n';
}
void Server::fileWrite(const std::string& filename, const std::string& data) const {
    const std::filesystem::path relative(filename);
    if (relative.has_parent_path() || relative.empty())
        throw std::invalid_argument("Nom de journal invalide");
    std::ofstream file(sessionDirectory / relative, std::ios::app);
    file << data << '\n';
    file.close();
    if (!file) throw std::runtime_error("Ecriture impossible : " + (sessionDirectory / relative).string());
}
void Server::receive(int id, const std::string& type, float value, int tick, const std::string& unit) {
    receiveValue(id, type, value, tick, unit, false);
}
void Server::receive(int id, const std::string& type, int value, int tick, const std::string& unit) {
    receiveValue(id, type, value, tick, unit, false);
}
void Server::receive(int id, const std::string& type, bool value, int tick) {
    receiveValue(id, type, value ? 1.0 : 0.0, tick, "", true);
}
void Server::setAlertThresholds(const std::string& type, std::optional<double> lower,
                                std::optional<double> upper, const std::string& unit) {
    if (type.empty() || type.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos ||
        (!lower && !upper) || (lower && !std::isfinite(*lower)) ||
        (upper && !std::isfinite(*upper)) || (lower && upper && *lower > *upper) ||
        unit.find_first_of(";\r\n") != std::string::npos)
        throw std::invalid_argument("Seuils d'alerte invalides");
    const auto found = stats.find(type);
    if (found != stats.end() && (found->second.unit != unit || found->second.boolean))
        throw std::invalid_argument("Unite ou type incompatible avec les seuils");
    thresholds[type] = {lower, upper, unit};
}
void Server::checkAlert(int id, const std::string& type, double value, int tick, const std::string& unit) {
    const auto rule = thresholds.find(type);
    if (rule == thresholds.end()) return;
    const auto& limits = rule->second;
    const bool low = limits.lower && value < *limits.lower;
    const bool high = limits.upper && value > *limits.upper;
    if (!low && !high) return;
    const double limit = low ? *limits.lower : *limits.upper;
    const std::string direction = low ? "BAS" : "HAUT";
    const auto path = sessionDirectory / "alerts.csv";
    if (!std::filesystem::exists(path) || std::filesystem::file_size(path) == 0)
        fileWrite("alerts.csv", "Tick;SensorID;Type;Valeur;Unite;Direction;Seuil");
    std::ostringstream row;
    row.imbue(std::locale::classic());
    row << tick << ';' << id << ';' << type << ';' << std::setprecision(9)
        << value << ';' << unit << ';' << direction << ';' << limit;
    fileWrite("alerts.csv", row.str());
    ++alertCount;
    std::ostringstream message;
    message << "ALERTE " << direction << " | Tick " << tick << " | Sensor " << id
            << " (" << type << ") : " << value << ' ' << unit
            << (low ? " < " : " > ") << limit << ' ' << unit;
    consoleWrite(message.str());
}
void Server::receiveValue(int id, const std::string& type, double value,
                          int tick, const std::string& unit, bool boolean) {
    if (id <= 0 || tick < 0 || !std::isfinite(value) || type.empty() ||
        type.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") != std::string::npos ||
        unit.find_first_of(";\r\n") != std::string::npos)
        throw std::invalid_argument("Mesure invalide");
    const auto found = stats.find(type);
    if (found != stats.end() && (found->second.unit != unit || found->second.boolean != boolean))
        throw std::invalid_argument("Metadonnees incompatibles pour ce type");
    const auto rule = thresholds.find(type);
    if (rule != thresholds.end() && (boolean || rule->second.unit != unit))
        throw std::invalid_argument("Mesure incompatible avec les seuils configures");
    const auto filename = "log_" + type + ".csv";
    const auto path = sessionDirectory / filename;
    if (!std::filesystem::exists(path) || std::filesystem::file_size(path) == 0)
        fileWrite(filename, "Tick;SensorID;Type;Valeur;Unite");
    std::ostringstream val;
    val.imbue(std::locale::classic());
    val << std::setprecision(9) << value;
    fileWrite(filename, std::to_string(tick) + ";" + std::to_string(id) + ";" + type + ";" + val.str() + ";" + unit);
    checkAlert(id, type, value, tick, unit);
    // On ajoute la mesure aux statistiques seulement si l'ecriture a reussi.
    auto& s = stats[type];
    if (s.count == 0) { s.minVal = s.maxVal = value; s.unit = unit; s.boolean = boolean; }
    else { if (value < s.minVal) s.minVal = value; if (value > s.maxVal) s.maxVal = value; }
    s.sum += value;
    ++s.count;
    std::ostringstream message;
    message << "Tick " << tick << " | Sensor " << id << " (" << type << ") : ";
    if (boolean) message << (value != 0 ? "Allume (1)" : "Eteint (0)");
    else message << std::fixed << std::setprecision(2) << value << ' ' << unit;
    consoleWrite(message.str());
}
void Server::printStatistics() const {
    std::cout << "\n--- Statistiques de la simulation ---\n";
    std::cout << alertCount << " alerte(s) enregistree(s).\n";
    if (stats.empty()) std::cout << "Aucune mesure.\n";
    for (const auto& entry : stats) {
        const auto& s = entry.second;
        std::ostringstream line;
        line << entry.first << " : " << std::fixed;
        if (s.boolean) line << std::setprecision(1) << 100.0 * s.sum / s.count << "% des mesures a 1";
        else line << std::setprecision(2) << "moyenne=" << s.sum / s.count << ' ' << s.unit
                  << ", min=" << s.minVal << ", max=" << s.maxVal;
        line << " (" << s.count << " mesures)";
        std::cout << line.str() << '\n';
    }
}
std::ostream& operator<<(std::ostream& os, const Server& server) {
    return os << "Server(name=" << server.name << ")";
}
