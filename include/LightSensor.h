#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include "Sensor.h"

class LightSensor : public Sensor {
private:
    bool value;

public:
    // Constructeurs, copie et destructeur (Coplien)
    LightSensor();
    LightSensor(Server* server, int period = 1);
    LightSensor(int id, Server* server, int period = 1);
    LightSensor(const LightSensor& other);
    LightSensor& operator=(const LightSensor& other);
    virtual ~LightSensor();

    // Choisit si la lumiere est allumee ou eteinte
    void execute() override;

    // Dernier etat de la lumiere
    bool getValue() const { return value; }

    friend std::ostream& operator<<(std::ostream& os, const LightSensor& sensor);
};

#endif 
