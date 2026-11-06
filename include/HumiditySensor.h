#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include "Sensor.h"

class HumiditySensor : public Sensor {
private:
    float value;
    float minVal;
    float maxVal;

public:
    // Constructeurs, copie et destructeur (Coplien)
    HumiditySensor();
    HumiditySensor(Server* server, float minVal = 40.0f, float maxVal = 70.0f, int period = 3);
    HumiditySensor(int id, Server* server, float minVal = 40.0f, float maxVal = 70.0f, int period = 3);
    HumiditySensor(const HumiditySensor& other);
    HumiditySensor& operator=(const HumiditySensor& other);
    virtual ~HumiditySensor();

    // Prend une nouvelle mesure d'humidite
    void execute() override;

    float getValue() const { return value; }

    friend std::ostream& operator<<(std::ostream& os, const HumiditySensor& sensor);
};

#endif
