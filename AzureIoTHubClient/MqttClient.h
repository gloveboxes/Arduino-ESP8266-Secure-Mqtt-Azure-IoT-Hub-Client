#ifndef MqttClient_h
#define MqttClient_h

#include <PubSubClient.h>
#include <ESP8266WiFi.h>


class MqttClient : public PubSubClient
{
  public:
    MqttClient(Client& client) : PubSubClient(client) {
      int x = 0;  
    }
    void mqttConnect(WiFiClientSecure& tlsClient);
    String createIotHubSas(char *key, String url);

    void setConnectionString(String cs);
    bool verifyServerFingerprint(WiFiClientSecure& tlsClient);

    void send(char* json){
      publish(mqttTopicPublish, json);
    }
    
    
    void mqttDelay(int milliseconds){
      if (this->connected()){
        long startTime = millis();    
        while (millis() - startTime < milliseconds){
          delay(1);
          this->loop();
        }  
      }
      else
      {
        delay(milliseconds);
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
    void generateSas();
    String splitStringByIndex(String data, char separator, int index);
    char* format(const char *input, const char *value);
    char* format(const char *input, const char *value1, const char *value2);
    const char* GetStringValue(String value);
    String urlEncode(const char* msg);
    
    
};

#endif
