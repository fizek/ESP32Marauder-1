/* FLASH SETTINGS
Board: M5Core2
Flash Frequency: 80MHz
Partition Scheme: Minimal SPIFFS
https://www.online-utility.org/image/convert/to/XBM
*/

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


void setup()
{

  //Serial.begin(115200);
  M5.begin();
  checkSDUpdater( SD, MENU_BIN, 0 ); // 0 = no delay (trigger by using ESP.restart() instead)

  //TODO: make this a setting
  //pinMode(FLASH_BUTTON, INPUT);

  #if defined HAS_BATTERY && BATTERY_ANALOG_ON == 1
    pinMode(BATTERY_PIN, OUTPUT);
    pinMode(CHARGING_PIN, INPUT);
  #endif

  display_obj.RunSetup();
  tft.setTextColor(TFT_CYAN, TFT_BLACK);

  tft.drawJpg( marauder192x144_jpg, marauder192x144_jpg_len, tft.width()/2-192/2, tft.height()/2-144/2, 192, 144 );
  tft.drawCentreString(display_obj.version_number, 120, 250, 2);

  //tft.println("Marauder " + display_obj.version_number + "\n");
  //tft.setCursor(0, 250);
  //tft.println("Started Serial");

  Serial.println("\n\n--------------------------------\n");
  Serial.println("         ESP32 Marauder      \n");
  Serial.println("            " + display_obj.version_number + "\n");
  Serial.println("       By: justcallmekoko\n");
  Serial.println("--------------------------------\n\n");

  //Serial.println("Internal Temp: " + (String)((temprature_sens_read() - 32) / 1.8));
  //Serial.println(wifi_scan_obj.freeRAM());

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

  #if defined HAS_BATTERY
    // Battery stuff
    //Serial.println(wifi_scan_obj.freeRAM());
    battery_obj.RunSetup();
  #endif

  tft.println("Checked battery configuration");

  // Temperature stuff
  //Serial.println(wifi_scan_obj.freeRAM());
  temp_obj.RunSetup();
  tft.println("Initialized temperature interface");


  #if defined HAS_BATTERY
    battery_obj.battery_level = battery_obj.getBatteryLevel();
    if (battery_obj.i2c_supported) {
      Serial.println("IP5306 I2C Supported: true");
    } else {
      Serial.println("IP5306 I2C Supported: false");
    }
  #endif

  //Serial.println(wifi_scan_obj.freeRAM());

  #if defined HAS_RGBLED
    // Do some LED stuff
    led_obj.RunSetup();
    tft.println("Initialized LED Interface");
  #endif



  tft.println("Starting...");

  delay(5000);

  /*
  display_obj.clearScreen();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Draw the title screen
  //tft.drawJpgFile( SPIFFS, "marauder3L1-landscape.jpg", 0 , 0); // 240 x 320 image
  tft.drawJpg( marauder192x144_jpg, marauder192x144_jpg_len, tft.width()/2-192/2, tft.height()/2-144/2, 192, 144 );
  tft.drawCentreString(display_obj.version_number, 120, 250, 2);
  //showCenterText(version_number, 250);
  delay(5000);*/

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
