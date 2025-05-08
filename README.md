# ESP32 Solar Monitor v0.4.3
## Vollständige Bedienungsanleitung

## Inhaltsverzeichnis
1. [Überblick](#überblick)
2. [Hardware & Pin-Belegung](#hardware--pin-belegung)
3. [Installation](#installation)
4. [Systemarchitektur](#systemarchitektur)
5. [Erste Inbetriebnahme](#erste-inbetriebnahme)
6. [Access Point Modus](#access-point-modus)
7. [Konfigurationsdateien](#konfigurationsdateien)
8. [Menüstruktur](#menüstruktur)
9. [Einstellungen](#einstellungen)
10. [Datenansichten](#datenansichten)
11. [Steuerungsfunktionen](#steuerungsfunktionen)
12. [Heartbeat-Funktionalität](#heartbeat-funktionalität)
13. [Rollladen-Steuerung](#rollladen-steuerung)
14. [MQTT-Integration](#mqtt-integration)
15. [ioBroker-Integration](#iobroker-integration)
16. [Webserver & Fernsteuerung](#webserver--fernsteuerung)
17. [Fehlersuche](#fehlersuche)
18. [Anhang: Erweiterungsmöglichkeiten](#anhang-erweiterungsmöglichkeiten)

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
- **NEU**: Access Point Modus für einfache Erstkonfiguration ohne vorhandenes WLAN

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

## Erste Inbetriebnahme

1. **Konfigurationsdateien anpassen:**
   - Verbinden Sie das Gerät mit dem PC und öffnen Sie die Arduino IDE
   - Laden Sie die Konfigurationsdateien in den "data"-Ordner des Projekts
   - Passen Sie die Dateien an Ihre Umgebung an
   - Laden Sie die Dateien mit dem "ESP32 Sketch Data Upload" Tool hoch
   - Alternativ können Sie die Dateien später über die Weboberfläche bearbeiten

2. **Gerät starten:**
   - Nach dem Einschalten prüft der Solar Monitor auf eine gültige WLAN-Konfiguration
   - Falls keine gültige Konfiguration vorhanden ist, startet er automatisch im Access Point Modus
   - Bei vorhandener Konfiguration verbindet er sich mit dem konfigurierten WLAN
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

## Access Point Modus

Der ESP32 Solar Monitor bietet einen integrierten Access Point (AP) Modus, der automatisch aktiviert wird, wenn keine gültige WLAN-Konfiguration vorhanden ist. Dies erleichtert die Erstkonfiguration erheblich, da keine vorherige Konfigurationsdatei erstellt werden muss.

### Automatische Aktivierung

Der Access Point Modus wird in folgenden Situationen automatisch aktiviert:
- Bei der ersten Inbetriebnahme ohne vorhandene Konfiguration
- Nach einem Zurücksetzen auf Werkseinstellungen
- Wenn die gespeicherte WLAN-Konfiguration ungültig ist oder Standard-Werte enthält
- Nach mehreren fehlgeschlagenen Verbindungsversuchen mit den gespeicherten Daten

### Zugriff auf den Access Point

1. **Access Point finden:**
   - Nach dem Start des AP-Modus erscheint auf dem Display eine Meldung mit der SSID
   - Die SSID lautet: "SolarMonitor-XXXX" (wobei XXXX die letzten 4 Zeichen der MAC-Adresse sind)
   - Der AP benötigt kein Passwort für den Zugriff

2. **Verbindung herstellen:**
   - Suchen Sie auf Ihrem Smartphone, Tablet oder Computer nach dem WLAN-Netzwerk
   - Verbinden Sie sich mit dem "SolarMonitor-XXXX" Netzwerk
   - Nach erfolgreicher Verbindung sollte ein Captive Portal erscheinen
   - Falls das Portal nicht automatisch erscheint, öffnen Sie einen Browser und geben Sie die IP-Adresse `192.168.4.1` ein

### WLAN-Konfiguration über das Web-Interface

Das Captive Portal bietet eine benutzerfreundliche Oberfläche zur Konfiguration der WLAN-Verbindung:

1. **Netzwerkauswahl:**
   - Im linken Bereich wird eine Liste der verfügbaren WLAN-Netzwerke angezeigt
   - Für jedes Netzwerk werden der Name (SSID) und die Signalstärke angezeigt
   - Geschützte Netzwerke werden mit einem Schloss-Symbol gekennzeichnet

2. **Netzwerkdetails eingeben:**
   - Klicken Sie auf ein Netzwerk in der Liste, um es auszuwählen
   - Der Name (SSID) wird automatisch ins Formular auf der rechten Seite übernommen
   - Geben Sie das Passwort für das ausgewählte WLAN ein
   - Klicken Sie auf "Speichern und verbinden"

3. **Verbindungsprozess:**
   - Nach dem Speichern werden die Einstellungen in die Konfigurationsdatei geschrieben
   - Der ESP32 startet neu und versucht, eine Verbindung mit den eingegebenen Daten herzustellen
   - Während des Neustarts wird eine entsprechende Meldung angezeigt
   - Bei erfolgreicher Verbindung wechselt das Gerät in den normalen Betriebsmodus
   - Bei fehlgeschlagener Verbindung kehrt das Gerät zum AP-Modus zurück

### Status-Feedback im Display

Während des Access Point Modus zeigt das Display wichtige Informationen:
- Die SSID des Access Points (SolarMonitor-XXXX)
- Die IP-Adresse des Access Points (192.168.4.1)
- Anweisungen zur Verbindung und Konfiguration
- Den aktuellen Status des Captive Portals

### Simulationsmodus im AP-Betrieb

Im Access Point Modus wird automatisch der Simulationsmodus aktiviert, um trotz fehlender MQTT-Verbindung Daten anzuzeigen:
- Simulierte Solarleistung mit realistischen Tag-/Nachtschwankungen
- Simulierte Batterieleistung und Ladezustand
- Simulierter Verbrauch und Netzbezug

Dieser Modus ermöglicht es, die Benutzeroberfläche zu testen, auch wenn noch keine Verbindung zur tatsächlichen Solaranlage besteht.

### Sicherheitshinweise

- Der Access Point ist standardmäßig unverschlüsselt, um die Erstkonfiguration zu erleichtern
- Es wird empfohlen, die Konfiguration zügig durchzuführen und den AP-Modus nicht länger als nötig aktiv zu lassen
- Nach erfolgreicher Konfiguration wird der AP-Modus automatisch deaktiviert
- Ein manueller Wechsel zurück in den AP-Modus ist jederzeit über das Einstellungsmenü möglich

### Troubleshooting

- **Problem**: Der Access Point erscheint nicht in der WLAN-Liste
  - **Lösung**: Starten Sie den ESP32 neu und warten Sie etwa 30 Sekunden
  
- **Problem**: Captive Portal öffnet sich nicht automatisch
  - **Lösung**: Öffnen Sie manuell einen Browser und navigieren Sie zu `192.168.4.1`
  
- **Problem**: Nach dem Speichern der Konfiguration verbindet sich der ESP32 nicht mit dem WLAN
  - **Lösung 1**: Überprüfen Sie das eingegebene Passwort auf Tippfehler
  - **Lösung 2**: Stellen Sie sicher, dass das WLAN 2,4 GHz unterstützt (5 GHz wird nicht unterstützt)
  - **Lösung 3**: Positionieren Sie den ESP32 näher am Router für ein besseres Signal

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
- "Access Point aktivieren"-Button zum manuellen Wechsel in den AP-Modus

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
# ESP32 Solar Monitor: Rolladen-Steuerung

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
6. Über die Weboberfläche können Sie die Rollläden auch aus der Ferne steuern:
   - Wählen Sie den Menüpunkt "Solar Monitor" im Web-Interface
   - Wählen Sie einen Rolladen aus dem Dropdown-Menü
   - Verwenden Sie die Steuerungstasten oder den Positionsschieberegler
   - Die Statusanzeige wird automatisch alle 5 Sekunden aktualisiert

### Technische Hintergründe
- Die Rollladen-Positionen werden in Prozent angegeben (0% = offen, 100% = geschlossen)
- Die Bewegungsberechnung erfolgt anhand der eingestellten Geschwindigkeit (Standard: 3% pro Sekunde)
- Die Steuerung erfolgt direkt über die BlindControl-Objekte in ioBroker
- Die MQTT-Topics für die Rollladensteuerung haben folgende Struktur:
  ```
  mqtt.0/esp32solar/rolladen/X/currentPosition  // Aktuelle Position
  mqtt.0/esp32solar/rolladen/X/targetPosition   // Zielposition
  mqtt.0/esp32solar/rolladen/X/direction        // Richtung (up/down/stop)
  mqtt.0/esp32solar/rolladen/X/moving           // Bewegungsstatus (true/false)
  ```
  wobei X die Nummer des Rolladens (1-6) ist

### Anpassung der Rollladengeschwindigkeit
Die Geschwindigkeit der Rollläden kann in der `config.json` angepasst werden:
```json
{
  "rolladen": {
    "speed_percent_per_second": 3,
    "enabled": [true, true, true, true, true, true]
  }
}
```
- `speed_percent_per_second`: Definiert die Bewegungsgeschwindigkeit in Prozent pro Sekunde
- `enabled`: Ein Array mit 6 Boolean-Werten zur Aktivierung/Deaktivierung einzelner Rollläden

### Gruppen-Steuerung
In zukünftigen Versionen ist eine Gruppen-Steuerung geplant, mit der mehrere Rollläden gleichzeitig bedient werden können. Diese Funktion befindet sich derzeit in der Entwicklung.

### Automatisierungsintegration
Die Rollladensteuerung ist vollständig in ioBroker integriert und kann somit in Automatisierungsszenarien eingebunden werden:

- Automatisches Schließen bei hoher Sonneneinstrahlung
- Energieoptimierte Steuerung basierend auf Temperatur und Sonneneinstrahlung
- Zeitgesteuerte Bewegungsprofile über den Tag
- Einbindung in "Anwesenheit"-Szenarien für erhöhte Sicherheit

Diese Automatisierungen werden direkt in ioBroker konfiguriert und kommunizieren dann über MQTT mit dem ESP32 Solar Monitor.

### Implementierungsdetails

Die Rollladensteuerung ist in folgenden Dateien implementiert:

1. **IoBrokerManager.cpp**: Enthält die Funktionen zur Kommunikation mit ioBroker
   ```cpp
   bool IoBrokerManager::moveRolladen(const String &id, const String &direction) {
     if (id.length() == 0 || direction.length() == 0) {
       return false;
     }
     
     String topic = "esp32solar/rolladen/" + id + "/direction";
     return mqttClient.publish(topic.c_str(), direction.c_str());
   }
   
   bool IoBrokerManager::setRolladenTargetPosition(const String &id, int position) {
     if (id.length() == 0 || position < 0 || position > 100) {
       return false;
     }
     
     String topic = "esp32solar/rolladen/" + id + "/targetPosition";
     return mqttClient.publish(topic.c_str(), String(position).c_str());
   }
   ```

2. **ViewManager.cpp**: Enthält die Benutzeroberfläche für die Rollladensteuerung
   ```cpp
   void ViewManager::showView(const String &functionName) {
     // ...
     if (functionName == "controlRolladen") {
       updateRolladen();
     }
     // ...
   }
   
   void ViewManager::updateRolladen() {
     // Zeichne Benutzeroberfläche für Rollladensteuerung
     // ...
     
     // Zeichne Rolladen-Buttons (R1-R6)
     for (int i = 1; i <= 6; i++) {
       // ...
     }
     
     // Zeichne Steuerungsbuttons (AUF/STOP/AB)
     // ...
     
     // Zeichne Positionsbuttons (0%, 25%, 50%, 75%, 100%)
     // ...
     
     // Zeige aktuellen Status
     // ...
   }
   ```

3. **MqttManager.cpp**: Verarbeitet die eingehenden MQTT-Nachrichten für die Rollläden
   ```cpp
   void MqttManager::messageReceived(char* topic, byte* payload, unsigned int length) {
     // ...
     
     // Rollladen-Status verarbeiten
     if (String(topic).startsWith("esp32solar/rolladen/")) {
       // Extrahiere Rolladen-ID und Parameter
       // Aktualisiere Zustandsdaten
       // ...
     }
     
     // ...
   }
   ```

### Fehlerbehebung bei der Rollladensteuerung

Folgende Probleme können bei der Rollladensteuerung auftreten:

1. **Rollläden reagieren nicht auf Befehle**
   - Stellen Sie sicher, dass die ioBroker-Verbindung aktiv ist (Heartbeat-Icon pulsiert)
   - Überprüfen Sie die MQTT-Topics im ioBroker
   - Prüfen Sie, ob die korrekten Rolladen-IDs verwendet werden

2. **Falsche Positionsanzeige**
   - Kalibrieren Sie den Rolladen in ioBroker neu
   - Stellen Sie sicher, dass die Bewegungsrichtungen korrekt konfiguriert sind
   - Überprüfen Sie die korrekte Berechnung der Bewegungszeit

3. **Rollläden stoppen nicht an der Zielposition**
   - Passen Sie den Parameter `speed_percent_per_second` an die tatsächliche Geschwindigkeit Ihrer Rollläden an
   - Prüfen Sie die BlindControl-Einstellungen in ioBroker

4. **Nicht alle Rollläden werden angezeigt**
   - Überprüfen Sie die `enabled`-Einstellungen in der `config.json`
   - Stellen Sie sicher, dass die entsprechenden ioBroker-Objekte existieren

### Beispiel für ioBroker-Skript zur Rollladensteuerung

```javascript
// ioBroker-Skript: Automatische Rollladensteuerung basierend auf Sonneneinstrahlung
// Dieses Skript steuert die Rollläden abhängig von der Sonneneinstrahlung und Temperatur

// Schwellenwerte
const BRIGHTNESS_THRESHOLD = 40000; // in Lux
const TEMPERATURE_THRESHOLD = 25;   // in °C

// Ausrichtung der Fenster (0 = Nord, 90 = Ost, 180 = Süd, 270 = West)
const WINDOW_ORIENTATION = {
  "1": 90,  // Rolladen 1: Ost
  "2": 90,  // Rolladen 2: Ost
  "3": 180, // Rolladen 3: Süd
  "4": 180, // Rolladen 4: Süd
  "5": 270, // Rolladen 5: West
  "6": 270  // Rolladen 6: West
};

// Aktuelle Werte abrufen
let sunAzimuth = getState('astro.0.sunpos.azimuth').val; // Sonnenrichtung in Grad
let brightness = getState('weather.0.current.brightness').val; // Helligkeit in Lux
let temperature = getState('weather.0.current.temperature').val; // Temperatur in °C

// Verarbeite jeden Rolladen
Object.keys(WINDOW_ORIENTATION).forEach(rolladenId => {
  let windowOrientation = WINDOW_ORIENTATION[rolladenId];
  
  // Prüfe, ob die Sonne auf das Fenster scheint (±45°)
  let sunOnWindow = Math.abs(sunAzimuth - windowOrientation) < 45;
  
  // Aktuelle Rolladenposition
  let currentPosition = getState('mqtt.0.esp32solar.rolladen.' + rolladenId + '.currentPosition').val;
  
  // Entscheidungslogik
  if (sunOnWindow && brightness > BRIGHTNESS_THRESHOLD && temperature > TEMPERATURE_THRESHOLD) {
    // Sonne scheint stark auf das Fenster und es ist warm -> Rolladen schließen (70%)
    if (currentPosition < 70) {
      setState('mqtt.0.esp32solar.rolladen.' + rolladenId + '.targetPosition', 70);
      log('Rolladen ' + rolladenId + ' wird auf 70% geschlossen (Sonnenschutz)');
    }
  } else if (!sunOnWindow || brightness < (BRIGHTNESS_THRESHOLD / 2)) {
    // Keine direkte Sonne oder deutlich weniger hell -> Rolladen öffnen
    if (currentPosition > 0) {
      setState('mqtt.0.esp32solar.rolladen.' + rolladenId + '.targetPosition', 0);
      log('Rolladen ' + rolladenId + ' wird geöffnet');
    }
  }
});
```

### Zukünftige Erweiterungen

Für zukünftige Versionen der Rollladensteuerung sind folgende Funktionen geplant:

1. **Positions-Presets**: Speichern und Abrufen vordefinierter Positionswerte
2. **Zentralsteuerung**: Globale Steuerung aller Rollläden
3. **Szenarien**: Vordefinierte Szenarien wie "Morgen", "Abend", "Kino", etc.
4. **Zeitsteuerung**: Direkte Programmierung von Zeitplänen im ESP32
5. **Sonnenstand-Steuerung**: Automatische Anpassung basierend auf Sonnenposition
6. **Wetterabhängige Steuerung**: Integration von Wetterdaten zur intelligenten Steuerung
7. **Stromverbrauchsoptimierung**: Koordinierte Steuerung in Abhängigkeit vom PV-Überschuss
8. **Verbesserte Visualisierung**: Grafische Darstellung der Rollladen-Positionen

Diese Funktionen werden nach und nach in zukünftigen Updates implementiert werden.

---
## Fehlersuche (Fortsetzung)

### Rollladensteuerung (Fortsetzung)

1. **Problem**: Rollläden reagieren nicht auf Steuerungsbefehle
   - **Lösung 1**: Prüfen Sie die Verbindung zu ioBroker (Heartbeat-Indikator)
   - **Lösung 2**: Stellen Sie sicher, dass die BlindControl-Objekte in ioBroker korrekt konfiguriert sind
   - **Lösung 3**: Überprüfen Sie die MQTT-Topics für die Rollladensteuerung
   - **Lösung 4**: Testen Sie die Steuerung über die Weboberfläche, um Probleme einzugrenzen

2. **Problem**: Rollläden-Positionsanzeige aktualisiert sich nicht
   - **Lösung 1**: Prüfen Sie die MQTT-Topics für die Rollladenpositionen in ioBroker
   - **Lösung 2**: Stellen Sie sicher, dass die ioBroker-Instanz läuft und kommuniziert
   - **Tipp**: Versuchen Sie, die Rollläden manuell aus ioBroker zu steuern, um die Kommunikation zu testen

### Heartbeat-Funktionalität

1. **Problem**: Herz bleibt grau trotz aktiver ioBroker-Instanz
   - **Lösung 1**: Prüfen Sie, ob das Topic `esp32solar/heartbeat/response` in ioBroker korrekt eingerichtet ist
   - **Lösung 2**: Überprüfen Sie das JavaScript für die Heartbeat-Funktion in ioBroker
   - **Lösung 3**: Fügen Sie folgendes Skript in ioBroker ein:
     ```javascript
     on({ id: [].concat(['mqtt.0.esp32solar.heartbeat.beat']), change: 'ne' }, async (obj) => {
       let value = obj.state.val;
       let oldValue = obj.oldState.val;
       setState('mqtt.0.esp32solar.heartbeat.response', (String(getState('mqtt.0.esp32solar.heartbeat.beat').val) + ',ACK'));
     });
     ```
   - **Tipp**: Verwenden Sie die Systeminfo-Seite, um die MQTT-Verbindung zu prüfen

### JSON-Probleme

1. **Problem**: JSON-Konfigurationen werden nicht erkannt
   - **Lösung 1**: Überprüfen Sie die JSON-Syntax mit einem Online-Validator
   - **Lösung 2**: Achten Sie auf korrekte Klammern und Anführungszeichen
   - **Lösung 3**: Vermeiden Sie Sonderzeichen in Strings
   - **Lösung 4**: Verwenden Sie UTF-8-Kodierung für Dateien mit Umlauten

### Bekannte Probleme

- Die WLAN-Verbindung kann unter bestimmten Umständen instabil sein; die Version 0.4.3 enthält robustere Verbindungsroutinen
- Bei einigen Routern kann die Verbindungszeit länger als erwartet sein
- Die Touch-Kalibrierung kann je nach Hardware-Variante leicht abweichen
- Der Access Point Modus funktioniert möglicherweise nicht optimal auf allen Geräten
- Bei großen Konfigurationsdateien kann es zu Speicherengpässen kommen

### Allgemeine Tipps

- Verwenden Sie den seriellen Monitor (115200 Baud) für ausführliche Debug-Informationen
- Bei wiederkehrenden Problemen können Sie einen Reset durchführen
- Nutzen Sie die Systeminfo-Seite für Diagnose der Verbindungen und des Speicherverbrauchs
- Erstellen Sie regelmäßig Backups Ihrer Konfigurationsdateien über die Weboberfläche
- Prüfen Sie vor dem Speichern von Änderungen immer die JSON-Syntax
- Beim ersten Start oder nach einem Reset nutzen Sie den Access Point Modus zur einfachen Konfiguration

### Factory Reset

Wenn alles andere fehlschlägt, können Sie einen Factory Reset durchführen:

1. Über das Menü: Einstellungen → Werkseinstellungen → Bestätigen
2. Über die Weboberfläche: System → "Zurücksetzen auf Werkseinstellungen" klicken
3. Manuell: Halten Sie den Reset-Button während des Einschaltens für 10 Sekunden gedrückt

Nach dem Factory Reset wird der Access Point Modus automatisch aktiviert, um eine Neukonfiguration zu ermöglichen.

---

## Anhang: Erweiterungsmöglichkeiten

Der ESP32 Solar Monitor ist modular aufgebaut und kann leicht um neue Funktionen erweitert werden. Mit ca. 218 KB freiem HEAP-Speicher gibt es noch viel Raum für Erweiterungen.

### Wie alles zusammenarbeitet

#### 1. Startvorgang

1. `setup()` in ESP32_SolarMonitor.ino:
   - SPIFFS-Initialisierung
   - Display & Touch-Initialisierung
   - Konfiguration laden
   - WLAN-Verbindung (oder AP-Modus)
   - MQTT-Verbindung
   - Webserver starten
   - OTA-Updates aktivieren

2. Datenpfad:
   ```
   MQTT-Broker → MqttManager → DataManager → ViewManager → Display
   ```

3. Steuerungspfad:
   ```
   Display → TouchInput → MenuSystem → ViewManager → MqttManager → MQTT-Broker
   ```

### Hinzufügen neuer Funktionen

#### Neuen Menüpunkt hinzufügen

1. Bearbeiten Sie `menu.json`:
   ```json
   {
     "name": "Neue Funktion",
     "function": "showNewFunction",
     "icon": "custom"
   }
   ```

2. Implementieren Sie die Funktion in `ViewManager.cpp`:
   ```cpp
   void ViewManager::showNewFunction() {
     // Display löschen
     tft.fillScreen(BACKGROUND);
     
     // Überschrift zeichnen
     drawHeader("Neue Funktion");
     
     // Eigene Funktionalität implementieren
     tft.setCursor(20, 80);
     tft.setTextColor(TEXT_COLOR);
     tft.println("Dies ist meine neue Funktion");
     
     // Beispiel: Ein einfacher Button
     drawButton(120, 150, 80, 40, "Klick mich", TFT_BLUE);
     
     // Zurück-Button zeichnen
     drawBackButton();
   }
   ```

3. Fügen Sie die Touch-Verarbeitung in `loop()` hinzu:
   ```cpp
   // Im Detailansicht-Abschnitt von loop()
   else if (currentDetailFunction == "showNewFunction") {
     // Prüfen auf Button-Klick
     if (isInBounds(x, y, 120, 150, 200, 190)) {
       // Aktion ausführen
       tft.fillRect(20, 200, 280, 20, BACKGROUND);
       tft.setCursor(20, 200);
       tft.println("Button wurde geklickt!");
     }
   }
   ```

#### Neuen Datentyp hinzufügen

1. Erweitern Sie `SolarData` in `DataManager.h`:
   ```cpp
   struct SolarData {
     // Bestehende Felder...
     
     // Neues Feld
     float newDataPoint;
   };
   ```

2. Fügen Sie einen neuen Topic in `mqtt_topics.json` hinzu:
   ```json
   {
     "name": "new_data_point",
     "topic": "solar_assistant/total/new_data_point/state",
     "description": "Mein neuer Datenpunkt",
     "unit": "X",
     "color": "TFT_GREEN"
   }
   ```

3. Verarbeiten Sie die Daten in `MqttManager.cpp`:
   ```cpp
   void MqttManager::messageReceived(char* topic, byte* payload, unsigned int length) {
     // Bestehende Verarbeitung...
     
     // Neuen Datenpunkt verarbeiten
     else if (strcmp(topicName, "new_data_point") == 0) {
       data.newDataPoint = value.toFloat();
     }
   }
   ```

#### Neues API-Endpoint hinzufügen

1. Fügen Sie ein neues API-Endpoint in `WebServer.cpp` hinzu:
   ```cpp
   server.on("/api/newFunction", HTTP_GET, [](AsyncWebServerRequest *request) {
     JsonDocument doc;
     
     // Daten sammeln
     doc["value"] = dataManager.getData().newDataPoint;
     doc["timestamp"] = millis();
     
     // Weitere Daten...
     
     // Als JSON senden
     String response;
     serializeJson(doc, response);
     request->send(200, "application/json", response);
   });
   ```

### Integration von verschiedenen Schnittstellen

#### 1. RFLink Integration
RFLink ermöglicht die Kommunikation mit 433/868 MHz Funkgeräten:

```cpp
// Beispiel für RFLink-Integration
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

#### 2. Modbus Integration
Modbus wird häufig für die Kommunikation mit Wechselrichtern verwendet:

```cpp
// Beispiel für Modbus-Integration
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

#### 3. OneWire/DS18B20 Temperatursensoren

```cpp
// Temperatursesor-Integration
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

void setupTemperatureSensors() {
  sensors.begin();
}

float readTemperature(int index) {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(index);
}
```

#### 4. API-Anbindung an externe Dienste

```cpp
// Beispiel für Wetter-API-Integration
#include <HTTPClient.h>

String getWeatherData(const String &city) {
  HTTPClient http;
  String url = "https://api.weatherapi.com/v1/current.json?key=YOUR_API_KEY&q=" + city;
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    return payload;
  }
  
  http.end();
  return "";
}
```

### Erweiterung der Anzeigefunktionen

#### 1. Grafiken und Diagramme

```cpp
// Einfaches Balkendiagramm
void drawBarChart(int x, int y, int width, int height, float values[], int numValues, uint16_t barColor) {
  int barWidth = width / numValues;
  float maxValue = 0;
  
  // Maximum finden
  for (int i = 0; i < numValues; i++) {
    if (values[i] > maxValue) maxValue = values[i];
  }
  
  // Rahmen zeichnen
  tft.drawRect(x, y, width, height, TFT_WHITE);
  
  // Balken zeichnen
  for (int i = 0; i < numValues; i++) {
    int barHeight = (values[i] / maxValue) * (height - 10);
    tft.fillRect(x + 5 + i * barWidth, y + height - barHeight - 5, barWidth - 2, barHeight, barColor);
  }
}
```

#### 2. Benutzerdefinierbares Dashboard

```cpp
// Dashboard-Widget-Struktur
struct DashboardWidget {
  String type;         // "gauge", "value", "chart", etc.
  String dataSource;   // Name des Datenpunkts
  int x, y;            // Position
  int width, height;   // Größe
  uint16_t color;      // Farbe
};

// Widget-Rendering-Funktion
void renderWidget(const DashboardWidget &widget) {
  if (widget.type == "gauge") {
    drawGauge(widget.x, widget.y, widget.width, dataManager.getValue(widget.dataSource), widget.color);
  }
  else if (widget.type == "value") {
    drawValueBox(widget.x, widget.y, widget.width, widget.height, dataManager.getValue(widget.dataSource), dataManager.getUnit(widget.dataSource), widget.color);
  }
  // Weitere Widget-Typen...
}
```

### Verbesserung der Energiesimulation

```cpp
// Komplexeres Batterie-Simulationsmodell
float simulateBatteryPower(float pvPower, float loadPower, float currentSOC) {
  float maxChargePower = 5000;  // W
  float maxDischargePower = 5000;  // W
  float chargingEfficiency = 0.95;
  float dischargingEfficiency = 0.95;
  
  // Nettoenergie
  float netPower = pvPower - loadPower;
  
  // Laden/Entladen basierend auf Überschuss/Defizit
  if (netPower > 0) {
    // Überschuss -> Laden
    float chargePower = min(netPower, maxChargePower);
    // Ladeleistung reduzieren, wenn fast voll
    if (currentSOC > 90) {
      chargePower *= (100 - currentSOC) / 10;
    }
    return chargePower * chargingEfficiency;
  } else {
    // Defizit -> Entladen
    float dischargePower = min(-netPower, maxDischargePower);
    // Entladeleistung reduzieren, wenn fast leer
    if (currentSOC < 20) {
      dischargePower *= currentSOC / 20;
    }
    return -dischargePower / dischargingEfficiency;
  }
}
```

### Verbesserung der Benutzeroberfläche

#### 1. Anpassbare Themes

```cpp
// Theme-Struktur
struct UITheme {
  uint16_t backgroundColor;
  uint16_t textColor;
  uint16_t highlightColor;
  uint16_t accentColor;
  uint16_t warningColor;
  uint16_t successColor;
};

// Vordefinierte Themes
UITheme darkTheme = {
  TFT_BLACK,  // Hintergrund
  TFT_WHITE,  // Text
  TFT_CYAN,   // Hervorhebung
  TFT_BLUE,   // Akzent
  TFT_RED,    // Warnung
  TFT_GREEN   // Erfolg
};

UITheme lightTheme = {
  TFT_WHITE,    // Hintergrund
  TFT_BLACK,    // Text
  TFT_NAVY,     // Hervorhebung
  TFT_BLUE,     // Akzent
  TFT_RED,      // Warnung
  TFT_DARKGREEN // Erfolg
};

// Aktives Theme
UITheme currentTheme = darkTheme;
```

#### 2. Animationen

```cpp
// Einfache Animation für Übergänge
void slideTransition(int direction) {
  // direction: 0 = links, 1 = rechts, 2 = oben, 3 = unten
  
  uint16_t screenBuffer[320];  // Puffer für eine Bildschirmzeile/spalte
  
  if (direction == 0 || direction == 1) {
    // Horizontale Animation
    int step = (direction == 0) ? 10 : -10;
    
    for (int x = (direction == 0) ? 0 : 320; (direction == 0) ? x <= 320 : x >= 0; x += step) {
      // Bildschirm nach links/rechts schieben
      for (int y = 0; y < 240; y++) {
        // Zeile einlesen
        for (int i = 0; i < 320; i++) {
          if (i + x >= 0 && i + x < 320) {
            screenBuffer[i] = tft.readPixel(i + x, y);
          } else {
            screenBuffer[i] = BACKGROUND;
          }
        }
        
        // Zeile ausgeben
        for (int i = 0; i < 320; i++) {
          tft.drawPixel(i, y, screenBuffer[i]);
        }
      }
      
      delay(5);  // Kurze Verzögerung für Animation
    }
  }
  // Ähnliches Prinzip für vertikale Animation...
}
```

### Nächste Schritte und geplante Features

# ESP32 Solar Monitor: Realistische Zukunftspläne

Bei einem Standard-ESP32 mit ca. 180 KB freiem Speicher müssen die Zukunftspläne realistisch eingeschätzt werden. Die folgende Tabelle zeigt, welche Funktionen mit den vorhandenen Ressourcen umsetzbar sind:

| Feature | Umsetzbarkeit | Speicheraufwand | Komplexität | Erläuterung |
|---------|---------------|-----------------|-------------|-------------|
| **Verbesserter Access Point Modus** | ✅ Sehr gut | Gering | Niedrig | Erweiterung des bestehenden Codes mit minimalen Speicheranforderungen |
| **MQTT-Discovery** | ✅ Gut | Gering | Niedrig | Implementierung von standardisierten MQTT-Discovery-Paketen |
| **Grundlegende Sensorik** | ✅ Gut | Gering-Mittel | Niedrig | Integration von Temperatur-, Feuchtigkeits- und Lichtsensoren über bestehende Schnittstellen |
| **Einfache Steuerungserweiterungen** | ✅ Gut | Gering-Mittel | Niedrig | Zusätzliche Steuerungsobjekte mit bestehender MQTT-Struktur |
| **Grundlegende Datenanalyse** | ✅ Bedingt | Mittel | Mittel | Einfache Statistiken, gleitende Durchschnitte, Min/Max-Werte |
| **Integration weiterer Energiequellen** | ✅ Bedingt | Gering | Niedrig | Daten-Mapping ohne komplexe Verarbeitung |
| **Optimierte OTA-Updates** | ✅ Bedingt | Mittel | Mittel | Verbesserte Fehlerbehandlung, aber ohne automatisches Backup |
| **Einfache grafische Darstellungen** | ⚠️ Eingeschränkt | Hoch | Mittel | Einfache Balken- und Liniendiagramme mit reduzierter Datenmenge |
| **Multi-Language Support** | ⚠️ Eingeschränkt | Hoch | Mittel | Wenige Sprachen, externe Speicherung der Sprachdateien im SPIFFS |
| **Einfaches Energiemanagement** | ⚠️ Eingeschränkt | Mittel | Hoch | Grundlegende Entscheidungslogik für wenige Verbraucher |
| **Basis-Batteriemonitoring** | ⚠️ Eingeschränkt | Mittel | Mittel | Einfache Statusanalyse ohne komplexe Vorhersagen |
| **Erweitertes Logging** | ❌ Nicht empfohlen | Sehr hoch | Mittel | Nur mit zusätzlicher SD-Karte realistisch umsetzbar |
| **KI-basierte Prognosen** | ❌ Nicht möglich | Extrem hoch | Sehr hoch | Übersteigt die Rechenleistung des ESP32 |
| **Komplexe Dashboards** | ❌ Nicht möglich | Sehr hoch | Hoch | Zu ressourcenintensiv für den begrenzten Speicher |
| **Smartphone-App** | ➡️ Extern | - | - | Muss als separates Projekt entwickelt werden |
| **Mehrbenutzersystem** | ❌ Nicht möglich | Sehr hoch | Hoch | Authentifizierung und Rechteverwaltung überfordern den Speicher |

## Empfehlungen für die Priorität der Umsetzung

### Phase 1: Kurzfristig umsetzbar
1. **Verbesserter Access Point Modus**
   - Erweiterte Konfigurationsmöglichkeiten
   - Verbesserte Captive-Portal-Funktionalität
   - Status-Feedback während der Konfiguration

2. **MQTT-Discovery**
   - Automatische Erkennung in Home Assistant
   - Standard-konformes Discovery-Format
   - Keine manuelle Konfiguration nötig

3. **Grundlegende Sensorik**
   - Integration von bis zu 2-3 Sensoren (DS18B20, DHT22, etc.)
   - Anzeige der Sensorwerte im UI
   - Weiterleitung der Daten über MQTT

### Phase 2: Mittelfristig mit Optimierungen
4. **Einfache Steuerungserweiterungen**
   - Integration weiterer Aktoren (Relais, etc.)
   - Gruppensteuerung für Rollläden
   - Einfache Szenarien (Tag/Nacht, Anwesend/Abwesend)

5. **Optimierte OTA-Updates**
   - Verbesserte Fehlerbehandlung
   - Update-Prüfung vor Installation
   - Rollback-Funktion bei fehlgeschlagenem Update

6. **Einfaches Energiemanagement**
   - Regelbasierte Steuerung von bis zu 3 Verbrauchern
   - Abhängig von PV-Überschuss
   - Einfache Zeitpläne

### Phase 3: Nur mit weiteren Ressourcen
7. **Externe Datenanalyse**
   - Datenübertragung an externe Systeme (Raspberry Pi, Cloud)
   - Auslagerung rechenintensiver Analysen
   - Rückübertragung der Ergebnisse zum ESP32

8. **Modularisierung**
   - Optionale Features, die bei Bedarf aktiviert werden können
   - Dynamisches Ressourcenmanagement
   - Konfigurierbare Build-Optionen für unterschiedliche Hardware-Varianten

## Hardwareerweiterungen für mehr Funktionalität

Einige der komplexeren Features könnten mit folgenden Hardware-Erweiterungen realisiert werden:

| Hardware-Erweiterung | Zusätzliche Funktionalität | Kostenabschätzung |
|----------------------|----------------------------|-------------------|
| **ESP32-S3** | Mehr RAM und CPU-Leistung für komplexere Berechnungen | ~10€ |
| **ESP32 mit PSRAM** | Zusätzlicher Speicher (4-8MB) für Datenverarbeitung | ~12€ |
| **SD-Karte** | Erweitertes Logging, Datenspeicherung, Mehrsprachigkeit | ~5€ plus Adapter |
| **Raspberry Pi** als Ergänzung | Komplexe Analysen, KI, Dashboards, App-Backend | ~50€ und mehr |
| **Zusätzliche Sensoren** | Temperatur, Feuchtigkeit, Licht, Bewegung | ~3-15€ pro Sensor |

## Fazit

Die aktuelle Speicherkapazität von ca. 180 KB freiem RAM ermöglicht fokussierte Erweiterungen der Kernfunktionalität. Für den bestmöglichen Ressourceneinsatz empfehlen wir eine schrittweise Implementierung der Features, beginnend mit denen aus Phase 1. 

Komplexere Funktionen sollten entweder auf leistungsfähigere Hardware ausgelagert oder als optionale Module gestaltet werden, die nicht permanent im Speicher bleiben. Mit einer klugen Ressourcenverwaltung und Modularisierung kann der ESP32 Solar Monitor trotz der Speicherbeschränkungen zu einem leistungsfähigen Steuerungszentrum für Solaranlagen ausgebaut werden.

### Community-Beiträge

Der ESP32 Solar Monitor ist ein offenes Projekt und lebt von Beiträgen der Community. Wenn Sie Verbesserungsvorschläge haben oder an der Entwicklung teilnehmen möchten, finden Sie das Projekt auf GitHub.

Mögliche Beitragsformen:
- Bugfixes und Fehlerbehebungen
- Neue Funktionen und Erweiterungen
- Verbesserungen der Dokumentation
- Übersetzungen der Benutzeroberfläche
- Hardware-Designs und -Anpassungen
- Beispiel-Konfigurationen für verschiedene Solarsysteme

### Abhängigkeiten

Der ESP32 Solar Monitor nutzt folgende Bibliotheken:

- **TFT_eSPI**: Display-Ansteuerung
- **XPT2046_Touchscreen**: Touchscreen-Ansteuerung
- **ArduinoJson**: JSON-Verarbeitung (Version 7.x)
- **PubSubClient**: MQTT-Kommunikation
- **ESPAsyncWebServer**: Webserver-Funktionalität
- **AsyncTCP**: Asynchrone TCP-Kommunikation
- **SPIFFS**: Dateisystemverwaltung
- **DNSServer**: DNS-Server für den Access Point Modus
- **Update**: OTA-Update-Funktionalität
- **WiFi**: WLAN-Konnektivität
- **ESPmDNS**: Multicast DNS für einfache Erreichbarkeit
- **SPI**: SPI-Kommunikation mit Display und Touchscreen

### Fazit

Der ESP32 Solar Monitor ist ein offenes und erweiterbares Projekt zur Überwachung und Steuerung von Solaranlagen und Smart-Home-Geräten. Die modulare Architektur ermöglicht eine einfache Anpassung und Erweiterung. Mit Version 0.4.3 wurden wichtige Funktionen wie Heartbeat-Überwachung, Rollladensteuerung, Webserver-Funktionalität und der Access Point Modus für einfache Erstkonfiguration hinzugefügt, die den Monitor zu einem umfassenden Steuerungszentrum für Ihr smartes Zuhause machen.

Beiträge und Verbesserungsvorschläge sind jederzeit willkommen!

---

_Diese Dokumentation wurde für Version 0.4.3 des ESP32 Solar Monitor erstellt._
