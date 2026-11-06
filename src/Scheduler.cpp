#include "Scheduler.h"
#include "Sensor.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

Scheduler::Scheduler() {}

Scheduler::Scheduler(const Scheduler& other) : sensors(other.sensors) {}

Scheduler& Scheduler::operator=(const Scheduler& other) {
    if (this != &other) {
        this->sensors = other.sensors;
    }
    return *this;
}

Scheduler::~Scheduler() {}

void Scheduler::addSensor(Sensor* sensor) {
    if (sensor && std::find(sensors.begin(), sensors.end(), sensor) == sensors.end()) {
        sensors.push_back(sensor);
    }
}

void Scheduler::simulation(int ticks) {
    if (ticks < 0) throw std::invalid_argument("Nombre de ticks negatif");
    std::cout << " Debut de la simulation (" << ticks << " ticks) " << std::endl;
    for (int t = 0; t < ticks; ++t) {
        std::cout << "[Tick " << t + 1 << "]" << std::endl;
        for (Sensor* s : sensors) {
            if (s) {
                s->update();
            }
        }
    }
    std::cout << "Fin de la simulation " << std::endl;
}

std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler) {
    os << "Scheduler gerant " << scheduler.sensors.size() << " capteurs.";
    return os;
}
