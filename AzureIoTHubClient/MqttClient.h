#ifndef MqttClient_h
#define MqttClient_h

#include <PubSubClient.h>
#include <ESP8266WiFi.h>


class MqttClient : public PubSubClient
{
  public:
    MqttClient(WiFiClientSecure& tlsClient) : PubSubClient(tlsClient) {
      _tlsClient = &tlsClient;
    }

    void close();
    void setConnectionString(String cs);
    

    int send(char* json){
      if (mqttConnect()) { return publish(mqttTopicPublish, json); } else {return 0;}
    }
    
    
    void mqttDelay(int milliseconds){
      long startTime = millis();    
      while (millis() - startTime < milliseconds){
        delay(1);
        this->loop();
      }  
    }


    const char* host;
    const char* certificateFingerprint;
    char *key;
    const char *deviceId;
    char *hubUser;
    char *hubPass = new char[0];
    char *mqttTopicPublish;
    char *mqttTopicSubscribe;
    time_t  sasExpiryTime = 0;
    int sasExpiryPeriodInSeconds = 60 * 15; // Default to 15 minutes
    String sasUrl;

  private:
    WiFiClientSecure* _tlsClient;
    bool mqttConnect();
    bool generateSas();
    String createIotHubSas(char *key, String url);
    bool verifyServerFingerprint();
    String splitStringByIndex(String data, char separator, int index);
    char* format(const char *input, const char *value);
    char* format(const char *input, const char *value1, const char *value2);
    const char* GetStringValue(String value);
    String urlEncode(const char* msg);
    
    
};

#endif
