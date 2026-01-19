/**
 * DisplayLock.h - Thread-sicherer Zugriff auf Display und Touch
 * 
 * Dieses Modul stellt Mutexe für den Thread-sicheren Zugriff auf
 * das TFT-Display und den Touch-Controller bereit.
 */

#ifndef DISPLAY_LOCK_H
#define DISPLAY_LOCK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class DisplayLock {
private:
  static SemaphoreHandle_t displayMutex;
  static SemaphoreHandle_t touchMutex;
  static bool initialized;
  
public:
  // Initialisierung der Mutexe (MUSS in setup() aufgerufen werden!)
  static void init();
  
  // Display sperren (mit Timeout)
  // WICHTIG: init() muss vorher aufgerufen worden sein!
  static bool lockDisplay(TickType_t timeout = portMAX_DELAY) {
    if (displayMutex == NULL) {
      Serial.println("[ERROR] DisplayLock nicht initialisiert! Rufe init() in setup() auf.");
      return false;
    }
    return xSemaphoreTake(displayMutex, timeout) == pdTRUE;
  }
  
  // Display freigeben
  static void unlockDisplay() {
    if (displayMutex != NULL) {
      xSemaphoreGive(displayMutex);
    }
  }
  
  // Touch sperren (mit Timeout)
  // WICHTIG: init() muss vorher aufgerufen worden sein!
  static bool lockTouch(TickType_t timeout = portMAX_DELAY) {
    if (touchMutex == NULL) {
      Serial.println("[ERROR] DisplayLock nicht initialisiert! Rufe init() in setup() auf.");
      return false;
    }
    return xSemaphoreTake(touchMutex, timeout) == pdTRUE;
  }
  
  // Touch freigeben
  static void unlockTouch() {
    if (touchMutex != NULL) {
      xSemaphoreGive(touchMutex);
    }
  }
};

// Statische Membervariablen - Definition in DisplayLock.cpp
// (Hier nur Deklaration)

// RAII-Helper für automatisches Entsperren
class DisplayGuard {
private:
  bool locked;
  
public:
  DisplayGuard() {
    locked = DisplayLock::lockDisplay(pdMS_TO_TICKS(100));
  }
  
  ~DisplayGuard() {
    if (locked) {
      DisplayLock::unlockDisplay();
    }
  }
  
  bool isLocked() const { return locked; }
};

#endif // DISPLAY_LOCK_H
