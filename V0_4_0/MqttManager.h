/**
 * MqttManager.h - Verwaltet MQTT-Verbindung und Topics
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include <functional>
#include "config.h"

// MQTT Topic Struktur
struct MqttTopic {
  String name;             // Interner Name (z.B. "battery_soc")
  String topic;            // MQTT Topic (z.B. "solar/battery/soc")
  String value;            // Aktueller Wert
  unsigned long lastUpdate; // Zeitstempel der letzten Aktualisierung

  MqttTopic(const String& n, const String& t) : 
    name(n), topic(t), value("N/A"), lastUpdate(0) {}
};

class MqttManager {
private:
  WiFiClient wifiClient;
  PubSubClient mqttClient;
  
  String broker;
  int port;
  String clientId;
  bool connected = false;
  unsigned long lastReconnectAttempt = 0;
  
  // Als Friend deklarieren, damit die Funktion auf private-Elemente zugreifen kann
  friend void mqttCallback(char* topic, byte* payload, unsigned int length);
  
  std::vector<MqttTopic> topics;
  
public:
  MqttManager();
  
  bool begin(const String &broker = MQTT_BROKER, int port = MQTT_PORT);
  void update();
  static void staticCallback(char* topic, byte* payload, unsigned int length);
  void handleCallback(char* topic, byte* payload, unsigned int length);
  bool loadDefaultTopics();
  bool loadTopicsFromConfig(const String &filename);
  
  bool subscribe(const String &name, const String &topic);
  String getValue(const String &name);

  
  bool isConnected() { return connected; }
  
  // Getter für topics hinzufügen (optional, für Debugging)
  const std::vector<MqttTopic>& getTopics() const { return topics; }
  
  typedef std::function<void()> DataCallback;
  DataCallback onDataUpdate = nullptr;
};


extern MqttManager mqttManager;

#endif // MQTT_MANAGER_H