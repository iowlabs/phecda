#ifndef __USD_IOW_H__
#define __USD_IOW_H__

#include "FS.h"
#include "SD.h"
#include "SPI.h"
//#include "sd_diskio.h"
//#include "ff.h"
//#include "FS.h"
//#include "vfs_api.h"

class USD_IOW
{
    private:
        int SD_CS;
        bool sd_init = false;
        bool iow_status = true;

    public:
        USD_IOW();
        bool uSD_init(uint8_t CS);
        void newFile(const char * path, const char * path_test);
        void newFile(const char * path);
        float getFreeSpace();
        void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
        void createDir(fs::FS &fs, const char * path);
        void removeDir(fs::FS &fs, const char * path);
        void readFile(fs::FS &fs, const char * path);
        void writeFile(fs::FS &fs, const char * path, const char * message);
        void appendFile(fs::FS &fs, const char * path, const char * message);
        void renameFile(fs::FS &fs, const char * path1, const char * path2);
        void deleteFile(fs::FS &fs, const char * path);
        bool get_init();
        bool get_iow_status();
        void statusFile(fs::FS &fs, const char * path);
};


#endif  // End of __USD_IOW_H__ definition check
