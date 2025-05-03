#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "config.h"
#include "DataManager.h"  // Wichtig: DataManager.h einbinden
#include "IoBrokerManager.h"  // Wichtig: IoBrokerManager.h einbinden

void setupWebServer();
void handleWebServer();

#endif // WEB_SERVER_H