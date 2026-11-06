#include "HumiditySensor.h"
#include "Server.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <stdexcept>

HumiditySensor::HumiditySensor() : HumiditySensor(nullptr) {}
HumiditySensor::HumiditySensor(Server* server, float minVal, float maxVal, int period)
    : Sensor("Humidity", server, period), value(minVal), minVal(minVal), maxVal(maxVal) {
    if (!std::isfinite(minVal) || !std::isfinite(maxVal) || minVal > maxVal || !std::isfinite(maxVal - minVal))
        throw std::invalid_argument("Plage Humidity invalide");
    value = static_cast<float>((static_cast<double>(minVal) + maxVal) / 2.0);
}
HumiditySensor::HumiditySensor(int id, Server* server, float minVal, float maxVal, int period)
    : Sensor(id, "Humidity", server, period), value(minVal), minVal(minVal), maxVal(maxVal) {
    if (!std::isfinite(minVal) || !std::isfinite(maxVal) || minVal > maxVal || !std::isfinite(maxVal - minVal))
        throw std::invalid_argument("Plage Humidity invalide");
    value = static_cast<float>((static_cast<double>(minVal) + maxVal) / 2.0);
}
HumiditySensor::HumiditySensor(const HumiditySensor& other)
    : Sensor(other), value(other.value), minVal(other.minVal), maxVal(other.maxVal) {}
HumiditySensor& HumiditySensor::operator=(const HumiditySensor& other) {
    if (this != &other) { Sensor::operator=(other); value = other.value; minVal = other.minVal; maxVal = other.maxVal; }
    return *this;
}
HumiditySensor::~HumiditySensor() = default;
void HumiditySensor::execute() {
    static std::mt19937 gen(std::random_device{}());
    // L'humidite varie de 2 points maximum, par exemple de 50 % a 52 %.
    std::uniform_real_distribution<double> delta(-2.0, 2.0);
    value = static_cast<float>(std::clamp(static_cast<double>(value) + delta(gen),
                                        static_cast<double>(minVal), static_cast<double>(maxVal)));
    if (server) server->receive(id, type, value, elapsedTicks, "%");
}
std::ostream& operator<<(std::ostream& os, const HumiditySensor& sensor) {
    return os << "HumiditySensor [id=" << sensor.id << ", value=" << sensor.value << " %, periode=" << sensor.period << "]";
}
