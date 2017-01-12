#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"
#include "Globals.h"
#include "MqttClient.h"

class Sensor
{
  public:
    Sensor(Telemetry* data, MqttClient* mqttClient);
    void measure();

  protected:
    bool initialised;
    Telemetry* _data; 
    MqttClient* _mqttClient;
};

#endif
