/**
 * DataManager.h - Verwaltet Solardaten und Aktualisierungen
 */

#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config.h"

// Vorwärtsdeklaration der MQTT-Manager-Klasse
class MqttManager;

// Struktur für Solardaten
struct SolarData {
  float batterySOC;        // Batterie State of Charge in Prozent
  float pvPower;           // PV Leistung in Watt
  float gridPower;         // Netzleistung in Watt (negativ = Einspeisung)
  float loadPower;         // Verbrauch in Watt
  float batteryPower;      // Batterieleistung in Watt
  float dailyYield;        // Tagesertrag in kWh
  float batteryVoltage;    // Batteriespannung in Volt
  float autarky;           // Autarkie in Prozent
  
  // Standardwerte setzen
  SolarData() : 
    batterySOC(0), 
    pvPower(0), 
    gridPower(0), 
    loadPower(0),
    batteryPower(0), 
    dailyYield(0), 
    batteryVoltage(0), 
    autarky(0) {}
};

// Struktur für historische Daten
struct HistoricalData {
  unsigned long timestamp;  // Zeitstempel des Datensatzes
  SolarData data;           // Die eigentlichen Solardaten
  
  HistoricalData() : timestamp(0) {}
  HistoricalData(unsigned long ts, const SolarData& sd) : timestamp(ts), data(sd) {}
};

class DataManager {
private:
  SolarData data;
  bool simulationMode = true;
  unsigned long lastUpdate = 0;
  unsigned long lastCacheUpdate = 0;
  
  // Historische Daten speichern
  static const int MAX_HISTORY_ENTRIES = 48; // 1 Eintrag pro Stunde für einen Tag
  HistoricalData history[MAX_HISTORY_ENTRIES];
  int historyIndex = 0;
  
  // Cache-Funktionen
  bool saveDataToCache();
  bool loadDataFromCache();

  bool checkStorageSpace();  
public:
  DataManager();
  
  // Daten aktualisieren
  void updateFromMqtt(MqttManager& mqttManager);
  void simulateData();  // Für Testzwecke
  
  // Daten aus dem Cache laden, falls MQTT nicht verfügbar
  bool checkAndLoadCachedData();
  
  // Getter
  SolarData& getData() { return data; }
  
  // Historische Daten
  bool addHistoricalDataPoint();
  HistoricalData* getHistoricalData() { return history; }
  int getHistoryCount() const;
  // Neue Funktionen für erweiterte Historienverwaltung
  bool loadHistoricalDataFromStorage();
  String getDayString();
  float getMaxValueForToday(const String& field);
  float getAvgValueForToday(const String& field);

  // Simulationsmodus ein/ausschalten
  void setSimulationMode(bool mode) { simulationMode = mode; }
  bool isSimulationMode() { return simulationMode; }
  
  // Periodische Aktualisierung
  void update();
};

extern DataManager dataManager;

#endif // DATA_MANAGER_H