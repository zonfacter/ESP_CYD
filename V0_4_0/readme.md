# ESP32 Solar Monitor v0.4.3
## Ergänzung zur Bedienungsanleitung

## Inhaltsverzeichnis
1. [Neuerungen in Version 0.4.3](#neuerungen-in-version-043)
2. [Heartbeat-Funktionalität](#heartbeat-funktionalität)
3. [Rollladen-Steuerung](#rollladen-steuerung)
4. [ioBroker-Integration](#iobroker-integration)
5. [Webserver & Fernsteuerung](#webserver--fernsteuerung)
6. [Steuerungsfunktionen über Web-Interface](#steuerungsfunktionen-über-web-interface)
7. [Fehlersuche und Tipps](#fehlersuche-und-tipps)

---

## Neuerungen in Version 0.4.3

Der ESP32 Solar Monitor wurde in Version 0.4.3 um zahlreiche neue Funktionen erweitert, die sowohl die Benutzerfreundlichkeit als auch die Integrationsmöglichkeiten verbessern. Zu den wichtigsten Neuerungen gehören:

- **Heartbeat-Funktionalität**: Visuelle Überwachung der ioBroker-Verbindung
- **Erweiterte Rollladensteuerung**: Vollständige Kontrolle von bis zu 6 Rollläden
- **Verbesserte ioBroker-Integration**: Direktanbindung an BlindControl-Objekte
- **Webserver für Fernzugriff**: Konfiguration und Steuerung über Webbrowser
- **Optimierte Benutzeroberfläche**: Partielle Bildschirmaktualisierung für bessere Reaktionszeit
- **Verbesserte Systemstabilität**: Robustere WLAN-Verbindung und Fehlerbehandlung

Diese Neuerungen machen den ESP32 Solar Monitor zu einer noch leistungsfähigeren Zentrale für Ihr Smart Home mit Solaranlage.

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

Die neue Rollladensteuerung bietet eine umfassende Kontrolle über bis zu 6 Rollläden mit einer intuitiven Benutzeroberfläche.

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

---

## Steuerungsfunktionen über Web-Interface

Das Web-Interface bietet umfangreiche Steuerungsmöglichkeiten für Smart Home-Geräte.

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

## Fehlersuche und Tipps

### Heartbeat-Funktionalität
- **Problem**: Herz bleibt grau trotz aktiver ioBroker-Instanz
  - **Lösung**: Prüfen Sie, ob das Topic `esp32solar/heartbeat/response` in ioBroker korrekt eingerichtet ist
  - **Tipp**: Fügen Sie ein JavaScript in ioBroker ein, das auf `esp32solar/heartbeat/beat` hört und eine Antwort sendet

### Rollladensteuerung
- **Problem**: Rollläden reagieren nicht auf Steuerungsbefehle
  - **Lösung 1**: Prüfen Sie die Verbindung zu ioBroker (Heartbeat-Indikator)
  - **Lösung 2**: Stellen Sie sicher, dass die BlindControl-Objekte in ioBroker korrekt konfiguriert sind
  - **Tipp**: Verwenden Sie die Systeminfo-Seite, um den MQTT-Verbindungsstatus zu überprüfen

- **Problem**: Rollläden-Positionsanzeige aktualisiert sich nicht
  - **Lösung**: Prüfen Sie die MQTT-Topics für die Rollladenpositionen in ioBroker
  - **Tipp**: Versuchen Sie, die Rollläden manuell aus ioBroker zu steuern, um die Kommunikation zu testen

### Webserver
- **Problem**: Webseite nicht erreichbar
  - **Lösung 1**: Prüfen Sie die IP-Adresse auf dem Display
  - **Lösung 2**: Stellen Sie sicher, dass sich Ihr Gerät im selben Netzwerk befindet
  - **Tipp**: Verwenden Sie die mDNS-Adresse http://solarmonitor.local, falls möglich

- **Problem**: Änderungen an der Konfiguration werden nicht übernommen
  - **Lösung**: Starten Sie den ESP32 nach dem Speichern der Konfiguration neu
  - **Tipp**: Erstellen Sie vor größeren Änderungen ein Backup der Konfigurationsdateien

### Allgemeine Tipps
- **Performance**: Die Anzeige wird nun teilweise aktualisiert statt komplett neu gezeichnet, was die Reaktionszeit verbessert
- **Speichernutzung**: Nutzen Sie den Webserver, um den freien Speicher zu überwachen
- **Backup**: Laden Sie regelmäßig Ihre Konfigurationsdateien über die Weboberfläche herunter
- **Stabilität**: Wenn Probleme auftreten, prüfen Sie zuerst die WLAN-Signalstärke

---

Diese Anleitung ergänzt die bestehende Dokumentation für den ESP32 Solar Monitor und erklärt die neuen Funktionen der Version 0.4.3. Für grundlegende Einstellungen und Bedienung konsultieren Sie bitte die Hauptanleitung der Version 0.4.1.
