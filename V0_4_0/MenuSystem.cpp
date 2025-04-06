/**
 * MenuSystem.cpp - Implementierung des Touch-Menüs
 */

#include "MenuSystem.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"

// Globale Instanz wird in der externen Datei definiert (Hauptdatei)
// Hier nur extern deklariert
extern MenuSystem menuSystem;
extern TFT_eSPI tft;

void MenuSystem::addTab(const String &title) {
  tabs.push_back(MenuTab(title));
}

void MenuSystem::addMenuItem(const String &tabTitle, const String &name, const String &functionName) {
  // Suche den Tab mit dem angegebenen Titel
  for (auto &tab : tabs) {
    if (tab.title == tabTitle) {
      tab.addItem(MenuItem(name, functionName));
      return;
    }
  }
  
  // Tab nicht gefunden, erstelle neuen Tab
  MenuTab newTab(tabTitle);
  newTab.addItem(MenuItem(name, functionName));
  tabs.push_back(newTab);
}

bool MenuSystem::loadFromJson(const String &filename) {
  // Bestehende Tabs und Menüeinträge leeren
  tabs.clear();
  
  // JSON-Konfiguration laden
  JsonDocument doc; // Die Größenbeschränkung ist nicht mehr erforderlich
  if (!configManager.loadJsonConfig(filename, doc)) {
    return false;
  }
  
  // Tabs und Menüeinträge aus JSON laden
  JsonArray tabsArray = doc["tabs"];
  if (!tabsArray || tabsArray.size() == 0) {
    DEBUG_PRINTLN("Keine Tabs in der Menü-Konfiguration gefunden");
    
    // Erstelle ein Standard-Menü anstatt abzustürzen
    addTab("System");
    addMenuItem("System", "Standardansicht", "drawSolarStatus");
    
    // Vollständiges Redraw erforderlich
    needsFullRedraw = true;
    
    return false;
  }
  
  for (JsonObject tabObj : tabsArray) {
    String tabTitle = tabObj["title"].as<String>();
    
    // Tab hinzufügen
    addTab(tabTitle);
    
    // Menüeinträge für Tab laden
    JsonArray itemsArray = tabObj["items"];
    if (itemsArray) {
      for (JsonObject itemObj : itemsArray) {
        String itemName = itemObj["name"].as<String>();
        String functionName = itemObj["function"].as<String>();
        
        // Menüeintrag hinzufügen
        addMenuItem(tabTitle, itemName, functionName);
      }
    }
  }
  
  DEBUG_PRINT("Menü-Konfiguration geladen: ");
  DEBUG_PRINT(tabs.size());
  DEBUG_PRINTLN(" Tabs");
  
  // Grundeinstellungen zurücksetzen
  currentTab = 0;
  scrollPosition = 0;
  selectedMenuItem = -1;
  touchedMenuItem = -1;
  
  // Vollständiges Redraw erforderlich
  needsFullRedraw = true;
  
  return true;
}

void MenuSystem::drawMenu(bool fullRedraw) {
  if (fullRedraw || needsFullRedraw) {
    // Bildschirm löschen
    tft.fillScreen(BACKGROUND);
    
    // Tabs zeichnen
    drawTabs();
    
    // Statusleiste zeichnen
    drawStatusBar();
    
    // Scroll-Pfeile zeichnen
    drawScrollArrows();
    
    // Alle Menüpunkte zeichnen
    int maxItems = tabs[currentTab].items.size();
    for (int i = 0; i < min(MENU_VISIBLE_ITEMS, maxItems - scrollPosition); i++) {
      drawMenuItem(i + scrollPosition, i, (i + scrollPosition) == touchedMenuItem);
    }
    
    needsFullRedraw = false;
  } else {
    // Partielles Redraw - nur was sich geändert hat
    
    // Scroll-Pfeile aktualisieren, falls nötig
    if (prevTouchedUpScroll != touchedUpScroll || 
        prevTouchedDownScroll != touchedDownScroll ||
        prevScrollPosition != scrollPosition) {
      drawScrollArrows();
    }
    
    // Menüpunkte bei Bedarf neu zeichnen
    int maxItems = tabs[currentTab].items.size();
    if (prevScrollPosition != scrollPosition) {
      // Bei Scroll-Änderung alle sichtbaren Punkte neu zeichnen
      for (int i = 0; i < min(MENU_VISIBLE_ITEMS, maxItems - scrollPosition); i++) {
        drawMenuItem(i + scrollPosition, i, (i + scrollPosition) == touchedMenuItem);
      }
    } else if (previousTouchedMenuItem != touchedMenuItem) {
      // Nur die geänderten Elemente neu zeichnen
      if (previousTouchedMenuItem >= 0 && 
          previousTouchedMenuItem >= scrollPosition && 
          previousTouchedMenuItem < scrollPosition + MENU_VISIBLE_ITEMS) {
        drawMenuItem(previousTouchedMenuItem, previousTouchedMenuItem - scrollPosition, false);
      }
      
      if (touchedMenuItem >= 0 && 
          touchedMenuItem >= scrollPosition && 
          touchedMenuItem < scrollPosition + MENU_VISIBLE_ITEMS) {
        drawMenuItem(touchedMenuItem, touchedMenuItem - scrollPosition, true);
      }
    }
  }
  
  // Speichere aktuelle Zustände für nächstes partielles Redraw
  prevScrollPosition = scrollPosition;
  previousTouchedMenuItem = touchedMenuItem;
  prevTouchedUpScroll = touchedUpScroll;
  prevTouchedDownScroll = touchedDownScroll;
}

void MenuSystem::drawTabs() {
  for (size_t i = 0; i < tabs.size(); i++) {
    int tabX = i * TAB_WIDTH + 10;
    uint16_t tabColor = (i == (size_t)currentTab) ? TAB_ACTIVE_COLOR : TAB_INACTIVE_COLOR;
    
    // Tab zeichnen
    tft.fillRoundRect(tabX, 10, TAB_WIDTH - 5, TAB_HEIGHT, 5, tabColor);
    tft.drawRoundRect(tabX, 10, TAB_WIDTH - 5, TAB_HEIGHT, 5, TFT_WHITE);
    
    // Tab-Text
    tft.setTextColor(TEXT_COLOR, tabColor);
    tft.setTextSize(1);
    
    // Text zentrieren
    int textWidth = tabs[i].title.length() * 6; // Ungefähre Breite bei Textgröße 1
    int textX = tabX + (TAB_WIDTH - 5 - textWidth) / 2;
    
    tft.setCursor(textX, 22);
    tft.print(tabs[i].title);
  }
  
  // Bereich zwischen Tabs und Menü löschen
  tft.fillRect(0, TAB_HEIGHT + 10, SCREEN_WIDTH, MENU_START_Y - TAB_HEIGHT - 10, BACKGROUND);
}

void MenuSystem::drawStatusBar() {
  // Hintergrund für Statusleiste
  tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, BACKGROUND);
  
  // Hier nur ein Platzhalter - die eigentliche StatusBar-Implementierung
  // sollte außerhalb dieser Klasse erfolgen, um Abhängigkeiten zu WiFi etc.
  // nicht in diese Klasse zu integrieren
  tft.setTextSize(1);
  tft.setTextColor(TEXT_COLOR, BACKGROUND);
  tft.setCursor(10, SCREEN_HEIGHT - 15);
  tft.print("Menü aktiv");
}

void MenuSystem::drawMenuItem(int index, int screenIndex, bool selected) {
  if (currentTab >= (int)tabs.size() || index >= (int)tabs[currentTab].items.size()) {
    return;  // Sicherheitscheck
  }
  
  int y = MENU_START_Y + screenIndex * MENU_ITEM_HEIGHT;
  uint16_t itemColor;
  uint16_t textColor;
  
  if (selected) {
    // Aktuell berührt
    itemColor = HIGHLIGHT_COLOR;
    textColor = TEXT_COLOR;
  } else if (index == selectedMenuItem && currentTab == currentTab) {
    // Vorher ausgewählt
    itemColor = BORDER_COLOR;
    textColor = TEXT_COLOR;
  } else {
    // Standard
    itemColor = BACKGROUND;
    textColor = TEXT_COLOR;
  }
  
  // Rechteck um Menüpunkt zeichnen
  tft.fillRoundRect(MENU_START_X, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT - 5, 5, itemColor);
  tft.drawRoundRect(MENU_START_X, y, MENU_ITEM_WIDTH, MENU_ITEM_HEIGHT - 5, 5, BORDER_COLOR);
  
  // Text zeichnen
  tft.setTextColor(textColor, itemColor);
  tft.setTextSize(2);
  tft.setCursor(MENU_START_X + 20, y + (MENU_ITEM_HEIGHT - 5)/2 - 7);
  tft.print(tabs[currentTab].items[index].name);
}

void MenuSystem::drawScrollArrows() {
  // Bereich für die Scroll-Pfeile löschen
  int arrowX = MENU_START_X + MENU_ITEM_WIDTH + 10;
  int arrowRegionY = MENU_START_Y;
  int arrowRegionHeight = MENU_VISIBLE_ITEMS * MENU_ITEM_HEIGHT;
  
  tft.fillRect(arrowX, arrowRegionY, SCROLL_ARROW_WIDTH + 5, arrowRegionHeight, BACKGROUND);
  
  // Pfeil nach oben
  int upArrowY = MENU_START_Y + MENU_VISIBLE_ITEMS * MENU_ITEM_HEIGHT / 2 - 30;
  if (scrollPosition > 0) {
    uint16_t arrowColor = touchedUpScroll ? SCROLL_ACTIVE_COLOR : SCROLL_INACTIVE_COLOR;
    
    tft.fillTriangle(
      arrowX + SCROLL_ARROW_WIDTH/2, upArrowY,
      arrowX, upArrowY + 15,
      arrowX + SCROLL_ARROW_WIDTH, upArrowY + 15,
      arrowColor
    );
  }
  
  // Pfeil nach unten
  int downArrowY = MENU_START_Y + MENU_VISIBLE_ITEMS * MENU_ITEM_HEIGHT / 2 + 10;
  if (scrollPosition < (int)tabs[currentTab].items.size() - MENU_VISIBLE_ITEMS) {
    uint16_t arrowColor = touchedDownScroll ? SCROLL_ACTIVE_COLOR : SCROLL_INACTIVE_COLOR;
    
    tft.fillTriangle(
      arrowX + SCROLL_ARROW_WIDTH/2, downArrowY + 15,
      arrowX, downArrowY,
      arrowX + SCROLL_ARROW_WIDTH, downArrowY,
      arrowColor
    );
  }
}
void MenuSystem::handleTouch(int x, int y) {
  // Speichere vorherige Zustände für partielles Redraw
  prevTouchedUpScroll = touchedUpScroll;
  prevTouchedDownScroll = touchedDownScroll;
  previousTouchedMenuItem = touchedMenuItem;
  
  // Zurücksetzen der Berührungsflächen
  touchedUpScroll = false;
  touchedDownScroll = false;
  touchedMenuItem = -1;
  
  // Prüfe, ob ein Tab berührt wurde
  for (size_t i = 0; i < tabs.size(); i++) {
    int tabX = i * TAB_WIDTH + 10;
    if (isInBounds(x, y, tabX, 10, tabX + TAB_WIDTH - 5, TAB_HEIGHT)) {
      if (currentTab != (int)i) {
        currentTab = i;
        scrollPosition = 0;  // Zurück zum Anfang bei Tab-Wechsel
        needsFullRedraw = true; // Vollständiges Redraw erforderlich
      }
      return; // Weitere Prüfungen überspringen
    }
  }
  
  // Prüfe auf Scroll-nach-oben Button
  int arrowX = MENU_START_X + MENU_ITEM_WIDTH + 10;
  int upArrowY = MENU_START_Y + MENU_VISIBLE_ITEMS * MENU_ITEM_HEIGHT / 2 - 30;
  
  if (scrollPosition > 0 && 
      isInBounds(x, y, arrowX, upArrowY, arrowX + SCROLL_ARROW_WIDTH, upArrowY + 15)) {
    scrollPosition--;
    touchedUpScroll = true;
    return;
  }
  
  // Prüfe auf Scroll-nach-unten Button
  int downArrowY = MENU_START_Y + MENU_VISIBLE_ITEMS * MENU_ITEM_HEIGHT / 2 + 10;
  if (scrollPosition < (int)tabs[currentTab].items.size() - MENU_VISIBLE_ITEMS && 
      isInBounds(x, y, arrowX, downArrowY, arrowX + SCROLL_ARROW_WIDTH, downArrowY + 15)) {
    scrollPosition++;
    touchedDownScroll = true;
    return;
  }
  
  // Prüfe, ob ein Menüpunkt berührt wurde
  for (int i = 0; i < min(MENU_VISIBLE_ITEMS, (int)tabs[currentTab].items.size() - scrollPosition); i++) {
    int index = i + scrollPosition;
    int menuItemY = MENU_START_Y + i * MENU_ITEM_HEIGHT;
    
    if (isInBounds(x, y, 
                  MENU_START_X, menuItemY, 
                  MENU_START_X + MENU_ITEM_WIDTH, menuItemY + MENU_ITEM_HEIGHT)) {
      touchedMenuItem = index;
      
      // Menüauswahl verarbeiten, wenn Callback gesetzt ist
      if (onMenuSelection) {
        onMenuSelection(tabs[currentTab].items[index].functionName);
      }
      
      // Menüpunkt als ausgewählt markieren
      selectedMenuItem = index;
      
      return;
    }
  }
}

// Hilfsfunktion zur Überprüfung der Bereichsgrenzen
bool MenuSystem::isInBounds(int x, int y, int x1, int y1, int x2, int y2) {
  return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

// Funktion zur Rückgabe der aktuell ausgewählten Funktion
String MenuSystem::getSelectedFunction() const {
  if (currentTab >= 0 && currentTab < (int)tabs.size() && 
      selectedMenuItem >= 0 && selectedMenuItem < (int)tabs[currentTab].items.size()) {
    return tabs[currentTab].items[selectedMenuItem].functionName;
  }
  return "";
}
      