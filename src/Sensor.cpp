#include "Sensor.h"
#include <limits>
#include <stdexcept>

long long Sensor::nextId = 1;
std::set<int> Sensor::usedIds;
int Sensor::allocateId() {
    while (nextId <= std::numeric_limits<int>::max() && usedIds.count(static_cast<int>(nextId))) ++nextId;
    if (nextId > std::numeric_limits<int>::max()) throw std::overflow_error("Identifiants epuises");
    const int result = static_cast<int>(nextId++);
    usedIds.insert(result);
    return result;
}
int Sensor::reserveId(int requested) {
    if (requested <= 0 || !usedIds.insert(requested).second)
        throw std::invalid_argument("Identifiant negatif, nul ou deja utilise");
    return requested;
}
Sensor::Sensor() : Sensor("Generic") {}
Sensor::Sensor(const std::string& type, Server* server, int period)
    : id(allocateId()), type(type), server(server), period(period), tickCounter(0) {
    if (period <= 0) throw std::invalid_argument("Periode strictement positive requise");
}
Sensor::Sensor(int id, const std::string& type, Server* server, int period)
    : id(reserveId(id)), type(type), server(server), period(period), tickCounter(0) {
    if (period <= 0) throw std::invalid_argument("Periode strictement positive requise");
}
Sensor::Sensor(const Sensor& other)
    : id(allocateId()), type(other.type), server(other.server), period(other.period),
      tickCounter(other.tickCounter), elapsedTicks(other.elapsedTicks) {}
Sensor& Sensor::operator=(const Sensor& other) {
    if (this != &other) {
        // On copie les donnees, mais on garde l'identifiant de ce capteur.
        type = other.type; server = other.server; period = other.period;
        tickCounter = other.tickCounter; elapsedTicks = other.elapsedTicks;
    }
    return *this;
}
Sensor::~Sensor() = default;
void Sensor::setPeriod(int p) {
    if (p <= 0) throw std::invalid_argument("Periode strictement positive requise");
    period = p;
    tickCounter = 0; // On repart de zero avec la nouvelle periode.
}
void Sensor::update() {
    if (elapsedTicks == std::numeric_limits<int>::max()) throw std::overflow_error("Limite de ticks atteinte");
    ++elapsedTicks;
    if (++tickCounter >= period) { tickCounter = 0; execute(); }
}
std::ostream& operator<<(std::ostream& os, const Sensor& sensor) {
    return os << "Sensor [id=" << sensor.id << ", type=" << sensor.type << ", periode=" << sensor.period << "]";
}
