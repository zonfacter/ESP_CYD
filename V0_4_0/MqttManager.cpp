/**
 * MqttManager.cpp - Implementierung der MQTT-Funktionen
 */

#include "MqttManager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Globale Instanz
MqttManager mqttManager;

// Statische Wrapper-Funktion für MQTT-Callback
void MqttManager::staticCallback(char* topic, byte* payload, unsigned int length) {
  // Weiterleitung an die Instanzmethode
  mqttManager.handleCallback(topic, payload, length);
}

// Instanzmethode für die Callback-Verarbeitung
void MqttManager::handleCallback(char* topic, byte* payload, unsigned int length) {
  // Konvertiere Payload zu String
  String message;
  message.reserve(length);
  
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  DEBUG_PRINT("MQTT Nachricht [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("]: ");
  DEBUG_PRINTLN(message);
  
  // Aktualisiere das entsprechende Topic
  String topicStr = String(topic);
  bool topicFound = false;
  
  for (auto& mqttTopic : topics) {
    if (mqttTopic.topic == topicStr) {
      mqttTopic.value = message;
      mqttTopic.lastUpdate = millis();
      topicFound = true;
      
      // Benachrichtigung über Datenänderung
      if (onDataUpdate) {
        onDataUpdate();
      }
      break;
    }
  }
  
  if (!topicFound) {
    DEBUG_PRINT("Warnung: Unbekanntes Topic empfangen: ");
    DEBUG_PRINTLN(topicStr);
  }
}

MqttManager::MqttManager() : mqttClient(wifiClient) {
  // Verwende die letzten 4 Zeichen der MAC-Adresse für die Client-ID
  String mac = WiFi.macAddress();
  mac.replace(":", ""); // Entferne die Doppelpunkte
  clientId = MQTT_CLIENT_ID + mac.substring(mac.length() - 4);
  
  DEBUG_PRINT("MQTT Client-ID: ");
  DEBUG_PRINTLN(clientId);
}

bool MqttManager::begin(const String &broker, int port) {
  this->broker = broker;
  this->port = port;
  
  String mac = WiFi.macAddress();
  mac.replace(":", ""); // Entferne die Doppelpunkte
  clientId = MQTT_CLIENT_ID + mac.substring(mac.length() - 4); // Letzte 4 Zeichen
  
  DEBUG_PRINT("MQTT Client-ID: ");
  DEBUG_PRINTLN(clientId);
  
  // Server und Callback einrichten
  mqttClient.setServer(broker.c_str(), port);
  mqttClient.setCallback(MqttManager::staticCallback);
  
  // Default Topics laden
  loadDefaultTopics();
  
  // Verbindung versuchen
  if (!mqttClient.connected()) {
    DEBUG_PRINT("Verbinde mit Standard MQTT-Manager-Broker ");
    DEBUG_PRINT(broker);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(port);
    
    if (mqttClient.connect(clientId.c_str())) {
      connected = true;
      DEBUG_PRINTLN("MQTT verbunden!");
      
      // Abonniere alle konfigurierten Topics
      for (const auto& topic : topics) {
        mqttClient.subscribe(topic.topic.c_str());
        DEBUG_PRINT("Abonniert: ");
        DEBUG_PRINTLN(topic.topic);
      }
    } else {
      DEBUG_PRINT("Standard MQTT-Manager-Verbindung fehlgeschlagen, rc=");
      DEBUG_PRINTLN(mqttClient.state());
      connected = false;
    }
  }
  
  return connected;
}

void MqttManager::update() {
  // MQTT-Verbindung prüfen und wiederherstellen
  if (!mqttClient.connected()) {
    connected = false;
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt > 5000) {  // Alle 5 Sekunden versuchen
      lastReconnectAttempt = now;
      
      DEBUG_PRINTLN("Standard MQTT-Manager: Verbindung verloren, versuche erneut...");
      
      if (mqttClient.connect(clientId.c_str())) {
        DEBUG_PRINTLN("Standard MQTT-Manager: Verbindung wiederhergestellt");
        connected = true;
        
        // Abonniere alle konfigurierten Topics
        for (const auto& topic : topics) {
          mqttClient.subscribe(topic.topic.c_str());
          DEBUG_PRINT("Erneut abonniert: ");
          DEBUG_PRINTLN(topic.topic);
        }
      } else {
        DEBUG_PRINT("Standard MQTT-Manager-Reconnect fehlgeschlagen, rc=");
        DEBUG_PRINTLN(mqttClient.state());
      }
    }
  } else {
    // MQTT Client Loop
    mqttClient.loop();
  }
}

bool MqttManager::subscribe(const String &name, const String &topic) {
  // Prüfe, ob Topic bereits existiert
  for (const auto& t : topics) {
    if (t.name == name) {
      return true;  // Bereits abonniert
    }
  }
  
  // Füge neues Topic hinzu
  topics.push_back(MqttTopic(name, topic));
  
  // Abonniere, falls verbunden
  if (mqttClient.connected()) {
    bool result = mqttClient.subscribe(topic.c_str());
    DEBUG_PRINT("Topic abonniert: ");
    DEBUG_PRINTLN(topic);
    return result;
  }
  
  return true;  // Wird abonniert, sobald verbunden
}

String MqttManager::getValue(const String &name) {
  for (const auto& t : topics) {
    if (t.name == name) {
      return t.value;
    }
  }
  
  return "N/A";  // Topic nicht gefunden
}


bool MqttManager::loadDefaultTopics() {
  // Standard-Topics für Solar Monitoring
  subscribe("battery_soc", "solar_assistant/total/battery_state_of_charge/state");
  subscribe("load_power", "solar_assistant/inverter_1/load_power_essential/state");
  subscribe("grid_power", "solar_assistant/inverter_1/grid_power/state");
  subscribe("pv_power", "solar_assistant/inverter_1/pv_power/state");
  subscribe("battery_power", "solar_assistant/total/battery_power/state");
  subscribe("battery_voltage", "solar_assistant/inverter_1/battery_voltage/state");
  subscribe("daily_yield", "solar_assistant/inverter_1/energy_day/state");
  
  DEBUG_PRINTLN("Default SolarAssistant-MQTT-Topics geladen");
  return true;
}

bool MqttManager::loadTopicsFromConfig(const String &filename) {
  // Laden der MQTT-Topics aus einer JSON-Konfigurationsdatei
  if (!SPIFFS.exists(filename)) {
    DEBUG_PRINT("MQTT-Konfigurationsdatei nicht gefunden: ");
    DEBUG_PRINTLN(filename);
    return false;
  }
  
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    DEBUG_PRINTLN("Fehler beim Öffnen der MQTT-Konfigurationsdatei");
    return false;
  }
  
  // JSON deserialisieren
  JsonDocument doc; // Die Größenbeschränkung ist nicht mehr erforderlich
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    DEBUG_PRINT("JSON Parsing Fehler: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // Topics aus JSON laden
  JsonArray topicList = doc["topics"];
  if (topicList.isNull()) {
    DEBUG_PRINTLN("Keine Topics in der Konfigurationsdatei gefunden");
    return false;
  }
  
  // Bestehende Topics löschen
  topics.clear();
  
  // Neue Topics aus JSON hinzufügen
  for (JsonObject topicObj : topicList) {
    String name = topicObj["name"].as<String>();
    String topic = topicObj["topic"].as<String>();
    
    if (name.length() > 0 && topic.length() > 0) {
      DEBUG_PRINT("MQTT Topic geladen: ");
      DEBUG_PRINT(name);
      DEBUG_PRINT(" -> ");
      DEBUG_PRINTLN(topic);
      
      subscribe(name, topic);
    }
  }
  
  DEBUG_PRINTLN("MQTT-Topics aus Konfiguration geladen");
  return true;
}



// In MqttManager.cpp
bool MqttManager::publish(const String &topic, const String &payload, bool retained) {
  if (!mqttClient.connected()) {
    DEBUG_PRINTLN("MQTT nicht verbunden, konnte nicht veröffentlichen");
    return false;
  }
  
  DEBUG_PRINT("Veröffentliche auf Topic: ");
  DEBUG_PRINT(topic);
  DEBUG_PRINT(" Payload: ");
  DEBUG_PRINTLN(payload);
  
  return mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}

// Diese Methode wartet auf eine Bestätigung auf einem bestimmten Topic
bool MqttManager::publishWithConfirmation(const String &topic, const String &payload, const String &confirmationTopic, unsigned long timeout) {
  // Statische Variablen für den Lambda-Callback
  static String expectedTopic = "";
  static bool confirmed = false;
  
  // Variablen zurücksetzen
  expectedTopic = confirmationTopic;
  confirmed = false;
  
  // Eigenständiger Lambda-Callback
  auto confirmationCallback = [](char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    
    if (topicStr == expectedTopic) {
      confirmed = true;
      DEBUG_PRINTLN("Bestätigung erhalten!");
    } else {
      // Normale Verarbeitung für andere Topics
      mqttManager.handleCallback(topic, payload, length);
    }
  };
  
  // Temporär den Callback setzen
  mqttClient.setCallback(confirmationCallback);
  
  // Bestätigungstopic abonnieren
  mqttClient.subscribe(confirmationTopic.c_str());
  
  // Nachricht senden
  bool published = publish(topic, payload);
  if (!published) {
    // Fehlgeschlagen, Standard-Callback wiederherstellen
    mqttClient.setCallback(staticCallback);
    return false;
  }
  
  // Auf Bestätigung warten
  unsigned long startTime = millis();
  while (!confirmed && (millis() - startTime < timeout)) {
    mqttClient.loop();
    delay(10);
  }
  
  // Standard-Callback wiederherstellen
  mqttClient.setCallback(staticCallback);
  
  return confirmed;
}