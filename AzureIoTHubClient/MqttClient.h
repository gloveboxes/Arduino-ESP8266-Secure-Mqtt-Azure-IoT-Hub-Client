#ifndef MqttClient_h
#define MqttClient_h

#include <PubSubClient.h>

#ifdef ARDUINO_ARCH_SAMD
#include <WiFi101.h>
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif

class MqttClient : public PubSubClient
{
public:
#ifdef ARDUINO_ARCH_ESP8266
  MqttClient(WiFiClientSecure &tlsClient) : PubSubClient(tlsClient)
  {
    _tlsClient = &tlsClient;
  }
#endif

#ifdef ARDUINO_ARCH_SAMD
  MqttClient(WiFiSSLClient &tlsClient) : PubSubClient(tlsClient)
  {
    _tlsClient = &tlsClient;
  }
#endif

  void close();
  void setConnectionString(String cs);

  int send(char *json)
  {
    if (mqttConnect())
    {
      return publish(mqttTopicPublish, json);
    }
    else
    {
      return 0;
    }
  }

  void mqttDelay(int milliseconds)
  {
    long startTime = millis();
    while (millis() - startTime < milliseconds)
    {
      delay(1);
      this->loop();
    }
  }

  const char *host;
  const char *certificateFingerprint;
  char *key;
  const char *deviceId;
  char *hubUser;
  char *hubPass = new char[0];
  char *mqttTopicPublish;
  char *mqttTopicSubscribe;
  time_t sasExpiryTime = 0;
  int sasExpiryPeriodInSeconds = 60 * 15; // Default to 15 minutes
  String sasUrl;

private:
  bool mqttConnect();
  bool generateSas();
  String createIotHubSas(char *key, String url);
  bool verifyServerFingerprint();
  String splitStringByIndex(String data, char separator, int index);
  char *format(const char *input, const char *value);
  char *format(const char *input, const char *value1, const char *value2);
  const char *GetStringValue(String value);
  String urlEncode(const char *msg);

#ifdef ARDUINO_ARCH_ESP8266
  WiFiClientSecure *_tlsClient;
#endif

#ifdef ARDUINO_ARCH_SAMD
  WiFiSSLClient *_tlsClient;
#endif
};

#endif
