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

// Globale Instanz
// ViewManager viewManager(tft, dataManager); // In C++ darf eine globale Variable nur einmal definiert werden.

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
  
  // Prüfe, ob die Funktion existiert
  auto it = viewFunctions.find(functionName);
  if (it != viewFunctions.end()) {
    // Funktion aufrufen
    ViewFunction func = it->second;
    (this->*func)();
    
    // Statusleiste zeichnen
    drawStatusBar();
    
    return true;
  } else {
    // Funktion nicht gefunden
    tft.setTextColor(TEXT_COLOR, BACKGROUND);
    tft.setTextSize(1);
    tft.setCursor(20, 70);
    tft.println("Ansicht nicht implementiert: " + functionName);
    
    drawStatusBar();
    
    return false;
  }
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
void ViewManager::drawBatteryStatus() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Holen der aktuellen Daten vom DataManager
  SolarData& solarData = dataManager.getData();
  
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
  if (solarData.batterySOC < 20) {
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
  tft.println("Firmware: v0.4.0");
  
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