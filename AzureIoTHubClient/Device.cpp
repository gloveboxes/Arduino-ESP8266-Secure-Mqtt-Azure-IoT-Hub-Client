#include "Device.h"

void Device::initialise(const char *ssid, const char *pwd)
{
  wifiPairs = 1;
  _ssid = new const char *[wifiPairs];
  _pwd = new const char *[wifiPairs];
  _ssid[0] = ssid;
  _pwd[0] = pwd;
}

void Device::initialise(const char **ssid, const char **pwd, int ssidCount, int pwdCount)
{
  _ssid = ssid;
  _pwd = pwd;
#ifdef ARDUINO_ARCH_ESP8266
  WiFi.mode(WIFI_STA); // Ensure WiFi in Station/Client Mode
#endif

  wifiPairs = ssidCount < pwdCount ? ssidCount : pwdCount;
  wifiPairs /= sizeof(char *);
}

bool Device::connectWifi()
{
  bool newConnection = false;
  LastWifiTime = 0;
  const int WifiTimeoutMilliseconds = 30000; // 60 seconds

  Serial.println("Connecting Wifi");

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Reset Wifi");

#ifdef ARDUINO_ARCH_ESP8266
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA); // Ensure WiFi in Station/Client Mode
#endif
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    newConnection = true;

    if (millis() < LastWifiTime)
    {
      Serial.print(".");
      delay(500);
      continue;
    }

    if (WifiIndex >= wifiPairs)
    {
      WifiIndex = 0;
    }

    Serial.println("trying " + String(_ssid[WifiIndex]));

    WiFi.begin(_ssid[WifiIndex], _pwd[WifiIndex]);

    LastWifiTime = millis() + WifiTimeoutMilliseconds;

    WifiIndex++; //increment wifi indexready for the next ssid/pwd pair in case the current wifi pair dont connect
  }

  if (newConnection)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  return newConnection;
}

//void Device::initCloudConfig() {
//  EEPROM.begin(512);
//  char* eepromData;
//  int length;
//
//  const int BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(0);
//  StaticJsonBuffer<1000> jsonBuffer;
//  int address = 2;
//
//  length = word(EEPROM.read(0), EEPROM.read(1));
//  eepromData = new char[length];
//
//  for (address = 2; address < length + 2; address++) {
//    eepromData[address - 2] = EEPROM.read(address);
//  }
//  eepromData[address - 2] = '\0';
//
//
//  JsonObject& root = jsonBuffer.parseObject(eepromData);
//  if (!root.success())
//  {
//    Serial.println("parseObject() failed");
//    return;
//  }
//
//
////  sensor.geo = GetValue(root["geo"]);
//
//
//  wifiPairs = root["wifi"];
//  ssid = new const char*[wifiPairs];
//  pwd = new const char*[wifiPairs];
//
//  for (int i = 0; i < wifiPairs; i++)
//  {
//    ssid[i] = GetValue(root["ssid"][i]);
//    pwd[i] = GetValue(root["pwd"][i]);
//  }
//}

const char *Device::GetValue(const char *value)
{
  char *temp = new char[strlen(value) + 1];
  strcpy(temp, value);
  return temp;
}
