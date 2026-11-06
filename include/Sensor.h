#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <string>
#include <set>

class Server;

class Sensor {
protected:
    const int id;
    std::string type;
    Server* server; // Le serveur doit exister tant que le capteur l'utilise.
    int period;       // Nombre de ticks entre deux mesures
    int tickCounter;  // Ticks passes depuis la derniere mesure
    int elapsedTicks = 0;
private:
    static long long nextId;
    static std::set<int> usedIds;
    static int allocateId();
    static int reserveId(int requested);
public:
    // Constructeurs, copie et destructeur (Coplien)
    Sensor();
    Sensor(const std::string& type, Server* server = nullptr, int period = 1);
    Sensor(int id, const std::string& type, Server* server = nullptr, int period = 1);
    Sensor(const Sensor& other);
    Sensor& operator=(const Sensor& other);
    virtual ~Sensor();

    // Appelee par l'ordonnanceur a chaque tick
    virtual void update();

    // Chaque type de capteur definit sa facon de prendre une mesure
    virtual void execute() = 0;

    // Acces aux informations du capteur
    int getId() const { return id; }
    std::string getType() const { return type; }
    Server* getServer() const { return server; }
    int getPeriod() const { return period; }
    void setPeriod(int p);
    int getElapsedTicks() const { return elapsedTicks; }

    friend std::ostream& operator<<(std::ostream& os, const Sensor& sensor);
};

#endif // SENSOR_H
