/**
 * ViewManager.h - Verwaltet die verschiedenen Detailansichten
 */

#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <map>
#include <functional>
#include "config.h"
#include "DataManager.h"

// Vorwärtsdeklaration der Klasse
class ViewManager;

// Externe Deklaration - wird erst nach der Klassendefinition verwendet
extern ViewManager viewManager;

class ViewManager {
private:
  TFT_eSPI &tft;
  DataManager &dataManager;
  
  String currentView;
  
  // Typedef für Funktionszeiger auf Memberfunktionen
  typedef void (ViewManager::*ViewFunction)();
  
  // Map für Funktionszeiger zu Ansichten
  std::map<String, ViewFunction> viewFunctions;
  
public:
  ViewManager(TFT_eSPI &tft, DataManager &dataManager);
  
  // Zeigt eine Detailansicht an
  bool showView(const String &functionName);
  
  // Zeichnet den Zurück-Button
  void drawBackButton();
  
  // Zeichnet die Statusleiste
  void drawStatusBar();
  
  // Hilfsfunktionen
  void drawButton(int x, int y, int w, int h, String label, uint16_t color);
  bool isBackButtonTouched(int x, int y);
  
  // Verschiedene Detailansichten
  void drawSolarStatus();
  void drawBatteryStatus();
  void drawGridStatus();
  void drawPvPower();
  void drawConsumption();
  void drawAutarky();
  void drawDailyValues();
  void drawStatistics();
  
  // Steuerungsfunktionen
  void controlHeating();
  void controlPool();
  
  // Einstellungsfunktionen
  void setupWifi();
  void setupMqtt();
  void setupDisplay();
  void showSystemInfo();
};

#endif // VIEW_MANAGER_H