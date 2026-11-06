#include "SoundSensor.h"
#include "Server.h"
#include <random>
#include <algorithm>
#include <cmath>
#include <stdexcept>

SoundSensor::SoundSensor() : SoundSensor(nullptr) {}
SoundSensor::SoundSensor(Server* server, int minVal, int maxVal, int period)
    : Sensor("Sound", server, period), value(minVal), minVal(minVal), maxVal(maxVal) {
    if (!std::isfinite(minVal) || !std::isfinite(maxVal) || minVal > maxVal)
        throw std::invalid_argument("Plage Sound invalide");
    value = static_cast<int>((static_cast<long long>(minVal) + maxVal) / 2);
}
SoundSensor::SoundSensor(int id, Server* server, int minVal, int maxVal, int period)
    : Sensor(id, "Sound", server, period), value(minVal), minVal(minVal), maxVal(maxVal) {
    if (!std::isfinite(minVal) || !std::isfinite(maxVal) || minVal > maxVal)
        throw std::invalid_argument("Plage Sound invalide");
    value = static_cast<int>((static_cast<long long>(minVal) + maxVal) / 2);
}
SoundSensor::SoundSensor(const SoundSensor& other)
    : Sensor(other), value(other.value), minVal(other.minVal), maxVal(other.maxVal) {}
SoundSensor& SoundSensor::operator=(const SoundSensor& other) {
    if (this != &other) { Sensor::operator=(other); value = other.value; minVal = other.minVal; maxVal = other.maxVal; }
    return *this;
}
SoundSensor::~SoundSensor() = default;
void SoundSensor::execute() {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> delta(-5, 5);
    // On calcule en long long pour ne pas depasser la capacite d'un int.
    value = static_cast<int>(std::clamp(static_cast<long long>(value) + delta(gen),
                                      static_cast<long long>(minVal), static_cast<long long>(maxVal)));
    if (server) server->receive(id, type, value, elapsedTicks, "dB");
}
std::ostream& operator<<(std::ostream& os, const SoundSensor& sensor) {
    return os << "SoundSensor [id=" << sensor.id << ", value=" << sensor.value << " dB, periode=" << sensor.period << "]";
}
