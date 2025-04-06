# ESP32 Solar Monitor

## Übersicht
Dieses Projekt ist ein umfassender Solar-Monitor für ESP32, der Daten von einem Solar Assistant MQTT-Broker empfängt und auf einem TFT-Display anzeigt. Der Monitor bietet Echtzeit-Informationen zu PV-Leistung, Batteriestatus, Netzeinspeisung und vielem mehr.

Die aktuelle Version 0.4.0 bietet ein modulares, konfigurierbares System mit einem dynamischen Menü und verschiedenen Detailansichten zur Überwachung einer Solaranlage.

## Hardware
- ESP32 Entwicklungsboard
- ILI9341 TFT-Display (320x240)
- XPT2046 Touchscreen-Controller
- MQTT-fähiger Solar Assistant (z.B. Victron GX-Geräte)

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
    bblanchon/ArduinoJson @ ^7.3.1
    knolleary/PubSubClient @ ^2.8

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

# Changelog für Solar Monitor v0.4.0

## Neue Funktionen
- Modulare Strukturierung des Codes mit spezialisierten Manager-Klassen
- JSON-basierte Konfiguration für einfachere Anpassungen
- MQTT-Verbindung zur Datenerfassung vom Solar Assistant
- Verbesserte WLAN-Verbindung mit robuster Fehlerbehandlung
- Dynamisch ladbares Menüsystem aus JSON-Konfiguration
- Detailansichten für verschiedene Solarsystem-Parameter
- Simulation-Modus für Testbetrieb ohne tatsächliche Solaranlage

## Verbesserungen
- Zuverlässigere WLAN-Verbindung durch optimierte Verbindungsroutine
- Bessere Fehlerbehandlung und Debug-Informationen
- Touchscreen-Kalibrierung und -Verarbeitung optimiert
- Dynamische Menüstruktur, die über JSON-Dateien anpassbar ist
- Klare Trennung von Daten, Anzeige und Steuerungslogik

## Fehlerbehebungen
- Behoben: Probleme mit der WLAN-Verbindung
- Behoben: Fehlerhafte JSON-String-Literale in der Konfiguration
- Behoben: Touch-Erkennung außerhalb des gültigen Bildschirmbereichs

# Dokumentation/Anleitung

## Systemarchitektur

Der ESP32 Solar Monitor verwendet eine modulare Architektur mit mehreren Manager-Klassen:

1. **ConfigManager**: Verwaltet die Konfigurationsdateien im SPIFFS-Dateisystem
2. **DataManager**: Speichert und verarbeitet die Solardaten
3. **MenuSystem**: Generiert und steuert das UI-Menüsystem
4. **MqttManager**: Kommuniziert mit dem MQTT-Broker
5. **ViewManager**: Rendert die verschiedenen Detailansichten

Diese modulare Struktur ermöglicht eine einfache Erweiterung und Wartung des Codes.

## Konfigurationsdateien

Der Solar Monitor verwendet drei JSON-Konfigurationsdateien:

1. **config.json**: Enthält grundlegende Einstellungen (WLAN, MQTT, Display)
2. **menu.json**: Definiert die Menüstruktur
3. **mqtt_topics.json**: Definiert die MQTT-Topics für Solardaten

### Beispiel config.json:
```json
{
  "wlan": {
    "ssid": "MeinWLAN",
    "password": "MeinPasswort"
  },
  "mqtt": {
    "broker": "192.168.1.100",
    "port": 1883,
    "client_id_prefix": "ESP32SolarMonitor-"
  },
  "display": {
    "brightness": 100,
    "timeout": 600,
    "theme": "dark"
  },
  "touch": {
    "min_x": 200,
    "max_x": 3700,
    "min_y": 240,
    "max_y": 3800
  },
  "simulation_mode": false,
  "update_interval": 5000
}
```

## Menüsystem: Struktur und Erweiterung

### Grundlegende Struktur
Das Menüsystem basiert auf einer JSON-Datei (`menu.json`), die die komplette Menüstruktur definiert. Die Grundstruktur sieht wie folgt aus:

```json
{
  "tabs": [
    {
      "title": "Tab-Titel",
      "items": [
        {
          "name": "Menüpunkt-Name",
          "function": "functionName",
          "icon": "icon-name"
        },
        // Weitere Menüpunkte...
      ]
    },
    // Weitere Tabs...
  ]
}
```

### Erweiterung des Menüs
Um das Menü zu erweitern oder anzupassen:

1. **Öffne die `menu.json` Datei**:
   Die Datei befindet sich im SPIFFS-Speicher und kann über den ConfigManager bearbeitet werden.

2. **Füge neue Tabs oder Menüpunkte hinzu**:
   - Für einen neuen Tab: Füge ein neues Objekt zum "tabs"-Array hinzu
   - Für einen neuen Menüpunkt: Füge ein neues Objekt zum "items"-Array eines bestehenden Tabs hinzu

3. **Definiere die Funktionalität**:
   Jeder Menüpunkt benötigt einen `function`-Parameter, der auf eine implementierte Funktion in der ViewManager-Klasse verweist.

4. **Implementiere die Ansichtsfunktion**:
   In der `ViewManager.cpp` musst du die entsprechende Funktion implementieren, die aufgerufen wird, wenn der Menüpunkt ausgewählt wird.

### Beispiel für einen neuen Menüpunkt

```json
{
  "name": "Wetterdaten",
  "function": "showWeatherData",
  "icon": "cloud"
}
```

Dann in ViewManager.cpp:

```cpp
void ViewManager::showWeatherData() {
  // Display-Hintergrund löschen
  tft.fillScreen(BACKGROUND);
  
  // Überschrift
  drawHeader("Wetterdaten");
  
  // Hier die eigentliche Anzeigefunktionalität implementieren
  tft.setCursor(20, 60);
  tft.print("Temperatur: ");
  tft.print(weatherData.temperature);
  tft.println(" °C");
  
  // Weitere Anzeigeelemente...
  
  // Zurück-Button
  drawBackButton();
}
```

## Funktionen der Grafiken/Views

### Hauptansichten

1. **Solar Status**:
   Zeigt einen Überblick über den aktuellen Status des Solarsystems mit den wichtigsten Werten wie PV-Leistung, Batteriestatus und Netzeinspeisung.

2. **Batterie Status**:
   Detaillierte Anzeige des Batteriestatus, einschließlich Ladezustand (SoC), aktuelle Ladeleistung, und Spannung.

3. **Netzstatus**:
   Zeigt den aktuellen Netzbezug oder die Einspeisung ins Netz an, mit visueller Darstellung der Richtung und Leistung.

4. **PV Leistung**:
   Detaillierte Anzeige der aktuellen PV-Leistung, mit Tages- und Gesamterträgen.

5. **Verbrauch**:
   Anzeige des aktuellen Stromverbrauchs des Hauses mit historischen Daten.

6. **Autarkie**:
   Visualisierung der aktuellen Autarkie, also wie viel des Strombedarfs durch eigene Erzeugung gedeckt wird.

7. **Tageswerte**:
   Zusammenfassung der wichtigsten Werte des aktuellen Tages: Ertrag, Verbrauch, Einspeisung, Bezug.

8. **Statistik**:
   Längerfristige statistische Daten zu Ertrag, Verbrauch, und Autarkie.

### Steuerungsansichten

Diese Ansichten sind für die zukünftige Erweiterung vorbereitet, um verschiedene Hausgeräte zu steuern:

- Heizung
- Pool
- Garten
- Licht
- Steckdosen
- Lüftung
- Rollladen
- Kameras

### Funktionen der Anzeigeelemente

1. **Header**: 
   Ein einheitlicher Header mit Titel und Zurück-Button für alle Detailansichten.

2. **Wertanzeigen**:
   Numerische und textuelle Darstellung der Daten mit passenden Einheiten.

3. **Farbkodierung**:
   - Grün: Positive Werte (z.B. Einspeisung ins Netz, Batterieladung)
   - Rot: Negative Werte (z.B. Netzbezug, Batterieentladung)
   - Gelb: Batteriestatus
   - Blau: Netzwerte

4. **Zurück-Button**:
   Ermöglicht die Navigation zurück zum Hauptmenü.

## MQTT-Integration

Der Solar Monitor verwendet MQTT, um Daten von einem Solar Assistant zu empfangen. Die Standard-MQTT-Topics sind:

- **Batterieladezustand**: `solar_assistant/total/battery_state_of_charge/state`
- **Verbrauchsleistung**: `solar_assistant/inverter_1/load_power_essential/state`
- **Netzleistung**: `solar_assistant/inverter_1/grid_power/state`
- **PV-Leistung**: `solar_assistant/inverter_1/pv_power/state`
- **Batterieleistung**: `solar_assistant/total/battery_power/state`
- **Batteriespannung**: `solar_assistant/inverter_1/battery_voltage/state`
- **Tagesertrag**: `solar_assistant/inverter_1/energy_day/state`
- **Gesamtertrag**: `solar_assistant/inverter_1/energy_total/state`

Diese können in der `mqtt_topics.json` angepasst werden.

## Wie alles zusammenarbeitet

1. **Startvorgang**:
   - SPIFFS-Initialisierung
   - Laden der Konfigurationen
   - WLAN-Verbindung
   - MQTT-Verbindung
   - Menü-Initialisierung

2. **Normaler Betrieb**:
   - MqttManager empfängt regelmäßig neue Daten
   - DataManager aktualisiert die Werte
   - Bei Touch-Events verarbeitet das MenuSystem die Interaktion
   - ViewManager zeigt die gewählte Detailansicht an

3. **Fehlerbehandlung**:
   - Bei WLAN- oder MQTT-Fehlern schaltet das System in den Simulationsmodus
   - Automatische Wiederverbindungsversuche

## Erweiterungsmöglichkeiten

1. **Neue Datenquellen**:
   Du kannst weitere MQTT-Topics in der `mqtt_topics.json` hinzufügen, um mehr Daten aus deinem Solar Assistant zu erfassen.

2. **Neue Visualisierungen**:
   Implementiere neue Ansichtsfunktionen in ViewManager für andere Darstellungen wie Graphen oder Diagramme.

3. **Steuerungsfunktionen**:
   Füge MQTT-Publish-Funktionalität hinzu, um Steuerungsbefehle an dein Hausautomationssystem zu senden.

4. **Datenlogging**:
   Implementiere eine Funktion zum Speichern historischer Daten auf einer SD-Karte oder im SPIFFS.

5. **Webinterface**:
   Füge einen Webserver hinzu, um die Konfiguration über einen Browser zu ermöglichen.

## Tipps für die Fehlersuche

1. **Serieller Monitor**:
   Die DEBUG-Ausgaben helfen bei der Diagnose von Verbindungs- und Datenproblemen.

2. **MQTT-Tests**:
   Nutze einen MQTT-Client wie MQTT Explorer, um zu prüfen, ob die erwarteten Daten vom Broker ankommen.

3. **JSON-Validierung**:
   Überprüfe deine JSON-Konfigurationen mit einem Online-Validator, bevor du sie auf das Gerät lädst.

4. **WiFi-Verbindung**:
   Bei Verbindungsproblemen die RSSI-Werte überprüfen und gegebenenfalls den ESP32 näher am Router platzieren.

## Bekannte Probleme
- Die WLAN-Verbindung kann unter bestimmten Umständen instabil sein; die aktuelle Version enthält robustere Verbindungsroutinen.
- Bei einigen Routern kann die Verbindungszeit länger als erwartet sein.

## Nächste Schritte und geplante Features
- Graphische Darstellung der Daten über Zeiträume
- Statistische Auswertungen der Solardaten
- Integration weiterer Sensoren (z.B. Temperatur, Wetter)
- Steuerung von Hausautomation basierend auf Solarertrag
- Over-the-Air Updates

## Abhängigkeiten
- TFT_eSPI: Display-Ansteuerung
- XPT2046_Touchscreen: Touchscreen-Ansteuerung
- ArduinoJson: JSON-Verarbeitung
- PubSubClient: MQTT-Kommunikation

## Lizenz
Frei für den persönlichen und kommerziellen Gebrauch.
