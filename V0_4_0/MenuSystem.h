/**
 * MenuSystem.h - Verwaltet das Touch-Menü mit JSON-Konfiguration
 */

#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>
#include <functional>
#include "config.h"

class MenuItem {
public:
  String name;
  String functionName;
  
  MenuItem(const String &name, const String &functionName) 
    : name(name), functionName(functionName) {}
};

class MenuTab {
public:
  String title;
  std::vector<MenuItem> items;
  
  MenuTab(const String &title) : title(title) {}
  
  void addItem(const MenuItem &item) {
    items.push_back(item);
  }
};

class MenuSystem {
private:
  TFT_eSPI &tft;
  std::vector<MenuTab> tabs;
  
  int currentTab = 0;
  int scrollPosition = 0;
  int selectedMenuItem = -1;
  int touchedMenuItem = -1;
  
  bool touchedUpScroll = false;
  bool touchedDownScroll = false;
  
  bool needsFullRedraw = true;
  
  // Vorherige Zustände für partielles Redraw
  int prevScrollPosition = 0;
  int previousTouchedMenuItem = -1;
  bool prevTouchedUpScroll = false;
  bool prevTouchedDownScroll = false;
  
public:
  MenuSystem(TFT_eSPI &tft) : tft(tft) {}
  
  // Menü-Verwaltung
  void addTab(const String &title);
  void addMenuItem(const String &tabTitle, const String &name, const String &functionName);
  bool loadFromJson(const String &filename);
  
  // Zeichnen-Funktionen
  void drawMenu(bool fullRedraw = false);
  void drawTabs();
  void drawStatusBar();
  void drawMenuItem(int index, int screenIndex, bool selected);
  void drawScrollArrows();
  
  // Touch-Handling
  void handleTouch(int x, int y);
  bool isInBounds(int x, int y, int x1, int y1, int x2, int y2);
  
  // Getter/Setter
  int getCurrentTab() const { return currentTab; }
  int getSelectedMenuItem() const { return selectedMenuItem; }
  String getSelectedFunction() const;
  
  bool isMenuActive() const { return selectedMenuItem < 0; }
  void resetSelection() { selectedMenuItem = -1; touchedMenuItem = -1; }
  
  typedef std::function<void(const String&)> MenuCallback;
  MenuCallback onMenuSelection = nullptr;
};

extern MenuSystem menuSystem;

#endif // MENU_SYSTEM_H