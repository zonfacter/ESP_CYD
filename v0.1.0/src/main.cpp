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
    delay(2000);
    pinMode(LED_GREEN_PIN, OUTPUT);
    delay(2000);
    pinMode(LED_BLUE_PIN, OUTPUT);
    delay(2000);
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
