/**
 * ViewManager.cpp - Implementierung der Detailansichten
 */

#include "ViewManager.h"
#include "MqttManager.h"
#include <WiFi.h>

// Externe Globale Variablen
extern TFT_eSPI tft;
extern MqttManager mqttManager;
extern DataManager dataManager;
extern ConfigManager configManager;


// Globale Instanz
// ViewManager viewManager(tft, dataManager); // In C++ darf eine globale Variable nur einmal definiert werden.

// Diese Änderungen sollten in ViewManager.cpp eingefügt werden

ViewManager::ViewManager(TFT_eSPI &tft, DataManager &dataManager) 
  : tft(tft), dataManager(dataManager) {
  
  // Registriere alle Ansichtsfunktionen in der Map
  viewFunctions["drawSolarStatus"] = &ViewManager::drawSolarStatus;
  viewFunctions["drawBatteryStatus"] = &ViewManager::drawBatteryStatus;
  viewFunctions["drawGridStatus"] = &ViewManager::drawGridStatus;
  viewFunctions["drawPvPower"] = &ViewManager::drawPvPower;
  viewFunctions["drawConsumption"] = &ViewManager::drawConsumption;
  viewFunctions["drawAutarky"] = &ViewManager::drawAutarky;
  viewFunctions["drawDailyValues"] = &ViewManager::drawDailyValues;
  viewFunctions["drawStatistics"] = &ViewManager::drawStatistics;
  
  viewFunctions["controlHeating"] = &ViewManager::controlHeating;
  viewFunctions["controlPool"] = &ViewManager::controlPool;
  
  viewFunctions["setupWifi"] = &ViewManager::setupWifi;
  viewFunctions["setupMqtt"] = &ViewManager::setupMqtt;
  viewFunctions["setupDisplay"] = &ViewManager::setupDisplay;
  viewFunctions["showSystemInfo"] = &ViewManager::showSystemInfo;

  // Registriere alle Update-Funktionen in der Map
  updateFunctions["drawSolarStatus"] = &ViewManager::updateSolarStatus;
  updateFunctions["drawBatteryStatus"] = &ViewManager::updateBatteryStatus;
  updateFunctions["drawGridStatus"] = &ViewManager::updateGridStatus;
  updateFunctions["drawPvPower"] = &ViewManager::updatePvPower;
  updateFunctions["drawConsumption"] = &ViewManager::updateConsumption;
  updateFunctions["drawAutarky"] = &ViewManager::updateAutarky;
  updateFunctions["drawDailyValues"] = &ViewManager::updateDailyValues;
  updateFunctions["drawStatistics"] = &ViewManager::updateStatistics;
  
  updateFunctions["controlHeating"] = &ViewManager::updateHeating;
  updateFunctions["controlPool"] = &ViewManager::updatePool;
  
  updateFunctions["setupWifi"] = &ViewManager::updateWifi;
  updateFunctions["setupMqtt"] = &ViewManager::updateMqtt;
  updateFunctions["setupDisplay"] = &ViewManager::updateDisplay;
  updateFunctions["showSystemInfo"] = &ViewManager::updateSystemInfo;
}

bool ViewManager::showView(const String &functionName) {
  currentView = functionName;
  
  // Bildschirm löschen
  tft.fillScreen(BACKGROUND);
  
  // Zurück-Button zeichnen
  drawBackButton();
  
  // Titel zeichnen
  tft.setTextSize(2);
  tft.setTextColor(TITLE_COLOR, BACKGROUND);
  tft.setCursor(90, 15);
  tft.print(functionName); // Hier besser einen benutzerfreundlichen Titel verwenden
  
  // Trennlinie
  tft.drawLine(10, 45, SCREEN_WIDTH - 10, 45, TFT_DARKGREY);
  
  // Setze den Flag für initialen Draw
  isInitialDraw = true;
  
  // Speichere aktuelle Daten als Referenz
  lastDrawnData = dataManager.getData();
  
  // Prüfe, ob die Funktion existiert
  auto it = viewFunctions.find(functionName);
  if (it != viewFunctions.end()) {
    // Funktion aufrufen
    ViewFunction func = it->second;
    (this->*func)();
    
    // Statusleiste zeichnen
    drawStatusBar();
    
    // Nach dem ersten Zeichnen Flag zurücksetzen
    isInitialDraw = false;
    
    return true;
  } else {
    // Funktion nicht gefunden
    tft.setTextColor(TEXT_COLOR, BACKGROUND);
    tft.setTextSize(1);
    tft.setCursor(20, 70);
    tft.println("Ansicht nicht implementiert: " + functionName);
    
    drawStatusBar();
    
    isInitialDraw = false;
    
    return false;
  }
}

bool ViewManager::updateView() {
  // Nur aktualisieren, wenn wir eine aktuelle Ansicht haben
  if (currentView.length() == 0) {
    return false;
  }
  
  // Prüfe, ob die Update-Funktion existiert
  auto it = updateFunctions.find(currentView);
  if (it != updateFunctions.end()) {
    // Aktualisiere Statusleiste ohne Flackern
    drawStatusBar();
    
    // Funktion aufrufen
    UpdateFunction func = it->second;
    (this->*func)();
    
    // Aktualisiere gespeicherte Daten
    lastDrawnData = dataManager.getData();
    
    return true;
  }
  
  return false;
}

// Beispielupdater für SolarStatus
void ViewManager::updateSolarStatus() {
  SolarData& currentData = dataManager.getData();
  tft.setTextSize(1);
  
  // PV Leistung aktualisieren, wenn sich der Wert geändert hat
  if (currentData.pvPower != lastDrawnData.pvPower) {
    // Lösche den alten Wert 
    tft.fillRect(200, 80, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 80);
    tft.setTextColor(TFT_GREEN, BACKGROUND);
    tft.print(currentData.pvPower);
    tft.print(" W");
  }
  
  // Verbrauch aktualisieren, wenn sich der Wert geändert hat
  if (currentData.loadPower != lastDrawnData.loadPower) {
    // Lösche den alten Wert
    tft.fillRect(200, 100, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 100);
    tft.setTextColor(TFT_RED, BACKGROUND);
    tft.print(currentData.loadPower);
    tft.print(" W");
  }
  
  // Netzbezug/Einspeisung aktualisieren
  if (currentData.gridPower != lastDrawnData.gridPower) {
    // Lösche den alten Wert
    tft.fillRect(200, 120, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 120);
    if (currentData.gridPower < 0) {
      tft.setTextColor(TFT_GREEN, BACKGROUND); // Einspeisung
      tft.print(abs(currentData.gridPower));
      tft.print(" W (Einspeisung)");
    } else {
      tft.setTextColor(TFT_RED, BACKGROUND); // Bezug
      tft.print(currentData.gridPower);
      tft.print(" W (Bezug)");
    }
  }
  
  // Batterie aktualisieren
  if (currentData.batteryPower != lastDrawnData.batteryPower) {
    // Lösche den alten Wert
    tft.fillRect(200, 140, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 140);
    if (currentData.batteryPower > 0) {
      tft.setTextColor(TFT_GREEN, BACKGROUND); // Laden
      tft.print(currentData.batteryPower);
      tft.print(" W (Laden)");
    } else {
      tft.setTextColor(TFT_RED, BACKGROUND); // Entladen
      tft.print(abs(currentData.batteryPower));
      tft.print(" W (Entladen)");
    }
  }
  
  // Autarkie aktualisieren
  if (currentData.autarky != lastDrawnData.autarky) {
    // Lösche den alten Wert
    tft.fillRect(200, 160, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 160);
    tft.setTextColor(TFT_CYAN, BACKGROUND);
    tft.print(currentData.autarky);
    tft.print(" %");
  }
  
  // Batteriestand aktualisieren
  if (currentData.batterySOC != lastDrawnData.batterySOC) {
    // Lösche den alten Wert
    tft.fillRect(200, 180, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 180);
    tft.setTextColor(TFT_YELLOW, BACKGROUND);
    tft.print(currentData.batterySOC);
    tft.print(" %");
  }
}

// Beispielupdater für BatteryStatus
// Update-Funktion für die Batterieansicht
void ViewManager::updateBatteryStatus() {
  SolarData& currentData = dataManager.getData();
  tft.setTextSize(1);
  
  // Batterieparameter aus der Konfiguration laden
  JsonDocument config;
  float batteryCapacityAh = 360.0;  // Standardwert
  float batteryNomVoltage = 51.2;   // Standardwert für 16S LiFePO4
  float targetSOC = 80.0;           // Standard-Ziel-SOC
  float minSOC = 20.0;              // Standard-Mindest-SOC
  
  if (configManager.loadJsonConfig("/config.json", config)) {
    if (config.containsKey("battery")) {
      batteryCapacityAh = config["battery"]["capacity_ah"].as<float>();
      batteryNomVoltage = config["battery"]["nominal_voltage"].as<float>();
      targetSOC = config["battery"]["target_soc"].as<float>();
      minSOC = config["battery"]["min_soc"].as<float>();
    }
  }
  
  // State of Charge aktualisieren
  if (currentData.batterySOC != lastDrawnData.batterySOC) {
    // Lösche den alten Wert
    tft.fillRect(200, 80, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 80);
    tft.setTextColor(TFT_YELLOW, BACKGROUND);
    tft.print(currentData.batterySOC);
    tft.print(" %");
    
    // Batterie-Balken aktualisieren
    int barWidth = 200;
    int barHeight = 30;
    int barX = 60;
    int barY = 100;
    
    // Rahmen
    tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
    
    // Alte Füllung löschen
    tft.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, BACKGROUND);
    
    // Füllstand
    int fillWidth = map(currentData.batterySOC, 0, 100, 0, barWidth - 2);
    
    // Farbverlauf je nach Ladezustand
    uint16_t barColor;
    if (currentData.batterySOC < minSOC) {
      barColor = TFT_RED;
    } else if (currentData.batterySOC < 50) {
      barColor = TFT_ORANGE;
    } else {
      barColor = TFT_GREEN;
    }
    
    tft.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, barColor);
    
    // Wenn sich der SOC geändert hat, müssen wir auch die gespeicherte Energie aktualisieren
    tft.fillRect(200, 210, 120, 10, BACKGROUND);
    tft.setCursor(200, 210);
    tft.setTextColor(TFT_YELLOW, BACKGROUND);
    float currentEnergy = (currentData.batterySOC / 100.0) * batteryCapacityAh * batteryNomVoltage;
    float storedkWh = currentEnergy / 1000.0;
    tft.print(storedkWh, 2);
    tft.print(" kWh");
  }
  
  // Batterieleistung aktualisieren
  if (currentData.batteryPower != lastDrawnData.batteryPower || 
      currentData.batterySOC != lastDrawnData.batterySOC) {
    // Lösche den alten Wert
    tft.fillRect(200, 150, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 150);
    if (currentData.batteryPower > 0) {
      tft.setTextColor(TFT_GREEN, BACKGROUND);
      tft.print(currentData.batteryPower);
      tft.print(" W (Laden)");
    } else {
      tft.setTextColor(TFT_RED, BACKGROUND);
      tft.print(abs(currentData.batteryPower));
      tft.print(" W (Entladen)");
    }
    
    // Zeit bis zum Ziel-SOC aktualisieren, da sich die Leistung geändert hat
    tft.fillRect(20, 190, 280, 10, BACKGROUND);
    tft.setCursor(20, 190);
    tft.setTextColor(TEXT_COLOR, BACKGROUND);
    
    // Berechnung der Zeit bis zum Ziel-SOC
    float socDifference = 0;
    String timeString = "";
    bool isTimeCalculable = false;
    
    // Batterie-Energieinhalt in Wh berechnen
    float batteryCapacityWh = batteryCapacityAh * batteryNomVoltage;
    
    // Aktueller Energieinhalt in Wh
    float currentEnergy = (currentData.batterySOC / 100.0) * batteryCapacityWh;
    
    if (currentData.batteryPower > 10) {  // Ladend mit signifikanter Leistung
      // Ziel: Aufladen bis zum Ziel-SOC
      // Verbleibende Energie bis zum Ziel
      float targetEnergy = (targetSOC / 100.0) * batteryCapacityWh;
      float energyToTarget = targetEnergy - currentEnergy;
      
      if (energyToTarget > 0) {
        // Stunden bis zum Ziel-SOC bei aktueller Ladeleistung
        float hoursToTarget = energyToTarget / currentData.batteryPower;
        
        if (hoursToTarget > 0 && hoursToTarget < 100) {
          int hours = (int)hoursToTarget;
          int minutes = (int)((hoursToTarget - hours) * 60);
          
          timeString = String(hours) + "h " + String(minutes) + "min";
          socDifference = targetSOC - currentData.batterySOC;
          isTimeCalculable = true;
        }
      }
    } 
    else if (currentData.batteryPower < -10) {  // Entladend mit signifikanter Leistung
      // Ziel: Verbleibende Zeit bis Mindest-SOC
      float minEnergy = (minSOC / 100.0) * batteryCapacityWh;
      float energyToMin = currentEnergy - minEnergy;
      
      if (energyToMin > 0) {
        // Stunden bis zum Minimum-SOC bei aktueller Entladeleistung
        float hoursToMin = energyToMin / abs(currentData.batteryPower);
        
        if (hoursToMin > 0 && hoursToMin < 100) {
          int hours = (int)hoursToMin;
          int minutes = (int)((hoursToMin - hours) * 60);
          
          timeString = String(hours) + "h " + String(minutes) + "min";
          socDifference = currentData.batterySOC - minSOC;
          isTimeCalculable = true;
        }
      }
    }
    
    // Zeit bis zum Ziel-SOC anzeigen
    if (isTimeCalculable) {
      if (currentData.batteryPower > 0) {
        tft.print("Zeit bis " + String(targetSOC, 0) + "% SOC:");
        tft.setCursor(200, 190);
        tft.setTextColor(TFT_GREEN, BACKGROUND);
      } else {
        tft.print("Zeit bis " + String(minSOC, 0) + "% SOC:");
        tft.setCursor(200, 190);
        tft.setTextColor(TFT_RED, BACKGROUND);
      }
      tft.print(timeString);
    } else {
      if (currentData.batterySOC >= targetSOC && currentData.batteryPower > 0) {
        tft.print("Ziel-SOC erreicht");
      } else if (currentData.batterySOC <= minSOC && currentData.batteryPower < 0) {
        tft.print("Min-SOC erreicht");
      } else if (abs(currentData.batteryPower) < 10) {
        tft.print("Batterie inaktiv");
      } else {
        tft.print("Keine Zeitberechnung möglich");
      }
    }
  }
  
  // Batteriespannung aktualisieren
  if (currentData.batteryVoltage != lastDrawnData.batteryVoltage) {
    // Lösche den alten Wert
    tft.fillRect(200, 170, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 170);
    tft.setTextColor(TFT_CYAN, BACKGROUND);
    tft.print(currentData.batteryVoltage);
    tft.print(" V");
  }
}

// Beispielupdater für GridStatus
void ViewManager::updateGridStatus() {
  SolarData& currentData = dataManager.getData();
  tft.setTextSize(1);
  
  // Wenn sich der Grid-Power-Wert geändert hat
  if (currentData.gridPower != lastDrawnData.gridPower) {
    // Lösche den alten Wert
    tft.fillRect(200, 80, 120, 10, BACKGROUND);
    
    // Neuen Wert schreiben
    tft.setCursor(200, 80);
    if (currentData.gridPower < 0) {
      tft.setTextColor(TFT_GREEN, BACKGROUND);
      tft.print(abs(currentData.gridPower));
      tft.print(" W (Einspeisung)");
    } else {
      tft.setTextColor(TFT_RED, BACKGROUND);
      tft.print(currentData.gridPower);
      tft.print(" W (Bezug)");
    }
    
    // Visualisierung des Energieflusses aktualisieren
    int centerX = 160;
    int centerY = 130;
    int radius = 50;
    
    // Lösche den Bereich um die Visualisierung
    tft.fillRect(centerX - radius - 60, centerY - radius, 
                 radius * 2 + 90, radius * 2, BACKGROUND);
    
    // Kreisrahmen als "Haus"
    tft.drawCircle(centerX, centerY, radius, TFT_WHITE);
    
    // Energieflussrichtung mit Pfeilen darstellen
    if (currentData.gridPower < 0) {
      // Einspeisung: Pfeile vom Haus zum Netz
      tft.fillTriangle(
        centerX + radius + 20, centerY,
        centerX + radius, centerY - 10,
        centerX + radius, centerY + 10,
        TFT_GREEN
      );
      
      tft.setTextColor(TFT_GREEN, BACKGROUND);
      tft.setCursor(centerX + radius + 30, centerY - 5);
      tft.print("Netz");
    } else {
      // Bezug: Pfeile vom Netz zum Haus
      tft.fillTriangle(
        centerX - radius - 20, centerY,
        centerX - radius, centerY - 10,
        centerX - radius, centerY + 10,
        TFT_RED
      );
      
      tft.setTextColor(TFT_RED, BACKGROUND);
      tft.setCursor(centerX - radius - 60, centerY - 5);
      tft.print("Netz");
    }
    
    // Haus-Symbol in der Mitte
    tft.setTextColor(TFT_WHITE, BACKGROUND);
    tft.setCursor(centerX - 15, centerY - 5);
    tft.print("Haus");
  }
}

// Stub-Methoden für die anderen Ansichten (müssen entsprechend implementiert werden)
void ViewManager::updatePvPower() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updateConsumption() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updateAutarky() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updateDailyValues() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updateStatistics() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updateHeating() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updatePool() {
  // Implementierung entsprechend der Ansicht
}

void ViewManager::updateWifi() {
  // WLAN-Status aktualisieren
  tft.setCursor(20, 110);
  tft.fillRect(90, 110, 230, 10, BACKGROUND);
  tft.print("Status: ");
  tft.println(WiFi.status() == WL_CONNECTED ? "Verbunden" : "Getrennt");
}

void ViewManager::updateMqtt() {
  // MQTT-Status aktualisieren
  tft.setCursor(20, 110);
  tft.fillRect(90, 110, 230, 10, BACKGROUND);
  tft.print("Status: ");
  tft.println(mqttManager.isConnected() ? "Verbunden" : "Getrennt");
}

void ViewManager::updateDisplay() {
  // Display-Einstellungen aktualisieren
  // In der aktuellen Implementierung gibt es nichts dynamisches zu aktualisieren
}

void ViewManager::updateSystemInfo() {
  // Aktualisiere Laufzeit
  tft.fillRect(90, 170, 230, 10, BACKGROUND);
  tft.setCursor(20, 170);
  tft.print("Laufzeit: ");
  tft.print(millis() / 1000 / 60);
  tft.println(" Minuten");
  
  // Aktualisiere Speicherinformation
  tft.fillRect(90, 150, 230, 10, BACKGROUND);
  tft.setCursor(20, 150);
  tft.print("Speicher: ");
  tft.print(ESP.getFreeHeap() / 1024);
  tft.println(" KB frei");
}
void ViewManager::drawBackButton() {
  tft.fillRoundRect(10, 10, 50, 30, 5, TFT_DARKGREY);
  tft.drawRoundRect(10, 10, 50, 30, 5, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(15, 20);
  tft.print("Zurück");
}

void ViewManager::drawStatusBar() {
  // Hintergrund für Statusleiste
  tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, BACKGROUND);
  
  // WiFi-Status
  tft.setTextSize(1);
  tft.setTextColor(WiFi.status() == WL_CONNECTED ? STATUS_COLOR : TFT_RED, BACKGROUND);
  tft.setCursor(10, SCREEN_HEIGHT - 15);
  tft.print("WiFi: ");
  tft.print(WiFi.status() == WL_CONNECTED ? "Verbunden" : "Getrennt");
  
  // MQTT-Status
  tft.setCursor(SCREEN_WIDTH - 120, SCREEN_HEIGHT - 15);
  tft.setTextColor(mqttManager.isConnected() ? STATUS_COLOR : TFT_RED, BACKGROUND);
  tft.print("MQTT: ");
  tft.print(mqttManager.isConnected() ? "Verbunden" : "Getrennt");
  
  // Aktuelle Zeit (simuliert) oder Datenquelle
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(SCREEN_WIDTH / 2 - 55, SCREEN_HEIGHT - 15);
  tft.print("Daten: ");
  tft.print(dataManager.isSimulationMode() ? "Simulation" : "MQTT");
}

void ViewManager::drawButton(int x, int y, int w, int h, String label, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, 5, color);
  tft.drawRoundRect(x, y, w, h, 5, TFT_WHITE);
  
  tft.setTextColor(TFT_WHITE, color);
  tft.setTextSize(1);
  
  // Text zentrieren
  int textWidth = label.length() * 6; // Ungefähre Textbreite bei Textgröße 1
  int textX = x + (w - textWidth) / 2;
  int textY = y + h/2 - 4;
  
  tft.setCursor(textX, textY);
  tft.print(label);
}

bool ViewManager::isBackButtonTouched(int x, int y) {
  return (x >= 10 && x <= 60 && y >= 10 && y <= 40);
}

// Solar Status anzeigen
void ViewManager::drawSolarStatus() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Übersichtstabelle
  tft.setCursor(20, 60);
  tft.println("System Status Übersicht:");
  
  // Holen der aktuellen Daten vom DataManager
  SolarData& solarData = dataManager.getData();
  
  // PV Leistung
  tft.setCursor(20, 80);
  tft.print("PV Leistung:");
  tft.setCursor(200, 80);
  tft.setTextColor(TFT_GREEN, BACKGROUND);
  tft.print(solarData.pvPower);
  tft.print(" W");
  
  // Verbrauch
  tft.setCursor(20, 100);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Verbrauch:");
  tft.setCursor(200, 100);
  tft.setTextColor(TFT_RED, BACKGROUND);
  tft.print(solarData.loadPower);
  tft.print(" W");
  
  // Netzbezug/Einspeisung
  tft.setCursor(20, 120);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Netz:");
  tft.setCursor(200, 120);
  if (solarData.gridPower < 0) {
    tft.setTextColor(TFT_GREEN, BACKGROUND); // Einspeisung
    tft.print(abs(solarData.gridPower));
    tft.print(" W (Einspeisung)");
  } else {
    tft.setTextColor(TFT_RED, BACKGROUND); // Bezug
    tft.print(solarData.gridPower);
    tft.print(" W (Bezug)");
  }
  
  // Batterie
  tft.setCursor(20, 140);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Batterie:");
  tft.setCursor(200, 140);
  if (solarData.batteryPower > 0) {
    tft.setTextColor(TFT_GREEN, BACKGROUND); // Laden
    tft.print(solarData.batteryPower);
    tft.print(" W (Laden)");
  } else {
    tft.setTextColor(TFT_RED, BACKGROUND); // Entladen
    tft.print(abs(solarData.batteryPower));
    tft.print(" W (Entladen)");
  }
  
  // Autarkie
  tft.setCursor(20, 160);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Autarkie:");
  tft.setCursor(200, 160);
  tft.setTextColor(TFT_CYAN, BACKGROUND);
  tft.print(solarData.autarky);
  tft.print(" %");
  
  // Batteriestand
  tft.setCursor(20, 180);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Batterieladung:");
  tft.setCursor(200, 180);
  tft.setTextColor(TFT_YELLOW, BACKGROUND);
  tft.print(solarData.batterySOC);
  tft.print(" %");
}

// Batterie Status anzeigen
// Batterie Status anzeigen mit Zeitberechnung bis zum Ziel-SOC
void ViewManager::drawBatteryStatus() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Holen der aktuellen Daten vom DataManager
  SolarData& solarData = dataManager.getData();
  
  // Batterieparameter aus der Konfiguration laden
  JsonDocument config;
  float batteryCapacityAh = 360.0;  // Standardwert
  float batteryNomVoltage = 51.2;   // Standardwert für 16S LiFePO4
  float targetSOC = 80.0;           // Standard-Ziel-SOC
  float minSOC = 20.0;              // Standard-Mindest-SOC
  
  if (configManager.loadJsonConfig("/config.json", config)) {
    if (config.containsKey("battery")) {
      batteryCapacityAh = config["battery"]["capacity_ah"].as<float>();
      batteryNomVoltage = config["battery"]["nominal_voltage"].as<float>();
      targetSOC = config["battery"]["target_soc"].as<float>();
      minSOC = config["battery"]["min_soc"].as<float>();
    }
  }
  
  tft.setCursor(20, 60);
  tft.println("Batterie Status:");
  
  // State of Charge
  tft.setCursor(20, 80);
  tft.print("Ladezustand (SOC):");
  tft.setCursor(200, 80);
  tft.setTextColor(TFT_YELLOW, BACKGROUND);
  tft.print(solarData.batterySOC);
  tft.print(" %");
  
  // Batterie-Balken zeichnen
  int barWidth = 200;
  int barHeight = 30;
  int barX = 60;
  int barY = 100;
  
  // Rahmen
  tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  
  // Füllstand
  int fillWidth = map(solarData.batterySOC, 0, 100, 0, barWidth);
  
  // Farbverlauf je nach Ladezustand
  uint16_t barColor;
  if (solarData.batterySOC < minSOC) {
    barColor = TFT_RED;
  } else if (solarData.batterySOC < 50) {
    barColor = TFT_ORANGE;
  } else {
    barColor = TFT_GREEN;
  }
  
  tft.fillRect(barX, barY, fillWidth, barHeight, barColor);
  
  // Batterieleistung
  tft.setCursor(20, 150);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Batterieleistung:");
  tft.setCursor(200, 150);
  if (solarData.batteryPower > 0) {
    tft.setTextColor(TFT_GREEN, BACKGROUND);
    tft.print(solarData.batteryPower);
    tft.print(" W (Laden)");
  } else {
    tft.setTextColor(TFT_RED, BACKGROUND);
    tft.print(abs(solarData.batteryPower));
    tft.print(" W (Entladen)");
  }
  
  // Batteriespannung
  tft.setCursor(20, 170);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Batteriespannung:");
  tft.setCursor(200, 170);
  tft.setTextColor(TFT_CYAN, BACKGROUND);
  tft.print(solarData.batteryVoltage);
  tft.print(" V");
  
  // Berechnung der Zeit bis zum Ziel-SOC
  float socDifference = 0;
  String timeString = "";
  bool isTimeCalculable = false;
  
  // Batterie-Energieinhalt in Wh berechnen
  float batteryCapacityWh = batteryCapacityAh * batteryNomVoltage;
  
  // Aktueller Energieinhalt in Wh
  float currentEnergy = (solarData.batterySOC / 100.0) * batteryCapacityWh;
  
  if (solarData.batteryPower > 10) {  // Ladend mit signifikanter Leistung
    // Ziel: Aufladen bis zum Ziel-SOC
    // Verbleibende Energie bis zum Ziel
    float targetEnergy = (targetSOC / 100.0) * batteryCapacityWh;
    float energyToTarget = targetEnergy - currentEnergy;
    
    if (energyToTarget > 0) {
      // Stunden bis zum Ziel-SOC bei aktueller Ladeleistung
      float hoursToTarget = energyToTarget / solarData.batteryPower;
      
      if (hoursToTarget > 0 && hoursToTarget < 100) {
        int hours = (int)hoursToTarget;
        int minutes = (int)((hoursToTarget - hours) * 60);
        
        timeString = String(hours) + "h " + String(minutes) + "min";
        socDifference = targetSOC - solarData.batterySOC;
        isTimeCalculable = true;
      }
    }
  } 
  else if (solarData.batteryPower < -10) {  // Entladend mit signifikanter Leistung
    // Ziel: Verbleibende Zeit bis Mindest-SOC
    float minEnergy = (minSOC / 100.0) * batteryCapacityWh;
    float energyToMin = currentEnergy - minEnergy;
    
    if (energyToMin > 0) {
      // Stunden bis zum Minimum-SOC bei aktueller Entladeleistung
      float hoursToMin = energyToMin / abs(solarData.batteryPower);
      
      if (hoursToMin > 0 && hoursToMin < 100) {
        int hours = (int)hoursToMin;
        int minutes = (int)((hoursToMin - hours) * 60);
        
        timeString = String(hours) + "h " + String(minutes) + "min";
        socDifference = solarData.batterySOC - minSOC;
        isTimeCalculable = true;
      }
    }
  }
  
  // Zeit bis zum Ziel-SOC anzeigen
  tft.setCursor(20, 190);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  if (isTimeCalculable) {
    if (solarData.batteryPower > 0) {
      tft.print("Zeit bis " + String(targetSOC, 0) + "% SOC:");
      tft.setCursor(200, 190);
      tft.setTextColor(TFT_GREEN, BACKGROUND);
    } else {
      tft.print("Zeit bis " + String(minSOC, 0) + "% SOC:");
      tft.setCursor(200, 190);
      tft.setTextColor(TFT_RED, BACKGROUND);
    }
    tft.print(timeString);
  } else {
    if (solarData.batterySOC >= targetSOC && solarData.batteryPower > 0) {
      tft.print("Ziel-SOC erreicht");
    } else if (solarData.batterySOC <= minSOC && solarData.batteryPower < 0) {
      tft.print("Min-SOC erreicht");
    } else if (abs(solarData.batteryPower) < 10) {
      tft.print("Batterie inaktiv");
    } else {
      tft.print("Keine Zeitberechnung möglich");
    }
  }
  
  // Gespeicherte Energie
  tft.setCursor(20, 210);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.print("Gespeicherte Energie:");
  tft.setCursor(200, 210);
  tft.setTextColor(TFT_YELLOW, BACKGROUND);
  float storedkWh = currentEnergy / 1000.0;
  tft.print(storedkWh, 2);
  tft.print(" kWh");
}


// Weitere Detailansichten hier implementieren...
// Beispiel für eine weitere Ansicht
void ViewManager::drawGridStatus() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Holen der aktuellen Daten vom DataManager
  SolarData& solarData = dataManager.getData();
  
  tft.setCursor(20, 60);
  tft.println("Netzstatus:");
  
  // Aktuelle Leistung
  tft.setCursor(20, 80);
  tft.print("Aktuelle Leistung:");
  tft.setCursor(200, 80);
  if (solarData.gridPower < 0) {
    tft.setTextColor(TFT_GREEN, BACKGROUND);
    tft.print(abs(solarData.gridPower));
    tft.print(" W (Einspeisung)");
  } else {
    tft.setTextColor(TFT_RED, BACKGROUND);
    tft.print(solarData.gridPower);
    tft.print(" W (Bezug)");
  }
  
  // Visualisierung des Energieflusses
  int centerX = 160;
  int centerY = 130;
  int radius = 50;
  
  // Kreisrahmen als "Haus"
  tft.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // Energieflussrichtung mit Pfeilen darstellen
  if (solarData.gridPower < 0) {
    // Einspeisung: Pfeile vom Haus zum Netz
    tft.fillTriangle(
      centerX + radius + 20, centerY,
      centerX + radius, centerY - 10,
      centerX + radius, centerY + 10,
      TFT_GREEN
    );
    
    tft.setTextColor(TFT_GREEN, BACKGROUND);
    tft.setCursor(centerX + radius + 30, centerY - 5);
    tft.print("Netz");
  } else {
    // Bezug: Pfeile vom Netz zum Haus
    tft.fillTriangle(
      centerX - radius - 20, centerY,
      centerX - radius, centerY - 10,
      centerX - radius, centerY + 10,
      TFT_RED
    );
    
    tft.setTextColor(TFT_RED, BACKGROUND);
    tft.setCursor(centerX - radius - 60, centerY - 5);
    tft.print("Netz");
  }
  
  // Haus-Symbol in der Mitte
  tft.setTextColor(TFT_WHITE, BACKGROUND);
  tft.setCursor(centerX - 15, centerY - 5);
  tft.print("Haus");
}

// Platzhalter für weitere Ansichten
void ViewManager::drawPvPower() {
  // Implementierung ähnlich wie in V0_3_0.ino
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 70);
  tft.println("PV Leistung Ansicht wird geladen...");
}

void ViewManager::drawConsumption() {
  // Implementierung ähnlich wie in V0_3_0.ino
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 70);
  tft.println("Verbrauch Ansicht wird geladen...");
}

void ViewManager::drawAutarky() {
  // Implementierung ähnlich wie in V0_3_0.ino
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 70);
  tft.println("Autarkie Ansicht wird geladen...");
}

void ViewManager::drawDailyValues() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 70);
  tft.println("Tageswerte Ansicht wird geladen...");
}

void ViewManager::drawStatistics() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 70);
  tft.println("Statistik Ansicht wird geladen...");
}

// Steuerungsfunktionen
void ViewManager::controlHeating() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Einfache Platzhalter-Anzeige für Steuerungsfunktionen
  tft.setCursor(20, 70);
  tft.println("Steuerungsfunktion: Heizung");
  
  // ON/OFF Schaltflächen
  drawButton(60, 100, 80, 40, "EIN", TFT_GREEN);
  drawButton(180, 100, 80, 40, "AUS", TFT_RED);
  
  // Status
  tft.setCursor(20, 160);
  tft.println("Status: Inaktiv");
  
  // Weitere Informationen
  tft.setCursor(20, 180);
  tft.println("Tippen Sie auf EIN oder AUS, um das Gerät zu steuern.");
}

void ViewManager::controlPool() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Einfache Platzhalter-Anzeige für Steuerungsfunktionen
  tft.setCursor(20, 70);
  tft.println("Steuerungsfunktion: Pool");
  
  // ON/OFF Schaltflächen
  drawButton(60, 100, 80, 40, "EIN", TFT_GREEN);
  drawButton(180, 100, 80, 40, "AUS", TFT_RED);
  
  // Status
  tft.setCursor(20, 160);
  tft.println("Status: Inaktiv");
  
  // Weitere Informationen
  tft.setCursor(20, 180);
  tft.println("Tippen Sie auf EIN oder AUS, um das Gerät zu steuern.");
}

// Einstellungsfunktionen
void ViewManager::setupWifi() {
  tft.setCursor(20, 70);
  tft.println("WLAN-Einstellungen:");
  
  tft.setCursor(20, 90);
  tft.print("SSID: ");
  tft.println(WiFi.SSID());
  
  tft.setCursor(20, 110);
  tft.print("Status: ");
  tft.println(WiFi.status() == WL_CONNECTED ? "Verbunden" : "Getrennt");
  
  tft.setCursor(20, 130);
  tft.print("IP: ");
  tft.println(WiFi.localIP().toString());
  
  tft.setCursor(20, 150);
  tft.print("Signal: ");
  tft.print(WiFi.RSSI());
  tft.println(" dBm");
  
  drawButton(100, 180, 120, 30, "Neu verbinden", TFT_BLUE);
}

void ViewManager::setupMqtt() {
  tft.setCursor(20, 70);
  tft.println("MQTT-Einstellungen:");
  
  tft.setCursor(20, 90);
  tft.print("Broker: ");
  tft.println(MQTT_BROKER);
  
  tft.setCursor(20, 110);
  tft.print("Status: ");
  tft.println(mqttManager.isConnected() ? "Verbunden" : "Getrennt");
  
  tft.setCursor(20, 130);
  tft.println("Topics: ");
  tft.setCursor(30, 150);
  tft.print("battery_soc: ");
  tft.println(mqttManager.getValue("battery_soc"));
  
  drawButton(100, 180, 120, 30, "Konfigurieren", TFT_BLUE);
}

void ViewManager::setupDisplay() {
  tft.setCursor(20, 70);
  tft.println("Display-Einstellungen:");
  
  tft.setCursor(20, 90);
  tft.println("Modus: Schwarzer Hintergrund");
  
  tft.setCursor(20, 110);
  tft.println("Helligkeit: 100%");
  
  tft.setCursor(20, 130);
  tft.println("Auto-Rotation: Aus");
  
  tft.setCursor(20, 150);
  tft.println("Timeout: 10 Minuten");
  
  drawButton(60, 180, 80, 30, "Hell", TFT_YELLOW);
  drawButton(180, 180, 80, 30, "Dunkel", TFT_BLUE);
}

void ViewManager::showSystemInfo() {
  tft.setCursor(20, 70);
  tft.println("Systeminformationen:");
  
  tft.setCursor(20, 90);
  tft.println("Gerät: ESP32 Solar Monitor");
  
  tft.setCursor(20, 110);
  tft.println("Firmware: v0.4.1");
  
  tft.setCursor(20, 130);
  tft.print("CPU: ESP32 ");
  tft.print(ESP.getCpuFreqMHz());
  tft.println("MHz");
  
  tft.setCursor(20, 150);
  tft.print("Speicher: ");
  tft.print(ESP.getFreeHeap() / 1024);
  tft.println(" KB frei");
  
  tft.setCursor(20, 170);
  tft.print("Laufzeit: ");
  tft.print(millis() / 1000 / 60);
  tft.println(" Minuten");
}