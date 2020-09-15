#ifndef SDInterface_h
#define SDInterface_h

#include "SD.h"
#include "Buffer.h"
#include "Display.h"
#include <Update.h>

extern Buffer buffer_obj;
extern Display display_obj;

// TODO: move this pin number to a config file
#define SD_CS 12

class SDInterface {

  private:

  public:
    uint8_t cardType;
    //uint64_t cardSizeBT;
    //uint64_t cardSizeKB;
    uint64_t cardSizeMB;
    //uint64_t cardSizeGB;
    bool supported = false;
    bool do_save = true;

    String card_sz;

    bool initSD( int sdCs=SD_CS );

    void addPacket(uint8_t* buf, uint32_t len);
    void openCapture(String file_name = "");
    void runUpdate();
    void performUpdate(Stream &updateSource, size_t updateSize);
    void main();
    //void savePacket(uint8_t* buf, uint32_t len);
};

#endif
