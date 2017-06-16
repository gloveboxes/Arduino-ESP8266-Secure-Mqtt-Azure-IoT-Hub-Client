#ifndef Bmp280_h
#define Bmp280_h

#include "Sensor.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

class Bmp280 : public Sensor
{
public:
  Bmp280(MqttClient *mqttClient) : Sensor(mqttClient){};
  void measure();

private:
  void initialise();
  Adafruit_BMP280 bmp280; // I2C
};

#endif
