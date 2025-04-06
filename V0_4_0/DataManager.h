/**
 * DataManager.h - Verwaltet Solardaten und Aktualisierungen
 */

#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>
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

class DataManager {
private:
  SolarData data;
  bool simulationMode = true;
  unsigned long lastUpdate = 0;
  
public:
  DataManager();
  
  // Daten aktualisieren
  void updateFromMqtt(MqttManager& mqttManager);
  void simulateData();  // Für Testzwecke
  
  // Getter
  SolarData& getData() { return data; }
  
  // Simulationsmodus ein/ausschalten
  void setSimulationMode(bool mode) { simulationMode = mode; }
  bool isSimulationMode() { return simulationMode; }
  
  // Periodische Aktualisierung
  void update();
};

extern DataManager dataManager;

#endif // DATA_MANAGER_H