# ESP32 Solar Monitor v0.4.3
## Vollständige Bedienungsanleitung

## Inhaltsverzeichnis
1. [Überblick](#überblick)
2. [Hardware & Pin-Belegung](#hardware--pin-belegung)
3. [Installation](#installation)
4. [Systemarchitektur](#systemarchitektur)
5. [Erste Inbetriebnahme](#erste-inbetriebnahme)
6. [Konfigurationsdateien](#konfigurationsdateien)
7. [Menüstruktur](#menüstruktur)
8. [Einstellungen](#einstellungen)
9. [Datenansichten](#datenansichten)
10. [Steuerungsfunktionen](#steuerungsfunktionen)
11. [Heartbeat-Funktionalität](#heartbeat-funktionalität)
12. [Rollladen-Steuerung](#rollladen-steuerung)
13. [MQTT-Integration](#mqtt-integration)
14. [ioBroker-Integration](#iobroker-integration)
15. [Webserver & Fernsteuerung](#webserver--fernsteuerung)
16. [Fehlersuche](#fehlersuche)
17. [Anhang: Erweiterungsmöglichkeiten](#anhang-erweiterungsmöglichkeiten)

---

## Überblick

Der ESP32 Solar Monitor ist eine umfassende Lösung zur Visualisierung und Überwachung von Solarsystemen. Diese Version 0.4.3 bietet eine modulare, konfigurierbare Plattform mit einem responsiven Touch-Interface und verschiedenen Detailansichten zur Überwachung aller wichtigen Parameter einer Solaranlage.

**Hauptfunktionen:**
- Echtzeit-Anzeige von PV-Leistung, Batteriestatus, Netzeinspeisung und Verbrauch
- Berechnung der verbleibenden Zeit bis zum Erreichen bestimmter Batterieladestände
- Benutzerfreundliches Touch-Interface mit Tabs und scrollbarem Menü
- Konfigurierbare MQTT-Verbindung zu Solar Assistant oder anderen Monitoring-Systemen
- Simulationsmodus für Demonstrationszwecke ohne tatsächliche Solaranlage
- **NEU**: Heartbeat-Funktionalität zur Überwachung der ioBroker-Verbindung
- **NEU**: Erweiterte Rollladen-Steuerung für bis zu 6 Rollläden
- **NEU**: Webserver für Fernzugriff und -steuerung
- **NEU**: Optimierte Benutzeroberfläche mit partiellen Updates

**Technische Spezifikationen:**
- Hardware: ESP32 mit 2,8" TFT-Display (ILI9341, 320x240) und XPT2046 Touchscreen
- WLAN-Konnektivität für drahtlose Integration
- MQTT-Protokoll zur Datenkommunikation
- SPIFFS-Dateisystem für Konfigurationsdateien
- Freier HEAP-Speicher: ca. 218 KB

## Hardware & Pin-Belegung

### Benötigte Hardware
- ESP32 Entwicklungsboard
- ILI9341 TFT-Display (320x240)
- XPT2046 Touchscreen-Controller
- Micro-USB-Kabel für Programmierung und Stromversorgung
- MQTT-fähiger Solar Assistant (z.B. Victron GX-Geräte) oder ioBroker

### Pin-Belegung

#### Display
- TFT_MISO: 12
- TFT_MOSI: 13
- TFT_SCLK: 14
- TFT_CS: 15
- TFT_DC: 2
- TFT_RST: 12
- TFT_BL: 21 (Hintergrundbeleuchtung)

#### Touchscreen
- TOUCH_IRQ: 36
- TOUCH_MOSI: 32
- TOUCH_MISO: 39
- TOUCH_CLK: 25
- TOUCH_CS: 33

## Systemarchitektur

Der ESP32 Solar Monitor verwendet eine modulare Architektur mit mehreren Manager-Klassen:

1. **ConfigManager**: Verwaltet die Konfigurationsdateien im SPIFFS-Dateisystem
2. **DataManager**: Speichert und verarbeitet die Solardaten
3. **MenuSystem**: Generiert und steuert das UI-Menüsystem
4. **MqttManager**: Kommuniziert mit dem MQTT-Broker
5. **ViewManager**: Rendert die verschiedenen Detailansichten
6. **IoBrokerManager**: Kommuniziert mit ioBroker für Steuerungsfunktionen
7. **WebServer**: Bietet eine Weboberfläche für Fernzugriff und -konfiguration

Diese modulare Struktur ermöglicht eine einfache Erweiterung und Wartung des Codes.

---

## Installation

### Software-Installation

#### Arduino IDE-Methode
1. **Arduino IDE vorbereiten:**
   - Arduino IDE (Version 1.8.19 oder höher) installieren
   - ESP32-Boardunterstützung über den Boardverwalter hinzufügen
   - Folgende Bibliotheken installieren:
     - TFT_eSPI (Version 2.5.43 oder höher)
     - XPT2046_Touchscreen
     - ArduinoJson (Version 7.0.0 oder höher)
     - PubSubClient
     - SPIFFS
     - AsyncTCP und ESPAsyncWebServer (für Webserver-Funktionalität)

2. **TFT_eSPI konfigurieren:**
   - In der User_Setup.h der TFT_eSPI-Bibliothek die Pin-Belegung anpassen:
     ```cpp
     #define TFT_MISO 12
     #define TFT_MOSI 13
     #define TFT_SCLK 14
     #define TFT_CS   15
     #define TFT_DC   2
     #define TFT_RST  12
     
     #define TOUCH_CS 33
     ```

3. **Firmware hochladen:**
   - Das Projekt in der Arduino IDE öffnen
   - ESP32 über USB anschließen
   - Den korrekten Port und das Board (ESP32) auswählen
   - "Hochladen" klicken, um die Firmware zu übertragen

4. **Dateisystem vorbereiten:**
   - In der Arduino IDE "ESP32 Sketch Data Upload" Tool verwenden
   - Dieses Tool lädt die Konfigurationsdateien (JSON) in den SPIFFS-Speicher des ESP32

#### PlatformIO-Methode
Alternativ kann das Projekt auch mit PlatformIO installiert werden:

1. **PlatformIO installieren:**
   - PlatformIO als Erweiterung für Visual Studio Code installieren

2. **Projekt konfigurieren:**
   - Folgende platformio.ini verwenden:
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
         me-no-dev/ESP Async WebServer
         me-no-dev/AsyncTCP

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

3. **Firmware hochladen:**
   - "Build" und "Upload" über PlatformIO ausführen

4. **Dateisystem hochladen:**
   - "Upload Filesystem Image" über PlatformIO ausführen

---

## Konfigurationsdateien

Der Solar Monitor verwendet drei JSON-Konfigurationsdateien, die im SPIFFS-Speicher gespeichert sind:

1. **config.json**: Enthält grundlegende Einstellungen für WLAN, MQTT, Display und Batterie
2. **menu.json**: Definiert die komplette Menüstruktur mit Tabs und Menüpunkten
3. **mqtt_topics.json**: Konfiguriert die MQTT-Topics für die Datenabfrage

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
    "type": "micro_usb",
    "inverted": false,
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
  "battery": {
    "capacity_ah": 360,
    "nominal_voltage": 51.2,
    "target_soc": 80,
    "min_soc": 20
  },
  "simulation_mode": false,
  "update_interval": 5000
}
```

### Beispiel menu.json:
```json
{
  "tabs": [
    {
      "title": "System",
      "items": [
        {
          "name": "Solar Status",
          "function": "drawSolarStatus",
          "icon": "sun"
        },
        {
          "name": "Batterie Status",
          "function": "drawBatteryStatus",
          "icon": "battery"
        },
        ...
      ]
    },
    {
      "title": "Steuerung",
      "items": [
        {
          "name": "Heizung",
          "function": "controlHeating",
          "icon": "heat"
        },
        ...
      ]
    },
    ...
  ]
}
```

### Beispiel mqtt_topics.json:
```json
{
  "topics": [
    {
      "name": "battery_soc",
      "topic": "solar_assistant/total/battery_state_of_charge/state",
      "description": "Batterieladezustand in Prozent",
      "unit": "%",
      "color": "TFT_YELLOW"
    },
    {
      "name": "load_power",
      "topic": "solar_assistant/inverter_1/load_power_essential/state",
      "description": "Verbrauchsleistung",
      "unit": "W",
      "color": "TFT_RED"
    },
    ...
  ]
}
```

## Erste Inbetriebnahme

1. **Konfigurationsdateien anpassen:**
   - Verbinden Sie das Gerät mit dem PC und öffnen Sie die Arduino IDE
   - Laden Sie die Konfigurationsdateien in den "data"-Ordner des Projekts
   - Passen Sie die Dateien an Ihre Umgebung an
   - Laden Sie die Dateien mit dem "ESP32 Sketch Data Upload" Tool hoch
   - Alternativ können Sie die Dateien später über die Weboberfläche bearbeiten

2. **Gerät starten:**
   - Nach dem Einschalten verbindet sich der Solar Monitor automatisch mit dem konfigurierten WLAN
   - Anschließend wird eine Verbindung zum MQTT-Broker hergestellt
   - Bei erfolgreicher Verbindung werden Daten in Echtzeit angezeigt
   - Bei fehlgeschlagener Verbindung wird der Simulationsmodus aktiviert

3. **Anzeige prüfen:**
   - Der Hauptbildschirm zeigt das Menü mit verschiedenen Tabs an
   - Die Statusleiste am unteren Bildschirmrand zeigt den Verbindungsstatus
   - In der oberen rechten Ecke befindet sich das Heartbeat-Symbol für die ioBroker-Verbindung

4. **Webserver testen:**
   - Verbinden Sie Ihr Gerät (PC, Tablet, Smartphone) mit demselben WLAN
   - Öffnen Sie einen Webbrowser und geben Sie die IP-Adresse des ESP32 ein (wird im Display angezeigt)
   - Alternativ: http://solarmonitor.local/ (falls mDNS unterstützt wird)
   - Sie sollten nun die Weboberfläche des Solar Monitors sehen

5. **Verbindungen prüfen:**
   - Überprüfen Sie in der Systeminfo-Seite die WLAN- und MQTT-Verbindungen
   - Stellen Sie sicher, dass die ioBroker-Verbindung aktiv ist (pulsierendes Herz)

---

## Menüstruktur

Der Solar Monitor verfügt über ein Touch-Menüsystem mit mehreren Tabs und Untermenüs. Hier ist die vollständige Menüstruktur im Überblick:

```
ESP32 Solar Monitor - Umsetzungsstatus der Menüeinträge

├── System Tab
│   ├── Solar Status        # VOLLSTÄNDIG IMPLEMENTIERT - Zeigt alle wichtigen Systemparameter in Echtzeit an
│   ├── Batterie Status     # VOLLSTÄNDIG IMPLEMENTIERT - Mit Zeitberechnung für Ziel-SOC/Min-SOC und grafischer Darstellung
│   ├── Netzstatus          # VOLLSTÄNDIG IMPLEMENTIERT - Mit Flussrichtungsanzeige und Leistungswerten
│   ├── PV Leistung         # TEILWEISE IMPLEMENTIERT - Grundfunktionen vorhanden, erweiterte Grafiken in Planung
│   ├── Verbrauch           # TEILWEISE IMPLEMENTIERT - Grundanzeige vorhanden, detaillierte Verbrauchsanalyse in Planung
│   ├── Autarkie            # TEILWEISE IMPLEMENTIERT - Prozentanzeige funktioniert, historische Darstellung in Planung
│   ├── Tageswerte          # PLATZHALTER - Grundstruktur vorhanden, aber noch keine vollständige Implementierung
│   └── Statistik           # NEU IN v0.4.3 - Basisfunktionen zur Anzeige historischer Daten implementiert
│
├── Steuerung Tab
│   ├── Heizung             # BASISIMPLEMENTIERUNG - Einfache Ein/Aus-Steuerung funktioniert
│   ├── Pool                # BASISIMPLEMENTIERUNG - Einfache Ein/Aus-Steuerung funktioniert
│   ├── Garten              # PLATZHALTER - Nur UI-Struktur ohne Funktionalität
│   ├── Licht               # VOLLSTÄNDIG IMPLEMENTIERT - Mit ioBroker-Integration und Statusrückmeldung
│   ├── Steckdosen          # PLATZHALTER - Nur UI-Struktur ohne Funktionalität
│   ├── Lüftung             # PLATZHALTER - Nur UI-Struktur ohne Funktionalität
│   ├── Rollladen           # VOLLSTÄNDIG IMPLEMENTIERT (NEU IN v0.4.3) - Umfassende Steuerung für bis zu 6 Rolläden
│   └── Kameras             # PLATZHALTER - Nur UI-Struktur ohne Funktionalität
│
└── Einstellungen Tab
    ├── WLAN Setup          # VOLLSTÄNDIG IMPLEMENTIERT - Mit Statusanzeige und Neuverbindungsfunktion
    ├── MQTT Setup          # VOLLSTÄNDIG IMPLEMENTIERT - Mit Topic-Anzeige und Konfigurationsmöglichkeit
    ├── Display             # TEILWEISE IMPLEMENTIERT - Grundeinstellungen funktionieren, erweiterte Optionen in Planung
    ├── Systeminfo          # VOLLSTÄNDIG IMPLEMENTIERT - Zeigt alle wichtigen Systeminformationen
    ├── Updates             # NEU IN v0.4.3 - OTA-Update-Funktion über Weboberfläche implementiert
    ├── Logs                # PLATZHALTER - Grundstruktur vorhanden, aber keine vollständige Logging-Funktion
    ├── Neustart            # VOLLSTÄNDIG IMPLEMENTIERT - Neustart-Funktion über UI möglich
    └── Werkseinstellungen  # TEILWEISE IMPLEMENTIERT - Zurücksetzen der Konfiguration möglich, aber noch nicht vollständig
```

**Navigation:**
- Tabs werden durch Antippen des Tab-Titels gewechselt
- Menüpunkte werden durch Antippen ausgewählt
- Navigation innerhalb langer Menülisten erfolgt über die Scroll-Pfeile rechts
- Zurück zum Hauptmenü gelangt man durch Antippen des "Zurück"-Buttons in der oberen linken Ecke jeder Detailansicht

---

## Einstellungen

### WLAN Setup
In dieser Ansicht können Sie die WLAN-Verbindungsparameter einsehen und ändern:
- Anzeige der aktuellen SSID und Verbindungsstatus
- IP-Adresse des Geräts
- Signalstärke (RSSI)
- "Neu verbinden"-Button zum Neuaufbau der Verbindung

### MQTT Setup
Hier können Sie die MQTT-Konfiguration einsehen und anpassen:
- Broker-Adresse und Verbindungsstatus
- Übersicht der abonnierten Topics
- "Konfigurieren"-Button für erweiterte Einstellungen

### Display
Einstellungen zur Anzeige und Darstellung:
- Farbschemawahl (Hell/Dunkel)
- Helligkeit
- Auto-Rotation
- Bildschirmschoner-Timeout

### Systeminfo
Zeigt allgemeine Systeminformationen an:
- Firmware-Version (0.4.3)
- CPU-Taktfrequenz
- Freier Speicher (ca. 218 KB)
- Laufzeit seit dem letzten Neustart
- WLAN- und MQTT-Verbindungsstatus
- ioBroker-Verbindungsstatus

---

## Datenansichten

### Solar Status
Die Hauptübersicht zeigt alle wichtigen Werte des Solarsystems:
- PV-Leistung (aktuell produzierte Solarenergie)
- Verbrauch (aktueller Stromverbrauch des Hauses)
- Netzstatus (Einspeisung oder Bezug)
- Batteriestatus (Laden/Entladen und Ladezustand)
- Autarkiegrad in Prozent

### Batterie Status
Detaillierte Ansicht des Batteriestatus mit folgenden Informationen:
- Aktueller Ladezustand (SOC) in Prozent
- Grafische Darstellung als Füllstandsbalken
- Aktuelle Batterieleistung (Laden/Entladen)
- Batteriespannung
- Berechnung der Zeit bis zum Erreichen des Ziel-SOC (beim Laden) oder Min-SOC (beim Entladen)
- Gespeicherte Energie in kWh

**Hinweis zur Zeitberechnung:** Diese Funktion berechnet basierend auf der aktuellen Lade-/Entladerate, wie lange es dauern wird, bis die Batterie einen bestimmten Ladezustand erreicht. Die Berechnung berücksichtigt die in `config.json` definierten Batterieparameter (Kapazität, Spannung, Ziel-SOC, Min-SOC).

### Netzstatus
Zeigt den aktuellen Austausch mit dem Stromnetz:
- Grafische Darstellung der Energieflussrichtung
- Aktuelle Leistung in Watt
- Unterscheidung zwischen Einspeisung (grün) und Bezug (rot)

### PV Leistung
Detailansicht der Solarstromproduktion:
- Aktuelle Leistung der Solarmodule
- Tagesertrag in kWh
- Spitzenleistung des Tages

### Verbrauch
Übersicht des Stromverbrauchs:
- Aktueller Gesamtverbrauch
- Verteilung auf verschiedene Verbraucher
- Tagesverbrauch

### Autarkie
Zeigt den Grad der Unabhängigkeit vom Stromnetz:
- Aktueller Autarkiegrad in Prozent
- Grafische Darstellung
- Verlauf über die Zeit

### Statistik
Erweiterte statistische Ansicht mit historischen Daten:
- Grafikdarstellung der historischen Daten (PV-Leistung, Verbrauch, Batterieleistung)
- Durchschnittswerte für verschiedene Parameter
- Langzeitentwicklung der Solaranlage

---

## Steuerungsfunktionen

Die Steuerungsfunktionen ermöglichen die Kontrolle verschiedener Haushaltsgeräte und -systeme. Folgende Steuerungen sind implementiert:

### Heizung
Grundlegende Steuerung der Heizungsanlage:
- Ein-/Ausschalten der Heizung
- Anzeige des aktuellen Status

### Pool
Steuerung der Poolpumpe:
- Ein-/Ausschalten der Pumpe
- Anzeige des aktuellen Status

### Licht
Steuerung der Hausbeleuchtung:
- Ein-/Ausschalten verschiedener Lichtgruppen
- Statusanzeige und Rückmeldung

### Rollladen
Vollständige Steuerung der Rollläden (Details im Abschnitt [Rollladen-Steuerung](#rollladen-steuerung))

---

## Heartbeat-Funktionalität

Die Heartbeat-Funktion bietet eine visuelle Echtzeit-Überwachung der Verbindung zum ioBroker-System.

### Funktionsweise
- In der oberen rechten Ecke des Displays wird ein kleines Herzsymbol angezeigt
- Bei aktiver ioBroker-Verbindung "pulsiert" das Herz in roter Farbe
- Bei verlorener Verbindung wird das Herz grau und statisch dargestellt

### Technische Details
- Der ESP32 sendet alle 60 Sekunden ein Heartbeat-Signal an ioBroker
- ioBroker antwortet mit einer Bestätigung
- Bei ausbleibender Antwort erkennt der ESP32 Verbindungsprobleme
- Die Animation zeigt den Zustand der Verbindung in Echtzeit an

### Vorteile
- Sofortige Erkennung von Verbindungsproblemen auf einen Blick
- Keine Fehlinterpretation veralteter Daten durch klare visuelle Anzeige
- Automatische Wiederverbindungsversuche bei Verbindungsabbruch

---

## Rollladen-Steuerung

Die Rollladensteuerung bietet eine umfassende Kontrolle über bis zu 6 Rollläden mit einer intuitiven Benutzeroberfläche.

### Funktionsumfang
- Steuerung von bis zu 6 individuellen Rollläden
- Positionssteuerung von 0% (komplett offen) bis 100% (komplett geschlossen)
- Direktsteuerung mit Auf-, Ab- und Stopp-Befehlen
- Schnellwahl-Positionierung (0%, 25%, 50%, 75%, 100%)
- Statusanzeige für Position und Bewegungsrichtung
- Farbliche Hervorhebung des aktiven Rolladens

### Bedienung
1. Wählen Sie im "Steuerung"-Tab den Menüpunkt "Rollladen"
2. Wählen Sie den gewünschten Rolladen (R1-R6) durch Antippen aus
   - Der aktuelle Rolladen wird in türkis hervorgehoben
   - Sich bewegende Rollläden werden in orange dargestellt
3. Verwenden Sie die Auf/Stopp/Ab-Tasten für direkte Steuerung
4. Alternativ können Sie direkt eine Zielposition über die Positionstasten (0%, 25%, 50%, 75%, 100%) anwählen
5. Der aktuelle Status (Position, Bewegungsrichtung) wird im unteren Bereich angezeigt

### Technische Hintergründe
- Die Rollladen-Positionen werden in Prozent angegeben (0% = offen, 100% = geschlossen)
- Die Bewegungsberechnung erfolgt anhand der eingestellten Geschwindigkeit (Standard: 3% pro Sekunde)
- Die Steuerung erfolgt direkt über die BlindControl-Objekte in ioBroker

---

## ioBroker-Integration

Version 0.4.3 bietet eine verbesserte Integration mit ioBroker für eine nahtlose Steuerung von Smarthome-Komponenten.

### Kommunikationswege
- **MQTT-Protokoll**: Hauptkommunikationskanal zwischen ESP32 und ioBroker
- **Topic-basierte Steuerung**: Strukturierte Kommunikation über definierte MQTT-Topics
- **Heartbeat-Mechanismus**: Zuverlässige Verbindungsüberwachung

### ioBroker-Konfiguration
Für die volle Funktionalität muss in ioBroker folgende Struktur vorhanden sein:

```
mqtt.0
├── esp32solar
│   ├── light
│   │   └── 1
│   │       ├── command
│   │       └── status
│   ├── rolladen
│   │   ├── 1
│   │   │   ├── currentPosition
│   │   │   ├── targetPosition
│   │   │   ├── direction
│   │   │   └── moving
│   │   ├── 2
│   │   └── ... (weitere Rollläden)
│   ├── heartbeat
│   │   ├── beat
│   │   └── response
│   └── device (Geräteinformationen)
└── 0_userdata.0.BlindControl.Shutter
    ├── 1
    │   ├── Input.obj_I_var_Target_Level
    │   ├── Output.obj_Q_LEVEL
    │   └── Output.obj_Q_BUSY
    ├── 2
    └── ... (weitere Rollläden)
```

### JavaScript-Integration in ioBroker
Fügen Sie folgendes Skript in ioBroker ein, um die Lichtsteuerung zu aktivieren:

```javascript
// ioBroker JavaScript für Lichtsteuerung
// Wird ausgelöst, wenn ein Befehl auf esp32solar/light/1/command eintrifft
on({id: 'mqtt.0.esp32solar.light.1.command', change: 'any'}, function(obj) {
    var value = obj.state.val;
    
    // Befehl ausführen (z.B. über Homematic, Zigbee oder andere Adapter)
    // Beispiel: setState('hm-rpc.0.KEQ0123456.1.STATE', value === 'ON');
    
    // Bestätigung zurücksenden
    setState('mqtt.0.esp32solar.light.1.status', value);
    
    log('Licht wurde geschaltet: ' + value);
});
```

Für die Heartbeat-Funktion fügen Sie dieses Skript hinzu:

```javascript
// ioBroker JavaScript für Heartbeat-Funktionalität
on({ id: [].concat(['mqtt.2.esp32solar.heartbeat.beat']), change: 'ne' }, async (obj) => {
  let value = obj.state.val;
  let oldValue = obj.oldState.val;
  setState('mqtt.2.esp32solar.heartbeat.response', (String(getState('mqtt.2.esp32solar.heartbeat.beat').val) + ',ACK'));
});
```

Für die vollständige Rollladensteuerung sollte das BlindControl-Widget in ioBroker verwendet werden.

---

## Webserver & Fernsteuerung

Eine der wichtigsten Neuerungen in Version 0.4.3 ist der integrierte Webserver, der Fernzugriff und -steuerung ermöglicht.

### Zugriff auf den Webserver
- Der Webserver ist automatisch aktiv, sobald der ESP32 mit dem WLAN verbunden ist
- Zugriff über die IP-Adresse des Geräts (wird beim Start im Display angezeigt)
- Alternativ über mDNS: http://solarmonitor.local (falls vom Router unterstützt)

### Verfügbare Webseiten
1. **Übersicht**: Dateien im SPIFFS-Speicher, Upload- und Löschfunktionen
2. **Monitor**: Live-Ansicht der Solardaten mit Batterie- und Systemstatus
3. **Konfiguration bearbeiten**: Direktes Bearbeiten der JSON-Konfigurationsdateien
4. **System-Info**: Detaillierte Hardware- und Softwareinformationen
5. **Firmware-Update**: OTA-Update-Funktion für einfache Firmware-Aktualisierungen

### Funktionen des Webservers
- **Datei-Management**: Hochladen, Herunterladen und Löschen von Konfigurationsdateien
- **Live-Monitoring**: Echtzeit-Ansicht aller Solardaten
- **Remote-Steuerung**: Fernbedienung für Rollläden und andere Geräte
- **OTA-Updates**: Firmware-Aktualisierung ohne USB-Kabel
- **Konfigurationseditor**: Syntax-Highlighting-Editor für JSON-Dateien

### Rollladensteuerung im Browser
Unter dem Menüpunkt "Solar Monitor" im Web-Interface:

1. **Rolladen-Auswahl**: Dropdown-Menü zur Auswahl eines der 6 Rollläden
2. **Steuerungstasten**: AUF, STOP, AB-Buttons für direkte Steuerung
3. **Positionsschieberegler**: Stufenlose Positionierung von 0-100%
4. **Statusanzeige**: Live-Update der aktuellen Position und Bewegungsrichtung
5. **Auto-Refresh**: Automatische Aktualisierung alle 5 Sekunden

### Bearbeitung der Konfiguration
Unter dem Menüpunkt "Konfiguration bearbeiten":

1. Wählen Sie die zu bearbeitende Datei (z.B. config.json, menu.json)
2. Bearbeiten Sie die Datei im Online-Editor mit Syntax-Highlighting
3. Speichern Sie die Änderungen durch Klicken auf "Speichern"
4. Bei wichtigen Konfigurationsänderungen wird ein Neustart empfohlen

### OTA-Firmware-Update
Unter dem Menüpunkt "Firmware-Update":

1. Laden Sie die neueste Firmware-Datei (.bin) von Ihrem Computer hoch
2. Klicken Sie auf "Update starten"
3. Warten Sie, bis der Update-Prozess abgeschlossen ist
4. Das Gerät startet automatisch neu mit der neuen Firmware

---

## Fehlersuche

### WLAN-Verbindungsprobleme
- **Problem**: Keine Verbindung zum WLAN
  - **Lösung 1**: Überprüfen Sie die SSID und das Passwort in der `config.json`
  - **Lösung 2**: Stellen Sie sicher, dass der ESP32 innerhalb der Reichweite Ihres WLAN-Routers ist
  - **Lösung 3**: Prüfen Sie, ob Ihr Router 2,4 GHz WLAN unterstützt (5 GHz wird nicht unterstützt)

### MQTT-Verbindungsprobleme
- **Problem**: Keine Verbindung zum MQTT-Broker
  - **Lösung 1**: Überprüfen Sie die IP-Adresse und den Port des MQTT-Brokers
  - **Lösung 2**: Stellen Sie sicher, dass der MQTT-Broker läuft und erreichbar ist
  - **Lösung 3**: Prüfen Sie die Topic-Konfiguration in `mqtt_topics.json`

### Heartbeat-Funktionalität
- **Problem**: Herz bleibt   grau trotz aktiver ioBroker-Instanz
  - **Lösung 1**: Prüfen Sie, ob das Topic `esp32solar/heartbeat/response` in ioBroker korrekt eingerichtet ist
  - **Lösung 2**: Überprüfen Sie das JavaScript für die Heartbeat-Funktion in ioBroker
  - **Lösung 3**: Fügen Sie folgendes Skript in ioBroker ein:
    ```javascript
    on({ id: [].concat(['mqtt.2.esp32solar.heartbeat.beat']), change: 'ne' }, async (obj) => {
      let value = obj.state.val;
      let oldValue = obj.oldState.val;
      setState('mqtt.2.esp32solar.heartbeat.response', (String(getState('mqtt.2.esp32solar.heartbeat.beat').val) + ',ACK'));
    });
    ```
  - **Tipp**: Verwenden Sie die Systeminfo-Seite, um die MQTT-Verbindung zu prüfen

### Rollladensteuerung
- **Problem**: Rollläden reagieren nicht auf Steuerungsbefehle
  - **Lösung 1**: Prüfen Sie die Verbindung zu ioBroker (Heartbeat-Indikator)
  - **Lösung 2**: Stellen Sie sicher, dass die BlindControl-Objekte in ioBroker korrekt konfiguriert sind
  - **Lösung 3**: Überprüfen Sie die MQTT-Topics für die Rollladensteuerung
  - **Lösung 4**: Testen Sie die Steuerung über die Weboberfläche, um Probleme einzugrenzen

- **Problem**: Rollläden-Positionsanzeige aktualisiert sich nicht
  - **Lösung 1**: Prüfen Sie die MQTT-Topics für die Rollladenpositionen in ioBroker
  - **Lösung 2**: Stellen Sie sicher, dass die ioBroker-Instanz läuft und kommuniziert
  - **Tipp**: Versuchen Sie, die Rollläden manuell aus ioBroker zu steuern, um die Kommunikation zu testen

### Webserver-Probleme
- **Problem**: Webseite nicht erreichbar
  - **Lösung 1**: Prüfen Sie die IP-Adresse auf dem Display des ESP32
  - **Lösung 2**: Stellen Sie sicher, dass sich Ihr Gerät im selben Netzwerk befindet
  - **Lösung 3**: Versuchen Sie, die mDNS-Adresse http://solarmonitor.local zu verwenden
  - **Lösung 4**: Überprüfen Sie, ob der Webserver läuft (siehe Systeminfo-Seite)

- **Problem**: Änderungen an der Konfiguration werden nicht übernommen
  - **Lösung 1**: Starten Sie den ESP32 nach dem Speichern der Konfiguration neu
  - **Lösung 2**: Prüfen Sie, ob die Konfigurationsdatei korrekt formatiert ist (gültiges JSON)
  - **Lösung 3**: Verwenden Sie einen JSON-Validator vor dem Speichern
  - **Tipp**: Erstellen Sie vor größeren Änderungen ein Backup der Konfigurationsdateien

### Display-Probleme
- **Problem**: Touch-Funktion reagiert nicht korrekt
  - **Lösung 1**: Kalibrierungswerte in `config.h` anpassen
  - **Lösung 2**: Bei USB-C-Varianten die Display-Invertierung in `config.json` prüfen
  - **Lösung 3**: Achten Sie auf korrekte Touchscreen-Verkabelung

- **Problem**: Display zeigt falsche Farben
  - **Lösung 1**: Prüfen Sie die "inverted"-Einstellung in der `config.json`
  - **Lösung 2**: Für USB-C Varianten: Setzen Sie `"type": "usb_c"` und `"inverted": true`
  - **Lösung 3**: Für Micro-USB Varianten: Setzen Sie `"type": "micro_usb"` und `"inverted": false`

### JSON-Probleme
- **Problem**: JSON-Konfigurationen werden nicht erkannt
  - **Lösung 1**: Überprüfen Sie die JSON-Syntax mit einem Online-Validator
  - **Lösung 2**: Achten Sie auf korrekte Klammern und Anführungszeichen
  - **Lösung 3**: Vermeiden Sie Sonderzeichen in Strings

### Bekannte Probleme
- Die WLAN-Verbindung kann unter bestimmten Umständen instabil sein; die Version 0.4.3 enthält robustere Verbindungsroutinen
- Bei einigen Routern kann die Verbindungszeit länger als erwartet sein
- Die Touch-Kalibrierung kann je nach Hardware-Variante leicht abweichen

### Allgemeine Tipps
- Verwenden Sie den seriellen Monitor (115200 Baud) für ausführliche Debug-Informationen
- Bei wiederkehrenden Problemen können Sie einen Reset durchführen
- Nutzen Sie die Systeminfo-Seite für Diagnose der Verbindungen und des Speicherverbrauchs
- Erstellen Sie regelmäßig Backups Ihrer Konfigurationsdateien über die Weboberfläche
- Prüfen Sie vor dem Speichern von Änderungen immer die JSON-Syntax

---

## Anhang: Erweiterungsmöglichkeiten

Der ESP32 Solar Monitor ist modular aufgebaut und kann leicht um neue Funktionen erweitert werden. Mit ca. 218 KB freiem HEAP-Speicher gibt es noch viel Raum für Erweiterungen.

### Wie alles zusammenarbeitet

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

### Menüsystem: Struktur und Erweiterung

#### Grundlegende Struktur
Das Menüsystem basiert auf einer JSON-Datei (`menu.json`), die die komplette Menüstruktur definiert:

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

#### Erweiterung des Menüs
Um das Menü zu erweitern oder anzupassen:

1. **Öffne die `menu.json` Datei**:
   Die Datei befindet sich im SPIFFS-Speicher und kann über den ConfigManager oder über die Weboberfläche bearbeitet werden.

2. **Füge neue Tabs oder Menüpunkte hinzu**:
   - Für einen neuen Tab: Füge ein neues Objekt zum "tabs"-Array hinzu
   - Für einen neuen Menüpunkt: Füge ein neues Objekt zum "items"-Array eines bestehenden Tabs hinzu

3. **Definiere die Funktionalität**:
   Jeder Menüpunkt benötigt einen `function`-Parameter, der auf eine implementierte Funktion in der ViewManager-Klasse verweist.

4. **Implementiere die Ansichtsfunktion**:
   In der `ViewManager.cpp` muss die entsprechende Funktion implementiert werden, die beim Auswählen des Menüpunkts aufgerufen wird.

### Erweiterung einer Menüfunktion am Beispiel eines neuen Menüpunkts

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

### Integration von verschiedenen Schnittstellen

Der ESP32 Solar Monitor kann mit verschiedenen Schnittstellen erweitert werden, um die Steuerungsfunktionen zu verbessern:

#### RFLink Integration
```cpp
// Beispiel für RFLink-Integration (433/868 MHz Funkgeräte)
#include <RFLink.h>

RFLink rflink(Serial2); // Verwenden des zweiten seriellen Ports des ESP32

void setupRFLink() {
  Serial2.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN); // Typische Baudrate für RFLink
  rflink.begin();
}

void sendRFLinkCommand(const String &protocol, const String &id, const String &command) {
  rflink.sendCommand(protocol, id, command);
}
```

#### Modbus Integration
```cpp
// Beispiel für Modbus-Integration (z.B. für Wechselrichter)
#include <ModbusMaster.h>

ModbusMaster modbus;

void setupModbus() {
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  modbus.begin(1, Serial2); // Slave-Adresse 1
}

float readModbusRegister(uint16_t reg) {
  uint8_t result = modbus.readHoldingRegisters(reg, 1);
  if (result == modbus.ku8MBSuccess) {
    return modbus.getResponseBuffer(0);
  }
  return -1;
}
```

#### CAN-Bus Integration
```cpp
// Beispiel für CAN-Bus-Integration (z.B. für BMS-Systeme)
#include <ESP32CAN.h>
#include <CAN_config.h>

CAN_device_t CAN_cfg;

void setupCANBus() {
  CAN_cfg.speed = CAN_SPEED_500KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_5;
  CAN_cfg.rx_pin_id = GPIO_NUM_4;
  CAN_cfg.rx_queue = xQueueCreate(10, sizeof(CAN_frame_t));
  ESP32Can.CANInit();
}

void sendCANMessage(uint32_t id, uint8_t* data, uint8_t length) {
  CAN_frame_t frame;
  frame.FIR.B.FF = CAN_frame_std;
  frame.MsgID = id;
  frame.FIR.B.DLC = length;
  memcpy(frame.data.u8, data, length);
  ESP32Can.CANWriteFrame(&frame);
}
```

#### Home Assistant MQTT Integration
Home Assistant verwendet das MQTT-Discovery-Protokoll, wodurch Geräte automatisch erkannt werden können:

```cpp
// Beispiel für Home Assistant MQTT-Discovery
void registerHomeAssistantDevice() {
  // Konfigurationsthema für einen Schalter
  String configTopic = "homeassistant/switch/esp32_monitor/rolladen/config";
  
  // Konfiguration als JSON
  JsonDocument config;
  config["name"] = "Rollladen";
  config["device_class"] = "switch";
  config["state_topic"] = "home/rollladen/status";
  config["command_topic"] = "home/rollladen/command";
  config["payload_on"] = "up";
  config["payload_off"] = "down";
  config["unique_id"] = "esp32_rollladen_1";
  
  // Geräteinformationen
  JsonObject device = config.createNestedObject("device");
  device["identifiers"] = "esp32_solar_monitor";
  device["name"] = "ESP32 Solar Monitor";
  device["model"] = "Solar Monitor v0.4.3";
  device["manufacturer"] = "DIY";
  
  // Serialisieren und veröffentlichen
  String configPayload;
  serializeJson(config, configPayload);
  mqttManager.publish(configTopic, configPayload);
}
```

### Erweiterung um weitere Gerätetypen

Die Rollladensteuerungsfunktionalität kann als Vorlage für die Implementierung weiterer Gerätetypen dienen. Hier einige Beispiele:

#### Heizungssteuerung erweitern
```cpp
// Erweiterung der Heizungssteuerung mit Temperatureinstellung
void ViewManager::controlHeatingAdvanced() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  tft.setCursor(20, 70);
  tft.println("Erweiterte Heizungssteuerung");
  
  // Aktuelle Temperatur anzeigen
  tft.setCursor(20, 90);
  tft.print("Aktuelle Temperatur: ");
  float currentTemp = mqttManager.getValue("heating_temperature").toFloat();
  tft.print(currentTemp);
  tft.println(" °C");
  
  // Zieltemperatur anzeigen
  tft.setCursor(20, 110);
  tft.print("Zieltemperatur: ");
  float targetTemp = mqttManager.getValue("heating_target").toFloat();
  tft.print(targetTemp);
  tft.println(" °C");
  
  // Temperatur-Einstelltasten
  drawButton(60, 140, 40, 40, "−", TFT_RED);
  drawButton(220, 140, 40, 40, "+", TFT_GREEN);
  
  // Aktuelle Zieltemperatur mittig anzeigen
  tft.setTextSize(2);
  tft.setTextColor(HIGHLIGHT_COLOR, BACKGROUND);
  tft.setCursor(140, 150);
  tft.print(targetTemp, 1);
  tft.println(" °C");
  
  // Status und Modus
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 200);
  tft.print("Status: ");
  bool heatingOn = mqttManager.getValue("heating_status") == "ON";
  tft.setTextColor(heatingOn ? TFT_GREEN : TFT_RED, BACKGROUND);
  tft.println(heatingOn ? "Heizung aktiv" : "Heizung inaktiv");
  
  // Modus-Buttons
  drawButton(40, 230, 80, 30, "Aus", heatingOn ? TFT_DARKGREY : TFT_RED);
  drawButton(130, 230, 80, 30, "Auto", heatingOn ? TFT_GREEN : TFT_DARKGREY);
  drawButton(220, 230, 80, 30, "Boost", TFT_ORANGE);
}
```

#### Smart Meter Integration
```cpp
// Smart Meter Detailansicht
void ViewManager::drawSmartMeterStatus() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  tft.setCursor(20, 70);
  tft.println("Smart Meter Status");
  
  // Aktuelle Werte anzeigen
  tft.setCursor(20, 90);
  tft.print("Aktueller Bezug: ");
  float currentImport = mqttManager.getValue("meter_import").toFloat();
  tft.print(currentImport);
  tft.println(" kW");
  
  tft.setCursor(20, 110);
  tft.print("Aktuelle Einspeisung: ");
  float currentExport = mqttManager.getValue("meter_export").toFloat();
  tft.print(currentExport);
  tft.println(" kW");
  
  // Tageswerte
  tft.setCursor(20, 140);
  tft.print("Bezug heute: ");
  float dailyImport = mqttManager.getValue("meter_daily_import").toFloat();
  tft.print(dailyImport);
  tft.println(" kWh");
  
  tft.setCursor(20, 160);
  tft.print("Einspeisung heute: ");
  float dailyExport = mqttManager.getValue("meter_daily_export").toFloat();
  tft.print(dailyExport);
  tft.println(" kWh");
  
  // Grafische Darstellung
  int centerX = 160;
  int centerY = 220;
  
  // Netz-Symbol
  tft.drawRect(centerX - 100, centerY - 25, 50, 50, TFT_WHITE);
  tft.setCursor(centerX - 90, centerY);
  tft.print("Netz");
  
  // Haus-Symbol
  tft.drawRect(centerX + 50, centerY - 25, 50, 50, TFT_WHITE);
  tft.setCursor(centerX + 60, centerY);
  tft.print("Haus");
  
  // Verbindungslinie
  tft.drawLine(centerX - 50, centerY, centerX + 50, centerY, TFT_WHITE);
  
  // Flussrichtungspfeil
  if (currentImport > currentExport) {
    // Netzbezug
    tft.fillTriangle(
      centerX - 10, centerY - 10,
      centerX + 10, centerY,
      centerX - 10, centerY + 10,
      TFT_RED
    );
  } else {
    // Einspeisung
    tft.fillTriangle(
      centerX + 10, centerY - 10,
      centerX - 10, centerY,
      centerX + 10, centerY + 10,
      TFT_GREEN
    );
  }
}
```

### Nächste Schritte und geplante Features

Die Zukunftspläne für den ESP32 Solar Monitor umfassen:

1. **Erweiterte Datenanalytik**: Langzeitstatistiken und Verbrauchsanalyse
2. **Energiemanagement**: Automatische Steuerung von Verbrauchern basierend auf PV-Überschuss
3. **Integration weiterer Energiequellen**: Unterstützung für Windkraft oder Wasserkraft
4. **Batterielebensdauer-Überwachung**: Detaillierte Analysen zum Batteriezustand und Alterungsprozess
5. **KI-basierte Verbrauchsprognosen**: Vorhersage des Stromverbrauchs basierend auf historischen Daten
6. **Erweiterte Steuerung**: Integration weiterer Hausautomationsgeräte
7. **Graphische Dashboards**: Erweiterte Visualisierungen auf dem Webinterface
8. **Over-the-Air Updates**: Vereinfachte Firmware-Aktualisierung
9. **Mehr Sensorik**: Integration von Temperatur-, Luftfeuchtigkeits- und Lichtsensoren
10. **Mobilanwendung**: Begleitende Smartphone-App für Remote-Monitoring

## Abhängigkeiten

Der ESP32 Solar Monitor nutzt folgende Bibliotheken:

- **TFT_eSPI**: Display-Ansteuerung
- **XPT2046_Touchscreen**: Touchscreen-Ansteuerung
- **ArduinoJson**: JSON-Verarbeitung (Version 7.x)
- **PubSubClient**: MQTT-Kommunikation
- **ESPAsyncWebServer**: Webserver-Funktionalität
- **AsyncTCP**: Asynchrone TCP-Kommunikation
- **SPIFFS**: Dateisystemverwaltung

## Fazit

Der ESP32 Solar Monitor ist ein offenes und erweiterbares Projekt zur Überwachung und Steuerung von Solaranlagen und Smart-Home-Geräten. Die modulare Architektur ermöglicht eine einfache Anpassung und Erweiterung. Mit Version 0.4.3 wurden wichtige Funktionen wie Heartbeat-Überwachung, Rollladensteuerung und Webserver-Funktionalität hinzugefügt, die den Monitor zu einem umfassenden Steuerungszentrum für Ihr smartes Zuhause machen.

Beiträge und Verbesserungsvorschläge sind jederzeit willkommen!

---

_Diese Dokumentation wurde für Version 0.4.3 des ESP32 Solar Monitor erstellt._
  float currentTemp = mqttManager.getValue("heating_temperature").toFloat();
  tft.print(currentTemp);
  tft.println(" °C");
  
  // Zieltemperatur anzeigen
  tft.setCursor(20, 110);
  tft.print("Zieltemperatur: ");
  float targetTemp = mqttManager.getValue("heating_target").toFloat();
  tft.print(targetTemp);
  tft.println(" °C");
  
  // Temperatur-Einstelltasten
  drawButton(60, 140, 40, 40, "−", TFT_RED);
  drawButton(220, 140, 40, 40, "+", TFT_GREEN);
  
  // Aktuelle Zieltemperatur mittig anzeigen
  tft.setTextSize(2);
  tft.setTextColor(HIGHLIGHT_COLOR, BACKGROUND);
  tft.setCursor(140, 150);
  tft.print(targetTemp, 1);
  tft.println(" °C");
  
  // Status und Modus
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(20, 200);
  tft.print("Status: ");
  bool heatingOn = mqttManager.getValue("heating_status") == "ON";
  tft.setTextColor(heatingOn ? TFT_GREEN : TFT_RED, BACKGROUND);
  tft.println(heatingOn ? "Heizung aktiv" : "Heizung inaktiv");
  
  // Modus-Buttons
  drawButton(40, 230, 80, 30, "Aus", heatingOn ? TFT_DARKGREY : TFT_RED);
  drawButton(130, 230, 80, 30, "Auto", heatingOn ? TFT_GREEN : TFT_DARKGREY);
  drawButton(220, 230, 80, 30, "Boost", TFT_ORANGE);
}
```

#### Smart Meter Integration
```cpp
// Smart Meter Detailansicht
void ViewManager::drawSmartMeterStatus() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  tft.setCursor(20, 70);
  tft.println("Smart Meter Status");
  
  // Aktuelle Werte anzeigen
  tft.setCursor(20, 90);
  tft.print("Aktueller Bezug: ");
  float currentImport = mqttManager.getValue("meter_import").toFloat();
  tft.print(currentImport);
  tft.println(" kW");
  
  tft.setCursor(20, 110);
  tft.print("Aktuelle Einspeisung: ");
  float currentExport = mqttManager.getValue("meter_export").toFloat();
  tft.print(currentExport);
  tft.println(" kW");
  
  // Tageswerte
  tft.setCursor(20, 140);
  tft.print("Bezug heute: ");
  float dailyImport = mqttManager.getValue("meter_daily_import").toFloat();
  tft.print(dailyImport);
  tft.println(" kWh");
  
  tft.setCursor(20, 160);
  tft.print("Einspeisung heute: ");
  float dailyExport = mqttManager.getValue("meter_daily_export").toFloat();
  tft.print(dailyExport);
  tft.println(" kWh");
  
  // Grafische Darstellung
  int centerX = 160;
  int centerY = 220;
  
  // Netz-Symbol
  tft.drawRect(centerX - 100, centerY - 25, 50, 50, TFT_WHITE);
  tft.setCursor(centerX - 90, centerY);
  tft.print("Netz");
  
  // Haus-Symbol
  tft.drawRect(centerX + 50, centerY - 25, 50, 50, TFT_WHITE);
  tft.setCursor(centerX + 60, centerY);
  tft.print("Haus");
  
  // Verbindungslinie
  tft.drawLine(centerX - 50, centerY, centerX + 50, centerY, TFT_WHITE);
  
  // Flussrichtungspfeil
  if (currentImport > currentExport) {
    // Netzbezug
    tft.fillTriangle(
      centerX - 10, centerY - 10,
      centerX + 10, centerY,
      centerX - 10, centerY + 10,
      TFT_RED
    );
  } else {
    // Einspeisung
    tft.fillTriangle(
      centerX + 10, centerY - 10,
      centerX - 10, centerY,
      centerX + 10, centerY + 10,
      TFT_GREEN
    );
  }
}
```

#### Wetterprognose Integration
```cpp
// Wetterprognose mit Solarertragsprognose
void ViewManager::drawWeatherForecast() {
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  
  tft.setCursor(20, 70);
  tft.println("Wetterprognose & PV-Ertragsprognose");
  
  // Aktuelle Wetterdaten
  tft.setCursor(20, 90);
  tft.print("Temperatur: ");
  float temperature = mqttManager.getValue("weather_temperature").toFloat();
  tft.print(temperature);
  tft.println(" °C");
  
  tft.setCursor(20, 110);
  tft.print("Wolken: ");
  int cloudCover = mqttManager.getValue("weather_clouds").toInt();
  tft.print(cloudCover);
  tft.println(" %");
  
  // Vorhersage für PV-Ertrag
  tft.setCursor(20, 140);
  tft.print("PV-Ertragsprognose heute: ");
  float expectedYield = mqttManager.getValue("forecast_pv_today").toFloat();
  tft.print(expectedYield);
  tft.println(" kWh");
  
  // 3-Tages-Prognose
  tft.setCursor(20, 170);
  tft.println("3-Tages-Prognose:");
  
  // Tabelle für die Prognose
  int tableX = 20;
  int tableY = 190;
  int colWidth = 90;
  int rowHeight = 20;
  
  // Tabellenkopf
  tft.drawRect(tableX, tableY, colWidth*3, rowHeight, TFT_WHITE);
  tft.setCursor(tableX + 10, tableY + 6);
  tft.print("Tag");
  tft.setCursor(tableX + colWidth + 10, tableY + 6);
  tft.print("Wetter");
  tft.setCursor(tableX + colWidth*2 + 10, tableY + 6);
  tft.print("PV-Ertrag");
  
  // Tag 1
  tft.drawRect(tableX, tableY + rowHeight, colWidth, rowHeight, TFT_WHITE);
  tft.setCursor(tableX + 10, tableY + rowHeight + 6);
  tft.print("Heute");
  
  tft.drawRect(tableX + colWidth, tableY + rowHeight, colWidth, rowHeight, TFT_WHITE);
  // Hier würde ein Wettersymbol gezeichnet werden
  tft.setCursor(tableX + colWidth + 10, tableY + rowHeight + 6);
  tft.print(cloudCover < 30 ? "Sonnig" : (cloudCover < 70 ? "Bewölkt" : "Bedeckt"));
  
  tft.drawRect(tableX + colWidth*2, tableY + rowHeight, colWidth, rowHeight, TFT_WHITE);
  tft.setCursor(tableX + colWidth*2 + 10, tableY + rowHeight + 6);
  tft.print(expectedYield);
  tft.print(" kWh");
  
  // Tag 2 und 3 würden ähnlich implementiert werden
}
```

### Fazit und Ausblick

Mit diesen Beispielen können Sie den ESP32 Solar Monitor um verschiedene Schnittstellen und Funktionen erweitern und an Ihre spezifischen Bedürfnisse anpassen. Der vorhandene freie HEAP-Speicher von 218 KB bietet ausreichend Raum für mehrere dieser Erweiterungen.

Die Zukunftspläne für den ESP32 Solar Monitor umfassen:

1. **Erweiterte Datenanalytik**: Langzeitstatistiken und Verbrauchsanalyse
2. **Energiemanagement**: Automatische Steuerung von Verbrauchern basierend auf PV-Überschuss
3. **Integration weiterer Energiequellen**: Unterstützung für Windkraft oder Wasserkraft
4. **Batterielebensdauer-Überwachung**: Detaillierte Analysen zum Batteriezustand und Alterungsprozess
5. **KI-basierte Verbrauchsprognosen**: Vorhersage des Stromverbrauchs basierend auf historischen Daten
6. **Grafische Dashboards**: Erweiterte Visualisierungen auf dem Webinterface

Der ESP32 Solar Monitor ist ein offenes Projekt, das ständig weiterentwickelt wird. Beiträge und Verbesserungsvorschläge sind willkommen!
