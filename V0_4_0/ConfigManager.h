/**
 * ConfigManager.h - Verwaltet Konfigurationen im SPIFFS
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "default_data.h"

class ConfigManager {
private:
  bool spiffsInitialized = false;
  
public:
  ConfigManager();
  
  // SPIFFS initialisieren
  bool begin();
  
  // JSON-Datei laden
  bool loadJsonConfig(const String &filename, JsonDocument &doc);
  
  // JSON-Datei speichern
  bool saveJsonConfig(const String &filename, const JsonDocument &doc);
  
  // Standard-Konfigurationen erstellen, falls nicht vorhanden
  void createDefaultConfigs();
  
  // Standard-Konfigurationsdateien erstellen
  bool createDefaultConfigFile();
  bool createDefaultMenuFile();
  bool createDefaultMqttTopicsFile();
  
  // Validierungsfunktionen
  bool isValidConfig(JsonDocument &doc);
  bool isValidMenu(JsonDocument &doc);
  bool isValidTopics(JsonDocument &doc);
  
  // Hilfsfunktionen
  void listFiles(); // Listet alle Dateien im SPIFFS
  bool fileExists(const String &filename);
};

extern ConfigManager configManager;

#endif // CONFIG_MANAGER_H