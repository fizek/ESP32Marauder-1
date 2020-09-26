/* FLASH SETTINGS
Board: LOLIN D32
Flash Frequency: 80MHz
Partition Scheme: Minimal SPIFFS
https://www.online-utility.org/image/convert/to/XBM
*/


//#include <ESP32-Chimera-Core.h> // https://github.com/tobozo/ESP32-Chimera-Core or regular M5Stack Core
//#define tft M5.Lcd

#define TOUCH_CS 21 // Touch CS pin for XPT2046 interface (SPI Shared)
#define SD_CS_PIN 4 // comment this out to use ESP32Marauder defaults
//#define HAS_BATTERY // uncomment this to use ESP32Marauder battery
//#define HAS_RGBLED // uncomment this to use ESP32Marauder RGB led


#include <WiFi.h>
#include <Wire.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <Arduino.h>


#include "Assets.h"
#include "Display.h"
#include "WiFiScan.h"
#include "MenuFunctions.h"
#include "SDInterface.h"
#include "Web.h"
#include "Buffer.h"
#include "BatteryInterface.h"
#include "TemperatureInterface.h"
#include "LedInterface.h"
//#include "icons.h"

/*
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
*/

Display display_obj;
WiFiScan wifi_scan_obj;
MenuFunctions menu_function_obj;
SDInterface sd_obj;
Web web_obj;
Buffer buffer_obj;
BatteryInterface battery_obj;
TemperatureInterface temp_obj;
LedInterface led_obj;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, PIN, NEO_GRB + NEO_KHZ800);

uint32_t currentTime  = 0;

static lgfx::Touch_XPT2046 touch;
static lgfx::Panel_ILI9341 panel;

void setup()
{

  Serial.begin(115200);
  M5.begin( false, false, false );

  auto p = new lgfx::Panel_ILI9341();
  p->spi_3wire = false;
  p->spi_cs    = TFT_CS;  // 14
  p->spi_dc    = TFT_DC;  // 27
  p->gpio_rst  = TFT_RST; // 33
  p->gpio_bl   = TFT_LED; // 32
  p->pwm_ch_bl = 7;

  tft.setPanel(p);

  auto t = new lgfx::Touch_XPT2046(); // sharing SPI with TFT
  t->spi_mosi = MOSI; // 23
  t->spi_miso = MISO; // 19
  t->spi_sclk = SCK;  // 18
  t->spi_cs   = TOUCH_CS; // TOUCH_CS may be custom build spefific ?
  t->spi_host = VSPI_HOST;
  t->bus_shared = true;
  t->freq     = 1600000;

  tft.touch(t);

  tft.begin();

  if (tft.width() > 240 || tft.height() > 240) tft.setTextSize(2);
  if (tft.touch())
  {
    if (tft.width() < tft.height()) tft.setRotation(3 & (tft.getRotation() + 1));
    tft.drawString("touch the arrow marker.", 0, tft.height()>>1);
    tft.calibrateTouch(nullptr, 0xFFFFFFU, 0x000000U, 30);
    tft.clear();
  }

  //TODO: make this a setting
  //pinMode(FLASH_BUTTON, INPUT);

  #if defined HAS_BATTERY && BATTERY_ANALOG_ON == 1
    pinMode(BATTERY_PIN, OUTPUT);
    pinMode(CHARGING_PIN, INPUT);
  #endif

  display_obj.RunSetup();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);

  tft.println("Marauder " + display_obj.version_number + "\n");
  tft.println("Started Serial");

  Serial.println("\n\n--------------------------------\n");
  Serial.println("         ESP32 Marauder      \n");
  Serial.println("            " + display_obj.version_number + "\n");
  Serial.println("       By: justcallmekoko\n");
  Serial.println("--------------------------------\n\n");

  //Serial.println("Internal Temp: " + (String)((temprature_sens_read() - 32) / 1.8));

  Serial.println(wifi_scan_obj.freeRAM());

  tft.println("Checked RAM");

  // Do some SD stuff
  #if defined SD_CS_PIN // let DIY profiles override the SD CS pin
    bool sdInited = sd_obj.initSD( SD_CS_PIN /* 12=ESP32Marauder, 4=SD_CS for LoLinD32Pro, etc */ );
  #else
    bool sdInited = sd_obj.initSD();
  #endif
  if( sdInited ) {
    Serial.println("SD Card supported");
    tft.println("Initialized SD Card");
  } else {
    Serial.println("SD Card NOT Supported");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Failed to Initialize SD Card");
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
  }

  // Run display setup
  Serial.println(wifi_scan_obj.freeRAM());
  display_obj.RunSetup();

  tft.println("Created Menu Structure");

  #if defined HAS_BATTERY
    // Battery stuff
    Serial.println(wifi_scan_obj.freeRAM());
    battery_obj.RunSetup();
  #endif

  tft.println("Checked battery configuration");

  // Temperature stuff
  Serial.println(wifi_scan_obj.freeRAM());
  temp_obj.RunSetup();

  tft.println("Initialized temperature interface");

  #if defined HAS_BATTERY
    battery_obj.battery_level = battery_obj.getBatteryLevel();
  #endif

  if (battery_obj.i2c_supported) {
    Serial.println("IP5306 I2C Supported: true");
  } else {
    Serial.println("IP5306 I2C Supported: false");
  }

  Serial.println(wifi_scan_obj.freeRAM());

  #if defined HAS_RGBLED
    // Do some LED stuff
    led_obj.RunSetup();
  #endif

  tft.println("Initialized LED Interface");

  tft.println("Starting...");

  delay(1000);

  display_obj.clearScreen();

  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Draw the title screen
  tft.drawJpgFile( SPIFFS, "/marauder3L.jpg", 0 , 0);     // 240 x 320 image

  //showCenterText(version_number, 250);
  tft.drawCentreString(display_obj.version_number, 120, 250, 2);

  delay(5000);

  // Build menus
  menu_function_obj.RunSetup();

}


void loop()
{
  // get the current time
  //if ((wifi_scan_obj.currentScanMode != WIFI_ATTACK_BEACON_SPAM))
  currentTime = millis();

  // Update all of our objects
  //if ((!display_obj.draw_tft) &&
  //    (wifi_scan_obj.currentScanMode != OTA_UPDATE))
  if (!display_obj.draw_tft)
  {
    display_obj.main();
    wifi_scan_obj.main(currentTime);
    sd_obj.main();
    battery_obj.main(currentTime);
    temp_obj.main(currentTime);
    //led_obj.main(currentTime);
    //if ((wifi_scan_obj.currentScanMode != WIFI_ATTACK_BEACON_SPAM))
    if ((wifi_scan_obj.currentScanMode != WIFI_PACKET_MONITOR) &&
        (wifi_scan_obj.currentScanMode != WIFI_SCAN_EAPOL))
      menu_function_obj.main(currentTime);
      if (wifi_scan_obj.currentScanMode == OTA_UPDATE)
        web_obj.main();
    delay(1);
  }
  else if ((display_obj.draw_tft) &&
           (wifi_scan_obj.currentScanMode != OTA_UPDATE))
  {
    display_obj.drawStylus();
  }
  //else
  //{
  //  web_obj.main();
  //}

  //Serial.println(wifi_scan_obj.currentScanMode);

  //Serial.print("Run Time: ");
  //Serial.print(millis() - currentTime);
  //Serial.println("ms");
}
