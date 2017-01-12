#include "MqttClient.h"
#include "sha256.h"
#include "Base64.h"
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager


String MqttClient::createIotHubSas(char *key, String url){  
  sasExpiryTime = now() + sasExpiryPeriodInSeconds;
  
  String stringToSign = url + "\n" + sasExpiryTime;

  // START: Create signature
  // https://raw.githubusercontent.com/adamvr/arduino-base64/master/examples/base64/base64.ino
  
  int keyLength = strlen(key);
  
  int decodedKeyLength = base64_dec_len(key, keyLength);
  char decodedKey[decodedKeyLength];  //allocate char array big enough for the base64 decoded key
  
  base64_decode(decodedKey, key, keyLength);  //decode key
  
  Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
  Sha256.print(stringToSign);  
  char* sign = (char*) Sha256.resultHmac();
  // END: Create signature
  
  // START: Get base64 of signature
  int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, HASH_LENGTH); 
  
  // SharedAccessSignature
  return "SharedAccessSignature sr=" + url + "&sig="+ urlEncode(encodedSign) + "&se=" + sasExpiryTime;
  // END: create SAS  
}

//http://hardwarefun.com/tutorials/url-encoding-in-arduino
String MqttClient::urlEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if (('a' <= *msg && *msg <= 'z')
      || ('A' <= *msg && *msg <= 'Z')
      || ('0' <= *msg && *msg <= '9')) {
      encodedMsg += *msg;
    }
    else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}

void MqttClient::generateSas(){
  if (timeStatus() == timeNotSet) { return; }

  if (now() > sasExpiryTime){
    delete[] hubPass;
    hubPass = (char*)GetStringValue(createIotHubSas(key, sasUrl));
  }
}

const char* MqttClient::GetStringValue(String value) {
  int len = value.length() + 1;
  char *temp = new char[len];
  value.toCharArray(temp, len);
  return temp;
}

void MqttClient::setConnectionString(String cs){
  host = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 0), '=', 1));
  deviceId = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 1), '=', 1));
  key = (char*)GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 2), '=', 1)); 
  Serial.print("Host ");
  Serial.println(host);

  hubUser = format("%s/%s", host, deviceId);
  mqttTopicPublish = format("devices/%s/messages/events/", deviceId);
  mqttTopicSubscribe = format("devices/%s/messages/devicebound/#", deviceId);

  const char* TARGET_URL = "/devices/";
  sasUrl = urlEncode(host) + urlEncode(TARGET_URL) + (String)deviceId;
  
}

char* MqttClient::format(const char *input, const char *value){
  int len = strlen(input) + strlen(value);
  char *temp = new char[len];
  sprintf(temp, input, value); 
  return temp;
}

char* MqttClient::format(const char *input, const char *value1, const char *value2){
  int len = strlen(input) + strlen(value1) + strlen(value2);
  char *temp = new char[len];
  sprintf(temp, input, value1, value2); 
  return temp;
}

// http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
String MqttClient::splitStringByIndex(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

bool MqttClient::verifyServerFingerprint(WiFiClientSecure& tlsClient){
  /* 
   http://hassansin.github.io/certificate-pinning-in-nodejs
   for information on generating fingerprint
   From Ubuntu subsystem on Windows 10
   echo -n | openssl s_client -connect IoTCampAU.azure-devices.net:443 | sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p' > cert.pem
   openssl x509 -noout -in cert.pem -fingerprint
  */
  

  if (tlsClient.verify(certificateFingerprint, host)) {
    Serial.print("Certificate fingerprint verified against ");
    Serial.print(host);
    Serial.println(" sucessfully");
  } else {
    Serial.println("Certificate fingerprint verification failed");
    ESP.restart();
  }
}

void MqttClient::mqttConnect(WiFiClientSecure& tlsClient) {
  generateSas();
  if (connected()){ return; }
  
  // Loop until we're reconnected
  while (!connected()) {
    Serial.println("Attempting MQTT Connection");
    // Attempt to connect

    if (connect(deviceId, hubUser, hubPass)) {
      
      Serial.println("MQTT Connected");
     
      verifyServerFingerprint(tlsClient);
     
      subscribe(mqttTopicSubscribe);
      loop();  //reads command...
    }
    else {
      Serial.print("MQTT Connection Failed, rc=");
      Serial.print(state());
      Serial.println(", trying again in 5 seconds");
      // Wait 5 seconds before retrying
      mqttDelay(5000);
    }
  }
}


