#include "TemperatureSensor.h"
#include "Server.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <stdexcept>

TemperatureSensor::TemperatureSensor() : TemperatureSensor(nullptr) {}
TemperatureSensor::TemperatureSensor(Server* server, float minVal, float maxVal, int period)
    : Sensor("Temperature", server, period), value(minVal), minVal(minVal), maxVal(maxVal) {
    if (!std::isfinite(minVal) || !std::isfinite(maxVal) || minVal > maxVal || !std::isfinite(maxVal - minVal))
        throw std::invalid_argument("Plage Temperature invalide");
    value = static_cast<float>((static_cast<double>(minVal) + maxVal) / 2.0);
}
TemperatureSensor::TemperatureSensor(int id, Server* server, float minVal, float maxVal, int period)
    : Sensor(id, "Temperature", server, period), value(minVal), minVal(minVal), maxVal(maxVal) {
    if (!std::isfinite(minVal) || !std::isfinite(maxVal) || minVal > maxVal || !std::isfinite(maxVal - minVal))
        throw std::invalid_argument("Plage Temperature invalide");
    value = static_cast<float>((static_cast<double>(minVal) + maxVal) / 2.0);
}
TemperatureSensor::TemperatureSensor(const TemperatureSensor& other)
    : Sensor(other), value(other.value), minVal(other.minVal), maxVal(other.maxVal) {}
TemperatureSensor& TemperatureSensor::operator=(const TemperatureSensor& other) {
    if (this != &other) { Sensor::operator=(other); value = other.value; minVal = other.minVal; maxVal = other.maxVal; }
    return *this;
}
TemperatureSensor::~TemperatureSensor() = default;
void TemperatureSensor::execute() {
    static std::mt19937 gen(std::random_device{}());
    // La temperature monte ou descend de 0.5 degre maximum a chaque mesure.
    std::uniform_real_distribution<double> delta(-0.5, 0.5);
    value = static_cast<float>(std::clamp(static_cast<double>(value) + delta(gen),
                                        static_cast<double>(minVal), static_cast<double>(maxVal)));
    if (server) server->receive(id, type, value, elapsedTicks, "degC");
}
std::ostream& operator<<(std::ostream& os, const TemperatureSensor& sensor) {
    return os << "TemperatureSensor [id=" << sensor.id << ", value=" << sensor.value << " degC, periode=" << sensor.period << "]";
}
