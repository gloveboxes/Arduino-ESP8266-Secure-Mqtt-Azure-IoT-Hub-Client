#include "OLED.h"

void OLED::text(const char* text){
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(WHITE);
  _oled.setCursor(0,0);
  _oled.print(text);
  _oled.display();
}

void OLED::sensorData(){
  _oled.clearDisplay();

  _oled.setTextSize(1);
  _oled.setTextColor(WHITE);
  _oled.setCursor(0,0);
  _oled.print("T:");
  _oled.println(_sensor->temperature);
  _oled.print("P:");
  _oled.println(_sensor->pressure);
  _oled.print("H:");
  _oled.println(_sensor->humidity);
  _oled.print("L:");
  _oled.println(_sensor->light);
  _oled.print("M:");
  _oled.println(ESP.getFreeHeap());
  _oled.print("I:");
  _oled.println(_sensor->msgId);

  _oled.display();
}
