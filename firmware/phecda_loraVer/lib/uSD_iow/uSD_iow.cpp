#include "uSD_iow.h"


USD_IOW::USD_IOW(int SD_CS)
{
    this->SD_CS = SD_CS;
    //uSD_init();
}

void USD_IOW::uSD_init()
{
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS))
  {
    Serial.println("Card Mount Failed");
    //return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    //return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if(cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if(cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.println("Initializing SD card...");

  if (!SD.begin(SD_CS))
  {
    Serial.println("ERROR - SD card initialization failed!");
    //return; // init failed
  }
}

void USD_IOW::newFile(const char * path) {
  File file = SD.open(path,FILE_APPEND);
  if (!file)
  {
    Serial.println("Creating file...");
    writeFile(SD, path, "Hora, Latitude, Longitude, Altitude, Speed, Heading, SIV, AccelX, AccelY, AccelZ, GyroX, GyroY, GyroZ\r\n");
  }
  file.close();
}

float USD_IOW::getFreeSpace() {
  float cardSize = SD.cardSize();
  float freeSpace = cardSize -  SD.usedBytes();
  float freeSpacePerc = round(freeSpace/cardSize*100);

  //Serial.printf("Free space: %.2f\n", freeSpacePerc);

  return freeSpacePerc;
}

void USD_IOW::listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels)
            {
                listDir(fs, file.name(), levels -1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void USD_IOW::createDir(fs::FS &fs, const char * path)
{
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path))
    {
        Serial.println("Dir created");
    }
    else
    {
        Serial.println("mkdir failed");
    }
}

void USD_IOW::removeDir(fs::FS &fs, const char * path)
{
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path))
    {
        Serial.println("Dir removed");
    }
    else
    {
        Serial.println("rmdir failed");
    }
}

void USD_IOW::readFile(fs::FS &fs, const char * path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

void USD_IOW::writeFile(fs::FS &fs, const char * path, const char * message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message))
    {
        Serial.println("File written");
    }
    else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void USD_IOW::appendFile(fs::FS &fs, const char * path, const char * message){
    //Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    file.print(message);
    //    Serial.println("Message appended");
    //else
    //    Serial.println("Append failed");
    file.close();
}

void USD_IOW::renameFile(fs::FS &fs, const char * path1, const char * path2)
{
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2))
    {
        Serial.println("File renamed");
    }
    else
    {
        Serial.println("Rename failed");
    }
}

void USD_IOW::deleteFile(fs::FS &fs, const char * path)
{
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path))
    {
        Serial.println("File deleted");
    }
    else
    {
        Serial.println("Delete failed");
    }
}
