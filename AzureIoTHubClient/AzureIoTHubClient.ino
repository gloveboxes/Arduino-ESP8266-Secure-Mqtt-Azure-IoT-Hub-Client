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
  https://krzychb.gitbooks.io/esp8266wifi-library/content/client-secure-examples.html
  https://krzychb.gitbooks.io/esp8266wifi-library/content/client-secure-class.html
  https://raw.githubusercontent.com/Azure/azure-iot-sdk-c/76906dc5fcb38eb3d2c67b670f638ed8965e502d/certs/certs.c
*/

#include <ESP8266WiFi.h>
#include "MqttClient.h"
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager
#include "globals.h"        // global structures and enums used by the applocation
#include "led.h"
#include "Bme280.h"
#include "bmp280.h"
#include "bmp180.h"

#define delay(s) client.mqttDelay(s)



const char* connectionString = "HostName=IoTCampAU.azure-devices.net;DeviceId=syd-master;SharedAccessKey=M3zMtx9teF9CtB/ngXfAsOQcTDpT61kOEN42OkMoFYw=";
const char* ssid = "NCW";
const char* pwd = "malolos5459";
const char* geo = "syd-master";
BoardType boardType = WeMos; // BoardType enumeration: NodeMCU, WeMos, SparkfunThing, Other (defaults to Other).

WiFiClientSecure tlsClient;
MqttClient client(tlsClient);


DeviceConfig device;

//Sensor sensor(&client);
//Bmp180 sensor(&client);
//Bmp280 sensor(&client);
Bme280 sensor(&client);

Led led(BUILTIN_LED);



IPAddress timeServer(62, 237, 86, 238); // Update these with values suitable for your network.

/* 
 http://hassansin.github.io/certificate-pinning-in-nodejs
 for information on generating the certificate fingerprint
 From Ubuntu subsystem on Windows 10
 echo -n | openssl s_client -connect IoTCampAU.azure-devices.net:8883 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' > cert.pem
 openssl x509 -noout -in cert.pem -fingerprint
*/
const char* certificateFingerprint = "38:5C:47:B1:97:DA:34:57:BB:DD:E7:7C:B9:11:8F:8D:1D:92:EB:F1";


void initDeviceConfig() { // Example device configuration
	device.boardType = boardType;            // BoardType enumeration: NodeMCU, WeMos, SparkfunThing, Other (defaults to Other). This determines pin number of the onboard LED for wifi and publish status. Other means no LED status
	device.deepSleepSeconds = 0;         // if greater than zero with call ESP8266 deep sleep (default is 0 disabled). GPIO16 needs to be tied to RST to wake from deepSleep. Causes a reset, execution restarts from beginning of sketch
	device.publishRateInSeconds = 1;     // limits publishing rate to specified seconds (default is 90 seconds).  Connectivity problems may result if number too small eg 2
  
  client.sasExpiryPeriodInSeconds = 15 * 60; // Renew Sas Token every 15 minutes
  client.certificateFingerprint = certificateFingerprint;
  client.setConnectionString(connectionString);
}

void setup() {
	Serial.begin(115200);
 
  initDeviceConfig();   
  initCloudConfig(ssid, pwd, geo);  
  
  wifiConnect();
  
  getCurrentTime();
  
	client.setServer(client.host, 8883);
	client.setCallback(callback); 
}

void wifiConnect() {

	delay(10);
  
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(device.ssid[0]);

  WiFi.begin(device.ssid[0], device.pwd[0]);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
 
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
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
  if (WiFi.status() != WL_CONNECTED){
    wifiConnect();
  }
  else {  
    client.mqttConnect(tlsClient);  
    sensor.measure();    
    client.send(sensor.toJSON()); 
    
    if (device.deepSleepSeconds > 0) {
      WiFi.mode(WIFI_OFF);
      ESP.deepSleep(1000000 * device.deepSleepSeconds, WAKE_RF_DEFAULT); // GPIO16 needs to be tied to RST to wake from deepSleep. Execute restarts from beginning of sketch
    }
    else {
      delay(device.publishRateInSeconds * 1000);  // limit publishing rate
    }
  }
}



