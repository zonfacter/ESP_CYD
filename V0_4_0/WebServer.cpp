
#include "WebServer.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "DataManager.h"    // Diese Header-Dateien müssen hier eingebunden werden
#include "IoBrokerManager.h" // damit die SolarData-Struktur bekannt ist
#include <DNSServer.h>
#include "ConfigManager.h" 


// Funktionsprototyp für den WLAN-Konfigurationshandler
void setupWifiSaveHandler();

// Externe Deklarationen für die fehlenden Variablen
extern DataManager dataManager;      // Korrekte Deklaration mit Typ
extern IoBrokerManager ioBrokerManager; // Korrekte Deklaration mit Typ

// Globale Instanz
AsyncWebServer server(80);
const byte DNS_PORT = 53;
bool webServerRunning = false;

// Externe Variablen
extern bool wifiConnected;
extern bool mqttConnected;
extern bool ioBrokerConnected;
extern TFT_eSPI tft;
extern ConfigManager configManager;
extern bool apModeActive;


DNSServer dnsServer;
// Globale Variablen für den Neustart
// Entferne oder kommentiere diese Zeilen in WebServer.cpp aus
// bool restartFlag = false;
// unsigned long restartTime = 0;
extern bool restartFlag;
extern unsigned long restartTime;

#ifndef TFT_BLACK
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xFFFF
#define TFT_YELLOW 0xFFE0
#endif

// Hilfsfunktion zum Überprüfen der Dateiendung
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

// HTML-Templates
const char* HTML_HEADER = R"(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Solar Monitor</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f5f5f5;
      color: #333;
    }
    .container {
      max-width: 1000px;
      margin: 0 auto;
      background-color: #fff;
      padding: 20px;
      border-radius: 5px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    }
    h1 {
      color: #2c3e50;
      border-bottom: 2px solid #3498db;
      padding-bottom: 10px;
    }
    h2 {
      color: #2980b9;
      margin-top: 25px;
    }
    ul {
      list-style-type: none;
      padding: 0;
    }
    li {
      margin-bottom: 10px;
      padding: 8px;
      background-color: #f8f9fa;
      border-left: 3px solid #3498db;
    }
    a {
      color: #3498db;
      text-decoration: none;
    }
    a:hover {
      text-decoration: underline;
    }
    .btn {
      background-color: #3498db;
      color: white;
      padding: 8px 16px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 14px;
      margin-right: 5px;
    }
    .btn:hover {
      background-color: #2980b9;
    }
    .btn-danger {
      background-color: #e74c3c;
    }
    .btn-danger:hover {
      background-color: #c0392b;
    }
    form {
      margin: 20px 0;
      padding: 15px;
      background-color: #f8f9fa;
      border-radius: 5px;
    }
    input[type="file"] {
      margin: 10px 0;
    }
    input[type="submit"] {
      background-color: #3498db;
      color: white;
      padding: 8px 16px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
    }
    input[type="submit"]:hover {
      background-color: #2980b9;
    }
    .editor-container {
      display: flex;
      flex-direction: column;
      margin: 20px 0;
    }
    #editor {
      height: 400px;
      border: 1px solid #ddd;
      border-radius: 4px;
    }
    .nav {
      background-color: #2c3e50;
      overflow: hidden;
      margin-bottom: 20px;
      border-radius: 5px;
    }
    .nav a {
      float: left;
      display: block;
      color: white;
      text-align: center;
      padding: 14px 16px;
      text-decoration: none;
    }
    .nav a:hover {
      background-color: #3498db;
    }
    .nav a.active {
      background-color: #3498db;
    }
    .status {
      margin-top: 20px;
      padding: 10px;
      border-radius: 5px;
      background-color: #e8f4fd;
    }
    pre {
      background-color: #f1f1f1;
      padding: 10px;
      border-radius: 4px;
      overflow-x: auto;
    }
    textarea {
      width: 100%;
      height: 300px;
      font-family: monospace;
      padding: 10px;
      border: 1px solid #ddd;
      border-radius: 4px;
      margin-bottom: 10px;
    }
    .card {
      margin: 10px;
      padding: 15px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
      width: 300px;
    }
    .flex-container {
      display: flex;
      flex-wrap: wrap;
    }
    .progress-bar {
      border: 1px solid #ddd;
      height: 30px;
      width: 100%;
      position: relative;
    }
    .progress-fill {
      height: 100%;
      position: relative;
    }
    .progress-text {
      position: absolute;
      top: 5px;
      width: 100%;
      text-align: center;
    }
    .slider {
      width: 100%;
      margin: 10px 0;
    }
        .networks { margin-top: 20px; }
    .network { 
      padding: 10px; 
      background-color: #f1f1f1; 
      margin-bottom: 5px; 
      cursor: pointer; 
      border-left: 3px solid #3498db;
    }
    .network:hover { 
      background-color: #e0e0e0; 
      border-left: 3px solid #2980b9;
    }
    label { 
      display: block; 
      margin-top: 10px; 
      font-weight: bold; 
    }
    input[type="text"], input[type="password"] { 
      width: 100%; 
      padding: 8px; 
      margin-top: 5px; 
      border: 1px solid #ddd; 
      border-radius: 4px;
    }
  </style>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/ace/1.4.12/ace.js"></script>
</head>
<body>
  <div class="container">
    <div class="nav">
      <a href="/">Übersicht</a>
      <a href="/monitor">Solar Monitor</a>
      <a href="/edit">Konfiguration bearbeiten</a>
      <a href="/system">System-Info</a>
      <a href="/update">Firmware-Update</a>
    </div>
)";

const char* HTML_FOOTER_START = R"(
  </div>
  <div class="status">
    <p>ESP32 Solar Monitor - Version: )";

const char* HTML_FOOTER_MIDDLE = R"(</p>
    <p>IP-Adresse: )";

const char* HTML_FOOTER_END = R"(</p>
  </div>
</body>
</html>
)";

// Funktion zum Erzeugen des Footer
String getHtmlFooter() {
  String footer = HTML_FOOTER_START;
  footer += APP_VERSION;
  footer += HTML_FOOTER_MIDDLE;
  footer += WiFi.localIP().toString();
  footer += HTML_FOOTER_END;
  return footer;
}

void setupWebServer() {
  if (!wifiConnected) {
    DEBUG_PRINTLN("WLAN nicht verbunden, Webserver kann nicht gestartet werden");
    return;
  }
  
  // mDNS konfigurieren für einfache Erreichbarkeit
  if (MDNS.begin("solarmonitor")) {
    DEBUG_PRINTLN("mDNS gestartet: http://solarmonitor.local");
  }

  // Startseite
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = HTML_HEADER;
    
    html += "<h1>ESP32 Solar Monitor Konfiguration</h1>";
    html += "<h2>Verfügbare Dateien:</h2><ul>";
    
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
      String fileName = file.name();
      html += "<li><a href='/download?file=" + fileName + "'>" + fileName + "</a> (" + file.size() + " Bytes) ";
      html += "<a href='/edit?file=" + fileName + "' class='btn'>Bearbeiten</a>";
      html += "<a href='/delete?file=" + fileName + "' class='btn btn-danger' onclick='return confirm(\"Datei wirklich löschen?\")'>Löschen</a>";
      html += "</li>";
      file = root.openNextFile();
    }
    
    html += "</ul><h2>Datei hochladen:</h2>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='file'><br><br>";
    html += "<input type='submit' value='Hochladen'>";
    html += "</form>";
    
    html += "<h2>Aktionen:</h2>";
    html += "<p><a href='/restart' class='btn' onclick='return confirm(\"ESP32 wirklich neustarten?\")'>ESP32 neustarten</a></p>";
    
    html += getHtmlFooter();
    request->send(200, "text/html", html);
  });
  
  setupWifiSaveHandler();
  
  // JSON-Editor - mit angepasstem Code für Datei-Parameter
  server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileName = "";
    if(request->hasParam("file")) {
      fileName = request->getParam("file")->value();
    }
    
    // Dateinamen korrigieren
    if (!fileName.startsWith("/") && fileName.length() > 0) {
      fileName = "/" + fileName;
    }
    
    String fileContent = "";
    String html = HTML_HEADER;
    html += "<h1>Datei bearbeiten</h1>";
    
    if (fileName.length() > 0) {
      if (!SPIFFS.exists(fileName)) {
        html += "<p>Datei nicht gefunden: " + fileName + "</p>";
      } else {
        File file = SPIFFS.open(fileName, "r");
        while (file.available()) {
          fileContent += (char)file.read();
        }
        file.close();
        
        html += "<h2>Datei: " + fileName + "</h2>";
        html += "<div class='editor-container'>";
        html += "<div id='editor'>" + fileContent + "</div>";
        html += "</div>";
        html += "<form method='POST' action='/save'>";
        html += "<input type='hidden' name='file' value='" + fileName + "'>";
        html += "<input type='hidden' name='content' id='content'>";
        html += "<input type='submit' value='Speichern' onclick='submitForm()'>";
        html += "</form>";
        html += "<script>";
        html += "var editor = ace.edit('editor');";
        html += "editor.setTheme('ace/theme/monokai');";
        if (fileName.endsWith(".json")) {
          html += "editor.session.setMode('ace/mode/json');";
        } else if (fileName.endsWith(".css")) {
          html += "editor.session.setMode('ace/mode/css');";
        } else if (fileName.endsWith(".js")) {
          html += "editor.session.setMode('ace/mode/javascript');";
        } else if (fileName.endsWith(".html")) {
          html += "editor.session.setMode('ace/mode/html');";
        } else {
          html += "editor.session.setMode('ace/mode/text');";
        }
        html += "function submitForm() {";
        html += "  document.getElementById('content').value = editor.getValue();";
        html += "}";
        html += "</script>";
      }
    } else {
      html += "<p>Keine Datei ausgewählt. <a href='/'>Zurück zur Übersicht</a></p>";
    }
    
    html += getHtmlFooter();
    request->send(200, "text/html", html);
  });
  
  // Datei speichern
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String fileName = "";
    String fileContent = "";
    
    if(request->hasParam("file", true)) {
      fileName = request->getParam("file", true)->value();
    }
    
    if(request->hasParam("content", true)) {
      fileContent = request->getParam("content", true)->value();
    }
    
    if (fileName.length() == 0) {
      request->send(400, "text/plain", "Fehler: Kein Dateiname angegeben");
      return;
    }
    
    File file = SPIFFS.open(fileName, "w");
    if (!file) {
      request->send(500, "text/plain", "Fehler beim Öffnen der Datei zum Schreiben");
      return;
    }
    
    file.print(fileContent);
    file.close();
    
    // Nach bestimmten Dateien automatisch neustarten
    bool needsRestart = fileName == "/config.json";
    
    String html = HTML_HEADER;
    html += "<h1>Datei gespeichert</h1>";
    html += "<p>Die Datei " + fileName + " wurde erfolgreich gespeichert.</p>";
    
    if (needsRestart) {
      html += "<p>Die Änderungen erfordern einen Neustart des ESP32.</p>";
      html += "<p><a href='/restart' class='btn'>Jetzt neustarten</a> oder <a href='/'>Zurück zur Übersicht</a></p>";
    } else {
      html += "<p><a href='/edit?file=" + fileName + "' class='btn'>Weiter bearbeiten</a> oder <a href='/'>Zurück zur Übersicht</a></p>";
    }
    
    html += getHtmlFooter();
    request->send(200, "text/html", html);
  });
  
  // Datei löschen
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileName = "";
    
    if(request->hasParam("file")) {
      fileName = request->getParam("file")->value();
    }
    
    if (!fileName.startsWith("/")) {
      fileName = "/" + fileName;
    }
    
    if (SPIFFS.exists(fileName)) {
      if (SPIFFS.remove(fileName)) {
        DEBUG_PRINTLN("Datei " + fileName + " wurde gelöscht.");
      } else {
        DEBUG_PRINTLN("Fehler beim Löschen der Datei " + fileName);
      }
    } else {
      DEBUG_PRINTLN("Datei " + fileName + " existiert nicht.");
    }
    
    AsyncWebServerResponse *response = request->beginResponse(303);
    response->addHeader("Location", "/");
    request->send(response);
  });
  
  // Datei-Download
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("file")) {
      request->send(400, "text/plain", "Fehler: Parameter 'file' fehlt");
      return;
    }
    
    String fileName = request->getParam("file")->value();
    if (!SPIFFS.exists(fileName)) {
      request->send(404, "text/plain", "Datei nicht gefunden");
      return;
    }
    
    request->send(SPIFFS, fileName, getContentType(fileName));
  });

  // Datei-Upload-Handler
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->redirect("/");
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File uploadFile;
    
    if(!index) {
      // Start des Uploads
      if(!filename.startsWith("/")) {
        filename = "/" + filename;
      }
      DEBUG_PRINT("Upload-Start: ");
      DEBUG_PRINTLN(filename);
      uploadFile = SPIFFS.open(filename, "w");
    }
    
    // Schreibe Daten
    if(uploadFile) {
      uploadFile.write(data, len);
    }
    
    // Upload abgeschlossen
    if(final) {
      if(uploadFile) {
        DEBUG_PRINT("Upload-Ende: ");
        DEBUG_PRINT(uploadFile.size());
        DEBUG_PRINTLN(" Bytes");
        uploadFile.close();
      }
    }
  });

  // WLAN-Konfiguration speichern - gemeinsamer Handler
  setupWifiSaveHandler();

  // Solar Monitor
  server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = HTML_HEADER;
    html += "<h1>Solar Monitor Überwachung</h1>";
    
    // Aktuelle MQTT-Daten anzeigen
    SolarData& data = dataManager.getData();
    
    html += "<div class='flex-container'>";
    
    // Solar Status Karte
    html += "<div class='card'>";
    html += "<h2>Solar Status</h2>";
    html += "<ul>";
    html += "<li>PV Leistung: <span style='color: green;'>" + String(data.pvPower) + " W</span></li>";
    html += "<li>Verbrauch: <span style='color: red;'>" + String(data.loadPower) + " W</span></li>";
    html += "<li>Netz: <span style='color: ";
    html += (data.gridPower < 0 ? "green" : "red");
    html += ";'>" + String(abs(data.gridPower)) + " W ";
    html += (data.gridPower < 0 ? "(Einspeisung)" : "(Bezug)");
    html += "</span></li>";
    html += "<li>Autarkie: <span style='color: cyan;'>" + String(data.autarky) + " %</span></li>";
    html += "</ul>";
    html += "</div>";
    
    // Batterie Status Karte
    html += "<div class='card'>";
    html += "<h2>Batterie Status</h2>";
    html += "<div class='progress-bar'>";
    html += "<div class='progress-fill' style='background-color: " + String(data.batterySOC < 20 ? "red" : (data.batterySOC < 50 ? "orange" : "green")) + "; width: " + String(data.batterySOC) + "%;'></div>";
    html += "<div class='progress-text'>" + String(data.batterySOC) + "%</div>";
    html += "</div><br>";
    html += "<ul>";
    html += "<li>Batterieleistung: <span style='color: " + String(data.batteryPower > 0 ? "green" : "red") + ";'>" + String(abs(data.batteryPower)) + " W " + (data.batteryPower > 0 ? "(Laden)" : "(Entladen)") + "</span></li>";
    html += "<li>Batteriespannung: <span style='color: cyan;'>" + String(data.batteryVoltage) + " V</span></li>";
    html += "</ul>";
    html += "</div>";
    
    // Rolladen-Steuerung
    html += "<div class='card'>";
    html += "<h2>Rolladen-Steuerung</h2>";
    
    // Rolladen-Auswahl
    html += "<div style='margin-bottom: 15px;'>";
    html += "<label>Rolladen auswählen: </label>";
    html += "<select id='rolladenSelect' onchange='updateRolladenInfo()'>";
    for (int i = 1; i <= 6; i++) {
      html += "<option value='" + String(i) + "'>Rolladen " + String(i) + "</option>";
    }
    html += "</select>";
    html += "</div>";
    
    // Rolladen-Steuerung
    html += "<div style='display: flex; justify-content: space-between; margin-bottom: 15px;'>";
    html += "<button class='btn' onclick='controlRolladen(\"up\")'>AUF</button>";
    html += "<button class='btn' onclick='controlRolladen(\"stop\")'>STOP</button>";
    html += "<button class='btn' style='background-color: #e74c3c;' onclick='controlRolladen(\"down\")'>AB</button>";
    html += "</div>";
    
    // Position-Schieberegler
    html += "<div style='margin-bottom: 15px;'>";
    html += "<label>Position: <span id='positionValue'>50</span>%</label><br>";
    html += "<input type='range' min='0' max='100' value='50' class='slider' id='positionSlider' oninput='updatePositionValue()' onchange='setPosition()'>";
    html += "</div>";
    
    // Status-Anzeige
    html += "<div id='rolladenStatus' style='padding: 10px; background-color: #f8f9fa; border-radius: 5px;'>";
    html += "Status: Unbekannt";
    html += "</div>";
    
    // JavaScript für die interaktiven Elemente
    html += "<script>";
    html += "// Aktualisieren der Daten über AJAX";
    html += "function updateData() {";
    html += "  fetch('/api/data')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      document.getElementById('pvPower').textContent = data.pvPower + ' W';";
    html += "      document.getElementById('batterySOC').textContent = data.batterySOC + ' %';";
    html += "      // Weitere Datenfelder...";
    html += "    });";
    html += "}";
    html += "// Daten alle 5 Sekunden aktualisieren";
    html += "setInterval(updateData, 5000);";
    html += "function updatePositionValue() {";
    html += "  document.getElementById('positionValue').innerText = document.getElementById('positionSlider').value;";
    html += "}";
    html += "function setPosition() {";
    html += "  const position = document.getElementById('positionSlider').value;";
    html += "  const rolladenId = document.getElementById('rolladenSelect').value;";
    html += "  fetch('/rolladenPosition?id=' + rolladenId + '&position=' + position)";
    html += "    .then(response => response.text())";
    html += "    .then(data => { document.getElementById('rolladenStatus').innerHTML = data; });";
    html += "}";
    html += "function controlRolladen(direction) {";
    html += "  const rolladenId = document.getElementById('rolladenSelect').value;";
    html += "  fetch('/rolladenControl?id=' + rolladenId + '&direction=' + direction)";
    html += "    .then(response => response.text())";
    html += "    .then(data => { document.getElementById('rolladenStatus').innerHTML = data; });";
    html += "}";
    html += "function updateRolladenInfo() {";
    html += "  const rolladenId = document.getElementById('rolladenSelect').value;";
    html += "  fetch('/rolladenInfo?id=' + rolladenId)";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      document.getElementById('positionSlider').value = data.position;";
    html += "      document.getElementById('positionValue').innerText = data.position;";
    html += "      document.getElementById('rolladenStatus').innerHTML = 'Status: ' + data.status;";
    html += "    });";
    html += "}";
    html += "// Initial update";
    html += "updateRolladenInfo();";
    html += "// Auto-refresh every 5 seconds";
    html += "setInterval(updateRolladenInfo, 5000);";
    html += "</script>";
    html += "</div>";
    
    html += "</div>"; // Ende des Flex-Containers
    
    html += getHtmlFooter();
    request->send(200, "text/html", html);
  });

  // API-Endpunkte für die AJAX-Anfragen
  server.on("/rolladenPosition", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("id") || !request->hasParam("position")) {
      request->send(400, "text/plain", "Fehlende Parameter");
      return;
    }
    
    String id = request->getParam("id")->value();
    int position = request->getParam("position")->value().toInt();
    
    bool success = ioBrokerManager.setRolladenTargetPosition(id, position);
    if (success) {
      request->send(200, "text/plain", "Position auf " + String(position) + "% gesetzt");
    } else {
      request->send(200, "text/plain", "Fehler beim Setzen der Position");
    }
  });

  server.on("/rolladenControl", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("id") || !request->hasParam("direction")) {
      request->send(400, "text/plain", "Fehlende Parameter");
      return;
    }
    
    String id = request->getParam("id")->value();
    String direction = request->getParam("direction")->value();
    
    bool success = ioBrokerManager.moveRolladen(id, direction);
    if (success) {
      String message = "Rolladen ";
      if (direction == "up") message += "fährt hoch";
      else if (direction == "down") message += "fährt runter";
      else message += "gestoppt";
      
      request->send(200, "text/plain", message);
    } else {
      request->send(200, "text/plain", "Fehler bei der Rolladensteuerung");
    }
  });

  server.on("/rolladenInfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("id")) {
      request->send(400, "text/plain", "Fehlende Rolladen-ID");
      return;
    }
    
    String id = request->getParam("id")->value();
    int currentPosition = 0;
    int targetPosition = 0;
    String direction = "";
    bool moving = false;
    
    bool success = ioBrokerManager.getRolladenStatus(id, currentPosition, targetPosition, direction, moving);
    
    String status = "Unbekannt";
    if (success) {
      if (moving) {
        if (direction == "up") status = "Fährt hoch";
        else if (direction == "down") status = "Fährt runter";
        else status = "In Bewegung";
      } else {
        status = "Gestoppt bei " + String(currentPosition) + "%";
      }
    }
    
    String response = "{\"position\":" + String(currentPosition) + ",\"target\":" + String(targetPosition) + ",\"moving\":" + String(moving ? "true" : "false") + ",\"direction\":\"" + direction + "\",\"status\":\"" + status + "\"}";
    request->send(200, "application/json", response);
  });

  server.on("/api/history", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Historische Daten als JSON zurückgeben
    JsonDocument historyDoc;
    
    // Aktuelle Daten
    JsonObject current = historyDoc["current"].to<JsonObject>();
    current["timestamp"] = millis();
    current["batterySOC"] = dataManager.getData().batterySOC;
    current["pvPower"] = dataManager.getData().pvPower;
    // Weitere aktuelle Daten...
    
    // Historische Datenpunkte
    JsonArray points = historyDoc["history"].to<JsonArray>();
    HistoricalData* history = dataManager.getHistoricalData();
    int count = dataManager.getHistoryCount();
    
    for (int i = 0; i < count; i++) {
      if (history[i].timestamp > 0) {
        JsonObject point = points.add<JsonObject>();
        point["timestamp"] = history[i].timestamp;
        point["batterySOC"] = history[i].data.batterySOC;
        point["pvPower"] = history[i].data.pvPower;
        // Weitere historische Daten...
      }
    }
    
    // Statistiken
    JsonObject stats = historyDoc["stats"].to<JsonObject>();
    stats["maxPvToday"] = dataManager.getMaxValueForToday("pv");
    stats["avgAutarkyToday"] = dataManager.getAvgValueForToday("autarky");
    stats["avgBatterySOC"] = dataManager.getAvgValueForToday("soc");
    
    // JSON als Antwort senden
    String response;
    serializeJson(historyDoc, response);
    request->send(200, "application/json", response);
  });

  // System-Infos anzeigen
  server.on("/system", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = HTML_HEADER;
    html += "<h1>System-Informationen</h1>";
    
    html += "<h2>Hardware</h2>";
    html += "<ul>";
    html += "<li>ESP32 Chip-Modell: " + String(ESP.getChipModel()) + "</li>";
    html += "<li>Chip Revision: " + String(ESP.getChipRevision()) + "</li>";
    html += "<li>CPU Frequenz: " + String(ESP.getCpuFreqMHz()) + " MHz</li>";
    html += "<li>Flash Größe: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB</li>";
    html += "<li>MAC-Adresse: " + WiFi.macAddress() + "</li>";
    html += "</ul>";
    
    html += "<h2>Speicher</h2>";
    html += "<ul>";
    html += "<li>Freier Heap: " + String(ESP.getFreeHeap() / 1024) + " KB</li>";
    html += "<li>Min. freier Heap: " + String(ESP.getMinFreeHeap() / 1024) + " KB</li>";
    html += "</ul>";
    
    html += "<h2>Netzwerk</h2>";
    html += "<ul>";
    html += "<li>Verbunden mit: " + String(WiFi.SSID()) + "</li>";
    html += "<li>Signalstärke: " + String(WiFi.RSSI()) + " dBm</li>";
    html += "<li>IP-Adresse: " + WiFi.localIP().toString() + "</li>";
    html += "</ul>";
    
    html += "<h2>Software</h2>";
    html += "<ul>";
    html += "<li>Firmware Version: " + String(APP_VERSION) + "</li>";
    html += "<li>Build Datum: " + String(APP_BUILD_DATE) + "</li>";
    html += "<li>Build Zeit: " + String(APP_BUILD_TIME) + "</li>";
    html += "<li>Laufzeit: " + String(millis() / 1000 / 60) + " Minuten</li>";
    html += "</ul>";
    
    html += "<h2>MQTT-Status</h2>";
    html += "<ul>";
    html += "<li>MQTT verbunden: " + String(mqttConnected ? "Ja" : "Nein") + "</li>";
    html += "<li>ioBroker verbunden: " + String(ioBrokerConnected ? "Ja" : "Nein") + "</li>";
    html += "</ul>";
    
    html += "<h2>SPIFFS</h2>";
    html += "<ul>";
    html += "<li>Gesamtgröße: " + String(SPIFFS.totalBytes() / 1024) + " KB</li>";
    html += "<li>Benutzt: " + String(SPIFFS.usedBytes() / 1024) + " KB</li>";
    html += "<li>Frei: " + String((SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024) + " KB</li>";
    html += "</ul>";
    
    html += getHtmlFooter();
    request->send(200, "text/html", html);
  });

  // Eigener OTA-Update-Handler
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = HTML_HEADER;
    html += "<h1>Firmware-Update</h1>";
    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<p>Aktuelle Firmware: " + String(APP_VERSION) + "</p>";
    html += "<p><input type='file' name='update'></p>";
    html += "<p><input type='submit' value='Update starten'></p>";
    html += "</form>";
    html += getHtmlFooter();
    request->send(200, "text/html", html);
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", 
      shouldReboot ? "Update erfolgreich! Gerät startet neu..." : "Update fehlgeschlagen!");
    response->addHeader("Connection", "close");
    request->send(response);
    if (shouldReboot) {
      delay(1000);
      ESP.restart();
    }
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      DEBUG_PRINTLN("Update starten");
      // Wenn Flash-Update
      int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
        Update.printError(Serial);
        return request->send(400, "text/plain", "OTA-Fehler: " + String(Update.errorString()));
      }
    }

// REST von setupWebServer() (Fortsetzung des Update-Handlers):

    if (Update.write(data, len) != len) {
      Update.printError(Serial);
      return request->send(400, "text/plain", "OTA-Fehler: " + String(Update.errorString()));
    }

    if (final) {
      if (Update.end(true)) {
        DEBUG_PRINTLN("Update erfolgreich: " + String(index + len) + " Bytes");
      } else {
        Update.printError(Serial);
      }
    }
  });

  // ESP32 neustarten
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = HTML_HEADER;
    html += "<h1>ESP32 wird neu gestartet</h1>";
    html += "<p>Der ESP32 wird jetzt neu gestartet. Bitte warte einen Moment...</p>";
    html += "<script>setTimeout(function() { window.location.href = '/'; }, 10000);</script>";
    html += getHtmlFooter();
    request->send(200, "text/html", html);
    delay(1000);
    ESP.restart();
  });

  // 404-Handler
  server.onNotFound([](AsyncWebServerRequest *request) {
    String message = "Datei nicht gefunden\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethode: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArgumente: ";
    message += request->params();
    message += "\n";
    
    for (uint8_t i = 0; i < request->params(); i++) {
      const AsyncWebParameter* p = request->getParam(i);
      message += " " + p->name() + ": " + p->value() + "\n";
    }
    
    request->send(404, "text/plain", message);
  });
  
  // Server starten
  server.begin();
  webServerRunning = true;
  DEBUG_PRINTLN("HTTP-Server gestartet: http://" + WiFi.localIP().toString());
}

// AccessPoint für die WiFi-Konfiguration starten
void startAccessPoint() {
  DEBUG_INFO("Starte Access Point für WLAN-Konfiguration...");
  
  // MAC-Adresse für eindeutige SSID verwenden
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String apSSID = "SolarMonitor-" + mac.substring(mac.length() - 4);
  
  // AP starten (ohne Passwort)
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(apSSID.c_str())) {
    DEBUG_INFO("Access Point erfolgreich gestartet");
  } else {
    DEBUG_ERROR("Fehler beim Starten des Access Points");
  }
  
  IPAddress IP = WiFi.softAPIP();
  DEBUG_INFO("AP IP-Adresse: " + IP.toString());
  
  // DNS-Server für Captive Portal starten
  if (dnsServer.start(DNS_PORT, "*", IP)) {
    DEBUG_INFO("DNS-Server erfolgreich gestartet");
  } else {
    DEBUG_ERROR("Fehler beim Starten des DNS-Servers");
  }
  
  // AP-Modus aktiv setzen
  apModeActive = true;
  
  // Webserver konfigurieren - WICHTIG: Diese Funktion NACH dem Start des AP-Modus aufrufen
  setupWebServerAP();
  
  // Status auf dem Display anzeigen
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 30);
  tft.println("WLAN-Konfiguration");
  tft.setTextSize(1);
  tft.setCursor(20, 70);
  tft.println("Verbinde dich mit:");
  tft.setCursor(20, 90);
  tft.setTextColor(TFT_YELLOW);
  tft.println(apSSID);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(20, 110);
  tft.println("IP: " + IP.toString());
  
  DEBUG_INFO("Access Point Setup abgeschlossen");
}

// Konfiguriert den Webserver für Access Point Modus
void setupWebServerAP() {
  DEBUG_INFO("Konfiguriere WebServer für AP-Modus...");
  
  // Hauptseite für WLAN-Konfiguration
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  DEBUG_INFO("GET / Anfrage empfangen");
  
  if (!apModeActive) {
    request->redirect("/");
    return;
  }
  
  String html = HTML_HEADER;
  
  html += "<h1>ESP32 Solar Monitor - WLAN-Konfiguration</h1>";
  html += "<p>Bitte wähle ein WLAN-Netzwerk aus und gib das Passwort ein.</p>";
  
  // Zwei-Spalten-Layout
  html += "<div style='display: flex; flex-wrap: wrap; gap: 20px;'>";
  
  // Linke Spalte: Netzwerke
  html += "<div style='flex: 1; min-width: 280px;'>";
  html += "<h3>Verfügbare Netzwerke:</h3>";
  html += "<div style='max-height: 300px; overflow-y: auto; border: 1px solid #ddd; border-radius: 4px;'>";
  
  int n = WiFi.scanNetworks();
  if (n == 0) {
    html += "<p style='padding: 10px;'>Keine Netzwerke gefunden</p>";
  } else {
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      String quality = "";
      
      // Signalstärke visualisieren
      if (rssi > -50) quality = "●●●●"; // Sehr gut
      else if (rssi > -65) quality = "●●●○"; // Gut
      else if (rssi > -75) quality = "●●○○"; // Mittel
      else quality = "●○○○"; // Schwach
      
      String encrypted = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "" : " 🔒";
      
      html += "<div onclick='selectNetwork(\"" + ssid + "\")' style='padding: 10px; cursor: pointer; border-bottom: 1px solid #eee;'>";
      html += "<strong>" + ssid + "</strong> " + encrypted + "<br>";
      html += "<span style='color: #3498db;'>" + quality + "</span> (" + rssi + " dBm)";
      html += "</div>";
    }
  }
  html += "</div>"; // Ende Netzwerkliste
  html += "</div>"; // Ende linke Spalte
  
  // Rechte Spalte: Formular
  html += "<div style='flex: 1; min-width: 280px;'>";
  html += "<h3>WLAN-Einstellungen:</h3>";
  html += "<form method='POST' action='/save-wifi' style='background-color: #f8f9fa; padding: 15px; border-radius: 4px;'>";
  html += "<div style='margin-bottom: 15px;'>";
  html += "<label for='ssid' style='display: block; margin-bottom: 5px; font-weight: bold;'>WLAN-Name (SSID):</label>";
  html += "<input type='text' id='ssid' name='ssid' required style='width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px;'>";
  html += "</div>";
  html += "<div style='margin-bottom: 15px;'>";
  html += "<label for='password' style='display: block; margin-bottom: 5px; font-weight: bold;'>WLAN-Passwort:</label>";
  html += "<input type='password' id='password' name='password' style='width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px;'>";
  html += "</div>";
  html += "<div style='margin-top: 20px;'>";
  html += "<input type='submit' value='Speichern und verbinden' style='background-color: #3498db; color: white; padding: 10px 16px; border: none; border-radius: 4px; cursor: pointer; width: 100%;'>";
  html += "</div>";
  html += "</form>";
  html += "</div>"; // Ende rechte Spalte
  
  html += "</div>"; // Ende Flex-Container
  
  // JavaScript unverändert
  html += "<script>";
  html += "function selectNetwork(ssid) {";
  html += "  document.getElementById('ssid').value = ssid;";
  html += "  document.getElementById('password').focus();"; // Fokus auf Passwortfeld setzen
  html += "}";
  html += "</script>";
  
  html += getHtmlFooter();
  request->send(200, "text/html", html);
  DEBUG_INFO("GET / Anfrage beantwortet");
});

  // Im AP-Modus alle unbekannten Anfragen zur Konfigurationsseite umleiten (Captive Portal)
  server.onNotFound([](AsyncWebServerRequest *request) {
    DEBUG_INFO("Unbekannte Anfrage empfangen: " + String(request->url()));
    
    if (apModeActive) {
      // Bei Captive Portal alle Anfragen zur Konfigurationsseite umleiten
      DEBUG_INFO("Leite um zu Captive Portal");
      request->redirect("http://" + WiFi.softAPIP().toString());
    } else {
      // Im normalen Modus die Standard-404-Seite anzeigen
      String message = "Datei nicht gefunden\n\n";
      message += "URI: ";
      message += request->url();
      
      request->send(404, "text/plain", message);
    }
  });
  
  // Webserver starten (falls nicht bereits aktiv)
  if (!webServerRunning) {
    server.begin();
    webServerRunning = true;
    DEBUG_INFO("WebServer im AP-Modus gestartet");
  } else {
    DEBUG_INFO("WebServer läuft bereits");
  }
}

// Eigenständige Funktion für den WLAN-Speicher-Handler
void setupWifiSaveHandler() {
  server.on("/save-wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    DEBUG_INFO("POST /save-wifi Anfrage empfangen");
    
    if (!request->hasParam("ssid", true) || !request->hasParam("password", true)) {
      request->send(400, "text/plain", "SSID oder Passwort fehlt");
      DEBUG_ERROR("SSID oder Passwort fehlt in der Anfrage");
      return;
    }
    
    String ssid = request->getParam("ssid", true)->value();
    String password = request->getParam("password", true)->value();
    
    DEBUG_INFO("Neue WLAN-Einstellungen empfangen: SSID=" + ssid);
    
    // Speichern der Konfiguration
    JsonDocument config;
    
    // Bestehende Konfiguration laden, falls vorhanden
    if (configManager.loadJsonConfig("/config.json", config)) {
      DEBUG_INFO("Bestehende Konfiguration geladen");
    }
    
    // Sicherstellen, dass wlan-Objekt existiert
    if (!config["wlan"].is<JsonObject>()) {
      config["wlan"] = JsonObject();
      DEBUG_INFO("Neues wlan-Objekt in Konfiguration erstellt");
    }
    
    // WLAN-Einstellungen aktualisieren
    config["wlan"]["ssid"] = ssid;
    config["wlan"]["password"] = password;
    
    // Konfiguration speichern
    bool saved = configManager.saveJsonConfig("/config.json", config);
    
    String html = HTML_HEADER;
    
    // HTML-sicheres Encoding der SSID für die Anzeige
    String htmlSafeSSID = ssid;
    htmlSafeSSID.replace("&", "&amp;");
    htmlSafeSSID.replace("<", "&lt;");
    htmlSafeSSID.replace(">", "&gt;");
    htmlSafeSSID.replace("\"", "&quot;");
    htmlSafeSSID.replace("'", "&#039;");
    
    html += "<h1>WLAN-Konfiguration " + String(saved ? "gespeichert" : "fehlgeschlagen") + "</h1>";
    
    if (saved) {
      html += "<p>Der Solar Monitor wird jetzt neu gestartet und versucht, ";
      html += "eine Verbindung mit <strong>" + htmlSafeSSID + "</strong> herzustellen.</p>";
      html += "<p>Wenn die Verbindung fehlschlägt, wird der Access Point wieder aktiviert.</p>";
      html += "<p>Bitte warte einen Moment...</p>";
      html += "<script>setTimeout(function() { window.location.href = '/'; }, 20000);</script>";
    } else {
      html += "<p>Fehler beim Speichern der Konfiguration. Bitte versuche es erneut.</p>";
      html += "<p><a href='/' class='btn'>Zurück zur Konfiguration</a></p>";
    }
    
    html += getHtmlFooter();
    request->send(200, "text/html", html);
    DEBUG_INFO("POST /save-wifi Anfrage beantwortet");
    
    if (saved) {
      // Neustart-Flag setzen und im loop() prüfen
      restartFlag = true;
      restartTime = millis() + 2000;  // 2 Sekunden Verzögerung vor Neustart
      DEBUG_INFO("Neustart in 2 Sekunden angefordert...");
    }
  });
}

void handleWebServer() {
  // DNS-Server für Captive Portal
  if (apModeActive) {
    //DEBUG_INFO("DNS-Server verarbeitet Anfrage...");
    dnsServer.processNextRequest();
    //DEBUG_INFO("DNS-Anfrage verarbeitet");
  }
  
  // Prüfen auf angeforderten Neustart
  if (restartFlag && millis() > restartTime) {
    DEBUG_INFO("Führe angeforderten Neustart durch...");
    restartFlag = false;  // Flag zurücksetzen (für den Fall dass der Neustart fehlschlägt)
    ESP.restart();
  }
}