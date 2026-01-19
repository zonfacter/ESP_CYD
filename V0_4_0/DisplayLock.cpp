/**
 * DisplayLock.cpp - Implementierung der Display-Lock-Funktionalität
 */

#include "DisplayLock.h"

// Statische Membervariablen initialisieren
SemaphoreHandle_t DisplayLock::displayMutex = NULL;
SemaphoreHandle_t DisplayLock::touchMutex = NULL;
bool DisplayLock::initialized = false;
