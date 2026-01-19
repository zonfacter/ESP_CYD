/**
 * ESP32_SolarMonitor.ino - Hauptsketch für den ESP32 Solar Monitor
 * Version 0.4.3 - Erweiterte Funktionalität und Verbesserungen
 * 
 * Dieses Projekt implementiert einen Solar-Monitor mit ESP32, der Daten von einer PV-Anlage
 * visualisiert und über WLAN und MQTT mit ioBroker kommuniziert.
 * 
 * Funktionalitäten:
 * - TFT-Display mit Touch zur Anzeige der Solarwerte
 * - Webserver für Konfiguration und Fernzugriff
 * - MQTT-Kommunikation mit ioBroker
 * - Rollladen-Steuerung anhand von Solarwerten
 * - OTA-Updates für einfache Firmware-Aktualisierung
 */
//=====================================================================
// BIBLIOTHEKEN UND INCLUDES
//=====================================================================

// System- und Hardware-Bibliotheken
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Dateisystem und Datenverarbeitung
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Netzwerk-Bibliotheken
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <DNSServer.h>

// OTA-Update
#include <ArduinoOTA.h>

// Eigene Konfigurationen und Konstanten
#include "config.h"
#include "version_constants.h"

// Manager-Klassen für verschiedene Funktionen
#include "DataManager.h"
#include "MqttManager.h"
#include "ConfigManager.h"
#include "MenuSystem.h"
#include "ViewManager.h"
#include "IoBrokerManager.h"
#include "WebServer.h"

//=====================================================================
// HARDWARE-INITIALISIERUNG
//=====================================================================

// Display Setup
TFT_eSPI tft = TFT_eSPI();

// Touchscreen SPI-Instanz - VSPI definieren
SPIClass touchSPI = SPIClass(VSPI);

// Touchscreen-Initialisierung mit XPT2046_CS und XPT2046_IRQ
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

//=====================================================================
// EXTERNE MANAGER-INSTANZEN
//=====================================================================

// Diese Manager-Instanzen sind bereits in ihren .cpp-Dateien definiert
// Daher verwenden wir hier 'extern' um auf sie zu verweisen
extern DataManager dataManager;
extern ConfigManager configManager;
extern IoBrokerManager ioBrokerManager;
extern MqttManager mqttManager;

// Diese Manager werden hier definiert (da sie nicht in eigenen .cpp-Dateien definiert sind)
MenuSystem menuSystem(tft);
ViewManager viewManager(tft, dataManager);

//=====================================================================
// GLOBALE ZUSTANDSVARIABLEN
//=====================================================================

// UI-Status
bool inDetailView = false;                  // Detailansicht aktiv?
String currentDetailFunction = "";          // Aktuelle Detailfunktion
int selectedRolladen = 6;                   // Ausgewählter Rolladen
int displayTimeout = DEFAULT_DISPLAY_TIMEOUT; // Display-Timeout in Sekunden

// Verbindungsstatus
bool wifiConnected = false;                 // WLAN-Verbindung hergestellt
bool mqttConnected = false;                 // MQTT-Verbindung hergestellt
bool ioBrokerConnected = false;             // ioBroker erreichbar
bool apModeActive = false;                  // Access Point Modus aktiv

// System-Status
bool restartFlag = false;                   // Neustart angefordert
unsigned long restartTime = 0;              // Zeitpunkt für Neustart
bool otaInProgress = false;                 // OTA-Update läuft

// Externe Variablen von anderen Modulen
extern bool webServerRunning;                // Definiert in WebServer.cpp

// Display und Benutzerinteraktion
static unsigned long lastTouchTime = 0;      // Letzte Touch-Zeit für Debounce
const unsigned long TOUCH_DEBOUNCE_MS = 150; // Touch-Entprellzeit
unsigned long lastUserInteractionTime = 0;   // Letzte Benutzeraktion
bool displaySleepMode = false;               // Display im Schlafmodus
uint8_t displayBrightness = 100;             // Standardhelligkeit

//=====================================================================
// EXTERNE FUNKTIONEN UND VARIABLEN
//=====================================================================

// Aus WebServer.cpp
extern DNSServer dnsServer;
extern const byte DNS_PORT;
extern void setupWebServerAP();
extern void startAccessPoint();
extern void handleWebServer();
extern void printSpiffsInfo();
//=====================================================================
// FUNKTIONSPROTOTYPEN
//=====================================================================

// Utility-Funktionen
bool isInBounds(int x, int y, int x1, int y1, int x2, int y2);
bool setup_wifi(const char* ssid, const char* password, int delayTime);
void setupOTA();
void checkDisplaySleep();
void wakeDisplay();
void printSystemDiagnostics();

//=====================================================================
// HAUPTFUNKTIONEN
//=====================================================================


void setup() {
  // Serielle Verbindung initialisieren
  Serial.begin(DEBUG_BAUD_RATE);
  delay(1500);
  DEBUG_PRINTLN("\n\n\n");
  DEBUG_PRINTLN("==========================================");
  DEBUG_PRINTLN(APP_VERSION_FULL);
  DEBUG_PRINTLN("Build: " APP_BUILD_DATE " " APP_BUILD_TIME);
  DEBUG_PRINTLN("==========================================");
  
  // WLAN vorbereitend initialisieren
  WiFi.persistent(false);  // Erst persistent ausschalten
  delay(500);
  WiFi.disconnect(true, true);  // Dann trennen ohne zu speichern
  WiFi.mode(WIFI_OFF);
  delay(500);
  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.setSleep(false);
  
  // SPIFFS und Konfigurationsmanager initialisieren
  if (!configManager.begin()) {
    Serial.println("SPIFFS Fehler!");
    // Wir gehen trotzdem weiter mit Standardwerten
  } else {
    // Historische Daten aus SPIFFS laden
    if (dataManager.loadHistoricalDataFromStorage()) {
      DEBUG_PRINTLN("Historische Daten erfolgreich geladen");
    } else {
      DEBUG_PRINTLN("Keine historischen Daten gefunden oder Fehler beim Laden");
    }
  }
  
  // Display initialisieren
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  // Display-Einstellungen aus Konfiguration laden
  JsonDocument displayConfig;
  if (configManager.loadJsonConfig("/config.json", displayConfig)) {
    if (displayConfig["display"].is<JsonObject>()) {
      // Invertierte Anzeige prüfen
      if (displayConfig["display"]["inverted"].is<bool>() && 
          displayConfig["display"]["inverted"].as<bool>()) {
        tft.invertDisplay(true);
      }
      
      // Display-Timeout laden
      if (displayConfig["display"]["timeout"].is<int>()) {
        displayTimeout = displayConfig["display"]["timeout"].as<int>();
      }
    }
  }
  
  // Touchscreen initialisieren
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  
  // Splashscreen anzeigen
  tft.setTextSize(1);
  tft.setCursor(40, 80);
  tft.println(APP_VERSION_FULL);
  tft.setCursor(80, 120);
  tft.println("Initialisiere...");
  
  // Logo anzeigen
  tft.fillCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 30, 40, TFT_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(40, SCREEN_HEIGHT - 60);
  tft.println("ESP32 Solar Monitor");
  delay(2000);
  
  // Bildschirm leeren
  tft.fillScreen(TFT_BLACK);
  
  // Standardwerte für WLAN setzen
  const char* ssid = DEFAULT_WIFI_SSID;
  const char* password = DEFAULT_WIFI_PASS;
  bool useDefaultWiFi = true;
  
  // WLAN-Konfiguration aus Speicher laden
  JsonDocument wifiConfig;
  if (configManager.loadJsonConfig("/config.json", wifiConfig)) {
    // Wenn gültige WLAN-Daten vorhanden sind, diese verwenden
    if (wifiConfig["wlan"].is<JsonObject>() && 
        wifiConfig["wlan"]["ssid"].is<const char*>() && 
        wifiConfig["wlan"]["password"].is<const char*>()) {
        
      ssid = wifiConfig["wlan"]["ssid"];
      password = wifiConfig["wlan"]["password"];
      
      // Prüfen, ob es nicht die Standardwerte sind
      if (strcmp(ssid, DEFAULT_WIFI_SSID) != 0 && 
          strcmp(password, DEFAULT_WIFI_PASS) != 0) {
        useDefaultWiFi = false;
      }
    }
  }
  
  // Access Point starten, wenn Standardwerte verwendet werden
  if (useDefaultWiFi) {
    DEBUG_WARNING("Standard-WLAN-Konfiguration erkannt.");
    DEBUG_INFO("Initialisiere AP-Modus...");
    
    tft.setTextSize(1);
    tft.setCursor(20, 70);
    tft.println("Starte Access Point Modus...");
    
    // AP-Modus initialisieren
    startAccessPoint();
    DEBUG_INFO("AP-Modus wurde initialisiert, apModeActive = " + String(apModeActive ? "true" : "false"));
    DEBUG_INFO("WebServer Status: " + String(webServerRunning ? "Läuft" : "Gestoppt"));

    // zusätzliche Diagnose-Funktionen ein, um den Status des SPIFFS besser zu verstehen
    printSpiffsInfo();

    // Simulationsmodus aktivieren
    dataManager.setSimulationMode(true);
  }
  else {
    // WLAN-Verbindung mit gespeicherten Daten aufbauen
    DEBUG_INFO("Starte WLAN-Verbindung mit gespeicherten Daten...");
    tft.setTextSize(1);
    tft.setCursor(20, 70);
    tft.println("Verbinde mit WLAN: ");
    tft.println(ssid);
    
    // Rolladen-Auswahl setzen
    viewManager.setSelectedRolladen(selectedRolladen);
    
    // WLAN verbinden
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      tft.print(".");
      attempts++;
    }
    
    // Verbindungsergebnis prüfen
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      tft.println("\nVerbunden!");
      tft.setCursor(20, tft.getCursorY() + 10);
      tft.print("IP: ");
      tft.println(WiFi.localIP().toString());
      
      // OTA Updates aktivieren
      ArduinoOTA.setHostname("ESP32SolarMonitor");
      ArduinoOTA.setPassword("admin");
      ArduinoOTA.begin();
      tft.println("OTA Updates aktiviert");
      
      // Webserver für Konfiguration starten
      setupWebServer();
      tft.println("Webserver gestartet");
      
      // Kurze Verzögerung vor MQTT-Verbindung
      delay(1000);
      
      // MQTT-Verbindungen initialisieren
      JsonDocument mqttConfig;
      if (configManager.loadJsonConfig("/config.json", mqttConfig)) {
        // MQTT-Hauptverbindung
        if (mqttConfig["mqtt"].is<JsonObject>()) {
          const char* mqtt_broker = mqttConfig["mqtt"]["broker"];
          int mqtt_port = mqttConfig["mqtt"]["port"];
          
          // MQTT verbinden
          mqttConnected = mqttManager.begin(mqtt_broker, mqtt_port);
          if (mqttConnected) {
            tft.println("MQTT verbunden!");
            
            // Topics laden
            if (mqttManager.loadTopicsFromConfig("/mqtt_topics.json")) {
              DEBUG_INFO("MQTT-Topics aus Datei geladen");
            } else {
              mqttManager.loadDefaultTopics();
              DEBUG_INFO("Standard-MQTT-Topics verwendet");
            }
            
            // Echtdaten-Modus aktivieren
            dataManager.setSimulationMode(false);
            
            // Callback für Datenaktualisierung
            mqttManager.onDataUpdate = []() {
              dataManager.updateFromMqtt(mqttManager);
              if (inDetailView) {
                viewManager.updateView();
              }
            };
          } else {
            tft.println("MQTT-Verbindung fehlgeschlagen");
            DEBUG_WARNING("MQTT-Verbindung fehlgeschlagen");
            dataManager.setSimulationMode(true);
          }
        }
        
        // Verzögerung zwischen MQTT-Verbindungen
        delay(1000);
        
        // ioBroker-Verbindung
        tft.println("Verbinde mit ioBroker...");
        ioBrokerConnected = ioBrokerManager.begin();
        
        if (ioBrokerConnected) {
          tft.println("ioBroker verbunden!");
          DEBUG_INFO("ioBroker erfolgreich verbunden");
        } else {
          tft.println("ioBroker-Verbindung fehlgeschlagen!");
          DEBUG_WARNING("ioBroker-Verbindung konnte nicht hergestellt werden");
        }
      }
    } else {
      // WLAN-Verbindung fehlgeschlagen
      wifiConnected = false;
      tft.println("\nWLAN-Verbindung fehlgeschlagen!");
      tft.println("Verwende Simulationsdaten...");
      DEBUG_WARNING("WLAN-Verbindung fehlgeschlagen");
      
      // Simulationsmodus beibehalten
      dataManager.setSimulationMode(true);
    }
  }
  
  // Kurze Verzögerung
  delay(2000);
  
  // Menüsystem laden
  tft.println("Lade Menüsystem...");
  if (menuSystem.loadFromJson("/menu.json")) {
    tft.println("Menü erfolgreich geladen");
  } else {
    tft.println("Fehler beim Laden des Menüs");
    DEBUG_ERROR("Menüsystem konnte nicht geladen werden");
  }
  
  // Kurze Verzögerung
  delay(1000);
  
  // Aktuelle Zeit für das Display-Timeout
  lastUserInteractionTime = millis();
  
  // Menü zeichnen
  menuSystem.drawMenu(true);
  
  // Datenaktualisierung starten
  dataManager.update();
  
  // Setup abgeschlossen
  DEBUG_PRINTLN("Setup abgeschlossen.");
  DEBUG_PRINTLN("WLAN: " + String(wifiConnected ? "Verbunden" : "Nicht verbunden"));
  DEBUG_PRINTLN("MQTT: " + String(mqttConnected ? "Verbunden" : "Nicht verbunden"));
  DEBUG_PRINTLN("ioBroker: " + String(ioBrokerConnected ? "Verbunden" : "Nicht verbunden"));
  DEBUG_PRINTLN("Simulationsmodus: " + String(dataManager.isSimulationMode() ? "Aktiv" : "Inaktiv"));
  
  // System-Diagnose
  printSystemDiagnostics();
}

void loop() {
   // Access Point Modus
  if (apModeActive) {
    // Im AP-Modus nur das Nötigste ausführen
    handleWebServer(); // Enthält jetzt DNS-Server-Handling
    
    // Touch-Events im AP-Modus nur für Display-Wake-Up
    if (!displaySleepMode && touch.tirqTouched() && touch.touched()) {
      lastUserInteractionTime = millis();
    }
    
    // Display-Timeout prüfen
    checkDisplaySleep();
    
    return; // Restliche Funktionen im AP-Modus überspringen
  }
    // Prüfen, ob ein Neustart angefordert wurde
  if (restartFlag && millis() > restartTime) {
    DEBUG_INFO("Führe angeforderten Neustart durch...");
    ESP.restart();
  }

  // OTA-Update Handling (wenn aktiviert)
  if (wifiConnected) {
    ArduinoOTA.handle();
    handleWebServer(); // Webserver-Anfragen verarbeiten    
    // Bei aktivem OTA keine anderen Aktionen durchführen
    if (otaInProgress) {
      return;
    }
    
    // MQTT-Verbindung aktualisieren - vor der Touch-Verarbeitung
    mqttManager.update();
    ioBrokerManager.update();
  } else {
    // WiFi-Reconnection Logic - versuche alle 30 Sekunden neu zu verbinden
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) {
      lastReconnectAttempt = millis();
      DEBUG_WARNING("WiFi getrennt - versuche Wiederverbindung...");
      
      if (WiFi.status() == WL_CONNECTED) {
        // Status hat sich geändert, aber Flag war falsch
        wifiConnected = true;
        DEBUG_INFO("WiFi-Verbindung wiederhergestellt!");
      } else {
        // Versuche Reconnect
        WiFi.reconnect();
      }
    }
    
    // Prüfe ob Reconnect erfolgreich war
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      DEBUG_INFO("WiFi erfolgreich wiederverbunden!");
      
      // Versuche MQTT auch wieder zu verbinden
      if (!mqttConnected) {
        JsonDocument mqttConfig;
        if (configManager.loadJsonConfig("/config.json", mqttConfig)) {
          if (mqttConfig["mqtt"].is<JsonObject>()) {
            const char* mqtt_broker = mqttConfig["mqtt"]["broker"];
            int mqtt_port = mqttConfig["mqtt"]["port"];
            mqttConnected = mqttManager.begin(mqtt_broker, mqtt_port);
          }
        }
      }
    }
  }

  // Position des Herzens (z.B. in der oberen rechten Ecke)
const int heartX = SCREEN_WIDTH - 1; // 20 Pixel vom rechten Rand
const int heartY = 1;                // 20 Pixel vom oberen Rand

// Prüfen, ob ein Heartbeat empfangen wurde
bool heartbeatActive = ioBrokerManager.getHeartbeatStatus();

// Herzschlagsymbol aktualisieren
viewManager.drawHeartbeat(heartbeatActive, heartX, heartY, 0.625); // Kleinere Skalierung (0.625 statt 8.0)

  // Display-Timeout prüfen
  checkDisplaySleep();
  
  // Datenmanager regelmäßig aktualisieren
  dataManager.update();
  
  // Prüfe auf Touch-Events (nur wenn Display aktiv)
  if (!displaySleepMode && touch.tirqTouched() && touch.touched()) {
    // Touch-Debouncing
    unsigned long currentTime = millis();
    if (currentTime - lastTouchTime < TOUCH_DEBOUNCE_MS) {
      // Touch-Event zu schnell nach dem letzten, ignorieren
      delay(10);  // Kurze Pause
      return;
    }
    lastTouchTime = currentTime;
    
    TS_Point p = touch.getPoint();
    
    // Aktualisiere Zeit des letzten Nutzer-Inputs
    lastUserInteractionTime = millis();
    
    // Wenn Display im Schlafmodus war, nur aufwecken und Touch ignorieren
    if (displaySleepMode) {
      wakeDisplay();
      delay(200); // Längere Entprellung beim Aufwecken
      return;
    }
    
    // Validierung von ungültigen Touch-Werten
    if (p.x == 8191 || p.y == 8191) {
      // Ungültige Touch-Werte, ignorieren
      delay(10);
      return;
    }
    
    // Touchpoint auf Displaykoordinaten mappen
    int x = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
    int y = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);
    
    // Prüfe auf gültige Werte innerhalb des Bildschirms
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
      DEBUG_PRINTLN("Touch außerhalb des Bildschirms, ignoriere...");
      return;
    }
    
    DEBUG_PRINT("Touch bei x=");
    DEBUG_PRINT(x);
    DEBUG_PRINT(", y=");
    DEBUG_PRINTLN(y);
    
    // Behandlung je nach Ansichtsmodus
    if (inDetailView) {
      // In Detailansicht: Prüfe auf Zurück-Button
      if (viewManager.isBackButtonTouched(x, y)) {
        inDetailView = false;
        currentDetailFunction = "";
        menuSystem.drawMenu(true);
        delay(200);
      }
      // Touch-Handling für die verschiedenen Detailansichten
      else if (currentDetailFunction == "controlLight") {
        // Prüfe auf "EIN"-Button
        if (isInBounds(x, y, 60, 100, 140, 140)) {
          // Licht einschalten über ioBroker - nur, wenn ioBroker verbunden ist
          if (ioBrokerConnected) {
            bool success = ioBrokerManager.setLight("1", true);
            
            if (success) {
              // Aktualisiere UI
              viewManager.updateLight();
            } else {
              // Fehler anzeigen
              tft.fillRect(20, 220, 280, 20, BACKGROUND);
              tft.setCursor(20, 220);
              tft.setTextColor(TFT_RED, BACKGROUND);
              tft.println("Fehler beim Schalten! Bitte erneut versuchen.");
              delay(2000);
              viewManager.updateLight();
            }
          } else {
            // Fehler anzeigen
            tft.fillRect(20, 220, 280, 20, BACKGROUND);
            tft.setCursor(20, 220);
            tft.setTextColor(TFT_RED, BACKGROUND);
            tft.println("ioBroker nicht verbunden!");
            delay(2000);
            viewManager.updateLight();
          }
        }
        // Prüfe auf "AUS"-Button
        else if (isInBounds(x, y, 180, 100, 260, 140)) {
          // Licht ausschalten über ioBroker - nur, wenn ioBroker verbunden ist
          if (ioBrokerConnected) {
            bool success = ioBrokerManager.setLight("1", false);
            
            if (success) {
              // Aktualisiere UI
              viewManager.updateLight();
            } else {
              // Fehler anzeigen
              tft.fillRect(20, 220, 280, 20, BACKGROUND);
              tft.setCursor(20, 220);
              tft.setTextColor(TFT_RED, BACKGROUND);
              tft.println("Fehler beim Schalten! Bitte erneut versuchen.");
              delay(2000);
              viewManager.updateLight();
            }
          } else {
            // Fehler anzeigen
            tft.fillRect(20, 220, 280, 20, BACKGROUND);
            tft.setCursor(20, 220);
            tft.setTextColor(TFT_RED, BACKGROUND);
            tft.println("ioBroker nicht verbunden!");
            delay(2000);
            viewManager.updateLight();
          }
        }
      }
      // Touch-Handling für die Rolladensteuerung
      else if (currentDetailFunction == "controlRolladen") {
        // nur fortfahren, wenn ioBroker verbunden ist
        if (!ioBrokerConnected) {
          tft.fillRect(20, 220, 280, 20, BACKGROUND);
          tft.setCursor(20, 220);
          tft.setTextColor(TFT_RED, BACKGROUND);
          tft.println("ioBroker nicht verbunden!");
          delay(2000);
          return;
        }
        
        // Button-Dimensionen und Koordinaten
        int buttonW = 45;
        int buttonH = 35;
        int startX = (SCREEN_WIDTH - 6 * buttonW - 5 * 5) / 2;
        
        // Prüfen auf Rolladen-Auswahl Buttons R1-R6
        for (int i = 1; i <= 6; i++) {
          int buttonX = startX + (i-1) * (buttonW + 5);
          if (isInBounds(x, y, buttonX, 80, buttonX + buttonW, 80 + buttonH)) {
            // Rolladen i wurde ausgewählt
            selectedRolladen = i;
            viewManager.setSelectedRolladen(i);
            viewManager.showView("controlRolladen"); // Ansicht neu laden
            delay(200);
            return;
          }
        }
        
        // Prüfen auf Auf/Stopp/Ab-Tasten
        if (isInBounds(x, y, 50, 120, 110, 150)) {
          // AUF-Taste gedrückt
          ioBrokerManager.moveRolladen(String(selectedRolladen), "up");
          viewManager.updateRolladen();
          delay(200);
        }
        else if (isInBounds(x, y, 130, 120, 190, 150)) {
          // STOP-Taste gedrückt
          ioBrokerManager.moveRolladen(String(selectedRolladen), "stop");
          viewManager.updateRolladen();
          delay(200);
        }
        else if (isInBounds(x, y, 210, 120, 270, 150)) {
          // AB-Taste gedrückt
          ioBrokerManager.moveRolladen(String(selectedRolladen), "down");
          viewManager.updateRolladen();
          delay(200);
        }
        
        // Prüfen auf Positions-Tasten (0%, 25%, 50%, 75%, 100%)
        int posButtonW = 40;
        int posButtonH = 25;
        int posY = 50;
        int posX = (SCREEN_WIDTH - 5 * posButtonW - 4 * 5) / 2;
        for (int i = 0; i < 5; i++) {
          int bx = posX + i * (posButtonW + 5);
          if (isInBounds(x, y, bx, posY, bx + posButtonH, posY + posButtonH)) {
            int target = i * 25; // 0%, 25%, 50%, 75%, 100%
            ioBrokerManager.setRolladenTargetPosition(String(selectedRolladen), target);
            viewManager.updateRolladen();
            delay(200);
          }
        }
      }
    } else {
      // Im Menü: An das Menüsystem weiterleiten
      menuSystem.handleTouch(x, y);
      menuSystem.drawMenu();
      
      // Prüfen, ob ein Menüpunkt ausgewählt wurde
      if (menuSystem.getSelectedMenuItem() >= 0) {
        String functionName = menuSystem.getSelectedFunction();
        if (functionName.length() > 0) {
          inDetailView = true;
          currentDetailFunction = functionName;
          // Für die erste Anzeige showView() verwenden
          viewManager.showView(functionName);
        }
        // Auswahl zurücksetzen
        menuSystem.resetSelection();
    }
  }  // <-- Klammer für if(inDetailView)
  } // <-- Diese Klammer fehlt für if(touch.touched())
  // Kleine Verzögerung
  delay(10);
}

// Hilfsfunktion: Prüft, ob ein Punkt (x,y) innerhalb eines Rechtecks (x1,y1,x2,y2) liegt
bool isInBounds(int x, int y, int x1, int y1, int x2, int y2) {
  return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

// WiFi-Verbindungsaufbau mit Statusanzeige
bool setup_wifi(const char* ssid, const char* password, int delayTime) {
  WiFi.disconnect(true);
  delay(500);
  delay(delayTime); // Eine längere Verzögerung vor dem Start der WLAN-Verbindung
  DEBUG_PRINTLN();
  DEBUG_PRINT("Verbinde mit ");
  DEBUG_PRINTLN(ssid);
  
  WiFi.disconnect(true);  // Trenne alle bestehenden Verbindungen
  delay(500);
  // Verbindungsmodus setzen
  WiFi.mode(WIFI_STA);
  delay(500);
  // Detaillierte Verbindungsinformationen
  DEBUG_PRINTLN("WLAN-Chip-Info:");
  DEBUG_PRINT("  Chip-ID: ");
  DEBUG_PRINTLN("Chip-Modell: " + String(ESP.getChipModel()));
  DEBUG_PRINT("  MAC-Adresse: ");
  DEBUG_PRINTLN(WiFi.macAddress());

  // WiFi-Verbindungsparameter
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  WiFi.begin(ssid, password);
  
  int dots = 0;
  int maxAttempts = 30; // 15 Sekunden maximale Wartezeit
  int attempt = 3;
  
  tft.setCursor(80, 160);
  tft.print("Verbinde mit WLAN: ");
  tft.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
    delay(500);
    tft.print(".");
    DEBUG_PRINT(".");
    dots++;
    attempt++;
    
    // Zeilenumbruch nach 20 Punkten
    if (dots % 20 == 0) {
      tft.println();
      tft.setCursor(80, tft.getCursorY());
      DEBUG_PRINTLN();
    }
    
    // Alle 5 Punkte den Status ausgeben
    if (dots % 5 == 0) {
      DEBUG_PRINT("Status: ");
      DEBUG_PRINTLN(WiFi.status());
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("WiFi verbunden");
    DEBUG_PRINT("IP-Adresse: ");
    DEBUG_PRINTLN(WiFi.localIP());
    return true;
  } else {
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("WiFi-Verbindung fehlgeschlagen");
    DEBUG_PRINT("Letzter Status: ");
    DEBUG_PRINTLN(WiFi.status());
    return false;
  }
}

// OTA-Update Konfiguration
void setupOTA() {
  // Hostname für OTA (mit MAC-Adresse)
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String hostname = "ESP32Solar-" + mac.substring(mac.length() - 4);
  
  // OTA-Setup
  ArduinoOTA.setHostname(hostname.c_str());
  
  // Optionales Passwort
  ArduinoOTA.setPassword("admin");
  
  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "Sketch";
    } else { // U_SPIFFS
      type = "Dateisystem";
    }
    
    DEBUG_PRINTLN("OTA Update gestartet: " + type);
    
    // Bildschirm mit Update-Information
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(40, 80);
    tft.println("OTA Update läuft...");
    tft.setCursor(40, 120);
    tft.print("Typ: ");
    tft.println(type);
    tft.setCursor(40, 160);
    tft.println("Bitte warten...");
  });
  
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN("\nOTA Update beendet");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(40, 100);
    tft.println("Update abgeschlossen!");
    tft.setCursor(40, 140);
    tft.println("Neustart erfolgt...");
    delay(2000);
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percentage = (progress / (total / 100));
    DEBUG_PRINTF("Fortschritt: %u%%\r", percentage);
    
    // Fortschrittsbalken zeichnen
    int barWidth = 240;
    int barHeight = 20;
    int barX = 40;
    int barY = 200;
    
    // Rahmen für Fortschrittsbalken (einmalig)
    static bool barInitialized = false;
    if (!barInitialized) {
      tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
      barInitialized = true;
    }
    
    // Fortschritt anzeigen
    int fillWidth = map(percentage, 0, 100, 0, barWidth - 2);
    tft.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, TFT_BLUE);
    
    // Prozentanzeige
    tft.fillRect(150, 160, 80, 20, TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(150, 160);
    tft.print(percentage);
    tft.print(" %");
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
    DEBUG_PRINT("OTA Fehler: ");
    if (error == OTA_AUTH_ERROR) {
      DEBUG_PRINTLN("Authentifizierung fehlgeschlagen");
      tft.setTextColor(TFT_RED);
      tft.setCursor(40, 230);
      tft.println("Auth Fehler!");
    } else if (error == OTA_BEGIN_ERROR) {
      DEBUG_PRINTLN("Begin fehlgeschlagen");
      tft.setTextColor(TFT_RED);
      tft.setCursor(40, 230);
      tft.println("Begin Fehler!");
    } else if (error == OTA_CONNECT_ERROR) {
      DEBUG_PRINTLN("Connect fehlgeschlagen");
      tft.setTextColor(TFT_RED);
      tft.setCursor(40, 230);
      tft.println("Connect Fehler!");
    } else if (error == OTA_RECEIVE_ERROR) {
      DEBUG_PRINTLN("Receive fehlgeschlagen");
      tft.setTextColor(TFT_RED);
      tft.setCursor(40, 230);
      tft.println("Receive Fehler!");
    } else if (error == OTA_END_ERROR) {
      DEBUG_PRINTLN("End fehlgeschlagen");
      tft.setTextColor(TFT_RED);
      tft.setCursor(40, 230);
      tft.println("End Fehler!");
    }
    
    delay(5000); // Fehlermeldung anzeigen
    // Zurück zum normalen Betrieb
    if (inDetailView) {
      viewManager.showView(currentDetailFunction);
    } else {
      menuSystem.drawMenu(true);
    }
  });
  
  ArduinoOTA.begin();
  DEBUG_PRINTLN("OTA bereit: " + hostname);
}

// Display aus dem Schlafmodus aufwecken
void wakeDisplay() {
  if (!displaySleepMode) {
    return; // Nichts zu tun, wenn bereits aktiv
  }
  
  DEBUG_PRINTLN("Wecke Display auf");
  
  // Helligkeit wiederherstellen
  // tft.setBrightness(displayBrightness); // Je nach Display-Bibliothek implementieren
  
  // Flag zurücksetzen
  displaySleepMode = false;
  
  // Letzten Benutzerinteraktionszeitpunkt aktualisieren
  lastUserInteractionTime = millis();
  
  // UI neu zeichnen
  if (inDetailView) {
    viewManager.showView(currentDetailFunction);
  } else {
    menuSystem.drawMenu(true);
  }
  
  DEBUG_PRINTLN("Display aufgeweckt");
}

// Display-Sleep Modus prüfen und aktivieren
void checkDisplaySleep() {
  // Keine Konfigurationsladung hier
  unsigned long currentTime = millis();
  unsigned long inactiveTime = (currentTime - lastUserInteractionTime) / 1000;
  
  if (!displaySleepMode && inactiveTime >= displayTimeout) {
    // Display in Schlafmodus versetzen...
  }
}

// Display aus der Konfiguration initialisieren
bool initializeDisplayFromConfig() {
  JsonDocument config;
  bool displayInverted = false;
  
  if (configManager.loadJsonConfig("/config.json", config)) {
    if (config["display"].is<JsonObject>()) {
      // Lese Display-Typ
      if (config["display"]["inverted"].is<bool>()) {
        displayInverted = config["display"]["inverted"].as<bool>();
      }
      
      // Optional: Timeout laden
      if (config["display"]["timeout"].is<int>()) {
        displayTimeout = config["display"]["timeout"].as<int>();
      }
    }
  }
  
  // Display initialisieren
  tft.init();
  tft.setRotation(1);
  
  if (displayInverted) {
    tft.invertDisplay(true);
  } else {
    tft.invertDisplay(false);
  }
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  return true;
}

void drawStartLogo() {
  // Bildschirm löschen
  tft.fillScreen(TFT_BLACK);
  
  // Konstanten für Logo-Position und Größe
  const int centerX = SCREEN_WIDTH / 2;
  const int centerY = SCREEN_HEIGHT / 2 - 30;
  const int sunRadius = 40;
  const int panelWidth = 100;
  const int panelHeight = 60;
  
  // Sonne zeichnen
  tft.fillCircle(centerX, centerY - 30, sunRadius, TFT_YELLOW);
  
  // Sonnenstrahlen
  for (int i = 0; i < 8; i++) {
    float angle = i * PI / 4.0;
    int x1 = centerX + cos(angle) * sunRadius;
    int y1 = (centerY - 30) + sin(angle) * sunRadius;
    int x2 = centerX + cos(angle) * (sunRadius + 15);
    int y2 = (centerY - 30) + sin(angle) * (sunRadius + 15);
    tft.drawLine(x1, y1, x2, y2, TFT_YELLOW);
  }
  
  // Solarpanel zeichnen
  int panelTop = centerY + 30;
  tft.fillRect(centerX - panelWidth/2, panelTop, panelWidth, panelHeight, TFT_NAVY);
  tft.drawRect(centerX - panelWidth/2, panelTop, panelWidth, panelHeight, TFT_WHITE);
  
  // Panel-Unterteilungen
  for (int i = 0; i < 3; i++) {
    int x = centerX - panelWidth/2 + (i+1) * (panelWidth/4);
    tft.drawLine(x, panelTop, x, panelTop + panelHeight, TFT_BLUE);
  }
  for (int i = 0; i < 1; i++) {
    int y = panelTop + (i+1) * (panelHeight/2);
    tft.drawLine(centerX - panelWidth/2, y, centerX + panelWidth/2, y, TFT_BLUE);
  }
  
  // Haus andeuten
  int houseWidth = 80;
  int houseHeight = 60;
  int houseX = centerX - houseWidth/2;
  int houseY = panelTop + panelHeight + 10;
  
  // Hauswände
  tft.fillRect(houseX, houseY, houseWidth, houseHeight, TFT_DARKGREY);
  
  // Dach
  tft.fillTriangle(
    houseX, houseY,
    houseX + houseWidth, houseY,
    houseX + houseWidth/2, houseY - 20,
    TFT_RED
  );
  
  // Fenster
  tft.fillRect(houseX + 15, houseY + 15, 20, 20, TFT_CYAN);
  tft.fillRect(houseX + houseWidth - 35, houseY + 15, 20, 20, TFT_CYAN);
  
  // Tür
  tft.fillRect(houseX + houseWidth/2 - 10, houseY + 30, 20, 30, TFT_BROWN);
  
  // Batteriesymbol
  int batteryWidth = 30;
  int batteryHeight = 50;
  int batteryX = houseX + houseWidth + 20;
  int batteryY = panelTop + 40;
  
  // Batterie-Körper
  tft.fillRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_DARKGREY);
  tft.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
  
  // Batterie-Nippel oben
  tft.fillRect(batteryX + (batteryWidth/4), batteryY - 5, batteryWidth/2, 5, TFT_DARKGREY);
  
  // Batterie-Ladezustand
  tft.fillRect(batteryX + 3, batteryY + 3, batteryWidth - 6, batteryHeight - 6, TFT_GREEN);
  
  // Verbindungslinien
  tft.drawLine(centerX, panelTop + panelHeight, centerX, houseY - 20, TFT_GREEN);
  tft.drawLine(centerX, houseY - 20, houseX + houseWidth/2, houseY - 20, TFT_GREEN);
  tft.drawLine(houseX + houseWidth, houseY + 20, batteryX, batteryY + 20, TFT_YELLOW);
  
  // Titeltext
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(centerX - 140, SCREEN_HEIGHT - 60);
  tft.println("ESP32 Solar Monitor");
  
  // Version
  tft.setTextSize(1);
  tft.setCursor(centerX - 30, SCREEN_HEIGHT - 30);
  tft.println(APP_VERSION);
  
  delay(2000);  // Logo 2 Sekunden anzeigen
}


void printSystemDiagnostics() {
  DEBUG_PRINTLN("=== System-Diagnose ===");
  DEBUG_PRINT("Freier Heap: ");
  DEBUG_PRINT("Heap-Info: Free=");
  DEBUG_PRINT(ESP.getFreeHeap());
  DEBUG_PRINT(" Total=");
  DEBUG_PRINTLN(ESP.getHeapSize());
  DEBUG_PRINT("WLAN-Status: ");
  DEBUG_PRINT(WiFi.status());
  DEBUG_PRINT(" (");
  // Statuscode interpretieren
  switch(WiFi.status()) {
    case WL_CONNECTED: DEBUG_PRINTLN("Verbunden)"); break;
    case WL_NO_SHIELD: DEBUG_PRINTLN("Kein WLAN-Shield)"); break;
    case WL_IDLE_STATUS: DEBUG_PRINTLN("Leerlauf)"); break;
    case WL_NO_SSID_AVAIL: DEBUG_PRINTLN("Keine SSID verfügbar)"); break;
    case WL_SCAN_COMPLETED: DEBUG_PRINTLN("Scan abgeschlossen)"); break;
    case WL_CONNECT_FAILED: DEBUG_PRINTLN("Verbindung fehlgeschlagen)"); break;
    case WL_CONNECTION_LOST: DEBUG_PRINTLN("Verbindung verloren)"); break;
    case WL_DISCONNECTED: DEBUG_PRINTLN("Getrennt)"); break;
    default: DEBUG_PRINTLN("Unbekannt)"); break;
  }
  // Weitere Diagnosen...
}