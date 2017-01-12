#include "Bmp280.h"
#define delay(s) _mqttClient->mqttDelay(s)


void Bmp280::initialise(){
  if (initialised) { return; }  
  bmp280.begin();
  delay(100);  
  initialised = true;
}

void Bmp280::measure(){
 const int numberOfSamples = 4;

  initialise();
  temperature = pressure = humidity = 0;
   
  for (int c = 0; c < numberOfSamples; c++) {  
    temperature += bmp280.readTemperature(); 
    pressure += (int)((int)( bmp280.readPressure() + 0.5) / 100);
    delay(500);
  }
  
  temperature /= numberOfSamples;
  pressure /= numberOfSamples;  
}

