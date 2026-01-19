/**
 * DisplayLock.cpp - Implementierung der Display-Lock-Funktionalität
 */

#include "DisplayLock.h"

// Statische Membervariablen initialisieren
SemaphoreHandle_t DisplayLock::displayMutex = NULL;
SemaphoreHandle_t DisplayLock::touchMutex = NULL;
bool DisplayLock::initialized = false;

// Initialisierung der Mutexe
void DisplayLock::init() {
  if (!initialized) {
    displayMutex = xSemaphoreCreateMutex();
    touchMutex = xSemaphoreCreateMutex();
    initialized = true;
    
    if (displayMutex == NULL || touchMutex == NULL) {
      Serial.println("[ERROR] Fehler beim Erstellen der Display-Mutexe!");
    } else {
      Serial.println("[INFO] DisplayLock-Mutexe erfolgreich erstellt");
    }
  }
}
