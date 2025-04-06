/**
 * default_data.cpp - Standard-Konfigurationsdaten
 */

#include "default_data.h"

// Konfigurationsdaten
const char* DEFAULT_CONFIG_JSON = R"({
  "wlan": {
    "ssid": "YOUR_SSID",
    "password": "YOUR_PASSWORD"
  },
  "mqtt": {
    "broker": "IP_ADRESS_MQTT_BROKER",
    "port": 1883,
    "client_id_prefix": "ESP32SolarMonitor-"
  },
  "display": {
    "brightness": 100,
    "timeout": 600,
    "theme": "dark"
  },
  "touch": {
    "min_x": 200,
    "max_x": 3700,
    "min_y": 240,
    "max_y": 3800
  },
  "simulation_mode": false,
  "update_interval": 5000
})";

// Menü-Struktur
const char* DEFAULT_MENU_JSON = R"({
  "tabs": [
    {
      "title": "System",
      "items": [
        {
          "name": "Solar Status",
          "function": "drawSolarStatus",
          "icon": "sun"
        },
        {
          "name": "Batterie Status",
          "function": "drawBatteryStatus",
          "icon": "battery"
        },
        {
          "name": "Netzstatus",
          "function": "drawGridStatus",
          "icon": "grid"
        },
        {
          "name": "PV Leistung",
          "function": "drawPvPower",
          "icon": "solar"
        },
        {
          "name": "Verbrauch",
          "function": "drawConsumption",
          "icon": "home"
        },
        {
          "name": "Autarkie",
          "function": "drawAutarky",
          "icon": "leaf"
        },
        {
          "name": "Tageswerte",
          "function": "drawDailyValues",
          "icon": "calendar"
        },
        {
          "name": "Statistik",
          "function": "drawStatistics",
          "icon": "chart"
        }
      ]
    },
    {
      "title": "Steuerung",
      "items": [
        {
          "name": "Heizung",
          "function": "controlHeating",
          "icon": "heat"
        },
        {
          "name": "Pool",
          "function": "controlPool",
          "icon": "water"
        },
        {
          "name": "Garten",
          "function": "controlGarden",
          "icon": "plant"
        },
        {
          "name": "Licht",
          "function": "controlLight",
          "icon": "bulb"
        },
        {
          "name": "Steckdosen",
          "function": "controlPlugs",
          "icon": "plug"
        },
        {
          "name": "Lüftung",
          "function": "controlVentilation",
          "icon": "fan"
        },
        {
          "name": "Rollladen",
          "function": "controlShutters",
          "icon": "window"
        },
        {
          "name": "Kameras",
          "function": "controlCameras",
          "icon": "camera"
        }
      ]
    },
    {
      "title": "Einstellungen",
      "items": [
        {
          "name": "WLAN Setup",
          "function": "setupWifi",
          "icon": "wifi"
        },
        {
          "name": "MQTT Setup",
          "function": "setupMqtt",
          "icon": "cloud"
        },
        {
          "name": "Display",
          "function": "setupDisplay",
          "icon": "monitor"
        },
        {
          "name": "Systeminfo",
          "function": "showSystemInfo",
          "icon": "info"
        },
        {
          "name": "Updates",
          "function": "checkUpdates",
          "icon": "update"
        },
        {
          "name": "Logs",
          "function": "viewLogs",
          "icon": "file"
        },
        {
          "name": "Neustart",
          "function": "restartSystem",
          "icon": "refresh"
        },
        {
          "name": "Werkseinstellungen",
          "function": "factoryReset",
          "icon": "trash"
        }
      ]
    }
  ]
})";

// MQTT-Topics
const char* DEFAULT_MQTT_TOPICS_JSON = R"({
  "topics": [
    {
      "name": "battery_soc",
      "topic": "solar_assistant/total/battery_state_of_charge/state",
      "description": "Batterieladezustand in Prozent",
      "unit": "%",
      "color": "TFT_YELLOW"
    },
    {
      "name": "load_power",
      "topic": "solar_assistant/inverter_1/load_power_essential/state",
      "description": "Verbrauchsleistung",
      "unit": "W",
      "color": "TFT_RED"
    },
    {
      "name": "grid_power",
      "topic": "solar_assistant/inverter_1/grid_power/state",
      "description": "Netzleistung negativ Einspeisung",
      "unit": "W",
      "color": "TFT_BLUE"
    },
    {
      "name": "pv_power",
      "topic": "solar_assistant/inverter_1/pv_power/state",
      "description": "PV-Leistung",
      "unit": "W",
      "color": "TFT_GREEN"
    },
    {
      "name": "battery_power",
      "topic": "solar_assistant/total/battery_power/state",
      "description": "Batterieleistung",
      "unit": "W",
      "color": "TFT_PURPLE"
    },
    {
      "name": "battery_voltage",
      "topic": "solar_assistant/inverter_1/battery_voltage/state",
      "description": "Batteriespannung",
      "unit": "V",
      "color": "TFT_CYAN"
    },
    {
      "name": "daily_yield",
      "topic": "solar_assistant/inverter_1/energy_day/state",
      "description": "Tagesertrag",
      "unit": "kWh",
      "color": "TFT_ORANGE"
    },
    {
      "name": "total_yield",
      "topic": "solar_assistant/inverter_1/energy_total/state",
      "description": "Gesamtertrag",
      "unit": "kWh",
      "color": "TFT_ORANGE"
    }
  ]
})";