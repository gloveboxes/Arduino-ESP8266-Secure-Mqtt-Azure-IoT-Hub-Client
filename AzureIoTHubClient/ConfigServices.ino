#include <EEPROM.h>

void initCloudConfig() {
	EEPROM.begin(512);
	char* eepromData;
	int length;

	const int BUFFER_SIZE = JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(0);
	StaticJsonBuffer<1000> jsonBuffer;
	int address = 2;

	length = word(EEPROM.read(0), EEPROM.read(1));
	eepromData = new char[length];

	for (address = 2; address < length + 2; address++) {
		eepromData[address - 2] = EEPROM.read(address);
	}
	eepromData[address - 2] = '\0';


	JsonObject& root = jsonBuffer.parseObject(eepromData);
	if (!root.success())
	{
		Serial.println("parseObject() failed");
		return;
	}

	client.host = GetValue(root["host"]);
	client.key = (char*)GetValue(root["key"]);
	client.deviceId = GetValue(root["deviceId"]);
	sensor.geo = GetValue(root["geo"]);


	device.wifiPairs = root["wifi"];
	device.ssid = new const char*[device.wifiPairs];
	device.pwd = new const char*[device.wifiPairs];

	for (int i = 0; i < device.wifiPairs; i++)
	{
		device.ssid[i] = GetValue(root["ssid"][i]);
		device.pwd[i] = GetValue(root["pwd"][i]);
	}
}

void initCloudConfig(const char *ssid, const char *pwd, const char *geo) {
	device.wifiPairs = 1;
	device.ssid = new const char*[device.wifiPairs];
	device.pwd = new const char*[device.wifiPairs];
	device.ssid[0] = ssid;
	device.pwd[0] = pwd;
	sensor.geo = geo;
}

const char *GetValue(const char* value) {
  char *temp = new char[strlen(value) + 1];
  strcpy(temp, value);
  return temp;
}





