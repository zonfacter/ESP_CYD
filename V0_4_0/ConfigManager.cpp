/**
 * ConfigManager.cpp - Implementierung der Konfigurationsfunktionen
 */

#include "ConfigManager.h"

// Globale Instanz
ConfigManager configManager;

ConfigManager::ConfigManager() {
  // Konstruktor
}

bool ConfigManager::begin() {
  // Mit Format-Option (false), damit keine Formatierung beim Fehler erfolgt
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS konnte nicht initialisiert werden!");
    DEBUG_PRINTLN("Versuche Formatierung...");
    
    // Wenn das nicht funktioniert, versuche zu formatieren
    if (!SPIFFS.begin(true)) {
      DEBUG_PRINTLN("SPIFFS-Formatierung fehlgeschlagen!");
      return false;
    }
    DEBUG_PRINTLN("SPIFFS formatiert und initialisiert");
  } else {
    DEBUG_PRINTLN("SPIFFS initialisiert");
  }
  
  spiffsInitialized = true;
  
  // Liste der Dateien ausgeben
  DEBUG_PRINTLN("Dateien im SPIFFS:");
  listFiles();
  
  // Standard-Konfigurationen erstellen, falls nicht vorhanden
  createDefaultConfigs();
  
  // Nach dem Erstellen nochmals alle Dateien anzeigen
  DEBUG_PRINTLN("Dateien nach dem Erstellen der Standardkonfigurationen:");
  listFiles();
  
  return true;
}

bool ConfigManager::loadJsonConfig(const String &filename, JsonDocument &doc) {
  if (!spiffsInitialized) {
    DEBUG_PRINTLN("SPIFFS nicht initialisiert!");
    return false;
  }
  
  if (!SPIFFS.exists(filename)) {
    DEBUG_PRINT("Konfigurationsdatei nicht gefunden: ");
    DEBUG_PRINTLN(filename);
    return false;
  }
  
  DEBUG_PRINT("Öffne Datei: ");
  DEBUG_PRINTLN(filename);
  
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    DEBUG_PRINTLN("Fehler beim Öffnen der Datei");
    return false;
  }
  
  DEBUG_PRINT("Dateigröße: ");
  DEBUG_PRINTLN(file.size());
  
  // Falls die Datei leer ist, keine Verarbeitung durchführen
  if (file.size() == 0) {
    DEBUG_PRINTLN("Datei ist leer!");
    file.close();
    return false;
  }
  
  // JSON deserialisieren
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    DEBUG_PRINT("JSON Parsing Fehler: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  DEBUG_PRINT("Konfigurationsdatei geladen: ");
  DEBUG_PRINTLN(filename);
  return true;

    // Erweitere Validierung basierend auf dem Dateityp
  bool isValid = true;
  if (filename == "/menu.json") {
    isValid = isValidMenu(doc);
    if (!isValid) DEBUG_PRINTLN("menu.json hat keine gültige Struktur!");
  }
  else if (filename == "/mqtt_topics.json") {
    isValid = isValidTopics(doc);
    if (!isValid) DEBUG_PRINTLN("mqtt_topics.json hat keine gültige Struktur!");
  }
  else if (filename == "/config.json") {
    isValid = isValidConfig(doc);
    if (!isValid) DEBUG_PRINTLN("config.json hat keine gültige Struktur!");
  }
  
  return isValid;
}

bool ConfigManager::saveJsonConfig(const String &filename, const JsonDocument &doc) {
  if (!spiffsInitialized) {
    DEBUG_PRINTLN("SPIFFS nicht initialisiert!");
    return false;
  }
  
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    DEBUG_PRINTLN("Fehler beim Öffnen der Datei zum Schreiben");
    return false;
  }
  
  // JSON serialisieren
  if (serializeJson(doc, file) == 0) {
    DEBUG_PRINTLN("Fehler beim Schreiben der JSON-Daten");
    file.close();
    return false;
  }
  
  file.close();
  DEBUG_PRINT("Konfigurationsdatei gespeichert: ");
  DEBUG_PRINTLN(filename);
  return true;
}

void ConfigManager::createDefaultConfigs() {
  DEBUG_PRINTLN("Prüfe auf vorhandene Konfigurationsdateien...");
  
  // Prüfe/erstelle config.json
  if (!fileExists("/config.json")) {
    DEBUG_PRINTLN("config.json existiert nicht, erstelle Standardkonfiguration...");
    createDefaultConfigFile();
  } else {
    DEBUG_PRINTLN("config.json existiert bereits");
    
    // Lade die Datei und prüfe, ob sie gültig ist
    JsonDocument testDoc;
    if (loadJsonConfig("/config.json", testDoc) && isValidConfig(testDoc)) {
      DEBUG_PRINTLN("config.json ist gültig");
    } else {
      DEBUG_PRINTLN("config.json ist ungültig, erstelle neu...");
      SPIFFS.remove("/config.json");
      createDefaultConfigFile();
    }
  }
  
  // Prüfe/erstelle menu.json
  if (!fileExists("/menu.json")) {
    DEBUG_PRINTLN("menu.json existiert nicht, erstelle Standardkonfiguration...");
    createDefaultMenuFile();
  } else {
    DEBUG_PRINTLN("menu.json existiert bereits");
    
    // Lade die Datei und prüfe, ob sie gültig ist
    JsonDocument testDoc;
    if (loadJsonConfig("/menu.json", testDoc) && isValidMenu(testDoc)) {
      DEBUG_PRINTLN("menu.json ist gültig");
    } else {
      DEBUG_PRINTLN("menu.json ist ungültig, erstelle neu...");
      SPIFFS.remove("/menu.json");
      createDefaultMenuFile();
    }
  }
  
  // Prüfe/erstelle mqtt_topics.json
  if (!fileExists("/mqtt_topics.json")) {
    DEBUG_PRINTLN("mqtt_topics.json existiert nicht, erstelle Standardkonfiguration...");
    createDefaultMqttTopicsFile();
  } else {
    DEBUG_PRINTLN("mqtt_topics.json existiert bereits");
    
    // Lade die Datei und prüfe, ob sie gültig ist
    JsonDocument testDoc;
    if (loadJsonConfig("/mqtt_topics.json", testDoc) && isValidTopics(testDoc)) {
      DEBUG_PRINTLN("mqtt_topics.json ist gültig");
    } else {
      DEBUG_PRINTLN("mqtt_topics.json ist ungültig, erstelle neu...");
      SPIFFS.remove("/mqtt_topics.json");
      createDefaultMqttTopicsFile();
    }
  }
}

void ConfigManager::listFiles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  if (!file) {
    DEBUG_PRINTLN("  Keine Dateien gefunden!");
    return;
  }
  
  while (file) {
    DEBUG_PRINT("  - ");
    DEBUG_PRINT(file.name());
    DEBUG_PRINT(": ");
    DEBUG_PRINT(file.size());
    DEBUG_PRINTLN(" Bytes");
    
    file = root.openNextFile();
  }
}

bool ConfigManager::fileExists(const String &filename) {
  if (!spiffsInitialized) {
    DEBUG_PRINTLN("SPIFFS nicht initialisiert beim Prüfen von Datei!");
    return false;
  }
  
  bool exists = SPIFFS.exists(filename);
  DEBUG_PRINT("Prüfe Datei ");
  DEBUG_PRINT(filename);
  DEBUG_PRINT(": ");
  DEBUG_PRINTLN(exists ? "Existiert" : "Existiert nicht");
  
  if (exists) {
    // Prüfe auch, ob die Datei Inhalt hat
    File file = SPIFFS.open(filename, "r");
    if (!file) {
      DEBUG_PRINTLN("  Konnte Datei nicht öffnen trotz exists=true");
      return false;
    }
    
    size_t size = file.size();
    file.close();
    
    DEBUG_PRINT("  Dateigröße: ");
    DEBUG_PRINTLN(size);
    
    if (size == 0) {
      DEBUG_PRINTLN("  Datei ist leer!");
      return false;
    }
  }
  
  return exists;
}

bool ConfigManager::createDefaultConfigFile() {
  DEBUG_PRINTLN("Erstelle default config.json...");
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    DEBUG_PRINTLN("Konnte config.json nicht zum Schreiben öffnen");
    return false;
  }
  
  file.print(DEFAULT_CONFIG_JSON);
  file.close();
  DEBUG_PRINTLN("Default config.json erstellt");
  return true;
}

bool ConfigManager::createDefaultMenuFile() {
  DEBUG_PRINTLN("Erstelle default menu.json...");
  File file = SPIFFS.open("/menu.json", "w");
  if (!file) {
    DEBUG_PRINTLN("Konnte menu.json nicht zum Schreiben öffnen");
    return false;
  }
  
  file.print(DEFAULT_MENU_JSON);
  file.close();
  DEBUG_PRINTLN("Default menu.json erstellt");
  return true;
}

bool ConfigManager::createDefaultMqttTopicsFile() {
  DEBUG_PRINTLN("Erstelle default mqtt_topics.json...");
  File file = SPIFFS.open("/mqtt_topics.json", "w");
  if (!file) {
    DEBUG_PRINTLN("Konnte mqtt_topics.json nicht zum Schreiben öffnen");
    return false;
  }
  
  file.print(DEFAULT_MQTT_TOPICS_JSON);
  file.close();
  DEBUG_PRINTLN("Default mqtt_topics.json erstellt");
  return true;
}

bool ConfigManager::isValidConfig(JsonDocument &doc) {
  return !doc.isNull() && doc.size() > 0 && doc["wlan"].is<JsonObject>();
}

bool ConfigManager::isValidMenu(JsonDocument &doc) {
  return !doc.isNull() && doc["tabs"].is<JsonArray>() && doc["tabs"].size() > 0;
}

bool ConfigManager::isValidTopics(JsonDocument &doc) {
  return !doc.isNull() && doc["topics"].is<JsonArray>() && doc["topics"].size() > 0;
}