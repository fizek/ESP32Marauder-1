#include "SDInterface.h"

bool SDInterface::initSD( int sdCs ) {
  String display_string = "";

  if (!SD.begin( sdCs )) {
    Serial.println("Failed to mount SD Card");
    this->supported = false;
    return false;
  }
  else {
    this->supported = true;
    this->cardType = SD.cardType();
    if (cardType == CARD_MMC)
      Serial.println("SD: MMC Mounted");
    else if(cardType == CARD_SD)
        Serial.println("SD: SDSC Mounted");
    else if(cardType == CARD_SDHC)
        Serial.println("SD: SDHC Mounted");
    else
        Serial.println("SD: UNKNOWN Card Mounted");

    //this->cardSizeBT = SD.cardSize();
    //this->cardSizeKB = SD.cardSize() / 1024;
    this->cardSizeMB = SD.cardSize() / (1024 * 1024);
    //this->cardSizeGB = SD.cardSize() / (1024 * 1024 * 1024);

    //Serial.printf("SD Card Size: %llu Bytes\n", this->cardSizeBT);
    //Serial.printf("SD Card Size: %lluKB\n", this->cardSizeKB);
    Serial.printf("SD Card Size: %lluMB\n", this->cardSizeMB);
    //Serial.printf("SD Card Size: %lluGB\n", this->cardSizeGB);

    if (this->supported) {
      //tft.println((byte)(sd_obj.cardSizeMB % 10));
      const int NUM_DIGITS = log10(this->cardSizeMB) + 1;

      char sz[NUM_DIGITS + 1];

      sz[NUM_DIGITS] =  0;
      for ( size_t i = NUM_DIGITS; i--; this->cardSizeMB /= 10)
      {
          sz[i] = '0' + (this->cardSizeMB % 10);
          display_string.concat((String)sz[i]);
      }

      //this->card_sz = display_string;
      this->card_sz = sz;
    }

    buffer_obj = Buffer();

    //if (this->supported)
    //  buffer_obj.open(&SD);

    return true;
  }
}

void SDInterface::addPacket(uint8_t* buf, uint32_t len) {
  if ((this->supported) && (this->do_save)) {
    //Serial.println("Adding packet to buffer...");
    buffer_obj.addPacket(buf, len);
  }
}

void SDInterface::openCapture(String file_name) {
  if (this->supported)
    buffer_obj.open(&SD, file_name);
}

void SDInterface::runUpdate() {
  //display_obj.clearScreen();
 tft.setTextWrap(false);
 tft.setFreeFont(NULL);
 tft.setCursor(0, 100);
 tft.setTextSize(1);
 tft.setTextColor(TFT_WHITE);

 tft.println("Opening /update.bin...");
  File updateBin = SD.open("/update.bin");
  if (updateBin) {
    if(updateBin.isDirectory()){
     tft.setTextColor(TFT_RED);
     tft.println("Error, could not find update.bin");
      Serial.println("Error, update.bin is not a file");
     tft.setTextColor(TFT_WHITE);
      updateBin.close();
      return;
    }

    size_t updateSize = updateBin.size();

    if (updateSize > 0) {
     tft.println("Starting SD Update...");
      Serial.println("Try to start update");
      this->performUpdate(updateBin, updateSize);
    }
    else {
     tft.setTextColor(TFT_RED);
     tft.println("Error, update.bin is empty");
      Serial.println("Error, file is empty");
     tft.setTextColor(TFT_WHITE);
      return;
    }

    updateBin.close();

      // whe finished remove the binary from sd card to indicate end of the process
   tft.println("rebooting...");
    Serial.println("rebooting...");
    //SD.remove("/update.bin");
    delay(1000);
    ESP.restart();
  }
  else {
   tft.setTextColor(TFT_RED);
   tft.println("Could not load update.bin from /");
    Serial.println("Could not load update.bin from sd root");
   tft.setTextColor(TFT_WHITE);
  }
}

void SDInterface::performUpdate(Stream &updateSource, size_t updateSize) {
  if (Update.begin(updateSize)) {
   tft.println("File size: " + String(updateSize));
   tft.println("Writing file to partition...");
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
     tft.println("Written: " + String(written) + " successfully");
      Serial.println("Written : " + String(written) + " successfully");
    }
    else {
     tft.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    if (Update.end()) {
      Serial.println("OTA done!");
      if (Update.isFinished()) {
       tft.println("Update complete");
        Serial.println("Update successfully completed. Rebooting.");
      }
      else {
       tft.setTextColor(TFT_RED);
       tft.println("Update could not complete");
        Serial.println("Update not finished? Something went wrong!");
       tft.setTextColor(TFT_WHITE);
      }
    }
    else {
     tft.println("Error Occurred. Error #: " + String(Update.getError()));
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
    }

  }
  else
  {
   tft.println("Not enough space to begin OTA");
    Serial.println("Not enough space to begin OTA");
  }
}

void SDInterface::main() {
  if ((this->supported) && (this->do_save)) {
    //Serial.println("Saving packet...");
    buffer_obj.forceSave(&SD);
  }
}

//void SDInterface::savePacket(uint8_t* buf, uint32_t len) {
//  if (this->supported)
//    buffer_obj.save(
//}
