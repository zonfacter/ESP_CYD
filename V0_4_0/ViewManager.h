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
#include "IoBrokerManager.h"

// Vorwärtsdeklaration der Klasse
class ViewManager;

// Externe Deklaration - wird erst nach der Klassendefinition verwendet
extern ViewManager viewManager;

class ViewManager {
private:
  TFT_eSPI &tft;
  DataManager &dataManager;
  
  String currentView;

  void drawHeart(int centerX, int centerY, float scale, uint16_t color);
  float heartAnimationParam = 0.0; // Parameter für die Herzanimation
  unsigned long lastHeartbeatTime = 0; // Zeit des letzten Herzschlags

  // Variablen für partielles Neuzeichnen
  bool isInitialDraw = true;
  SolarData lastDrawnData; // Speichert die zuletzt gezeichneten Daten
  

  int selectedRolladen = 1; // Standard: Rolladen 1 ist ausgewählt
  // Typedef für Funktionszeiger auf Memberfunktionen
  typedef void (ViewManager::*ViewFunction)();
  typedef void (ViewManager::*UpdateFunction)();
  
  // Maps für Funktionszeiger zu Ansichten und Updates
  std::map<String, ViewFunction> viewFunctions;
  std::map<String, UpdateFunction> updateFunctions;
  
public:
  ViewManager(TFT_eSPI &tft, DataManager &dataManager);
    // Getter und Setter für selectedRolladen
  int getSelectedRolladen() const { return selectedRolladen; }
  void setSelectedRolladen(int newRolladen) { selectedRolladen = newRolladen; }
  // Zeigt eine Detailansicht an (vollständiges Neuzeichnen)
  bool showView(const String &functionName);
  
  // Aktualisiert nur die Daten in der aktuellen Ansicht (partielles Neuzeichnen)
  bool updateView();
  
  // Zeichnet den Zurück-Button
  void drawBackButton();
  
  // Zeichnet die Statusleiste
  void drawStatusBar();

  // ioBroker Hearbeat Symbol
   void drawHeartbeat(bool active, int centerX, int centerY, float scale = 0.625);

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
  
  void controlLight();
  void updateLight();

  void controlPool();
  void updatePool();
  
  void controlRolladen();
  void updateRolladen();

 
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