# ESP32 Display & Touch Test

## Übersicht
Dieses Projekt demonstriert die Verwendung eines ESP32-Controllers mit einem TFT-Display und Touchscreen. Das Projekt zeigt, wie man beide Komponenten korrekt initialisiert und verwendet, sowie die RGB-LED zur visuellen Rückmeldung ansteuert.

## Hardware
- ESP32 Entwicklungsboard
- ILI9341 TFT-Display (320x240)
- XPT2046 Touchscreen-Controller
- RGB-LED (gemeinsame Anode)

## Pin-Belegung

### Display
- TFT_MISO: 12
- TFT_MOSI: 13
- TFT_SCLK: 14
- TFT_CS: 15
- TFT_DC: 2
- TFT_RST: 12
- TFT_BL: 21 (Hintergrundbeleuchtung)

### Touchscreen
- TOUCH_IRQ: 36
- TOUCH_MOSI: 32
- TOUCH_MISO: 39
- TOUCH_CLK: 25
- TOUCH_CS: 33

### RGB-LED
- RED: GPIO 4
- GREEN: GPIO 16
- BLUE: GPIO 17
- Anmerkung: Die LED hat eine gemeinsame Anode, daher ist LOW = AN und HIGH = AUS

## Konfiguration

### platformio.ini
```ini
[env:esp32dev]
platform = espressif32@^4.4.0
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = COMx  ; Anpassen an den eigenen COM-Port
upload_speed = 921600

lib_deps =
    bodmer/TFT_eSPI @ ^2.5.43
    PaulStoffregen/XPT2046_Touchscreen

build_flags =
    -D USER_SETUP_LOADED=1
    -D LOAD_GLCD=1
    -D LOAD_FONT2=1
    -D ILI9341_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_BL=21
    -D TFT_MISO=12
    -D TFT_MOSI=13
    -D TFT_SCLK=14
    -D TFT_CS=15
    -D TFT_DC=2
    -D TFT_RST=12
    -D TOUCH_CS=33
```

## Vollständiger Code

```cpp
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// RGB LED pins
#define LED_RED_PIN 4
#define LED_GREEN_PIN 16
#define LED_BLUE_PIN 17

// Hintergrundbeleuchtung
#define TFT_BL 21

// Eine separate SPI-Instanz für den Touchscreen
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// Kalibrierungswerte für den Touchscreen
#define TOUCH_MIN_X 200
#define TOUCH_MAX_X 3700
#define TOUCH_MIN_Y 240
#define TOUCH_MAX_Y 3800

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Status-Tracking für Touch
bool wasTouch = false;
unsigned long lastTouchTime = 0;

// LED-Farben anhand der beobachteten Werte
void setLEDRed() {
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_BLUE_PIN, HIGH);
}

void setLEDCyan() {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_BLUE_PIN, LOW);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Touch-Test mit korrigierten LED-Farben");
    
    // LEDs initialisieren
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    
    // LED Standby-Farbe (Rot)
    setLEDRed();
    
    // Hintergrundbeleuchtung aktivieren
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    // Display initialisieren
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // Touchscreen-SPI initialisieren
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    
    // UI zeichnen
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(20, 20);
    tft.println("Touch-Test");
    
    tft.setTextSize(1);
    tft.setCursor(20, 60);
    tft.println("Beruehren und ziehen Sie auf dem Display");
    tft.setCursor(20, 80);
    tft.println("LED: Rot=Bereit, Cyan=Touch");
    
    tft.drawRect(10, 100, 300, 130, TFT_BLUE);
    
    Serial.println("Setup abgeschlossen, warte auf Touch-Events");
}

void loop() {
    bool currentlyTouched = touch.tirqTouched();
    
    // Neue Berührung erkannt
    if (currentlyTouched && !wasTouch) {
        wasTouch = true;
        lastTouchTime = millis();
        
        // LED auf Cyan setzen bei Berührung
        setLEDCyan();
        
        // Touchscreen abfragen
        TS_Point p = touch.getPoint();
        
        // Rohdaten ausgeben
        Serial.print("Touch-Start: X=");
        Serial.print(p.x);
        Serial.print(", Y=");
        Serial.print(p.y);
        Serial.print(", Z=");
        Serial.println(p.z);
        
        // Koordinaten auf Display-Größe umrechnen
        int displayX = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
        int displayY = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);
        
        // Letzten Wert löschen
        tft.fillRect(20, 130, 280, 90, TFT_BLACK);
        
        // Punkt zeichnen
        tft.fillCircle(displayX, displayY, 5, TFT_RED);
        
        // Koordinaten anzeigen
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(20, 140);
        tft.print("X: ");
        tft.print(displayX);
        tft.print(", Y: ");
        tft.print(displayY);
        
        tft.setCursor(20, 160);
        tft.print("Rohwerte - X: ");
        tft.print(p.x);
        tft.print(", Y: ");
        tft.print(p.y);
        
        tft.setCursor(20, 180);
        tft.setTextColor(TFT_CYAN);
        tft.print("Touch aktiv - LED Cyan");
    }
    // Touch weiterhin gedrückt
    else if (currentlyTouched && wasTouch) {
        // Alle 100ms Touch-Position aktualisieren
        if (millis() - lastTouchTime > 100) {
            lastTouchTime = millis();
            
            TS_Point p = touch.getPoint();
            
            // Koordinaten auf Display-Größe umrechnen
            int displayX = map(p.x, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
            int displayY = map(p.y, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);
            
            // Nur den Punkt aktualisieren, ohne alles zu löschen
            tft.fillCircle(displayX, displayY, 5, TFT_YELLOW);
        }
    }
    // Touch wurde losgelassen
    else if (!currentlyTouched && wasTouch) {
        wasTouch = false;
        Serial.println("Touch beendet");
        
        // LED zurück auf Rot
        setLEDRed();
        
        tft.fillRect(20, 180, 280, 20, TFT_BLACK);
        tft.setCursor(20, 180);
        tft.setTextColor(TFT_RED);
        tft.print("Touch beendet - LED Rot");
        
        // Kurze Pause
        delay(50);
    }
    
    // Systemstatus anzeigen
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        
        // Zeit in der Ecke anzeigen
        tft.fillRect(SCREEN_WIDTH - 50, 5, 45, 10, TFT_BLACK);
        tft.setCursor(SCREEN_WIDTH - 50, 5);
        tft.setTextColor(TFT_WHITE);
        tft.print(millis()/1000);
        tft.print("s");
    }
    
    // Kleine Pause
    delay(10);
}
```

## Funktionalitäten

- **Display-Initialisierung**: Konfiguriert das TFT-Display mit den richtigen Pins
- **Touchscreen-Steuerung**: Verwendet eine separate SPI-Instanz für den XPT2046 Touchscreen
- **Touch-Erkennung**: Erkennt kontinuierlich Berührungen und zeigt die Koordinaten an
- **LED-Feedback**: Wechselt die RGB-LED-Farbe zwischen Rot (Standby) und Cyan (Touch)
- **Visuelle Rückmeldung**: Zeichnet Punkte auf dem Display, wo der Touchscreen berührt wird

## Verwendung

Nach dem Hochladen des Codes:
1. Das Display zeigt einen Startbildschirm mit Anweisungen
2. Berühren Sie das Display, um die Touch-Erkennung zu testen
3. Die LED wechselt die Farbe bei Berührung (rot → cyan)
4. Die Touch-Koordinaten werden auf dem Display angezeigt
5. Ziehen Sie über das Display, um einen Pfad zu zeichnen

## Changelog

### v0.1.0 (Aktuelle Version)
- Initiale Implementierung des Display- und Touch-Tests
- Korrekte Konfiguration der TFT_eSPI-Bibliothek
- Implementierung einer separaten SPI-Instanz für den Touchscreen
- Kalibrierung des Touchscreens
- Korrektur der LED-Pin-Belegung und der Ansteuerungslogik
- Kontinuierliche Touch-Erkennung mit visueller Rückmeldung

## Bekannte Probleme
- Keine bekannten Probleme im aktuellen Stand

## Nächste Schritte
- WLAN-Konnektivität implementieren
- MQTT-Client hinzufügen
- Menüsystem mit Schaltflächen erstellen
- Sensorwerte anzeigen und visualisieren

## Abhängigkeiten
- TFT_eSPI: Display-Ansteuerung
- XPT2046_Touchscreen: Touchscreen-Ansteuerung

## Lizenz
Frei für den persönlichen und kommerziellen Gebrauch.
