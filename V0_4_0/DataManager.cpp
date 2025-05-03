/**
 * DataManager.cpp - Implementierung der Datenmanagement-Funktionen
 */

#include "DataManager.h"
#include "MqttManager.h"

// Globale Instanz
DataManager dataManager;

DataManager::DataManager() {
  // Konstruktor
  // Versuche, Daten aus dem Cache zu laden
  loadDataFromCache();
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
  
  // Daten im Cache speichern (alle 5 Minuten)
  if (millis() - lastCacheUpdate > 300000) { // 5 Minuten
    saveDataToCache();
    lastCacheUpdate = millis();
    
    // Historischen Datenpunkt hinzufügen
    addHistoricalDataPoint();
  }
}

bool DataManager::saveDataToCache() {
  // Speichere aktuelle Daten im SPIFFS
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS konnte nicht initialisiert werden beim Cachen von Daten!");
    return false;
  }
  
  // JSON-Dokument erstellen
  JsonDocument doc;
  
  // Daten hinzufügen
  doc["timestamp"] = millis();
  doc["batterySOC"] = data.batterySOC;
  doc["pvPower"] = data.pvPower;
  doc["gridPower"] = data.gridPower;
  doc["loadPower"] = data.loadPower;
  doc["batteryPower"] = data.batteryPower;
  doc["dailyYield"] = data.dailyYield;
  doc["batteryVoltage"] = data.batteryVoltage;
  doc["autarky"] = data.autarky;
  
  // Datei zum Schreiben öffnen
  File file = SPIFFS.open("/solar_data_cache.json", "w");
  if (!file) {
    DEBUG_PRINTLN("Fehler beim Öffnen der Cache-Datei zum Schreiben");
    return false;
  }
  
  // JSON in die Datei schreiben
  if (serializeJson(doc, file) == 0) {
    DEBUG_PRINTLN("Fehler beim Schreiben der Daten in den Cache");
    file.close();
    return false;
  }
  
  file.close();
  DEBUG_PRINTLN("Solardaten erfolgreich im Cache gespeichert");
  return true;
}

bool DataManager::loadDataFromCache() {
  // Lade gespeicherte Daten aus SPIFFS
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS konnte nicht initialisiert werden beim Laden aus dem Cache!");
    return false;
  }
  
  if (!SPIFFS.exists("/solar_data_cache.json")) {
    DEBUG_PRINTLN("Keine Cache-Datei gefunden");
    return false;
  }
  
  File file = SPIFFS.open("/solar_data_cache.json", "r");
  if (!file) {
    DEBUG_PRINTLN("Fehler beim Öffnen der Cache-Datei zum Lesen");
    return false;
  }
  
  // JSON deserialisieren
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    DEBUG_PRINT("JSON Parsing Fehler: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // Daten aus JSON extrahieren
  unsigned long timestamp = doc["timestamp"];
  data.batterySOC = doc["batterySOC"];
  data.pvPower = doc["pvPower"];
  data.gridPower = doc["gridPower"];
  data.loadPower = doc["loadPower"];
  data.batteryPower = doc["batteryPower"];
  data.dailyYield = doc["dailyYield"];
  data.batteryVoltage = doc["batteryVoltage"];
  data.autarky = doc["autarky"];
  
  DEBUG_PRINT("Solardaten aus Cache geladen (Alter: ");
  DEBUG_PRINT((millis() - timestamp) / 1000 / 60);
  DEBUG_PRINTLN(" Minuten)");
  
  return true;
}

bool DataManager::checkAndLoadCachedData() {
  // Lade Daten aus dem Cache, falls MQTT nicht verfügbar ist
  if (simulationMode) {
    // Wenn wir im Simulationsmodus sind, verwenden wir den Cache
    return loadDataFromCache();
  }
  return false; // Im Online-Modus nicht notwendig
}

void DataManager::addHistoricalDataPoint() {
  // Füge aktuellen Datenpunkt zur Historie hinzu
  history[historyIndex] = HistoricalData(millis(), data);
  
  // Index für den nächsten Eintrag
  historyIndex = (historyIndex + 1) % MAX_HISTORY_ENTRIES;
  
  DEBUG_PRINTLN("Historischer Datenpunkt hinzugefügt");
}

int DataManager::getHistoryCount() const {
  // Zähle gültige Einträge (mit Zeitstempel > 0)
  int count = 0;
  for (int i = 0; i < MAX_HISTORY_ENTRIES; i++) {
    if (history[i].timestamp > 0) {
      count++;
    }
  }
  return count;
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
  
  // Füge historischen Datenpunkt hinzu (alle 30 Minuten in der Simulation)
  static unsigned long lastHistoricalUpdate = 0;
  if (millis() - lastHistoricalUpdate > 1800000) { // 30 Minuten
    addHistoricalDataPoint();
    lastHistoricalUpdate = millis();
  }
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