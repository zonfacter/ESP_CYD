/**
 * DataManager.cpp - Implementierung der Datenmanagement-Funktionen
 */

#include "DataManager.h"
#include "MqttManager.h"

// Globale Instanz
DataManager dataManager;

DataManager::DataManager() {
  // Konstruktor
}

void DataManager::updateFromMqtt(MqttManager& mqttManager) {
  // Daten aus MQTT Topics extrahieren
  // Zunächst als String holen und dann parsen
  
  String batterySOCStr = mqttManager.getValue("battery_soc");
  if (batterySOCStr != "N/A") {
    data.batterySOC = batterySOCStr.toFloat();
  }
  
  String pvPowerStr = mqttManager.getValue("pv_power");
  if (pvPowerStr != "N/A") {
    data.pvPower = pvPowerStr.toFloat();
  }
  
  String gridPowerStr = mqttManager.getValue("grid_power");
  if (gridPowerStr != "N/A") {
    data.gridPower = gridPowerStr.toFloat();
  }
  
  String loadPowerStr = mqttManager.getValue("load_power");
  if (loadPowerStr != "N/A") {
    data.loadPower = loadPowerStr.toFloat();
  }
  
  String batteryPowerStr = mqttManager.getValue("battery_power");
  if (batteryPowerStr != "N/A") {
    data.batteryPower = batteryPowerStr.toFloat();
  }
  
  String batteryVoltageStr = mqttManager.getValue("battery_voltage");
  if (batteryVoltageStr != "N/A") {
    data.batteryVoltage = batteryVoltageStr.toFloat();
  }
  
  String dailyYieldStr = mqttManager.getValue("daily_yield");
  if (dailyYieldStr != "N/A") {
    data.dailyYield = dailyYieldStr.toFloat();
  }
  
  // Autarkie berechnen
  if (data.loadPower > 0) {
    float selfSupply = data.pvPower + abs(min(0.0f, data.batteryPower));
    data.autarky = min(selfSupply / data.loadPower * 100, 100.0f);
  } else {
    data.autarky = 100.0f;
  }
  
  lastUpdate = millis();
}

void DataManager::simulateData() {
  // Diese Funktion ist eine Übernahme der alten Simulationsfunktion
  // Leichte Veränderungen der Werte um Dynamik zu simulieren
  data.pvPower += random(-100, 100);
  if (data.pvPower < 0) data.pvPower = 0;
  if (data.pvPower > 4000) data.pvPower = 4000;
  
  data.loadPower += random(-50, 50);
  if (data.loadPower < 200) data.loadPower = 200;
  if (data.loadPower > 3000) data.loadPower = 3000;
  
  // Netzwert berechnen (Überschuss geht ins Netz, negative Werte)
  float surplus = data.pvPower - data.loadPower;
  
  // Batteriesimulation
  if (surplus > 0) {
    // Überschuss: lade Batterie oder speise ins Netz ein
    if (data.batterySOC < 99) {
      // Noch Platz in der Batterie
      data.batteryPower = min(surplus, 2000.0f); // Max. 2kW Ladeleistung
      surplus -= data.batteryPower;
      
      // SOC erhöhen
      data.batterySOC += data.batteryPower / 5000.0; // Simulierte Ladegeschwindigkeit
      if (data.batterySOC > 100) data.batterySOC = 100;
    } else {
      // Batterie voll
      data.batteryPower = 0;
    }
    
    // Restlicher Überschuss ins Netz
    data.gridPower = -surplus;
  } else {
    // Defizit: entlade Batterie oder beziehe vom Netz
    if (data.batterySOC > 10) {
      // Batterie hat noch genug Energie
      data.batteryPower = max(-min(abs(surplus), 2000.0f), -2000.0f); // Max. 2kW Entladeleistung, negativ
      surplus += abs(data.batteryPower);
      
      // SOC verringern
      data.batterySOC += data.batteryPower / 5000.0; // Simulierte Entladegeschwindigkeit
      if (data.batterySOC < 0) data.batterySOC = 0;
    } else {
      // Batterie fast leer
      data.batteryPower = 0;
    }
    
    // Restliches Defizit vom Netz
    data.gridPower = abs(surplus);
  }
  
  // Autarkie berechnen
  if (data.loadPower > 0) {
    float selfSupply = data.pvPower + abs(min(0.0f, data.batteryPower));
    data.autarky = min(selfSupply / data.loadPower * 100, 100.0f);
  } else {
    data.autarky = 100;
  }
  
  // Batteriespannung simulieren (48V System)
  if (data.batterySOC < 20) {
    data.batteryVoltage = 47.0 + (data.batterySOC / 20.0);
  } else if (data.batterySOC > 80) {
    data.batteryVoltage = 48.0 + ((data.batterySOC - 80) / 20.0) * 1.5;
  } else {
    data.batteryVoltage = 48.0 + ((data.batterySOC - 50) / 30.0) * 0.5;
  }
  
  // Leichte Anpassung des Tagesertrags
  if (random(10) > 7) { // Nur manchmal erhöhen
    data.dailyYield += random(10) / 100.0;
  }
  
  // Debug-Ausgabe
  DEBUG_PRINTLN("Simulierte Daten aktualisiert:");
  DEBUG_PRINT("PV: "); DEBUG_PRINT(data.pvPower); DEBUG_PRINTLN(" W");
  DEBUG_PRINT("Last: "); DEBUG_PRINT(data.loadPower); DEBUG_PRINTLN(" W");
  DEBUG_PRINT("Netz: "); DEBUG_PRINT(data.gridPower); DEBUG_PRINTLN(" W");
  DEBUG_PRINT("Batterie: "); DEBUG_PRINT(data.batteryPower); DEBUG_PRINTLN(" W");
  DEBUG_PRINT("SOC: "); DEBUG_PRINT(data.batterySOC); DEBUG_PRINTLN(" %");
  DEBUG_PRINT("Autarkie: "); DEBUG_PRINT(data.autarky); DEBUG_PRINTLN(" %");
  
  lastUpdate = millis();
}

void DataManager::update() {
  // Periodische Aktualisierung abhängig vom Modus
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate > 5000) {  // Alle 5 Sekunden
    if (simulationMode) {
      simulateData();
    }
    // Im MQTT-Modus wird die Aktualisierung durch Callbacks ausgelöst
  }
}