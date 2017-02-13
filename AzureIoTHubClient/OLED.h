#ifndef Display_h
#define Display_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Sensor.h"

#define OLED_RESET 0  // GPIO0

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif



class OLED
{
  public:
    OLED(Sensor* sensor){
      _sensor = sensor;
      _oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
      _oled.clearDisplay();
      _oled.display();
    }
    void sensorData();
    void text(const char* text);


  private:
    Adafruit_SSD1306 _oled = Adafruit_SSD1306(OLED_RESET);
    Sensor* _sensor;
};

#endif
