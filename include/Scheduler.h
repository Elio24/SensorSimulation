#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <vector>

class Sensor;

class Scheduler {
private:
    std::vector<Sensor*> sensors; // Une copie de l'ordonnanceur utilise les memes capteurs, sans les detruire.

public:
    Scheduler();
    Scheduler(const Scheduler& other);
    Scheduler& operator=(const Scheduler& other);
    virtual ~Scheduler();

    void addSensor(Sensor* sensor);
    void simulation(int ticks);

    friend std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler);
};

#endif
