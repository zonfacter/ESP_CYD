/**
 * DataManager.cpp - Implementierung der Datenmanagement-Funktionen
 */

#include "DataManager.h"
#include "MqttManager.h"

// Globale Instanz
DataManager dataManager;

DataManager::DataManager() {
  // Mutex für Thread-Sicherheit erstellen
  dataMutex = xSemaphoreCreateMutex();
  if (dataMutex == NULL) {
    DEBUG_ERROR("Fehler beim Erstellen des DataManager-Mutex!");
  }
  
  // Konstruktor
  // Versuche, Daten aus dem Cache zu laden
  loadDataFromCache();
  // Versuche, historische Daten zu laden
  loadHistoricalDataFromStorage();
}

void DataManager::updateFromMqtt(MqttManager& mqttManager) {
  // Thread-Sicherheit: Mutex sperren
  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
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
    
    // Mutex freigeben vor zeitintensiven Operationen
    xSemaphoreGive(dataMutex);
    
    // Daten im Cache speichern (alle 5 Minuten)
    if (millis() - lastCacheUpdate > 300000) { // 5 Minuten
      saveDataToCache();
      lastCacheUpdate = millis();
      
      // Historischen Datenpunkt hinzufügen
      addHistoricalDataPoint();
    }
  } else {
    DEBUG_WARNING("Konnte Daten-Mutex nicht sperren in updateFromMqtt");
  }
}

bool DataManager::saveDataToCache() {
  // Speichere aktuelle Daten im SPIFFS (bereits initialisiert in setup)
  // Thread-Sicherheit: Mutex sperren für Datenzugriff
  SolarData dataCopy;
  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    dataCopy = data;  // Kopie erstellen
    xSemaphoreGive(dataMutex);
  } else {
    DEBUG_WARNING("Konnte Daten-Mutex nicht sperren in saveDataToCache");
    return false;
  }
  
  // JSON-Dokument erstellen (mit Kopie der Daten, kein Mutex nötig)
  JsonDocument doc;
  
  // Daten hinzufügen
  doc["timestamp"] = millis();
  doc["batterySOC"] = dataCopy.batterySOC;
  doc["pvPower"] = dataCopy.pvPower;
  doc["gridPower"] = dataCopy.gridPower;
  doc["loadPower"] = dataCopy.loadPower;
  doc["batteryPower"] = dataCopy.batteryPower;
  doc["dailyYield"] = dataCopy.dailyYield;
  doc["batteryVoltage"] = dataCopy.batteryVoltage;
  doc["autarky"] = dataCopy.autarky;
  
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

// Thread-sichere Getter-Implementierung
SolarData DataManager::getDataCopy() {
  SolarData copy;
  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    copy = data;
    xSemaphoreGive(dataMutex);
  } else {
    DEBUG_WARNING("Konnte Daten-Mutex nicht sperren in getDataCopy - Verwende Standardwerte");
    // copy ist bereits mit Standardwerten initialisiert (Constructor)
  }
  return copy;
}

bool DataManager::loadDataFromCache() {
  // Lade gespeicherte Daten aus SPIFFS (bereits initialisiert)
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

bool DataManager::addHistoricalDataPoint() {
  // Füge aktuellen Datenpunkt zur Historie hinzu
  history[historyIndex] = HistoricalData(millis(), data);
  
  // Index für den nächsten Eintrag
  historyIndex = (historyIndex + 1) % MAX_HISTORY_ENTRIES;
  
  DEBUG_INFO("Historischer Datenpunkt hinzugefügt");
  
  // Persistente Speicherung in SPIFFS
  // Begrenze Schreibvorgänge, um Flash-Verschleiß zu minimieren (z.B. jeder 6. Datenpunkt)
  static int persistCounter = 0;
  persistCounter++;
  
  if (persistCounter >= 6) {  // Jeder 6. Datenpunkt wird persistent gespeichert
    persistCounter = 0;
    
    // Prüfe und manage den Speicherplatz
    checkStorageSpace();
    
    // Sammle alle aktuellen historischen Daten
    JsonDocument historyDoc;
    historyDoc["timestamp"] = millis();
    historyDoc["count"] = getHistoryCount();
    
    // Erstelle Array für die Datenpunkte
    JsonArray dataPoints = historyDoc["points"].to<JsonArray>();

    // Füge alle gültigen Datenpunkte hinzu
    for (int i = 0; i < MAX_HISTORY_ENTRIES; i++) {
      if (history[i].timestamp > 0) {
        JsonObject point = dataPoints.add<JsonObject>();
        point["ts"] = history[i].timestamp;
        point["soc"] = history[i].data.batterySOC;
        point["pv"] = history[i].data.pvPower;
        point["grid"] = history[i].data.gridPower;
        point["load"] = history[i].data.loadPower;
        point["batt"] = history[i].data.batteryPower;
        point["yield"] = history[i].data.dailyYield;
        point["volt"] = history[i].data.batteryVoltage;
        point["auto"] = history[i].data.autarky;
      }
    }
    
    // Als JSON in SPIFFS speichern (bereits initialisiert)
    // Rotierendes Logging - mehrere Dateien verwenden, um Datenverlust zu vermeiden
    static int fileIndex = 0;
    fileIndex = (fileIndex + 1) % 3;  // 3 Dateien rotierend verwenden
    String filename = "/history_data_" + String(fileIndex) + ".json";
    
    File file = SPIFFS.open(filename, "w");
    if (!file) {
      DEBUG_ERROR("Fehler beim Öffnen der Historien-Datei zum Schreiben");
      return false;
    }
    
    // JSON in die Datei schreiben
    if (serializeJson(historyDoc, file) == 0) {
      DEBUG_ERROR("Fehler beim Schreiben der Historien-Daten");
      file.close();
      return false;
    }
    
    file.close();
    DEBUG_INFO("Historische Daten erfolgreich gespeichert in " + filename);
    
    // Täglichen Verlaufsdatenpunkt speichern (mit Datum als Dateiname)
    // Verwende einen eigenen Timer, damit dies nur einmal täglich passiert
    static unsigned long lastDailySave = 0;
    if (millis() - lastDailySave > 86400000) { // 24 Stunden in ms
      lastDailySave = millis();
      
      // Aktuelle Daten für Tagesverlauf speichern
      JsonDocument dailyDoc;
      
      // Füge für den täglichen Verlauf relevante Daten hinzu
      dailyDoc["date"] = getDayString(); // Funktion, die aktuelles Datum als String zurückgibt
      dailyDoc["daily_yield"] = data.dailyYield;
      dailyDoc["max_pv"] = getMaxValueForToday("pv");  // Hilfsfunktion implementieren
      dailyDoc["avg_soc"] = getAvgValueForToday("soc"); // Hilfsfunktion implementieren
      dailyDoc["avg_autarky"] = getAvgValueForToday("autarky"); // Hilfsfunktion implementieren
      
      // Speichere tägliche Zusammenfassung
      String dailyFilename = "/daily_" + getDayString() + ".json";
      File dailyFile = SPIFFS.open(dailyFilename, "w");
      if (dailyFile) {
        serializeJson(dailyDoc, dailyFile);
        dailyFile.close();
        DEBUG_INFO("Täglicher Verlaufsdatenpunkt gespeichert in " + dailyFilename);
      }
    }
    
    return true;
  }
  
  return true; // Erfolgreich (auch wenn kein Schreiben stattfand)
}

// Hilfsfunktion, um das aktuelle Datum als String zu erhalten
String DataManager::getDayString() {
  // In einer realen Anwendung würde hier ein RTC oder NTP verwendet
  // Für diese Beispielimplementierung verwenden wir einen einfachen Zähler
  static int dayCounter = 0;
  unsigned long uptime = millis() / 1000 / 60 / 60 / 24; // Tage seit Start
  
  if (uptime > dayCounter) {
    dayCounter = uptime;
  }
  
  return String(dayCounter);
}

// Speicherplatz-Überwachung
bool DataManager::checkStorageSpace() {
  if (!SPIFFS.begin(false)) {
    DEBUG_ERROR("SPIFFS konnte nicht für Speicherprüfung initialisiert werden!");
    return false;
  }
  
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  float usedPercentage = (float)usedBytes / totalBytes * 100;
  
  DEBUG_INFO("SPIFFS Speichernutzung: " + String(usedBytes) + " von " + String(totalBytes) + 
             " Bytes (" + String(usedPercentage) + "%)");
  
  // Wenn der Speicher zu voll ist (z.B. > 90%), alte Dateien löschen
  if (usedPercentage > 90) {
    DEBUG_WARNING("SPIFFS fast voll, lösche alte tägliche Dateien...");
    
    // Suche nach der ältesten täglichen Datei und lösche sie
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    String oldestFile = "";
    unsigned long oldestTime = millis();
    
    while (file) {
      String fileName = file.name();
      if (fileName.startsWith("/daily_")) {
        // Extrahiere den Zeitstempel oder Zähler aus dem Dateinamen
        int dayValue = fileName.substring(7, fileName.lastIndexOf(".")).toInt();
        if (dayValue < oldestTime) {
          oldestFile = fileName;
          oldestTime = dayValue;
        }
      }
      file = root.openNextFile();
    }
    
    if (oldestFile.length() > 0) {
      DEBUG_WARNING("Lösche älteste tägliche Datei: " + oldestFile);
      SPIFFS.remove(oldestFile);
      return true;
    }
  }
  
  return true;
}

// Hilfsfunktion zur Ermittlung des Maximalwerts eines bestimmten Datenpunkts
float DataManager::getMaxValueForToday(const String& field) {
  float maxValue = 0.0f;
  unsigned long dayStart = (millis() / 86400000) * 86400000; // Beginn des aktuellen Tages
  
  for (int i = 0; i < MAX_HISTORY_ENTRIES; i++) {
    if (history[i].timestamp > dayStart) {
      float value = 0.0f;
      
      // Wert basierend auf dem Feldnamen auswählen
      if (field == "pv") {
        value = history[i].data.pvPower;
      } else if (field == "load") {
        value = history[i].data.loadPower;
      } else if (field == "autarky") {
        value = history[i].data.autarky;
      } else if (field == "grid") {
        value = history[i].data.gridPower;
      } else if (field == "battery") {
        value = history[i].data.batteryPower;
      } else if (field == "voltage") {
        value = history[i].data.batteryVoltage;
      } else if (field == "yield") {
        value = history[i].data.dailyYield;
      } // ... weitere Felder
      
      if (value > maxValue) {
        maxValue = value;
      }
    }
  }
  
  return maxValue;
}

// Hilfsfunktion zur Ermittlung des Durchschnittswerts eines bestimmten Datenpunkts
float DataManager::getAvgValueForToday(const String& field) {
  float sum = 0.0f;
  int count = 0;
  unsigned long dayStart = (millis() / 86400000) * 86400000; // Beginn des aktuellen Tages
  
  for (int i = 0; i < MAX_HISTORY_ENTRIES; i++) {
    if (history[i].timestamp > dayStart) {
      float value = 0.0f;
      
      // Wert basierend auf dem Feldnamen auswählen
      if (field == "soc") {
        value = history[i].data.batterySOC;
      } else if (field == "autarky") {
        value = history[i].data.autarky;
      } else if (field == "pv") {
        value = history[i].data.pvPower;
      } else if (field == "load") {
        value = history[i].data.loadPower;
      } else if (field == "autarky") {
        value = history[i].data.autarky;
      } else if (field == "grid") {
        value = history[i].data.gridPower;
      } else if (field == "battery") {
        value = history[i].data.batteryPower;
      } else if (field == "voltage") {
        value = history[i].data.batteryVoltage;
      } else if (field == "yield") {
        value = history[i].data.dailyYield;
      }
      
      sum += value;
      count++;
    }
  }
  
  return (count > 0) ? (sum / count) : 0.0f;
}

// Lade die historischen Daten aus dem SPIFFS beim Start
bool DataManager::loadHistoricalDataFromStorage() {
  // SPIFFS bereits initialisiert in setup
  bool anyFileLoaded = false;
  
  // Versuche, die neueste der rotierenden Dateien zu laden
  for (int i = 0; i < 3; i++) {
    String filename = "/history_data_" + String(i) + ".json";
    
    if (SPIFFS.exists(filename)) {
      File file = SPIFFS.open(filename, "r");
      if (!file) {
        DEBUG_PRINTLN("Fehler beim Öffnen der Historien-Datei zum Lesen: " + filename);
        continue;
      }
      
      // JSON deserialisieren
      JsonDocument historyDoc;
      DeserializationError error = deserializeJson(historyDoc, file);
      file.close();
      
      if (error) {
        DEBUG_PRINT("JSON Parsing Fehler beim Laden der Historie: ");
        DEBUG_PRINTLN(error.c_str());
        continue;
      }
      
      // Daten in die Historie laden
      historyIndex = 0; // Von vorne beginnen
      JsonArray dataPoints = historyDoc["points"];
      
      for (JsonObject point : dataPoints) {
        if (historyIndex < MAX_HISTORY_ENTRIES) {
          history[historyIndex].timestamp = point["ts"];
          
          // Lade alle Felder der SolarData-Struktur
          history[historyIndex].data.batterySOC = point["soc"];
          history[historyIndex].data.pvPower = point["pv"];
          history[historyIndex].data.gridPower = point["grid"];
          history[historyIndex].data.loadPower = point["load"];
          history[historyIndex].data.batteryPower = point["batt"];
          history[historyIndex].data.dailyYield = point["yield"];
          history[historyIndex].data.batteryVoltage = point["volt"];
          history[historyIndex].data.autarky = point["auto"];
          
          historyIndex++;
        }
      }
      
      DEBUG_PRINT("Historische Daten geladen aus ");
      DEBUG_PRINT(filename);
      DEBUG_PRINT(": ");
      DEBUG_PRINT(historyIndex);
      DEBUG_PRINTLN(" Datenpunkte");
      
      anyFileLoaded = true;
      break; // Lade nur die erste gefundene Datei
    }
  }
  
  return anyFileLoaded;
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