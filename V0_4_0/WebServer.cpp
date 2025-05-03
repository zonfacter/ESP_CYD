
#include "WebServer.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "config.h"
#include "DataManager.h"    // Diese Header-Dateien müssen hier eingebunden werden
#include "IoBrokerManager.h" // damit die SolarData-Struktur bekannt ist

// Externe Deklarationen für die fehlenden Variablen
extern DataManager dataManager;      // Korrekte Deklaration mit Typ
extern IoBrokerManager ioBrokerManager; // Korrekte Deklaration mit Typ

// Globale Instanz
AsyncWebServer server(80);
bool webServerRunning = false;

// Externe Variablen
extern bool wifiConnected;
extern bool mqttConnected;
extern bool ioBrokerConnected;

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
// Füge einen neuen Menüpunkt für die Überwachung hinzu
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

void handleWebServer() {
  // Nichts zu tun, da der asynchrone Webserver im Hintergrund läuft
}