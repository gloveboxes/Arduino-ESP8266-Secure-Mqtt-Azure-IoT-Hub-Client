#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"
#include "Globals.h"
#include "MqttClient.h"
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson - installed via library manager
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager

class Sensor
{
  public:
    Sensor(MqttClient* mqttClient);
    void measure();
    char* toJSON();

    float temperature;
    float humidity;
    int pressure;
    int light;
    int msgId;
    const char *geo;

  protected:
    bool initialised;
    MqttClient* _mqttClient;

  private:
    char buffer[256];
    char isoTime[30];
    char* getISODateTime();
};

#endif
