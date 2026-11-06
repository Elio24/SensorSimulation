#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "Sensor.h"

class TemperatureSensor : public Sensor {
private:
    float value;
    float minVal;
    float maxVal;

public:
    // Constructeurs, copie et destructeur (Coplien)
    TemperatureSensor();
    TemperatureSensor(Server* server, float minVal = 12.0f, float maxVal = 26.0f, int period = 2);
    TemperatureSensor(int id, Server* server, float minVal = 12.0f, float maxVal = 26.0f, int period = 2);
    TemperatureSensor(const TemperatureSensor& other);
    TemperatureSensor& operator=(const TemperatureSensor& other);
    virtual ~TemperatureSensor();

    // Prend une nouvelle mesure de temperature
    void execute() override;

    // Derniere valeur du capteur
    float getValue() const { return value; }

    friend std::ostream& operator<<(std::ostream& os, const TemperatureSensor& sensor);
};

#endif // TEMPERATURE_SENSOR_H
