#ifndef globals_h
#define globals_h


enum BoardType {
  NodeMCU,
  WeMos,
  SparkfunThing,
  Other
};



struct DeviceConfig {
  int WifiIndex = 0;
  int wifiPairs = 0;
  const char **ssid;
  const char **pwd;
  BoardType boardType = Other;            // OperationMode enumeration: NodeMCU, WeMos, SparkfunThing, Other 
  unsigned int deepSleepSeconds = 0;      // Number of seconds for the ESP8266 chip to deepsleep for.  GPIO16 needs to be tied to RST to wake from deepSleep http://esp8266.github.io/Arduino/versions/2.0.0/doc/libraries.html
  unsigned int publishRateInSeconds = 60; // defaults to once a minute
  unsigned long lastPublishTime = 0;
};



#endif

