/**
 * ESP32_SolarMonitor.ino - Hauptsketch für den ESP32 Solar Monitor
 * Version 0.4.1 - Modulare Struktur mit JSON-Konfiguration
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "DataManager.h"
#include "MqttManager.h"
#include "ConfigManager.h"
#include "MenuSystem.h"
#include "ViewManager.h"

// Display Setup
TFT_eSPI tft = TFT_eSPI();

// Touchscreen SPI-Instanz
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// Instanzen der Manager-Klassen
MenuSystem menuSystem(tft);
ViewManager viewManager(tft, dataManager);

// Statusvariablen
bool inDetailView = false;
String currentDetailFunction = "";

// Hilfsfunktionen
bool isInBounds(int x, int y, int x1, int y1, int x2, int y2);

void setup() {
  // Serielle Verbindung initialisieren
  Serial.begin(DEBUG_BAUD_RATE);
  delay(1000);
  DEBUG_PRINTLN("ESP32 Solar Monitor - Version 0.4.1");
  
  // Random-Initialisierung für MQTT-Client-ID
  randomSeed(analogRead(0));
  
  // Display initialisieren
  tft.init();
  tft.setRotation(1); // Landscape
  tft.fillScreen(BACKGROUND);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  // Touchscreen initialisieren
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  
  // Splashscreen anzeigen
  tft.setTextSize(2);
  tft.setCursor(40, 80);
  tft.println("ESP32 Solar Monitor v0.4.1");
  tft.setCursor(80, 120);
  tft.println("Initialisiere...");
  
  // SPIFFS und Konfigurationsmanager initialisieren
  if (!configManager.begin()) {
    tft.setTextColor(TFT_RED, BACKGROUND);
    tft.setCursor(40, 160);
    tft.println("SPIFFS Fehler!");
    delay(3000);
  }
  
  // Versuche, Konfiguration aus SPIFFS zu laden
  JsonDocument config;
  if (configManager.loadJsonConfig("/config.json", config)) {
    // WLAN-Konfiguration laden
    const char* ssid = config["wlan"]["ssid"];
    const char* password = config["wlan"]["password"];
    
    // WLAN-Verbindung aufbauen
    DEBUG_PRINTLN("Starte WLAN-Verbindung...");
    DEBUG_PRINT("SSID: ");
    DEBUG_PRINTLN(ssid);
    DEBUG_PRINT("Passwort: ");
    DEBUG_PRINTLN(password);
    //WiFi.begin(ssid, password);
    setup_wifi(ssid, password);
    DEBUG_PRINT("WLAN-Status nach Verbindungsversuch: ");
    DEBUG_PRINTLN(WiFi.status());

    // Warte auf WLAN-Verbindung mit Statusanzeige
    tft.setTextSize(1);
    tft.setTextColor(TEXT_COLOR, BACKGROUND);
    tft.setCursor(80, 160);
    tft.print("Verbinde mit WLAN: ");
    tft.println(ssid);
    
    int dots = 0;
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 60000) { // Max. 20 Sekunden warten
      delay(500);
      tft.print(".");
      dots++;
      // Innerhalb der while-Schleife
      if (dots % 5 == 0) {
        DEBUG_PRINT("Status: ");
        DEBUG_PRINTLN(WiFi.status());
      }
      // Zeilenumbruch nach 20 Punkten
      if (dots % 20 == 0) {
        tft.println();
        tft.setCursor(80, tft.getCursorY());
      }
    }
    
    if (WiFi.status() == WL_CONNECTED) {

      DEBUG_PRINTLN("WLAN-Verbindung Timeout erreicht!");
      DEBUG_PRINT("Letzter Status: ");
      DEBUG_PRINTLN(WiFi.status());
      // Status-Codes: 0=IDLE, 1=NO_SSID_AVAIL, 3=CONNECTED, 4=CONNECT_FAILED, 6=DISCONNECTED
      
      DEBUG_PRINT("RSSI: ");
      DEBUG_PRINTLN(WiFi.RSSI()); // Signalstärke, falls verfügbar
      tft.println("\nVerbunden!");
      tft.setCursor(80, tft.getCursorY() + 10);
      tft.print("IP: ");
      tft.println(WiFi.localIP().toString());
      
      // MQTT-Konfiguration laden und initialisieren
      const char* mqtt_broker = config["mqtt"]["broker"];
      int mqtt_port = config["mqtt"]["port"];
      
      if (mqttManager.begin(mqtt_broker, mqtt_port)) {
        tft.println("MQTT verbunden!");
        
        // MQTT-Topics aus Konfigurationsdatei laden
        if (mqttManager.loadTopicsFromConfig("/mqtt_topics.json")) {
          tft.println("MQTT-Topics geladen");
        } else {
          tft.println("Standard-MQTT-Topics verwendet");
          mqttManager.loadDefaultTopics();
        }
        
        // Simulationsmodus ausschalten, da wir echte Daten haben
        dataManager.setSimulationMode(false);
        
        // Callback für Datenaktualisierung
        mqttManager.onDataUpdate = []() {
          dataManager.updateFromMqtt(mqttManager);
          
          // Wenn wir in einer Detailansicht sind, aktualisieren
          if (inDetailView) {
            viewManager.updateView(); // Verwende die neue updateView-Methode für partielles Neuzeichnen
          }
        };
        
      } else {
        tft.println("MQTT-Verbindung fehlgeschlagen");
        tft.println("Verwende Simulationsdaten...");
        
        // Simulationsmodus beibehalten
        dataManager.setSimulationMode(true);
      }
    } else {
      tft.println("\nWLAN-Verbindung fehlgeschlagen!");
      tft.println("Verwende Simulationsdaten...");
      
      // Simulationsmodus beibehalten
      dataManager.setSimulationMode(true);
    }
  } else {
    // Keine Konfiguration gefunden
    tft.setTextColor(TFT_YELLOW, BACKGROUND);
    tft.setCursor(40, 160);
    tft.println("Keine Konfiguration gefunden!");
    tft.println("Verwende Standardwerte...");
    
    // WLAN-Verbindung mit Standard-Werten aufbauen
    WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
    DEBUG_PRINT("WLAN-Status: ");
    DEBUG_PRINTLN(WiFi.status());
    // Simulationsmodus beibehalten
    dataManager.setSimulationMode(true);
  }
  
  delay(2000); // Kurz anzeigen
  
  // Menüsystem aus JSON laden
  if (menuSystem.loadFromJson("/menu.json")) {
    tft.setCursor(80, tft.getCursorY() + 10);
    tft.println("Menü geladen!");
  } else {
    tft.setCursor(80, tft.getCursorY() + 10);
    tft.println("Fehler beim Laden des Menüs!");
  }
  
  delay(1000);
  
  // Menü zeichnen
  menuSystem.drawMenu(true);
  
  // Simuliere Datenaktualisierung falls nötig
  dataManager.update();
  // Am Ende der Setup-Funktion:
  // Callback für Datenaktualisierung einfügen (falls er an anderer Stelle steht)
  mqttManager.onDataUpdate = []() {
    dataManager.updateFromMqtt(mqttManager);
    
    // Wenn wir in einer Detailansicht sind, aktualisieren
    if (inDetailView) {
      viewManager.updateView(); // Partielles Neuzeichnen
    }
  };
}

void loop() {
  // MQTT-Verbindung prüfen und aktualisieren
  mqttManager.update();
  
  // Datenmanager regelmäßig aktualisieren
  dataManager.update();
  
  // Prüfe auf Touch-Events
  if (touch.tirqTouched() && touch.touched()) {
    TS_Point p = touch.getPoint();
    
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
      // Weitere Touch-Handling in der Detailansicht könnte hier implementiert werden
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
    }
  }
  
  // Kleine Verzögerung
  delay(10);
}

// Hilfsfunktion: Prüft, ob ein Punkt (x,y) innerhalb eines Rechtecks (x1,y1,x2,y2) liegt
bool isInBounds(int x, int y, int x1, int y1, int x2, int y2) {
  return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

void setup_wifi(const char* ssid, const char* password) {
  delay(10);
  DEBUG_PRINTLN();
  DEBUG_PRINT("Verbinde mit ");
  DEBUG_PRINTLN(ssid);
  
  WiFi.disconnect(true);  // Trenne alle bestehenden Verbindungen
  delay(500);
  WiFi.mode(WIFI_STA);
  delay(500);
  
  WiFi.begin(ssid, password);
  
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    DEBUG_PRINT(".");
    dots++;
    
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
  
  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi verbunden");
  DEBUG_PRINT("IP-Adresse: ");
  DEBUG_PRINTLN(WiFi.localIP());
}