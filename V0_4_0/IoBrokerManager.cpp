// IoBrokerManager.cpp
#include "IoBrokerManager.h"
#include "ConfigManager.h"
#include "config.h"

// Globale Instanz
IoBrokerManager ioBrokerManager;

// Statischer Callback, der an den MQTT-Client weitergegeben wird
void IoBrokerManager::staticCallback(char* topic, byte* payload, unsigned int length) {
  // Weiterleitung an die Instanzmethode
  ioBrokerManager.handleCallback(topic, payload, length);
}

void IoBrokerManager::handleCallback(char* topic, byte* payload, unsigned int length) {
  // Konvertiere Payload zu String
  String message;
  message.reserve(length);
  
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  DEBUG_PRINT("ioBroker MQTT Nachricht [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("]: ");
  DEBUG_PRINTLN(message);
  
  // Topic analysieren und entsprechend verarbeiten
  String topicStr = String(topic);


if (topicStr.startsWith("0_userdata.0.BlindControl.Shutter.")) {
  // Topic-Format analysieren
  int rolladenIdPos = topicStr.indexOf(".Shutter.") + 9;
  int nextDotPos = topicStr.indexOf(".", rolladenIdPos);
  
  if (rolladenIdPos > 9 && nextDotPos > rolladenIdPos) {
    String rolladenId = topicStr.substring(rolladenIdPos, nextDotPos);
    
    // Prüfen, welcher Datenpunkt betroffen ist
    if (topicStr.endsWith("Input.obj_I_var_Target_Level")) {
      // Target Level (Zielposition) - Value ist ein Prozentsatz (0-100)
      int targetPosition = constrain(message.toInt(), 0, 100);
      
      // Suche nach dem entsprechenden Rolladen
      bool found = false;
      for (size_t i = 0; i < rolladenStatus.size(); i++) {
        if (rolladenStatus[i].id == rolladenId) {
          rolladenStatus[i].targetPosition = targetPosition;
          rolladenStatus[i].lastUpdate = millis();
          found = true;
          
          // Wenn die aktuelle Position ungleich der Zielposition ist, Status auf "in Bewegung" setzen
          if (rolladenStatus[i].currentPosition != targetPosition && !rolladenStatus[i].moving) {
            rolladenStatus[i].moving = true;
            rolladenStatus[i].direction = (targetPosition < rolladenStatus[i].currentPosition) ? "up" : "down";
            rolladenStatus[i].moveStartTime = millis();
            rolladenStatus[i].moveStartPosition = rolladenStatus[i].currentPosition;
          }
          break;
        }
      }
      
      // Wenn Rolladen nicht gefunden, neu anlegen
      if (!found) {
        RolladenStatus newStatus(rolladenId);
        newStatus.targetPosition = targetPosition;
        newStatus.lastUpdate = millis();
        rolladenStatus.push_back(newStatus);
      }
      
      DEBUG_PRINT("Rolladen ");
      DEBUG_PRINT(rolladenId);
      DEBUG_PRINT(" Zielposition: ");
      DEBUG_PRINTLN(targetPosition);
    }
    else if (topicStr.endsWith("Output.obj_Q_LEVEL")) {
      // Aktuelles Level (Ist-Position) - Value ist ein Prozentsatz (0-100)
      int currentPosition = constrain(message.toInt(), 0, 100);
      
      // Suche nach dem entsprechenden Rolladen
      bool found = false;
      for (size_t i = 0; i < rolladenStatus.size(); i++) {
        if (rolladenStatus[i].id == rolladenId) {
          rolladenStatus[i].currentPosition = currentPosition;
          rolladenStatus[i].lastUpdate = millis();
          
          // Wenn Soll- und Ist-Position gleich sind, Bewegung beenden
          if (rolladenStatus[i].targetPosition == currentPosition && rolladenStatus[i].moving) {
            rolladenStatus[i].moving = false;
            rolladenStatus[i].direction = "stop";
          }
          
          found = true;
          break;
        }
      }
      
      // Wenn Rolladen nicht gefunden, neu anlegen
      if (!found) {
        RolladenStatus newStatus(rolladenId);
        newStatus.currentPosition = currentPosition;
        newStatus.targetPosition = currentPosition; // Initiale Annahme
        newStatus.lastUpdate = millis();
        rolladenStatus.push_back(newStatus);
      }
      
      DEBUG_PRINT("Rolladen ");
      DEBUG_PRINT(rolladenId);
      DEBUG_PRINT(" aktuelle Position: ");
      DEBUG_PRINTLN(currentPosition);
    }
    else if (topicStr.endsWith("Output.obj_Q_BUSY")) {
      // Busy-Status - Value ist "true" oder "false" bzw. "1" oder "0"
      bool isBusy = (message == "true" || message == "1");
      
      // Suche nach dem entsprechenden Rolladen
      bool found = false;
      for (size_t i = 0; i < rolladenStatus.size(); i++) {
        if (rolladenStatus[i].id == rolladenId) {
          rolladenStatus[i].moving = isBusy;
          rolladenStatus[i].lastUpdate = millis();
          
          // Wenn nicht mehr busy, auf "stop" setzen
          if (!isBusy) {
            rolladenStatus[i].direction = "stop";
          }
          
          found = true;
          break;
        }
      }
      
      // Wenn Rolladen nicht gefunden, neu anlegen
      if (!found) {
        RolladenStatus newStatus(rolladenId);
        newStatus.moving = isBusy;
        newStatus.lastUpdate = millis();
        rolladenStatus.push_back(newStatus);
      }
      
      DEBUG_PRINT("Rolladen ");
      DEBUG_PRINT(rolladenId);
      DEBUG_PRINT(" Bewegungsstatus: ");
      DEBUG_PRINTLN(isBusy ? "In Bewegung" : "Gestoppt");
    }
  }
}
  // Format des Topics analysieren
  if (topicStr.indexOf(baseTopic) >= 0) {
    // Gerät und Befehl aus Topic extrahieren
    String relativeTopic = topicStr.substring(baseTopic.length() + 1); // Entfernt baseTopic + '/'
    int firstSlash = relativeTopic.indexOf('/');
    int secondSlash = relativeTopic.indexOf('/', firstSlash + 1);
    
    if (firstSlash > 0 && secondSlash > firstSlash) {
      String deviceType = relativeTopic.substring(0, firstSlash);
      String deviceId = relativeTopic.substring(firstSlash + 1, secondSlash);
      String command = relativeTopic.substring(secondSlash + 1);
      
      DEBUG_PRINT("Gerätetyp: ");
      DEBUG_PRINTLN(deviceType);
      DEBUG_PRINT("Geräte-ID: ");
      DEBUG_PRINTLN(deviceId);
      DEBUG_PRINT("Befehl: ");
      DEBUG_PRINTLN(command);
      
      // Verarbeite verschiedene Gerätetypen
      if (deviceType == "light") {
        // Lichtsteuerung
        if (command == "status") {
          // Status-Update für ein Licht
          bool found = false;
          for (auto& ls : lightStatus) {
            if (ls.id == deviceId) {
              ls.status = message;
              ls.lastUpdate = millis();
              found = true;
              break;
            }
          }
          
          if (!found) {
            DeviceStatus newStatus(deviceId);
            newStatus.status = message;
            newStatus.lastUpdate = millis();
            lightStatus.push_back(newStatus);
          }
        }
      } 
      else if (deviceType == "rolladen") {
        // Rolladensteuerung
        bool found = false;
        int rolladenIndex = -1;
        
        // Suche nach bestehendem Rolladen
        for (size_t i = 0; i < rolladenStatus.size(); i++) {
          if (rolladenStatus[i].id == deviceId) {
            found = true;
            rolladenIndex = i;
            break;
          }
        }
        
        // Wenn nicht gefunden, neuen Rolladen hinzufügen
        if (!found) {
          RolladenStatus newStatus(deviceId);
          rolladenStatus.push_back(newStatus);
          rolladenIndex = rolladenStatus.size() - 1;
        }
        
        // Status-Updates verarbeiten
        if (command == "currentPosition") {
          // Aktuelle Position (Ist-Position) (0-100%)
          int position = message.toInt();
          position = constrain(position, 0, 100);
          
          rolladenStatus[rolladenIndex].currentPosition = position;
          rolladenStatus[rolladenIndex].lastUpdate = millis();
          
          // Wenn die Ist-Position gleich der Soll-Position ist und sich der Rolladen bewegt, 
          // setzen wir moving auf false
          if (rolladenStatus[rolladenIndex].moving && 
              rolladenStatus[rolladenIndex].currentPosition == rolladenStatus[rolladenIndex].targetPosition) {
            rolladenStatus[rolladenIndex].moving = false;
            rolladenStatus[rolladenIndex].direction = "stop";
            // Senden eines Stop-Befehls über MQTT (zur Synchronisierung)
            String directionTopic = getRolladenTopic(deviceId) + "/direction";
            ioMqttClient.publish(directionTopic.c_str(), "stop");
          }
          
          DEBUG_PRINT("Rolladen ");
          DEBUG_PRINT(deviceId);
          DEBUG_PRINT(" Ist-Position gesetzt auf: ");
          DEBUG_PRINTLN(position);
        }
        else if (command == "targetPosition") {
          // Soll-Position (0-100%)
          int position = message.toInt();
          position = constrain(position, 0, 100);
          
          rolladenStatus[rolladenIndex].targetPosition = position;
          rolladenStatus[rolladenIndex].lastUpdate = millis();
          
          // Wenn Target != Current Position, dann Bewegung initiieren
          if (rolladenStatus[rolladenIndex].targetPosition != rolladenStatus[rolladenIndex].currentPosition) {
            // Richtung bestimmen und Bewegung starten
            // WICHTIG: Bei dir ist 0% offen (oben) und 100% geschlossen (unten)
            // Daher ist "up", wenn die Zielposition kleiner ist als die aktuelle Position
            String newDirection = (rolladenStatus[rolladenIndex].targetPosition < 
                                   rolladenStatus[rolladenIndex].currentPosition) ? "up" : "down";
            
            rolladenStatus[rolladenIndex].direction = newDirection;
            rolladenStatus[rolladenIndex].moving = true;
            rolladenStatus[rolladenIndex].moveStartTime = millis();
            rolladenStatus[rolladenIndex].moveStartPosition = rolladenStatus[rolladenIndex].currentPosition;
            
            // Senden der Richtung über MQTT
            String directionTopic = getRolladenTopic(deviceId) + "/direction";
            ioMqttClient.publish(directionTopic.c_str(), newDirection.c_str());
            
            // Senden des Bewegungsstatus über MQTT
            String movingTopic = getRolladenTopic(deviceId) + "/moving";
            ioMqttClient.publish(movingTopic.c_str(), "true");
          }
          
          DEBUG_PRINT("Rolladen ");
          DEBUG_PRINT(deviceId);
          DEBUG_PRINT(" Soll-Position gesetzt auf: ");
          DEBUG_PRINTLN(position);
        }
        else if (command == "direction") {
          // Richtungsupdate (up/down/stop)
          String oldDirection = rolladenStatus[rolladenIndex].direction;
          rolladenStatus[rolladenIndex].direction = message;
          
          // Wenn Richtungsänderung, aktualisiere Bewegungsstart
          if (oldDirection != message) {
            rolladenStatus[rolladenIndex].moveStartTime = millis();
            rolladenStatus[rolladenIndex].moveStartPosition = rolladenStatus[rolladenIndex].currentPosition;
          }
          
          // Bewegungsstatus setzen
          rolladenStatus[rolladenIndex].moving = (message != "stop");
          rolladenStatus[rolladenIndex].lastUpdate = millis();
          
          // Wenn Stop, dann setze Target = Current
          if (message == "stop") {
            rolladenStatus[rolladenIndex].targetPosition = rolladenStatus[rolladenIndex].currentPosition;
            
            // Aktualisiere TargetPosition in ioBroker
            String targetPosTopic = getRolladenTopic(deviceId) + "/targetPosition";
            String targetValue = String(rolladenStatus[rolladenIndex].targetPosition);
            ioMqttClient.publish(targetPosTopic.c_str(), targetValue.c_str(), true);
          }
          
          DEBUG_PRINT("Rolladen ");
          DEBUG_PRINT(deviceId);
          DEBUG_PRINT(" Richtung gesetzt auf: ");
          DEBUG_PRINTLN(message);
        }
        else if (command == "moving") {
          // Bewegungsstatus (true/false)
          bool isMoving = (message == "true" || message == "1");
          
          // Wenn Änderung im Bewegungsstatus
          if (rolladenStatus[rolladenIndex].moving != isMoving) {
            rolladenStatus[rolladenIndex].moving = isMoving;
            
            // Wenn Stopp, dann setze Direction=stop und Target=Current
            if (!isMoving) {
              rolladenStatus[rolladenIndex].direction = "stop";
              rolladenStatus[rolladenIndex].targetPosition = rolladenStatus[rolladenIndex].currentPosition;
              
              // Aktualisiere TargetPosition in ioBroker
              String targetPosTopic = getRolladenTopic(deviceId) + "/targetPosition";
              String targetValue = String(rolladenStatus[rolladenIndex].targetPosition);
              ioMqttClient.publish(targetPosTopic.c_str(), targetValue.c_str(), true);
              
              // Aktualisiere Direction in ioBroker
              String directionTopic = getRolladenTopic(deviceId) + "/direction";
              ioMqttClient.publish(directionTopic.c_str(), "stop", true);
            }
          }
          
          rolladenStatus[rolladenIndex].lastUpdate = millis();
          
          DEBUG_PRINT("Rolladen ");
          DEBUG_PRINT(deviceId);
          DEBUG_PRINT(" Bewegungsstatus gesetzt auf: ");
          DEBUG_PRINTLN(rolladenStatus[rolladenIndex].moving);
        }
        else if (command == "speed") {
          // Geschwindigkeit des Rollladens in % pro Sekunde
          int speed = message.toInt();
          speed = constrain(speed, 1, 20); // Begrenze auf sinnvolle Werte
          
          rolladenStatus[rolladenIndex].rolladenSpeed = speed;
          rolladenStatus[rolladenIndex].lastUpdate = millis();
          
          DEBUG_PRINT("Rolladen ");
          DEBUG_PRINT(deviceId);
          DEBUG_PRINT(" Geschwindigkeit gesetzt auf: ");
          DEBUG_PRINTLN(speed);
        }
        else if (command == "calibrate") {
          // Kalibrierungsbefehl
          bool calibrate = (message == "true" || message == "1");
          
          if (calibrate) {
            rolladenStatus[rolladenIndex].calibrated = true;
            
            // Sende Bestätigung zurück
            String calibratedTopic = getRolladenTopic(deviceId) + "/calibrated";
            ioMqttClient.publish(calibratedTopic.c_str(), "true", true);
            
            DEBUG_PRINT("Rolladen ");
            DEBUG_PRINT(deviceId);
            DEBUG_PRINTLN(" wurde kalibriert");
          }
        }
      }
      // Weitere Gerätetypen hier implementieren (Heizung, Pool, etc.)
      else if (deviceType == "heating") {
        // Heizungssteuerung
        if (command == "status") {
          // Status-Update für eine Heizung
          bool found = false;
          for (auto& hs : heatingStatus) {
            if (hs.id == deviceId) {
              hs.status = message;
              hs.lastUpdate = millis();
              found = true;
              break;
            }
          }
          
          if (!found) {
            HeatingStatus newStatus(deviceId);
            newStatus.status = message;
            newStatus.lastUpdate = millis();
            heatingStatus.push_back(newStatus);
          }
        }
        else if (command == "temperature") {
          // Temperatur-Update für eine Heizung
          bool found = false;
          for (auto& hs : heatingStatus) {
            if (hs.id == deviceId) {
              hs.temperature = message.toFloat();
              hs.lastUpdate = millis();
              found = true;
              break;
            }
          }
          
          if (!found) {
            HeatingStatus newStatus(deviceId);
            newStatus.temperature = message.toFloat();
            newStatus.lastUpdate = millis();
            heatingStatus.push_back(newStatus);
          }
        }
      }
      else if (deviceType == "pool") {
        // Poolsteuerung
        if (command == "status") {
          // Status-Update für einen Pool
          bool found = false;
          for (auto& ps : poolStatus) {
            if (ps.id == deviceId) {
              ps.status = message;
              ps.lastUpdate = millis();
              found = true;
              break;
            }
          }
          
          if (!found) {
            DeviceStatus newStatus(deviceId);
            newStatus.status = message;
            newStatus.lastUpdate = millis();
            poolStatus.push_back(newStatus);
          }
        }
      }
    }
  }
  // Innerhalb der bestehenden handleCallback-Methode, nach der topic-Analyse
  if (topicStr == baseTopic + "/heartbeat/response") {
    // Heartbeat-Antwort von ioBroker empfangen
    heartbeatReceived = true;
    lastHeartbeatReceived = millis();
    DEBUG_PRINT("Heartbeat von ioBroker empfangen: ");
    DEBUG_PRINTLN(message);
  }
}

IoBrokerManager::IoBrokerManager(const String &baseTopic)
  : ioMqttClient(ioWifiClient), baseTopic(baseTopic) {
  // Verwende die letzten 4 Zeichen der MAC-Adresse für die Client-ID
  // Temporäre Client-ID setzen, wird in begin() überschrieben
  clientId = "ESP32Solar_ioBroker_0000";
  
  DEBUG_PRINT("ioBroker Client-ID: ");
  DEBUG_PRINTLN(clientId);
}

bool IoBrokerManager::begin(const String &broker, 
                           int port, 
                           const String &username, 
                           const String &password) {
  // MQTT-Verbindungsdaten für ioBroker speichern
  this->broker = broker;
  this->port = port;
  this->username = username;
  this->password = password;
  
  // Verwende die letzten 4 Zeichen der MAC-Adresse für die Client-ID
  String mac = WiFi.macAddress();
  mac.replace(":", ""); // Entferne die Doppelpunkte
  clientId = "ESP32Solar_ioBroker_" + mac.substring(mac.length() - 4);
  

  // Mit ioBroker verbinden
  DEBUG_PRINTLN("Verbinde mit ioBroker MQTT-Manager...");
  DEBUG_PRINT("Broker: ");
  DEBUG_PRINTLN(broker);
  DEBUG_PRINT("Port: ");
  DEBUG_PRINTLN(port);
  DEBUG_PRINT("Client-ID: ");
  DEBUG_PRINTLN(clientId);
  
  // MQTT-Client konfigurieren
  ioMqttClient.setServer(broker.c_str(), port);
  ioMqttClient.setCallback(staticCallback);
  ioMqttClient.setSocketTimeout(120);  // Erhöhen des Timeouts auf 120 Sekunden
  ioMqttClient.setKeepAlive(120);     // 2 Minuten

  // Verbindung versuchen
  if (connectMqtt()) {
    DEBUG_PRINTLN("ioBroker MQTT verbunden!");
    
    // Basis-Topics abonnieren
    // 1. Generisches Status-Topic für alle Geräte
    String statusTopic = baseTopic + "/+/+/status";
    ioMqttClient.subscribe(statusTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(statusTopic);
    
    // 2. Rolladen-spezifische Topics
    String rolladenCurrentPositionTopic = baseTopic + "/rolladen/+/currentPosition";
    ioMqttClient.subscribe(rolladenCurrentPositionTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(rolladenCurrentPositionTopic);
    
    String rolladenTargetPositionTopic = baseTopic + "/rolladen/+/targetPosition";
    ioMqttClient.subscribe(rolladenTargetPositionTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(rolladenTargetPositionTopic);
    
    String rolladenDirectionTopic = baseTopic + "/rolladen/+/direction";
    ioMqttClient.subscribe(rolladenDirectionTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(rolladenDirectionTopic);
    
    String rolladenMovingTopic = baseTopic + "/rolladen/+/moving";
    ioMqttClient.subscribe(rolladenMovingTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(rolladenMovingTopic);
    
    String rolladenSpeedTopic = baseTopic + "/rolladen/+/speed";
    ioMqttClient.subscribe(rolladenSpeedTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(rolladenSpeedTopic);
    
    String rolladenCalibrateTopic = baseTopic + "/rolladen/+/calibrate";
    ioMqttClient.subscribe(rolladenCalibrateTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(rolladenCalibrateTopic);
    
    // 3. Heizung-spezifische Topics
    String heatingStatusTopic = baseTopic + "/heating/+/status";
    ioMqttClient.subscribe(heatingStatusTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(heatingStatusTopic);
    
    String heatingTemperatureTopic = baseTopic + "/heating/+/temperature";
    ioMqttClient.subscribe(heatingTemperatureTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(heatingTemperatureTopic);
    
    // 4. Pool-spezifische Topics
    String poolStatusTopic = baseTopic + "/pool/+/status";
    ioMqttClient.subscribe(poolStatusTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(poolStatusTopic);
    
    // 5. NEUE FUNKTION: Direkte ioBroker Rolladen-Topics abonnieren
    subscribeRolladenTopics();
    // Initialen Status veröffentlichen
    broadcastStatus();
    
    // Geräteinformationen veröffentlichen
    publishDeviceInfo();
    
    return true;
  } else {
    DEBUG_PRINTLN("ioBroker MQTT-Verbindung fehlgeschlagen!");
    return false;
  }
}

// Verbindungsaufbau mit Authentifizierung
bool IoBrokerManager::connectMqtt() {
  bool result = false;
  int retryCount = 0;
  
  while (!ioMqttClient.connected() && retryCount < 3) {
    DEBUG_PRINTLN("Verbindungsversuch " + String(retryCount + 1) + " von 3...");
    
    // Verbindungsoptionen basierend auf Authentifizierung
    if (username.length() > 0 && password.length() > 0) {
      DEBUG_PRINTLN("Verbinde mit Authentifizierung");
      result = ioMqttClient.connect(clientId.c_str(), username.c_str(), password.c_str());
    } else {
      DEBUG_PRINTLN("Verbinde ohne Authentifizierung");
      result = ioMqttClient.connect(clientId.c_str());
    }
    
    if (result) {
      connected = true;
      break;
    } else {
      retryCount++;
      DEBUG_PRINT("Fehler beim Verbinden, rc=");
      DEBUG_PRINTLN(ioMqttClient.state());
      DEBUG_PRINT("MQTT Status: ");
      DEBUG_PRINTLN(getMqttStateDescription(ioMqttClient.state()));
      delay(1000);  // Kurze Pause zwischen den Versuchen
    }
  }
  
  if (connected) {
    // Bestehende Topic-Abonnements
    
    // Heartbeat-Topic abonnieren
    String heartbeatResponseTopic = baseTopic + "/heartbeat/response";
    ioMqttClient.subscribe(heartbeatResponseTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(heartbeatResponseTopic);
    
    // Initialen Heartbeat senden
    lastHeartbeatTime = 0; // Erzwingt sofortiges Senden beim nächsten Update
    heartbeatCounter = 0;
    heartbeatReceived = false;
    lastHeartbeatReceived = millis();
  }

  return connected;
}

String IoBrokerManager::getMqttStateDescription(int state) {
  switch (state) {
    case -4: return "MQTT_CONNECTION_TIMEOUT";
    case -3: return "MQTT_CONNECTION_LOST";
    case -2: return "MQTT_CONNECT_FAILED";
    case -1: return "MQTT_DISCONNECTED";
    case 0: return "MQTT_CONNECTED";
    case 1: return "MQTT_CONNECT_BAD_PROTOCOL";
    case 2: return "MQTT_CONNECT_BAD_CLIENT_ID";
    case 3: return "MQTT_CONNECT_UNAVAILABLE";
    case 4: return "MQTT_CONNECT_BAD_CREDENTIALS";
    case 5: return "MQTT_CONNECT_UNAUTHORIZED";
    default: return "UNKNOWN_STATE";
  }
}

void IoBrokerManager::update() {
  // MQTT-Verbindung prüfen und wiederherstellen
  if (!ioMqttClient.connected()) {
    connected = false;
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt > 15000) {  // Alle 15 Sekunden versuchen
      lastReconnectAttempt = now;
      
      DEBUG_PRINTLN("ioBroker MQTT-Manager Verbindung verloren, versuche erneut...");
      
      if (connectMqtt()) {
        DEBUG_PRINTLN("ioBroker MQTT-Manager Verbindung wiederhergestellt");
        
        // Topics neu abonnieren
        String statusTopic = baseTopic + "/+/+/status";
        ioMqttClient.subscribe(statusTopic.c_str());
        
        // Rolladen-spezifische Topics
        String rolladenCurrentPositionTopic = baseTopic + "/rolladen/+/currentPosition";
        ioMqttClient.subscribe(rolladenCurrentPositionTopic.c_str());
        
        String rolladenTargetPositionTopic = baseTopic + "/rolladen/+/targetPosition";
        ioMqttClient.subscribe(rolladenTargetPositionTopic.c_str());
        
        String rolladenDirectionTopic = baseTopic + "/rolladen/+/direction";
        ioMqttClient.subscribe(rolladenDirectionTopic.c_str());
        
        String rolladenMovingTopic = baseTopic + "/rolladen/+/moving";
        ioMqttClient.subscribe(rolladenMovingTopic.c_str());
        
        String rolladenSpeedTopic = baseTopic + "/rolladen/+/speed";
        ioMqttClient.subscribe(rolladenSpeedTopic.c_str());
        
        String rolladenCalibrateTopic = baseTopic + "/rolladen/+/calibrate";
        ioMqttClient.subscribe(rolladenCalibrateTopic.c_str());
        
        // Heizung- und Pool-Topics
        String heatingStatusTopic = baseTopic + "/heating/+/status";
        ioMqttClient.subscribe(heatingStatusTopic.c_str());
        
        String heatingTemperatureTopic = baseTopic + "/heating/+/temperature";
        ioMqttClient.subscribe(heatingTemperatureTopic.c_str());
        
        String poolStatusTopic = baseTopic + "/pool/+/status";
        ioMqttClient.subscribe(poolStatusTopic.c_str());
      }
    }
  } else {
    // MQTT Client Loop
    ioMqttClient.loop();
    // Rolladen-Positionen aktualisieren
    updateRolladenPositions();
    // Neue Heartbeat-Funktion aufrufen
    updateHeartbeat(); 
  }
}

bool IoBrokerManager::subscribeRolladenTopics() {
  // Überprüfen, ob wir verbunden sind
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden, kann Topics nicht abonnieren");
    return false;
  }
  
  DEBUG_PRINTLN("Abonniere spezifische Rolladen-Topics in ioBroker...");
  
  // Für jeden Rolladen (1-6) die spezifischen Topics abonnieren
  for (int i = 1; i <= 6; i++) {
    String rolladenId = String(i);
    
    // Target Level abonnieren
    String targetLevelTopic = "0_userdata.0.BlindControl.Shutter." + rolladenId + ".Input.obj_I_var_Target_Level";
    ioMqttClient.subscribe(targetLevelTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(targetLevelTopic);
    
    // Aktuelles Level abonnieren
    String currentLevelTopic = "0_userdata.0.BlindControl.Shutter." + rolladenId + ".Output.obj_Q_LEVEL";
    ioMqttClient.subscribe(currentLevelTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(currentLevelTopic);
    
    // Busy-Status abonnieren
    String busyTopic = "0_userdata.0.BlindControl.Shutter." + rolladenId + ".Output.obj_Q_BUSY";
    ioMqttClient.subscribe(busyTopic.c_str());
    DEBUG_PRINT("Abonniert: ");
    DEBUG_PRINTLN(busyTopic);
  }
  
  return true;
}
// Geräteinformationen für ioBroker Discovery veröffentlichen
void IoBrokerManager::publishDeviceInfo() {
  JsonDocument deviceInfo;
  deviceInfo["name"] = "ESP32 Solar Monitor";
  deviceInfo["version"] = APP_VERSION;
  deviceInfo["ip"] = WiFi.localIP().toString();
  deviceInfo["mac"] = WiFi.macAddress();
  deviceInfo["free_heap"] = ESP.getFreeHeap();
  deviceInfo["uptime"] = millis() / 1000;
  
  // Modernere ArduinoJson-Syntax
  JsonArray capabilities = deviceInfo["capabilities"].to<JsonArray>();
  capabilities.add("light_control");
  capabilities.add("rolladen_control");
  capabilities.add("heating_control");
  capabilities.add("pool_control");
  capabilities.add("config_management");
  
  String payload;
  serializeJson(deviceInfo, payload);
  
  ioMqttClient.publish((baseTopic + "/device").c_str(), payload.c_str(), true);
}

void IoBrokerManager::broadcastStatus() {
  // Alle Lichtstatus veröffentlichen
  for (const auto& ls : lightStatus) {
    String topic = getLightTopic(ls.id) + "/status";
    ioMqttClient.publish(topic.c_str(), ls.status.c_str(), true);
  }
  
  // Alle Rolladenstatus veröffentlichen
  for (const auto& rs : rolladenStatus) {
    // Ist-Position veröffentlichen
    String currentPosTopic = getRolladenTopic(rs.id) + "/currentPosition";
    String currentPosValue = String(rs.currentPosition);
    ioMqttClient.publish(currentPosTopic.c_str(), currentPosValue.c_str(), true);
    
    // Soll-Position veröffentlichen
    String targetPosTopic = getRolladenTopic(rs.id) + "/targetPosition";
    String targetPosValue = String(rs.targetPosition);
    ioMqttClient.publish(targetPosTopic.c_str(), targetPosValue.c_str(), true);
    
    // Richtung veröffentlichen
    String directionTopic = getRolladenTopic(rs.id) + "/direction";
    ioMqttClient.publish(directionTopic.c_str(), rs.direction.c_str(), true);
    
    // Bewegungsstatus veröffentlichen
    String movingTopic = getRolladenTopic(rs.id) + "/moving";
    String movingValue = rs.moving ? "true" : "false";
    ioMqttClient.publish(movingTopic.c_str(), movingValue.c_str(), true);
    
    // Geschwindigkeit veröffentlichen
    String speedTopic = getRolladenTopic(rs.id) + "/speed";
    String speedValue = String(rs.rolladenSpeed);
    ioMqttClient.publish(speedTopic.c_str(), speedValue.c_str(), true);
    
    // Kalibrierungsstatus veröffentlichen
    String calibratedTopic = getRolladenTopic(rs.id) + "/calibrated";
    String calibratedValue = rs.calibrated ? "true" : "false";
    ioMqttClient.publish(calibratedTopic.c_str(), calibratedValue.c_str(), true);
  }
  
  // Alle Heizungsstatus veröffentlichen
  for (const auto& hs : heatingStatus) {
    // Status veröffentlichen
    String statusTopic = getHeatingTopic(hs.id) + "/status";
    ioMqttClient.publish(statusTopic.c_str(), hs.status.c_str(), true);
    
    // Temperatur veröffentlichen
    String tempTopic = getHeatingTopic(hs.id) + "/temperature";
    String tempValue = String(hs.temperature);
    ioMqttClient.publish(tempTopic.c_str(), tempValue.c_str(), true);
  }
  
  // Alle Poolstatus veröffentlichen
  for (const auto& ps : poolStatus) {
    String topic = getPoolTopic(ps.id) + "/status";
    ioMqttClient.publish(topic.c_str(), ps.status.c_str(), true);
  }
}

// Verbesserte Methode zum Aktualisieren der Rolladenpositionen
void IoBrokerManager::updateRolladenPositions() {
  unsigned long currentTime = millis();
  
  for (auto& rs : rolladenStatus) {
    if (rs.moving) {
      // Zeit seit letztem Update in Sekunden
      float timeDelta = (currentTime - rs.lastUpdate) / 1000.0;
      
      // Positionsänderung basierend auf Geschwindigkeit berechnen
      float positionChange = timeDelta * rs.rolladenSpeed;
      
      // Aktuelle Position entsprechend der Richtung aktualisieren
      int newPosition = rs.currentPosition;
      
      // WICHTIG: Bei dir ist 0% offen (oben) und 100% geschlossen (unten)
      // Daher ist "up" eine Abnahme des Prozentsatzes und "down" eine Zunahme
      if (rs.direction == "up") {
        newPosition -= positionChange;  // Wert verringern beim Hochfahren
        
        // Prüfen, ob die Soll-Position erreicht wurde
        if (newPosition <= rs.targetPosition) {  // Kleiner oder gleich, da höher = offener
          newPosition = rs.targetPosition;
          rs.moving = false;
          rs.direction = "stop";
        }
      } else if (rs.direction == "down") {
        newPosition += positionChange;  // Wert erhöhen beim Herunterfahren
        
        // Prüfen, ob die Soll-Position erreicht wurde
        if (newPosition >= rs.targetPosition) {  // Größer oder gleich, da niedriger = geschlossener
          newPosition = rs.targetPosition;
          rs.moving = false;
          rs.direction = "stop";
        }
      }
      
      // Position innerhalb der Grenzen halten
      newPosition = constrain(newPosition, 0, 100);
      rs.currentPosition = newPosition;
      
      // Aktualisiere den Zeitstempel
      rs.lastUpdate = currentTime;
      
      // Aktualisierte Werte veröffentlichen
      String currentPosTopic = getRolladenTopic(rs.id) + "/currentPosition";
      String currentPosValue = String(int(rs.currentPosition));
      ioMqttClient.publish(currentPosTopic.c_str(), currentPosValue.c_str(), true);
      
      // Wenn Bewegung gestoppt wurde, auch die anderen Status aktualisieren
      if (!rs.moving) {
        String directionTopic = getRolladenTopic(rs.id) + "/direction";
        ioMqttClient.publish(directionTopic.c_str(), "stop", true);
        
        String movingTopic = getRolladenTopic(rs.id) + "/moving";
        ioMqttClient.publish(movingTopic.c_str(), "false", true);
      }
    }
  }
}

// Hilfsmethoden für Topic-Erstellung
String IoBrokerManager::getLightTopic(const String &lightId) {
  return baseTopic + "/light/" + lightId;
}

String IoBrokerManager::getRolladenTopic(const String &rolladenId) {
  return baseTopic + "/rolladen/" + rolladenId;
}

String IoBrokerManager::getHeatingTopic(const String &heatingId) {
  return baseTopic + "/heating/" + heatingId;
}

String IoBrokerManager::getPoolTopic(const String &poolId) {
  return baseTopic + "/pool/" + poolId;
}

// Lichtsteuerung
bool IoBrokerManager::setLight(const String &lightId, bool state) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  String commandTopic = getLightTopic(lightId) + "/command";
  String statusTopic = getLightTopic(lightId) + "/status";
  String payload = state ? "ON" : "OFF";
  
  DEBUG_PRINT("Sende Lichtbefehl: ");
  DEBUG_PRINT(commandTopic);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(payload);
  
  // Veröffentliche Befehl
  bool result = ioMqttClient.publish(commandTopic.c_str(), payload.c_str());
  
  if (result) {
    // Aktualisiere lokalen Status sofort für bessere UI-Reaktivität
    bool found = false;
    for (auto& ls : lightStatus) {
      if (ls.id == lightId) {
        ls.status = payload;
        ls.lastUpdate = millis();
        found = true;
        break;
      }
    }
    
    if (!found) {
      DeviceStatus newStatus(lightId);
      newStatus.status = payload;
      newStatus.lastUpdate = millis();
      lightStatus.push_back(newStatus);
    }
  }
  
  return result;
}

bool IoBrokerManager::getLightStatus(const String &lightId, bool &status) {
  for (const auto& ls : lightStatus) {
    if (ls.id == lightId) {
      status = (ls.status == "ON");
      return true;
    }
  }
  
  // Status nicht gefunden
  return false;
}

// Erweiterte Rolladensteuerung
bool IoBrokerManager::moveRolladen(const String &rolladenId, const String &direction) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }

  // Prüfen, ob die Richtung gültig ist
  if (direction != "up" && direction != "down" && direction != "stop") {
    DEBUG_PRINTLN("Ungültige Richtung für Rollladen");
    return false;
  }

  String directionTopic = getRolladenTopic(rolladenId) + "/direction";
  String movingTopic = getRolladenTopic(rolladenId) + "/moving";
  String targetPosTopic = getRolladenTopic(rolladenId) + "/targetPosition";

  bool result = ioMqttClient.publish(directionTopic.c_str(), direction.c_str());

  if (!result) return false;

  // Lokalen Status aktualisieren
  bool found = false;
  for (size_t i = 0; i < rolladenStatus.size(); i++) {
    if (rolladenStatus[i].id == rolladenId) {
      found = true;
      rolladenStatus[i].direction = direction;
      rolladenStatus[i].moving = (direction != "stop");
      rolladenStatus[i].lastUpdate = millis();

      if (direction == "stop") {
        rolladenStatus[i].targetPosition = rolladenStatus[i].currentPosition;
        ioMqttClient.publish(targetPosTopic.c_str(), String(rolladenStatus[i].targetPosition).c_str(), true);
        ioMqttClient.publish(movingTopic.c_str(), "false", true);
        break;
      }

      // Bewegung gewünscht → Zielposition setzen
      int current = rolladenStatus[i].currentPosition;
      int target = (direction == "up") ? 0 : 100;

      // Wenn bereits am Ziel: Ziel minimal anpassen (Nudge), um Bewegung zu erzwingen
      if (current == target) {
        if (direction == "up") target = max(0, current - 1);
        if (direction == "down") target = min(100, current + 1);
      }

      rolladenStatus[i].targetPosition = target;
      rolladenStatus[i].moveStartTime = millis();
      rolladenStatus[i].moveStartPosition = current;

      // Topics veröffentlichen
      ioMqttClient.publish(targetPosTopic.c_str(), String(target).c_str(), true);
      ioMqttClient.publish(movingTopic.c_str(), "true", true);
      break;
    }
  }

  // Falls nicht gefunden, neuen Status anlegen
  if (!found) {
    RolladenStatus newStatus(rolladenId);
    newStatus.direction = direction;
    newStatus.moving = (direction != "stop");
    newStatus.lastUpdate = millis();

    if (direction != "stop") {
      newStatus.currentPosition = 50;  // Standardwert
      newStatus.moveStartTime = millis();
      newStatus.moveStartPosition = newStatus.currentPosition;
      newStatus.targetPosition = (direction == "up") ? 0 : 100;

      if (newStatus.targetPosition == newStatus.currentPosition) {
        newStatus.targetPosition += (direction == "up") ? -1 : +1;
        newStatus.targetPosition = constrain(newStatus.targetPosition, 0, 100);
      }

      ioMqttClient.publish(targetPosTopic.c_str(), String(newStatus.targetPosition).c_str(), true);
      ioMqttClient.publish(movingTopic.c_str(), "true", true);
    } else {
      newStatus.targetPosition = newStatus.currentPosition;
      ioMqttClient.publish(targetPosTopic.c_str(), String(newStatus.currentPosition).c_str(), true);
      ioMqttClient.publish(movingTopic.c_str(), "false", true);
    }

    rolladenStatus.push_back(newStatus);
  }

  return true;
}


// Neue Methode zur Zielpositionierung von Rolladen
bool IoBrokerManager::setRolladenTargetPosition(const String &rolladenId, int position) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  // Position auf 0-100% beschränken
  position = constrain(position, 0, 100);
  
  // Direktes ioBroker-Topic für Target Level
  String directTargetTopic = "0_userdata.0.BlindControl.Shutter." + rolladenId + ".Input.obj_I_var_Target_Level";
  String positionValue = String(position);
  
  // Veröffentliche den Befehl direkt an das ioBroker-Topic
  bool directResult = ioMqttClient.publish(directTargetTopic.c_str(), positionValue.c_str(), true);
  
  // Zusätzlich auch an das normale Topic senden (für Kompatibilität)
  String targetPosTopic = getRolladenTopic(rolladenId) + "/targetPosition";
  bool compatResult = ioMqttClient.publish(targetPosTopic.c_str(), positionValue.c_str(), true);
  
  if (directResult || compatResult) {
    // Aktualisiere lokalen Status
    bool found = false;
    for (size_t i = 0; i < rolladenStatus.size(); i++) {
      if (rolladenStatus[i].id == rolladenId) {
        rolladenStatus[i].targetPosition = position;
        rolladenStatus[i].lastUpdate = millis();
        
        // Wenn aktuelle Position unterschiedlich ist, starte eine Bewegung
        if (rolladenStatus[i].currentPosition != position) {
          // Richtung bestimmen
          String newDirection = (position < rolladenStatus[i].currentPosition) ? "up" : "down";
          rolladenStatus[i].direction = newDirection;
          rolladenStatus[i].moving = true;
          rolladenStatus[i].moveStartTime = millis();
          rolladenStatus[i].moveStartPosition = rolladenStatus[i].currentPosition;
          
          // Sende auch Direction und Moving-Status an normale Topics
          String directionTopic = getRolladenTopic(rolladenId) + "/direction";
          ioMqttClient.publish(directionTopic.c_str(), newDirection.c_str(), true);
          
          String movingTopic = getRolladenTopic(rolladenId) + "/moving";
          ioMqttClient.publish(movingTopic.c_str(), "true", true);
        }
        
        found = true;
        break;
      }
    }
    
    if (!found) {
      RolladenStatus newStatus(rolladenId);
      newStatus.targetPosition = position;
      rolladenStatus.push_back(newStatus);
    }
  }
  
  return (directResult || compatResult);
}

// Erweiterte Methode zum Holen des Rolladenstatus
bool IoBrokerManager::getRolladenStatus(const String &rolladenId, int &currentPosition, 
                                        int &targetPosition, String &direction, bool &moving) {
  for (const auto& rs : rolladenStatus) {
    if (rs.id == rolladenId) {
      currentPosition = rs.currentPosition;
      targetPosition = rs.targetPosition;
      direction = rs.direction;
      moving = rs.moving;
      return true;
    }
  }
  
  // Status nicht gefunden
  return false;
}

// Neue Methode zur Kalibrierung eines Rollladens
bool IoBrokerManager::calibrateRolladen(const String &rolladenId, int speed) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  // Geschwindigkeit auf sinnvolle Werte beschränken
  speed = constrain(speed, 1, 20);
  
  // Setze Geschwindigkeit
  String speedTopic = getRolladenTopic(rolladenId) + "/speed";
  String speedValue = String(speed);
  bool speedResult = ioMqttClient.publish(speedTopic.c_str(), speedValue.c_str(), true);
  
  // Sende Kalibrierungsbefehl
  String calibrateTopic = getRolladenTopic(rolladenId) + "/calibrate";
  bool calibrateResult = ioMqttClient.publish(calibrateTopic.c_str(), "true", true);
  
  if (speedResult && calibrateResult) {
    // Aktualisiere lokalen Status
    bool found = false;
    for (size_t i = 0; i < rolladenStatus.size(); i++) {
      if (rolladenStatus[i].id == rolladenId) {
        rolladenStatus[i].rolladenSpeed = speed;
        rolladenStatus[i].calibrated = true;
        rolladenStatus[i].lastUpdate = millis();
        found = true;
        break;
      }
    }
    
    if (!found) {
      RolladenStatus newStatus(rolladenId);
      newStatus.rolladenSpeed = speed;
      newStatus.calibrated = true;
      newStatus.lastUpdate = millis();
      rolladenStatus.push_back(newStatus);
    }
    
    return true;
  }
  
  return false;
}

// Heizungssteuerung
bool IoBrokerManager::setHeating(const String &heatingId, bool state) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  String commandTopic = getHeatingTopic(heatingId) + "/command";
  String payload = state ? "ON" : "OFF";
  
  // Veröffentliche Befehl
  bool result = ioMqttClient.publish(commandTopic.c_str(), payload.c_str());
  
  if (result) {
    // Aktualisiere lokalen Status
    bool found = false;
    for (auto& hs : heatingStatus) {
      if (hs.id == heatingId) {
        hs.status = payload;
        hs.lastUpdate = millis();
        found = true;
        break;
      }
    }
    
    if (!found) {
      HeatingStatus newStatus(heatingId);
      newStatus.status = payload;
      newStatus.lastUpdate = millis();
      heatingStatus.push_back(newStatus);
    }
  }
  
  return result;
}

bool IoBrokerManager::setHeatingTemperature(const String &heatingId, float temperature) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  String tempTopic = getHeatingTopic(heatingId) + "/set_temperature";
  String tempValue = String(temperature, 1); // Eine Dezimalstelle
  
  // Veröffentliche Temperaturbefehl
  bool result = ioMqttClient.publish(tempTopic.c_str(), tempValue.c_str());
  
  if (result) {
    // Aktualisiere lokalen Status
    bool found = false;
    for (auto& hs : heatingStatus) {
      if (hs.id == heatingId) {
        hs.temperature = temperature;
        hs.lastUpdate = millis();
        found = true;
        break;
      }
    }
    
    if (!found) {
      HeatingStatus newStatus(heatingId);
      newStatus.temperature = temperature;
      newStatus.lastUpdate = millis();
      heatingStatus.push_back(newStatus);
    }
  }
  
  return result;
}

bool IoBrokerManager::getHeatingStatus(const String &heatingId, bool &state, float &temperature) {
  for (const auto& hs : heatingStatus) {
    if (hs.id == heatingId) {
      state = (hs.status == "ON");
      temperature = hs.temperature;
      return true;
    }
  }
  
  // Status nicht gefunden
  return false;
}

// Poolpumpensteuerung
bool IoBrokerManager::setPoolPump(const String &poolId, bool state) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  String commandTopic = getPoolTopic(poolId) + "/command";
  String payload = state ? "ON" : "OFF";
  
  // Veröffentliche Befehl
  bool result = ioMqttClient.publish(commandTopic.c_str(), payload.c_str());
  
  if (result) {
    // Aktualisiere lokalen Status
    bool found = false;
    for (auto& ps : poolStatus) {
      if (ps.id == poolId) {
        ps.status = payload;
        ps.lastUpdate = millis();
        found = true;
        break;
      }
    }
    
    if (!found) {
      DeviceStatus newStatus(poolId);
      newStatus.status = payload;
      newStatus.lastUpdate = millis();
      poolStatus.push_back(newStatus);
    }
  }
  
  return result;
}

bool IoBrokerManager::getPoolPumpStatus(const String &poolId, bool &status) {
  for (const auto& ps : poolStatus) {
    if (ps.id == poolId) {
      status = (ps.status == "ON");
      return true;
    }
  }
  
  // Status nicht gefunden
  return false;
}

// Konfigurationsverwaltung
bool IoBrokerManager::updateConfig(JsonDocument &configUpdate) {
  // Lade aktuelle Konfiguration
  JsonDocument currentConfig;
  if (!configManager.loadJsonConfig("/config.json", currentConfig)) {
    DEBUG_PRINTLN("Fehler beim Laden der Konfiguration");
    return false;
  }
  
  // Aktualisiere nur die geänderten Werte
  for (JsonPair kv : configUpdate.as<JsonObject>()) {
    currentConfig[kv.key()] = kv.value();
  }
  
  // Speichere aktualisierte Konfiguration
  if (!configManager.saveJsonConfig("/config.json", currentConfig)) {
    DEBUG_PRINTLN("Fehler beim Speichern der aktualisierten Konfiguration");
    return false;
  }
  
  // Bestätige Aktualisierung
  ioMqttClient.publish((baseTopic + "/config/updated").c_str(), "true");
  
  return true;
}

// Implementierung der zusätzlichen Rollladen-Funktionen

// Diese Methode stellt den Zustand eines Rollladens zurück
bool IoBrokerManager::resetRolladenState(const String &rolladenId) {
  bool found = false;
  
  for (size_t i = 0; i < rolladenStatus.size(); i++) {
    if (rolladenStatus[i].id == rolladenId) {
      // Bewegung stoppen und Richtung zurücksetzen
      rolladenStatus[i].moving = false;
      rolladenStatus[i].direction = "stop";
      
      // Zielposition auf aktuelle Position setzen
      rolladenStatus[i].targetPosition = rolladenStatus[i].currentPosition;
      
      // Status an ioBroker übermitteln
      String directionTopic = getRolladenTopic(rolladenId) + "/direction";
      ioMqttClient.publish(directionTopic.c_str(), "stop", true);
      
      String movingTopic = getRolladenTopic(rolladenId) + "/moving";
      ioMqttClient.publish(movingTopic.c_str(), "false", true);
      
      String targetPosTopic = getRolladenTopic(rolladenId) + "/targetPosition";
      String targetPosValue = String(rolladenStatus[i].currentPosition);
      ioMqttClient.publish(targetPosTopic.c_str(), targetPosValue.c_str(), true);
      
      found = true;
      break;
    }
  }
  
  return found;
}

// Methode zum Setzen einer relativen Position (z.B. "10% mehr öffnen")
bool IoBrokerManager::adjustRolladenPosition(const String &rolladenId, int adjustment) {
  if (!connected) {
    DEBUG_PRINTLN("Nicht mit ioBroker verbunden!");
    return false;
  }
  
  // Suche nach dem Rolladen
  bool found = false;
  int currentPos = 0;
  
  for (size_t i = 0; i < rolladenStatus.size(); i++) {
    if (rolladenStatus[i].id == rolladenId) {
      currentPos = rolladenStatus[i].currentPosition;
      found = true;
      break;
    }
  }
  
  if (!found) {
    DEBUG_PRINT("Rolladen nicht gefunden: ");
    DEBUG_PRINTLN(rolladenId);
    return false;
  }
  
  // Berechne neue Position mit Begrenzung auf 0-100%
  int newTargetPos = constrain(currentPos + adjustment, 0, 100);
  
  // Setze die neue Zielposition
  return setRolladenTargetPosition(rolladenId, newTargetPos);
}

// Methode zum vollständigen Öffnen eines Rollladens (0%)
bool IoBrokerManager::openRolladen(const String &rolladenId) {
  return setRolladenTargetPosition(rolladenId, 0);
}

// Methode zum vollständigen Schließen eines Rollladens (100%)
bool IoBrokerManager::closeRolladen(const String &rolladenId) {
  return setRolladenTargetPosition(rolladenId, 100);
}

// Szenen-Methode: Alle Rollläden auf die gleiche Position setzen
bool IoBrokerManager::setAllRolladenPositions(int position) {
  bool success = true;
  
  // Position begrenzen
  position = constrain(position, 0, 100);
  
  // Für jeden Rolladen die Position setzen
  for (const auto& rs : rolladenStatus) {
    if (!setRolladenTargetPosition(rs.id, position)) {
      success = false;
    }
  }
  
  return success;
}

// Szenen-Methode: Alle Rollläden öffnen
bool IoBrokerManager::openAllRolladen() {
  return setAllRolladenPositions(0);
}

// Szenen-Methode: Alle Rollläden schließen
bool IoBrokerManager::closeAllRolladen() {
  return setAllRolladenPositions(100);
}

// Szenen-Methode: Sonnen-/Blendschutz (teilweise geschlossen)
bool IoBrokerManager::setSunProtection(const String &rolladenId, bool enable) {
  // Standardwert für Sonnenschutz (z.B. 30% geschlossen)
  const int sunProtectionPosition = 30;
  
  if (enable) {
    return setRolladenTargetPosition(rolladenId, sunProtectionPosition);
  } else {
    // Bei Deaktivierung vollständig öffnen
    return setRolladenTargetPosition(rolladenId, 0);
  }
}

// Eine verbesserte Abfrage für den aktuellen Status
String IoBrokerManager::getRolladenStatusJson(const String &rolladenId) {
  String result = "{}"; // Leeres JSON als Standardrückgabe
  
  for (const auto& rs : rolladenStatus) {
    if (rs.id == rolladenId) {
      // Status als JSON formatieren
      JsonDocument statusJson;
      statusJson["id"] = rs.id;
      statusJson["currentPosition"] = rs.currentPosition;
      statusJson["targetPosition"] = rs.targetPosition;
      statusJson["direction"] = rs.direction;
      statusJson["moving"] = rs.moving;
      statusJson["speed"] = rs.rolladenSpeed;
      statusJson["calibrated"] = rs.calibrated;
      
      // JSON serialisieren
      serializeJson(statusJson, result);
      break;
    }
  }
  
  return result;
}



// Eine Methode für eine verzögerte Bewegung (z.B. für Szenen)
bool IoBrokerManager::scheduleRolladenMove(const String &rolladenId, int targetPosition, unsigned long delayMs) {
  // Für eine einfache Implementierung ohne komplexe Timer können wir
  // einen Task-Manager oder Flaggen verwenden. Hier ein einfaches Beispiel:
  
  // Status für die verzögerte Bewegung speichern
  // In einem realen System würde dies eine Task-Queue verwenden
  
  // Für jetzt starten wir einfach einen neuen Thread oder verwenden millis() für eine verzögerte Ausführung
  
  // HINWEIS: Dies ist ein Platzhalter, der in einem echten System erweitert werden sollte
  
  DEBUG_PRINT("Verzögerte Bewegung geplant für Rolladen ");
  DEBUG_PRINT(rolladenId);
  DEBUG_PRINT(" zur Position ");
  DEBUG_PRINT(targetPosition);
  DEBUG_PRINT(" mit Verzögerung ");
  DEBUG_PRINT(delayMs);
  DEBUG_PRINTLN(" ms");
  
  // Für jetzt gibt diese Methode immer true zurück
  return true;
}

void IoBrokerManager::updateHeartbeat() {
  if (!connected) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Heartbeat senden (alle HEARTBEAT_INTERVAL Millisekunden)
  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL) {
    lastHeartbeatTime = currentTime;
    heartbeatCounter++;
    
    String heartbeatTopic = baseTopic + "/heartbeat/beat";
    String heartbeatMessage = String(heartbeatCounter) + "," + String(millis() / 1000);
    
    bool published = ioMqttClient.publish(heartbeatTopic.c_str(), heartbeatMessage.c_str(), true);
    
    if (published) {
      DEBUG_PRINT("Heartbeat gesendet: ");
      DEBUG_PRINTLN(heartbeatMessage);
    } else {
      DEBUG_PRINTLN("Fehler beim Senden des Heartbeats!");
      // Möglicherweise ist die Verbindung unterbrochen
      connected = false;
    }
    
    // Prüfen, ob wir einen Heartbeat von ioBroker empfangen haben
    if (currentTime - lastHeartbeatReceived > HEARTBEAT_INTERVAL * 3) {
      // Kein Heartbeat von ioBroker für mehr als 3 Intervalle
      DEBUG_PRINTLN("Warnung: Kein Heartbeat von ioBroker empfangen!");
      heartbeatReceived = false;
      
      // Optional: Versuchen Sie, die Verbindung neu herzustellen
      if (connected) {
        DEBUG_PRINTLN("Versuche Wiederverbindung zu ioBroker...");
        connected = false;
      }
    }
  }
}