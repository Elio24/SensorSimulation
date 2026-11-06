#ifndef SOUND_SENSOR_H
#define SOUND_SENSOR_H

#include "Sensor.h"

class SoundSensor : public Sensor {
private:
    int value;
    int minVal;
    int maxVal;

public:
    // Constructeurs, copie et destructeur (Coplien)
    SoundSensor();
    SoundSensor(Server* server, int minVal = 30, int maxVal = 85, int period = 1);
    SoundSensor(int id, Server* server, int minVal = 30, int maxVal = 85, int period = 1);
    SoundSensor(const SoundSensor& other);
    SoundSensor& operator=(const SoundSensor& other);
    virtual ~SoundSensor();

    // Prend une nouvelle mesure de bruit
    void execute() override;

    int getValue() const { return value; }

    friend std::ostream& operator<<(std::ostream& os, const SoundSensor& sensor);
};

#endif
