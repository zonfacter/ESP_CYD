/**
 * ViewManager.h - Verwaltet die verschiedenen Detailansichten
 */

#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <map>
#include <functional>
#include <ArduinoJson.h>
#include "config.h"
#include "DataManager.h"
#include "ConfigManager.h"  // Wichtig für JsonDocument und configManager

// Vorwärtsdeklaration der Klasse
class ViewManager;

// Externe Deklaration - wird erst nach der Klassendefinition verwendet
extern ViewManager viewManager;

class ViewManager {
private:
  TFT_eSPI &tft;
  DataManager &dataManager;
  
  String currentView;
  
  // Variablen für partielles Neuzeichnen
  bool isInitialDraw = true;
  SolarData lastDrawnData; // Speichert die zuletzt gezeichneten Daten
  
  // Typedef für Funktionszeiger auf Memberfunktionen
  typedef void (ViewManager::*ViewFunction)();
  typedef void (ViewManager::*UpdateFunction)();
  
  // Maps für Funktionszeiger zu Ansichten und Updates
  std::map<String, ViewFunction> viewFunctions;
  std::map<String, UpdateFunction> updateFunctions;
  
public:
  ViewManager(TFT_eSPI &tft, DataManager &dataManager);
  
  // Zeigt eine Detailansicht an (vollständiges Neuzeichnen)
  bool showView(const String &functionName);
  
  // Aktualisiert nur die Daten in der aktuellen Ansicht (partielles Neuzeichnen)
  bool updateView();
  
  // Zeichnet den Zurück-Button
  void drawBackButton();
  
  // Zeichnet die Statusleiste
  void drawStatusBar();
  
  // Hilfsfunktionen
  void drawButton(int x, int y, int w, int h, String label, uint16_t color);
  bool isBackButtonTouched(int x, int y);
  
  // Verschiedene Detailansichten und deren Update-Funktionen
  void drawSolarStatus();
  void updateSolarStatus();
  
  void drawBatteryStatus();
  void updateBatteryStatus();
  
  void drawGridStatus();
  void updateGridStatus();
  
  void drawPvPower();
  void updatePvPower();
  
  void drawConsumption();
  void updateConsumption();
  
  void drawAutarky();
  void updateAutarky();
  
  void drawDailyValues();
  void updateDailyValues();
  
  void drawStatistics();
  void updateStatistics();
  
  // Steuerungsfunktionen
  void controlHeating();
  void updateHeating();
  
  void controlPool();
  void updatePool();
  
  // Einstellungsfunktionen
  void setupWifi();
  void updateWifi();
  
  void setupMqtt();
  void updateMqtt();
  
  void setupDisplay();
  void updateDisplay();
  
  void showSystemInfo();
  void updateSystemInfo();
};

#endif // VIEW_MANAGER_H