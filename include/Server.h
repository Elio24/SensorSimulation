#ifndef SERVER_H
#define SERVER_H
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>

struct StatData {
    double sum = 0, minVal = 0, maxVal = 0;
    int count = 0;
    std::string unit;
    bool boolean = false;
};

struct AlertThresholds {
    std::optional<double> lower;
    std::optional<double> upper;
    std::string unit;
};

class Server {
    std::string name;
    std::filesystem::path sessionDirectory;
    std::map<std::string, StatData> stats;
    std::map<std::string, AlertThresholds> thresholds;
    std::size_t alertCount = 0;
    void checkAlert(int id, const std::string& type, double value, int tick, const std::string& unit);
    void receiveValue(int id, const std::string& type, double value,
                      int tick, const std::string& unit, bool boolean);
public:
    Server();
    explicit Server(const std::string& name, const std::filesystem::path& output = "logs");
    // La copie garde le meme dossier de logs, avec ses propres statistiques.
    Server(const Server& other);
    Server& operator=(const Server& other);
    virtual ~Server();
    void consoleWrite(const std::string& data) const;
    void fileWrite(const std::string& filename, const std::string& data) const;
    void receive(int id, const std::string& type, float value, int tick, const std::string& unit);
    void receive(int id, const std::string& type, int value, int tick, const std::string& unit);
    void receive(int id, const std::string& type, bool value, int tick);
    void printStatistics() const;
    // Si la valeur est juste egale au seuil, on ne lance pas d'alerte.
    void setAlertThresholds(const std::string& type, std::optional<double> lower,
                            std::optional<double> upper, const std::string& unit);
    std::size_t getAlertCount() const { return alertCount; }
    const std::map<std::string, StatData>& getStatistics() const { return stats; }
    const std::filesystem::path& getSessionDirectory() const { return sessionDirectory; }
    friend std::ostream& operator<<(std::ostream& os, const Server& server);
};
#endif
