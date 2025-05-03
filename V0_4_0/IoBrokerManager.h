// IoBrokerManager.h
#ifndef IOBROKER_MANAGER_H
#define IOBROKER_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "MqttManager.h"

class IoBrokerManager {
private:
  WiFiClient ioWifiClient;
  PubSubClient ioMqttClient;
  
  String broker;
  int port;
  String username;
  String password;
  String clientId;
  String baseTopic;
  
  bool connected = false;
  unsigned long lastReconnectAttempt = 0;
  
  // Interne Methode zur MQTT-Verbindung
  bool connectMqtt();
  
  // Abonnieren der spezifischen ioBroker-Topics für Rolladen
  bool subscribeRolladenTopics();
  void updateHeartbeat(); // Neue Methode zur Aktualisierung des Heartbeats


  // Callback für MQTT-Nachrichten
  static void staticCallback(char* topic, byte* payload, unsigned int length);
  void handleCallback(char* topic, byte* payload, unsigned int length);
  
  void updateRolladenPositions();

  // Hilfsfunktion zur Interpretation des MQTT-Status
  String getMqttStateDescription(int state);
  
  // Topics für verschiedene Geräte
  String getLightTopic(const String &lightId);
  String getRolladenTopic(const String &rolladenId);
  String getHeatingTopic(const String &heatingId);
  String getPoolTopic(const String &poolId);

  // Heartbeat-Funktionalität
  unsigned long lastHeartbeatTime = 0;
  const unsigned long HEARTBEAT_INTERVAL = 60000; // 60 Sekunden Intervall
  int heartbeatCounter = 0;
  bool heartbeatReceived = false;
  unsigned long lastHeartbeatReceived = 0;

  // Status-Speicher
  struct DeviceStatus {
    String id;
    String status;
    unsigned long lastUpdate;
    
    DeviceStatus(const String& id) : 
      id(id), status("unknown"), lastUpdate(0) {}
  };
  
  // Erweiterte Struktur für Rolladenstatus
  struct RolladenStatus {
    String id;
    int currentPosition;    // Ist-Position: 0-100%, 0%=offen, 100%=geschlossen
    int targetPosition;     // Soll-Position: 0-100%
    String direction;       // "up", "down", "stop"
    bool moving;            // Bewegt sich gerade
    unsigned long lastUpdate;
    unsigned long moveStartTime;  // Zeitpunkt des Bewegungsbeginns
    int moveStartPosition;        // Position zu Beginn der Bewegung
    int rolladenSpeed;            // Geschwindigkeit in % pro Sekunde
    bool calibrated;              // Ist der Rolladen kalibriert?
    
    RolladenStatus(const String& id) : 
      id(id), currentPosition(50), targetPosition(50), direction("stop"), 
      moving(false), lastUpdate(0), moveStartTime(0), moveStartPosition(50),
      rolladenSpeed(3), calibrated(false) {}
  };
  
  struct HeatingStatus {
    String id;
    String status;
    float temperature;
    unsigned long lastUpdate;
    
    HeatingStatus(const String& id) : 
      id(id), status("unknown"), temperature(0.0), lastUpdate(0) {}
  };

  std::vector<DeviceStatus> lightStatus;
  std::vector<RolladenStatus> rolladenStatus;
  std::vector<HeatingStatus> heatingStatus;
  std::vector<DeviceStatus> poolStatus;
public:
  IoBrokerManager(const String &baseTopic = "esp32solar");
  
  bool begin(const String &broker = "192.168.2.137", 
             int port = 1884, 
             const String &username = "DVES_USER", 
             const String &password = "DVES_PASS");
  
  void update(); // Regelmäßig aufrufen, um die Verbindung zu überwachen
  
  bool isConnected() const { return connected; }
  
  // Allgemeine Methoden für Gerätestatus-Updates
  void broadcastStatus(); // Sendet alle aktuellen Gerätezustände
  
  // Lichtsteuerung
  bool setLight(const String &lightId, bool state);
  bool getLightStatus(const String &lightId, bool &status);
  
  // Erweiterte Rolladensteuerung
  bool moveRolladen(const String &rolladenId, const String &direction); // "up", "down", "stop"
  bool setRolladenTargetPosition(const String &rolladenId, int position); // Soll-Position setzen (0-100%)
  bool getRolladenStatus(const String &rolladenId, int &currentPosition, int &targetPosition, 
                         String &direction, bool &moving);
  bool calibrateRolladen(const String &rolladenId, int speed); // Kalibrierung mit Geschwindigkeit in % pro Sekunde
  
  // Zusätzliche Rolladenfunktionen
  bool resetRolladenState(const String &rolladenId);
  bool adjustRolladenPosition(const String &rolladenId, int adjustment);
  bool openRolladen(const String &rolladenId);
  bool closeRolladen(const String &rolladenId);
  bool setAllRolladenPositions(int position);
  bool openAllRolladen();
  bool closeAllRolladen();
  bool setSunProtection(const String &rolladenId, bool enable);
  String getRolladenStatusJson(const String &rolladenId);
  bool scheduleRolladenMove(const String &rolladenId, int targetPosition, unsigned long delayMs);
  
  bool getHeartbeatStatus() const { return heartbeatReceived; }

  // Heizungssteuerung
  bool setHeating(const String &heatingId, bool state);
  bool setHeatingTemperature(const String &heatingId, float temperature);
  bool getHeatingStatus(const String &heatingId, bool &state, float &temperature);
  
  // Poolpumpensteuerung
  bool setPoolPump(const String &poolId, bool state);
  bool getPoolPumpStatus(const String &poolId, bool &status);
  
  // ESP32 Konfiguration über MQTT
  bool updateConfig(JsonDocument &configUpdate);
  bool requestConfigSync();

  void publishDeviceInfo();
};

extern IoBrokerManager ioBrokerManager;

#endif // IOBROKER_MANAGER_H