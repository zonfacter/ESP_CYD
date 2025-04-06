# ESP32 Solar Monitor v0.4.1
## Bedienungsanleitung

## Inhaltsverzeichnis
1. [Überblick](#überblick)
2. [Installation](#installation)
3. [Erste Inbetriebnahme](#erste-inbetriebnahme)
4. [Menüstruktur](#menüstruktur)
5. [Einstellungen](#einstellungen)
6. [Datenansichten](#datenansichten)
7. [Steuerungsfunktionen](#steuerungsfunktionen)
8. [Fehlersuche](#fehlersuche)
9. [Anhang: Erweiterungsmöglichkeiten](#anhang-erweiterungsmöglichkeiten)

---

## Überblick

Der ESP32 Solar Monitor ist eine umfassende Lösung zur Visualisierung und Überwachung von Solarsystemen. Diese Version 0.4.1 bietet eine modulare, konfigurierbare Plattform mit einem responsiven Touch-Interface und verschiedenen Detailansichten zur Überwachung aller wichtigen Parameter einer Solaranlage.

**Hauptfunktionen:**
- Echtzeit-Anzeige von PV-Leistung, Batteriestatus, Netzeinspeisung und Verbrauch
- Berechnung der verbleibenden Zeit bis zum Erreichen bestimmter Batterieladestände
- Benutzerfreundliches Touch-Interface mit Tabs und scrollbarem Menü
- Konfigurierbare MQTT-Verbindung zu Solar Assistant oder anderen Monitoring-Systemen
- Simulationsmodus für Demonstrationszwecke ohne tatsächliche Solaranlage

**Technische Spezifikationen:**
- Hardware: ESP32 mit 2,8" TFT-Display und XPT2046 Touchscreen
- WLAN-Konnektivität für drahtlose Integration
- MQTT-Protokoll zur Datenkommunikation
- SPIFFS-Dateisystem für Konfigurationsdateien
- Freier HEAP-Speicher: ca. 218 KB

---

## Installation

### Hardware-Voraussetzungen
- ESP32 Entwicklungsboard
- 2,8" TFT-Display mit ILI9341-Controller
- XPT2046 Touchscreen-Controller
- Micro-USB-Kabel für Programmierung und Stromversorgung

### Software-Installation
1. **Arduino IDE vorbereiten:**
   - Arduino IDE (Version 1.8.19 oder höher) installieren
   - ESP32-Boardunterstützung über den Boardverwalter hinzufügen
   - Folgende Bibliotheken installieren:
     - TFT_eSPI (Version 2.5.43 oder höher)
     - XPT2046_Touchscreen
     - ArduinoJson (Version 7.0.0 oder höher)
     - PubSubClient
     - SPIFFS

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

---

## Erste Inbetriebnahme

1. **Konfigurationsdateien anpassen:**
   - Die Datei `config.json` enthält die grundlegenden Einstellungen für WLAN, MQTT und Display
   - Passen Sie insbesondere diese Einstellungen an:
     ```json
     "wlan": {
       "ssid": "Ihr_WLAN_Name",
       "password": "Ihr_WLAN_Passwort"
     },
     "mqtt": {
       "broker": "IP_Adresse_Ihres_MQTT_Brokers",
       "port": 1883
     },
     "battery": {
       "capacity_ah": 360,
       "nominal_voltage": 51.2,
       "target_soc": 80,
       "min_soc": 20
     }
     ```

2. **Gerät starten:**
   - Nach dem Einschalten verbindet sich der Solar Monitor automatisch mit dem konfigurierten WLAN
   - Anschließend wird eine Verbindung zum MQTT-Broker hergestellt
   - Bei erfolgreicher Verbindung werden Daten in Echtzeit angezeigt
   - Bei fehlgeschlagener Verbindung wird der Simulationsmodus aktiviert

3. **Anzeige prüfen:**
   - Der Hauptbildschirm zeigt das Menü mit verschiedenen Tabs an
   - Die Statusleiste am unteren Bildschirmrand zeigt den Verbindungsstatus

---

## Menüstruktur

Der Solar Monitor verfügt über ein Touch-Menüsystem mit mehreren Tabs und Untermenüs. Hier ist die vollständige Menüstruktur im Überblick:

```
ESP32 Solar Monitor
├── System Tab
│   ├── Solar Status        # Gesamtübersicht des Solarsystems
│   ├── Batterie Status     # Detaillierte Batterieansicht mit Ladezeit-Berechnung
│   ├── Netzstatus          # Netzeinspeisung/-bezug
│   ├── PV Leistung         # Solarmodulleistung und Ertrag
│   ├── Verbrauch           # Stromverbrauch des Hauses
│   ├── Autarkie            # Autarkiegrad der Stromversorgung
│   ├── Tageswerte          # Zusammenfassung der Tageswerte
│   └── Statistik           # Längerfristige statistische Daten
│
├── Steuerung Tab
│   ├── Heizung             # Heizungssteuerung
│   ├── Pool                # Poolpumpensteuerung
│   ├── Garten              # Gartenbewässerung
│   ├── Licht               # Lichtsteuerung
│   ├── Steckdosen          # Schaltbare Steckdosen
│   ├── Lüftung             # Lüftungssteuerung
│   ├── Rollladen           # Rollladensteuerung
│   └── Kameras             # Kameraüberwachung
│
└── Einstellungen Tab
    ├── WLAN Setup          # WLAN-Verbindungskonfiguration
    ├── MQTT Setup          # MQTT-Broker-Einstellungen
    ├── Display             # Display-Einstellungen (Helligkeit, Timeout)
    ├── Systeminfo          # Systeminformationen (Version, Laufzeit, Speicher)
    ├── Updates             # Firmware-Update-Funktion
    ├── Logs                # Systemprotokolle
    ├── Neustart            # System-Neustart
    └── Werkseinstellungen  # Zurücksetzen auf Standardeinstellungen
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
- Firmware-Version (0.4.1)
- CPU-Taktfrequenz
- Freier Speicher (ca. 218 KB)
- Laufzeit seit dem letzten Neustart

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
- **NEU in v0.4.1**: Berechnung der Zeit bis zum Erreichen des Ziel-SOC (beim Laden) oder Min-SOC (beim Entladen)
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

---

## Steuerungsfunktionen

Die Steuerungsfunktionen ermöglichen die Kontrolle verschiedener Haushaltsgeräte und -systeme. In der aktuellen Version 0.4.1 sind die folgenden Steuerungen implementiert:

### Heizung
Grundlegende Steuerung der Heizungsanlage:
- Ein-/Ausschalten der Heizung
- Anzeige des aktuellen Status

### Pool
Steuerung der Poolpumpe:
- Ein-/Ausschalten der Pumpe
- Anzeige des aktuellen Status

Weitere Steuerungsfunktionen sind für zukünftige Updates vorgesehen. Siehe dazu den Anhang über Erweiterungsmöglichkeiten.

---

## Fehlersuche

### WLAN-Verbindungsprobleme
- Überprüfen Sie die SSID und das Passwort in der `config.json`
- Stellen Sie sicher, dass der ESP32 innerhalb der Reichweite Ihres WLAN-Routers ist
- Prüfen Sie, ob Ihr Router 2,4 GHz WLAN unterstützt (5 GHz wird nicht unterstützt)

### MQTT-Verbindungsprobleme
- Überprüfen Sie die IP-Adresse und den Port des MQTT-Brokers
- Stellen Sie sicher, dass der MQTT-Broker läuft und erreichbar ist
- Prüfen Sie die Topic-Konfiguration in `mqtt_topics.json`

### Display-Probleme
- Bei Touch-Problemen können Sie die Kalibrierungswerte in `config.h` anpassen
- Bei Darstellungsproblemen versuchen Sie einen Reset des Geräts

### Allgemeine Probleme
- Wenn der Monitor nicht korrekt funktioniert, versuchen Sie einen Reset
- Bei anhaltenden Problemen können Sie die Werkseinstellungen wiederherstellen
- Überprüfen Sie die Debug-Ausgaben über den seriellen Monitor (115200 Baud)

---

## Anhang: Erweiterungsmöglichkeiten

Der ESP32 Solar Monitor ist modular aufgebaut und kann leicht um neue Funktionen erweitert werden. Mit ca. 218 KB freiem HEAP-Speicher gibt es noch viel Raum für Erweiterungen.

### Erweiterung einer Menüfunktion am Beispiel "Rollladen"

Um einen nicht genutzten Menüpunkt wie "Rollladen" zu implementieren, folgen Sie diesen Schritten:

1. **ViewManager.h erweitern:**
   Fügen Sie Funktionsdeklarationen für die neue Ansicht hinzu:
   ```cpp
   // In der ViewManager.h
   void controlRollladen();
   void updateRollladen();
   ```

2. **ViewManager.cpp erweitern:**
   Implementieren Sie die Funktionen in ViewManager.cpp:
   ```cpp
   void ViewManager::controlRollladen() {
     tft.setTextSize(1);
     tft.setTextColor(TEXT_COLOR, BACKGROUND);
     
     tft.setCursor(20, 70);
     tft.println("Rollladensteuerung");
     
     // Schaltflächen für Rollladen hoch/runter
     drawButton(40, 100, 100, 40, "HOCH", TFT_GREEN);
     drawButton(180, 100, 100, 40, "RUNTER", TFT_RED);
     drawButton(110, 160, 100, 40, "STOP", TFT_BLUE);
     
     // Status anzeigen
     tft.setCursor(20, 210);
     tft.print("Status: ");
     
     // Hier kann der aktuelle Status abgefragt und angezeigt werden
     // z.B. aus einem MQTT-Topic
     String status = mqttManager.getValue("rollladen_status");
     if (status == "up") {
       tft.println("Hochgefahren");
     } else if (status == "down") {
       tft.println("Heruntergefahren");
     } else if (status == "moving") {
       tft.println("In Bewegung");
     } else {
       tft.println("Unbekannt");
     }
   }
   
   void ViewManager::updateRollladen() {
     // Ähnlich wie controlRollladen, aber nur Aktualisierung der Werte
     // ohne komplette Neuzeichnung des Bildschirms
     
     // Status aktualisieren
     tft.fillRect(80, 210, 240, 10, BACKGROUND);
     tft.setCursor(80, 210);
     
     String status = mqttManager.getValue("rollladen_status");
     if (status == "up") {
       tft.println("Hochgefahren");
     } else if (status == "down") {
       tft.println("Heruntergefahren");
     } else if (status == "moving") {
       tft.println("In Bewegung");
     } else {
       tft.println("Unbekannt");
     }
   }
   ```

3. **Funktionen im Konstruktor registrieren:**
   Im Konstruktor von ViewManager die Funktionen registrieren:
   ```cpp
   ViewManager::ViewManager(TFT_eSPI &tft, DataManager &dataManager) 
     : tft(tft), dataManager(dataManager) {
     
     // ... bestehender Code ...
     
     // Neue Funktion registrieren
     viewFunctions["controlRollladen"] = &ViewManager::controlRollladen;
     updateFunctions["controlRollladen"] = &ViewManager::updateRollladen;
   }
   ```

4. **Touch-Funktionalität hinzufügen:**
   Implementieren Sie die Touch-Erkennung für die Buttons:
   ```cpp
   // Diese Funktion wird aufgerufen, wenn in einer Detailansicht ein Touch erkannt wird
   void handleDetailTouch(int x, int y) {
     // ... bestehender Code ...
     
     // Für Rollladensteuerung
     if (currentDetailFunction == "controlRollladen") {
       // Prüfen auf "HOCH"-Button
       if (isInBounds(x, y, 40, 100, 140, 140)) {
         // MQTT-Befehl zum Hochfahren senden
         mqttManager.publish("rollladen/command", "up");
       }
       // Prüfen auf "RUNTER"-Button
       else if (isInBounds(x, y, 180, 100, 280, 140)) {
         // MQTT-Befehl zum Runterfahren senden
         mqttManager.publish("rollladen/command", "down");
       }
       // Prüfen auf "STOP"-Button
       else if (isInBounds(x, y, 110, 160, 210, 200)) {
         // MQTT-Befehl zum Stoppen senden
         mqttManager.publish("rollladen/command", "stop");
       }
     }
   }
   ```

5. **MQTT-Topics hinzufügen:**
   Erweitern Sie die `mqtt_topics.json` um neue Topics für die Rollladensteuerung:
   ```json
   {
     "name": "rollladen_status",
     "topic": "home/rollladen/status",
     "description": "Status der Rollläden",
     "unit": "",
     "color": "TFT_BLUE"
   },
   {
     "name": "rollladen_position",
     "topic": "home/rollladen/position",
     "description": "Position der Rollläden in Prozent",
     "unit": "%",
     "color": "TFT_BLUE"
   }
   ```

6. **MQTT-Publish-Funktion erweitern:**
   Fügen Sie eine Publish-Methode zum MqttManager hinzu:
   ```cpp
   // In MqttManager.h
   bool publish(const String &topic, const String &message);
   
   // In MqttManager.cpp
   bool MqttManager::publish(const String &topic, const String &message) {
     if (!mqttClient.connected()) {
       return false;
     }
     
     return mqttClient.publish(topic.c_str(), message.c_str());
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
  device["model"] = "Solar Monitor v0.4.1";
  device["manufacturer"] = "DIY";
  
  // Serialisieren und veröffentlichen
  String configPayload;
  serializeJson(config, configPayload);
  mqttManager.publish(configTopic, configPayload);
}
```

#### ioBroker Integration
ioBroker kann direkt über MQTT angesprochen werden:

```cpp
// Beispiel für ioBroker-Integration über MQTT
void setupIoBroker() {
  // MQTT-Topics für ioBroker
  mqttManager.subscribe("iobroker_status", "iobroker/status");
  mqttManager.subscribe("iobroker_command", "iobroker/command");
  
  // Status an ioBroker melden
  mqttManager.publish("iobroker/devices/esp32_monitor/info", "{\"version\":\"0.4.1\",\"ip\":\"" + WiFi.localIP().toString() + "\"}");
}
```

Mit diesen Beispielen können Sie den ESP32 Solar Monitor um verschiedene Schnittstellen erweitern und an Ihre spezifischen Bedürfnisse anpassen. Der vorhandene freie HEAP-Speicher von 218 KB bietet ausreichend Raum für mehrere dieser Erweiterungen.
