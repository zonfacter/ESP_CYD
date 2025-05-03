
/**
 * config.h - Zentrale Konfigurationsdatei
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "version_constants.h"

// Debugging
#define DEBUG_ENABLED true
#define DEBUG_BAUD_RATE 115200
#define DEBUG_SERIAL Serial


// Debug-Makros definieren
#if DEBUG_ENABLED
  #define DEBUG_PRINT(...) DEBUG_SERIAL.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) DEBUG_SERIAL.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_PRINTF(...)
#endif

// Display-Konfiguration
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touchscreen Pins für ESP32-2432S028R
#define XPT2046_IRQ 36   // Touchscreen Interrupt-Pin
#define XPT2046_MOSI 32  // SPI MOSI
#define XPT2046_MISO 39  // SPI MISO
#define XPT2046_CLK 25   // SPI Clock
#define XPT2046_CS 33    // SPI Chip Select

// Touchscreen-Kalibrierungswerte
#define TOUCH_MIN_X 200
#define TOUCH_MAX_X 3700
#define TOUCH_MIN_Y 240
#define TOUCH_MAX_Y 3800

// Menü-Konfiguration
#define MAX_MENU_ITEMS 8  // Anzahl der Menüpunkte pro Tab
#define MENU_ITEM_HEIGHT 40
#define MENU_ITEM_WIDTH 240
#define MENU_START_X 40
#define MENU_START_Y 70
#define MENU_VISIBLE_ITEMS 3  // Reduziert auf 3 sichtbare Einträge
#define SCROLL_ARROW_WIDTH 30 // Breite für die Pfeile

// Tab-Konfiguration
#define NUM_TABS 3
#define TAB_HEIGHT 30
#define TAB_WIDTH 100

// Farben
#define BACKGROUND TFT_BLACK
#define TEXT_COLOR TFT_WHITE
#define HIGHLIGHT_COLOR TFT_BLUE
#define BORDER_COLOR TFT_DARKGREY
#define TITLE_COLOR TFT_SKYBLUE
#define STATUS_COLOR TFT_GREEN
#define TAB_ACTIVE_COLOR TFT_NAVY
#define TAB_INACTIVE_COLOR TFT_DARKGREY
#define SCROLL_ACTIVE_COLOR TFT_ORANGE
#define SCROLL_INACTIVE_COLOR TFT_DARKGREY

// Display-Konfiguration
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define DISPLAY_COLOR_INVERTED false  // Auf true setzen für USB-C Variante mit invertierten Farben

// MQTT Konfiguration
#define MQTT_BROKER "192.168.2.54"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32SolarMonitor-"
#define MQTT_UPDATE_INTERVAL 15000  // 15 Sekunden

// OTA-Update Konfiguration
#define OTA_DEFAULT_PORT 3232
#define OTA_DEFAULT_HOSTNAME "ESP32SolarMonitor"

// Display Timeout (in Sekunden)
#define DEFAULT_DISPLAY_TIMEOUT 600  // 10 Minuten Standardwert

// Default WLAN-Daten
#define DEFAULT_WIFI_SSID "YOUR_SSID"
#define DEFAULT_WIFI_PASS "YOUR_PW"

#endif // CONFIG_H