#include "LightSensor.h"
#include "Server.h"
#include <random>

LightSensor::LightSensor() 
    : Sensor("Light", nullptr, 1), value(false) {}

LightSensor::LightSensor(Server* server, int period) 
    : Sensor("Light", server, period), value(false) {}

LightSensor::LightSensor(int id, Server* server, int period) 
    : Sensor(id, "Light", server, period), value(false) {}

LightSensor::LightSensor(const LightSensor& other) 
    : Sensor(other), value(other.value) {}

LightSensor& LightSensor::operator=(const LightSensor& other) {
    if (this != &other) {
        Sensor::operator=(other);
        this->value = other.value;
    }
    return *this;
}

LightSensor::~LightSensor() {}

void LightSensor::execute() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    value = (dist(gen) == 1);

    if (server) server->receive(id, type, value, elapsedTicks);
}

std::ostream& operator<<(std::ostream& os, const LightSensor& sensor) {
    os << "LightSensor [id=" << sensor.id << ", value=" << (sensor.value ? "Allumé" : "Éteint") << "]";
    return os;
}
