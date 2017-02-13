#include <Adafruit_BMP085_U.h>

/*
  README

  PROJECT OWNER: Dave Glover | dglover@microsoft.com | @dglover

  COPYRIGHT: Free to use, reuse and modify, no libability accepted

  CREATED: Jan 2017

  This solution streams telemetry and supports command and control over MQTT directly to Azure IoT Hub.

  SUMMARY:
  This project was implemented and tested on the NodeMCU V2 (also known as V1.0), 
  WeMos D1 Mini and Sparkfun ESP8266 Dev development boards on the ESP8266 platform
  flashed with Arduino core for ESP8266 WiFi chip V2.3 firmware.

  Important: 
  
  - If you need to delay your script be sure to use the mqttSafeDelay rather than the standard Delay command. 
    This ensures mqtt commands are processed correctly.
  - You must change the MQTT_MAX_PACKET_SIZE to 256 in the PubSubClient.h file. 
    See http://www.radupascal.com/2016/04/03/esp8266-arduino-iot-hub for more detail.
 

  PLATFORM: ESP8266

  DEVELOPMENT BOARDS:
  NodeMCU: http://nodemcu.com/index_en.html#fr_54747661d775ef1a3600009e (This project will reflash the NodeMCU in to the Arduino platform. IMHO LUA not stable)
  WeMod D1 Mini: http://www.wemos.cc/wiki/doku.php?id=en:d1_mini
  Sparkfun 8266 Thing Dev Board: https://learn.sparkfun.com/tutorials/esp8266-thing-development-board-hookup-guide/introduction
  Other ESP8266 Development Boards - Adafruit, and others

  Sparkfun ESP8266 Dev Board Notes:

  With my limited experience with the Sparkfun ESP8266 Dev board I needed to use the Generic ESP8266 Arduino IDE Board Manager.
  See notes at https://learn.sparkfun.com/tutorials/esp8266-thing-development-board-hookup-guide/setting-up-arduino
  Change to Generic ESP8266 Module, and in the sub menus, make sure the following sub-menu's are also set:
  - Flash Mode: DIO
  - Flash Frequency: 80MHz
  - Upload Using: Serial
  - CPU Frequency: 80MHz
  - Flash Size: 512K (no SPIFFS)
  - Reset Method: nodemcu


  FIRMWARE: Arduino core for ESP8266 WiFi chip V2.3 | https://github.com/esp8266/Arduino

  ARDUINO IDE: 1.8.0 (as at Jan 2017)  


  REQUIRED LIBRARIES: Install Arduino Libraries from Sketch -> Include Library -> Manage Libraries
  - ArduinoJson
  - Time
  - DHT (DON’T install the unified version)
  - Adafruit Sensor
  - Adafruit BME280
  - PubSubClient by Nick O'Leary
 

  DRIVERS:
  - NodeMCU - On Windows, Mac and Linux you will need to install the latest CP210x USB to UART Bridge VCP Drivers. (https://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx)
  - WeMos - On Windows and Mac install the latest Ch340G drivers. No drivers required for Linux. (http://www.wemos.cc/wiki/doku.php?id=en:ch340g)
  - Sparkfun ESP8266 Thing Dev - https://learn.sparkfun.com/tutorials/esp8266-thing-development-board-hookup-guide/hardware-setup


  ESP8266 ARDUINO IDE SUPPORT:

  1. Install Arduino 1.8.0 from the Arduino website.
  2. Start Arduino and open Preferences window.
  3. Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
  4. RESTART Arduino IDE
  5. Open Boards Manager from Tools > Board -> Boards Manager. Search for ESP8266 and install.
  6. Select NodeMUC or WeMos D1 Mini Board: Tools -> Board  -> NodeMCU 1.0 (ESP-12E module) or WeMos D1 Mini
  7. Set Port and Upload Speed: Tools. Note, you may need to try different port speeds to successfully flash the device.
   - Faster is better as each time you upload the code to your device you are re-flashing the complete ROM not just your code, but faster can be less reliable.

*/

/*
  Information on TLS verify certicates
  https://nofurtherquestions.wordpress.com/2016/03/14/making-an-esp8266-web-accessible/
  https://krzychb.gitbooks.io/esp8266wifi-library/content/mqttClient-secure-examples.html
  https://krzychb.gitbooks.io/esp8266wifi-library/content/mqttClient-secure-class.html
  https://raw.githubusercontent.com/Azure/azure-iot-sdk-c/76906dc5fcb38eb3d2c67b670f638ed8965e502d/certs/certs.c
*/

#include <ESP8266WiFi.h>
#include "MqttClient.h"
#include "Device.h"
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager
#include "Bme280.h"
#include "Bmp280.h"
#include "Bmp180.h"
#include "DhtSensor.h"
#include "DigitalPin.h"

#define delay(s) mqttClient.mqttDelay(s)  // this overrides the standard delay with a safe mqtt delay which calls mqtt.loop()



const char* connectionString = "HostName=IoTCampAU.azure-devices.net;DeviceId=syd-solar;SharedAccessKey=vbCneLEizMZ2x4PBDehd8BsvhHoUEqLU2mcZ2oQxXr8=";
const char* ssid = "NCW";
const char* pwd = "malolos5459";
const char* geo = "syd-solar";

WiFiClientSecure tlsClient;
MqttClient mqttClient(tlsClient);
Device device(ssid, pwd);

//Sensor sensor(&mqttClient);
Bmp180 sensor(&mqttClient);
//Bmp280 sensor(&mqttClient);
//Bme280 sensor(&mqttClient);
//DhtSensor sensor(&mqttClient, device, dht11);
//DhtSensor sensor(&mqttClient, device, dht22);

DigitalPin led(BUILTIN_LED, false, true); // initial state is off (false), invert true = high turns led off

IPAddress timeServer(62, 237, 86, 238); // Update these with values suitable for your network.

/* 
 http://hassansin.github.io/certificate-pinning-in-nodejs
 for information on generating the certificate fingerprint
 From Ubuntu subsystem on Windows 10
 echo -n | openssl s_mqttClient -connect IoTCampAU.azure-devices.net:8883 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' > cert.pem
 openssl x509 -noout -in cert.pem -fingerprint
*/
const char* certificateFingerprint = "38:5C:47:B1:97:DA:34:57:BB:DD:E7:7C:B9:11:8F:8D:1D:92:EB:F1";


void initDeviceConfig() { // Example device configuration
	device.deepSleepSeconds = 0;         // if greater than zero with call ESP8266 deep sleep (default is 0 disabled). GPIO16 needs to be tied to RST to wake from deepSleep. Causes a reset, execution restarts from beginning of sketch
	device.publishRateInSeconds = 4;     // limits publishing rate to specified seconds (default is 90 seconds).  Connectivity problems may result if number too small eg 2
  
  mqttClient.sasExpiryPeriodInSeconds = 15 * 60; // Renew Sas Token every 15 minutes
  mqttClient.certificateFingerprint = certificateFingerprint;
  mqttClient.setConnectionString(connectionString);
  sensor.geo = geo;
}

void setup() {
	Serial.begin(115200);
 
  initDeviceConfig();   
  device.connectWifi();  
  getCurrentTime();
  
	mqttClient.setServer(mqttClient.host, 8883);
	mqttClient.setCallback(callback); 
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (length > 0 ) {
    char command = (char)payload[0];
    // some command processing here based on message received
  }  
  led.toggle();  // for this sample, just toggle the led
}

void getCurrentTime() {
	int ntpRetryCount = 0;
	while (timeStatus() == timeNotSet && ++ntpRetryCount < 10) { // get NTP time
		setSyncProvider(getNtpTime);
		setSyncInterval(60 * 60);
	}
}

void loop() {  
  if (device.connectWifi()) { Serial.println("mqtt close"); mqttClient.close(); }

  sensor.measure();    
  led.on();
  mqttClient.send(sensor.toJSON());
  led.off();

  delay(device.publishRateInSeconds * 1000);  // limit publishing rate
}



