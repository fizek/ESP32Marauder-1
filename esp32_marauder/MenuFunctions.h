#ifndef MenuFunctions_h
#define MenuFunctions_h

#define BATTERY_ANALOG_ON 0

#include "WiFiScan.h"
#include "Display.h"
#include "BatteryInterface.h"
#include "SDInterface.h"
#include "Web.h"


extern Display display_obj;
extern WiFiScan wifi_scan_obj;
extern Web web_obj;
extern SDInterface sd_obj;
extern BatteryInterface battery_obj;

// Keypad start position, key sizes and spacing
#define KEY_X 120 // Centre of key
#define KEY_Y 50
#define KEY_W 240 // Width and height
#define KEY_H 22
#define KEY_SPACING_X 0 // X and Y gap
#define KEY_SPACING_Y 10 // space for fingers FFS !!
#define KEY_TEXTSIZE 1   // Font size multiplier
#define ICON_W 22
#define ICON_H 22
#define BUTTON_PADDING 22
//#define BUTTON_ARRAY_LEN 5

#define FLASH_BUTTON 0

#if BATTERY_ANALOG_ON == 1
#define BATTERY_PIN 13
#define ANALOG_PIN 34
#define CHARGING_PIN 27
#endif

// Icon definitions
#define ATTACKS 0
#define BEACON_SNIFF 1
#define BLUETOOTH 2
#define BLUETOOTH_SNIFF 3
#define DEAUTH_SNIFF 4
#define DRAW 5
#define PACKET_MONITOR 6
#define PROBE_SNIFF 7
#define SCANNERS 8
#define CC_SKIMMERS 9
#define SNIFFERS 10
#define WIFI 11
#define BEACON_SPAM 12
#define RICK_ROLL 13
#define REBOOT 14
#define GENERAL_APPS 15
#define UPDATE 16
#define DEVICE 17
#define DEVICE_INFO 18
#define SD_UPDATE 19
#define WEB_UPDATE 20
#define EAPOL 21
#define STATUS_BAT 22
#define STATUS_SD 23
#define PWNAGOTCHI 24
#define ESPRESSIF 25

struct Menu;

// Individual Nodes of a menu

struct MenuNode {
  String name;
  uint16_t color;
  int icon;
  TouchButton* button;
  std::function<void()> callable;
};

// Full Menus
struct Menu {
  String name;
  LinkedList<MenuNode>* list;
  Menu                * parentMenu;
  //uint8_t               selected;
};


class MenuFunctions
{
  private:

    String u_result = "";

    uint32_t initTime = 0;

    Menu* current_menu;

    // Main menu stuff
    Menu mainMenu;

    Menu wifiMenu;
    Menu bluetoothMenu;
    Menu generalMenu;
    Menu deviceMenu;

    // Device menu stuff
    Menu whichUpdateMenu;
    Menu failedUpdateMenu;
    Menu confirmMenu;
    Menu updateMenu;
    Menu infoMenu;

    // WiFi menu stuff
    Menu wifiSnifferMenu;
    Menu wifiScannerMenu;
    Menu wifiAttackMenu;

    // Bluetooth menu stuff
    Menu bluetoothSnifferMenu;
    Menu bluetoothScannerMenu;

    // Menu icons


    //TFT_eSPI_Button key[BUTTON_ARRAY_LEN];

    void addNodes(Menu* menu, String name, uint16_t color, Menu* child, int place, std::function<void()> callable);
    void drawStatusBar();
    void updateStatusBar();
    void battery(bool initial = false);
    void battery2(bool initial = false);
    void showMenuList(Menu* menu, int layer);
    void orientDisplay();

  public:
    MenuFunctions();

    uint16_t x = -1, y = -1;
    boolean pressed = false;

    void buildButtons(Menu* menu);
    void changeMenu(Menu* menu);
    void displayCurrentMenu();
    void main(uint32_t currentTime);
    void RunSetup();
};



#endif
