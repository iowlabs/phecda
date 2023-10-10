/*
Author: WAC@IOWLABS
VERSION: V0.0.1

NAME:
DESCRPIPTIONS:

LIBS:

TODO:

*/

/*-----------------
    Libs
-----------------*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ezo_i2c.h>
#include "Ezo_i2c_util.h"
#include <Wire.h>
#include "uSD_iow.h"
#include <RTClib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>



/*----------------
    Defines
-----------------*/
#define LED          2 // on board led
//#define TEMP_PIN     4
#define I2C_SCL      21
#define I2C_SDA      22
#define SD_CS        5
#define PH_EN        33
#define ORP_EN       32
#define DO_EN        12
#define EC_EN        13
#define RFM_CS       15
#define RFM_RST      4
#define RFM_DIO0     17


// Version (mayor, minor, patch, build)
#define VERSION     "iowph-v.0.1.0.b.1" //Atlas sensor board
#define ID          "ph01" // <------------ MUST change this for the respective pcb. AND modify the pins asignation on the file /boards/bsf_nodemcu_32s.
//#define DATALOGGER  1
#define DEBUG       1
#define INIT_CLOCK_BOOL false

#define N_SAMPLES   10
#define BAUDRATE    9600
#define SAMPLE_TIME 5000
#define SLEEP_TIME  30
#define TIME_FACTOR 1000000
#define RESET       asm("jmp 0x0000")
#define FILE_NAME   "/log.txt"

#define IDLE        0
#define CONTINUOUS  1
#define SLAVE       2
#define TEST        3

#define PH_ADDRESS    100
#define ORP_ADDRESS   101
#define TEMP_ADDRESS  102
#define OD_ADDRESS    103
#define EC_ADDRESS    104

#define PH_OFFSET 0.6



#define IDLE    0
#define RUNNING 1

// Parametros de MQTT
#define MQTT_PORT         1883
#define MQTT_USER         "iowlabs"
#define MQTT_PASSWORD     "!iow_woi!"
#define MQTT_PUBLISH_CH   "ph/tx"
#define MQTT_RECEIVER_CH  "ph/rx"

//Macros for enable serial prints
#if DEBUG
#define printd(s) {Serial.print((s));}
#define printlnd(s) {Serial.println((s));}
#else
#define printd(s)
#define printlnd(s)
#endif

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//----------------------------
//    Instances
//----------------------------
const size_t capacity_rx = JSON_OBJECT_SIZE(5) + 50;
DynamicJsonDocument doc_rx(capacity_rx);
const char* json_rx = "{\"cmd\":\"mode\",\"arg\":1}";
DeserializationError error_rx;

Ezo_board ph_sensor   = Ezo_board(PH_ADDRESS,"PH");
Ezo_board orp_sensor  = Ezo_board(ORP_ADDRESS,"ORP");
Ezo_board temp_sensor = Ezo_board(TEMP_ADDRESS,"RTD");
Ezo_board ec_sensor   = Ezo_board(EC_ADDRESS,"EC");
Ezo_board do_sensor   = Ezo_board(OD_ADDRESS,"OD");

SPIClass spi(HSPI);
#if DATALOGGER
  RTC_DS3231 rtc;
  USD_IOW uSD_card(SD_CS);
#endif

//Wifi and mqtt client
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

/*----------------
    VARIABLES
----------------*/


// Conexión a una red y servidor de MQTT
const char* ssid      = "RC 16 E";//"UDD";//
const char* password  = "123rc16e";//"udd_2019";//

const char* mqtt_server = "35.223.234.244";

  RTC_DS3231 rtc;
  USD_IOW uSD_card(SD_CS);
  RTC_DS3231 rtc;
  USD_IOW uSD_card(SD_CS);
//Parse var
const char* cmd;
int arg = 0;

uint8_t state         = CONTINUOUS;
long time_datalogger  = 0;

//Sensor var
float   ph    = 0.0;
float   temp  = 0.0;
float   orp   = 0.0;
float   ec    = 0.0;
float   od    = 0.0;

//Sensor var
float   ph_avg    = 0.0;
float   temp_avg  = 0.0;
float   orp_avg   = 0.0;
float   ec_avg    = 0.0;
float   od_avg    = 0.0;

bool SD_ini= false;;
bool SD_status= false;;
int  mqtt_try       = 0; // Cantidad de intentos para conectarse
bool mqtt_connected = false;
bool wifi_status = false;

String do_string = "";
String output = "";



// Logo IoWLabs
const unsigned char epd_bitmap_logo_v2_BLACK [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x03, 0xf0, 0x0f, 0xc0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0xff, 0xe3, 0xfc, 0x07, 0xe3, 0xf0, 0xfc, 0x7f, 0x1e, 0x00, 0x00, 0xc0, 0x1e, 0x60, 0x01, 0xc0,
	0xff, 0xe1, 0xf0, 0x01, 0xe3, 0xf0, 0xfc, 0x7f, 0x1e, 0x00, 0x07, 0xf8, 0x1f, 0xf8, 0x0f, 0xf8,
	0xff, 0xe1, 0xe0, 0x40, 0xe1, 0xf0, 0x7c, 0x7f, 0x1e, 0x00, 0x0f, 0xfc, 0x1f, 0xfc, 0x1f, 0xfe,
	0xff, 0xe1, 0xe1, 0xf0, 0xf1, 0xe0, 0x78, 0x7f, 0x1e, 0x00, 0x0f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe,
	0xff, 0xe1, 0xc3, 0xf8, 0x71, 0xe0, 0x78, 0xff, 0x1e, 0x00, 0x06, 0x1e, 0x1f, 0x1e, 0x3e, 0x1c,
	0xff, 0xe1, 0xc3, 0xf8, 0x70, 0xe0, 0x38, 0xff, 0x1e, 0x00, 0x00, 0x0e, 0x1e, 0x0e, 0x3c, 0x00,
	0xff, 0xe1, 0xc3, 0xf8, 0x78, 0xc2, 0x30, 0xff, 0x1e, 0x00, 0x00, 0x0e, 0x1e, 0x0e, 0x3f, 0x00,
	0xff, 0xe1, 0xc3, 0xf8, 0x78, 0xc6, 0x31, 0xff, 0x1e, 0x00, 0x07, 0xfe, 0x1e, 0x0e, 0x1f, 0xf8,
	0xff, 0xe1, 0xc3, 0xf8, 0x78, 0x46, 0x31, 0xff, 0x1e, 0x00, 0x0f, 0xfe, 0x1e, 0x0e, 0x1f, 0xfc,
	0xff, 0xe1, 0xc3, 0xf8, 0x7c, 0x47, 0x01, 0xff, 0x1e, 0x00, 0x1f, 0xfe, 0x1e, 0x0e, 0x07, 0xfe,
	0xff, 0xe1, 0xc3, 0xf8, 0x7c, 0x0f, 0x03, 0xff, 0x1e, 0x00, 0x1e, 0x0e, 0x1e, 0x0e, 0x00, 0x1e,
	0xff, 0xe1, 0xe1, 0xf0, 0xfc, 0x0f, 0x03, 0xff, 0x1e, 0x00, 0x1c, 0x0e, 0x1e, 0x0e, 0x10, 0x1e,
	0xff, 0xe1, 0xe0, 0x40, 0xfe, 0x0f, 0x83, 0xff, 0x1e, 0x00, 0x1e, 0x0e, 0x1e, 0x1e, 0x1c, 0x1e,
	0xff, 0xe1, 0xf0, 0x01, 0xfe, 0x0f, 0x87, 0xff, 0x1f, 0xff, 0x9f, 0xfe, 0x1f, 0xfe, 0x3f, 0xfe,
	0xff, 0xe3, 0xfc, 0x07, 0xff, 0x1f, 0x87, 0xff, 0x1f, 0xff, 0x9f, 0xfe, 0x1f, 0xfc, 0x3f, 0xfc,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0x8f, 0xfe, 0x1f, 0xfc, 0x1f, 0xf8,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x1f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x07, 0xf8, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0xfc, 0x03, 0xf0, 0x0f, 0xe0, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};




/*----------------
    FUNCTIONS
----------------*/
void    readAtlasSensor( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s);
void    readDummySensors( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s);
void    readSensors();
void    initRTC(void);
void    writeDatalogger(String msg);
String  pubSensors(void);
void    publishMqtt(char *serialData);
void    mainTask(void);
void    setupWiFi();
bool    reconnect();
String  status(void);
void    mostrarparametros(void);

void setup()
{
  Serial.begin(115200);
  display.begin(0x3c,true);

display.setRotation(2); //IF NOT INVERTED COMMENT THIS LINE
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SH110X_WHITE);
display.display(); // Initialize with display cleared
display.clearDisplay(); // Clear the buffer
  pinMode(LED,OUTPUT);
  pinMode(PH_EN,OUTPUT);
  pinMode(ORP_EN,OUTPUT);
  pinMode(DO_EN,OUTPUT);
  pinMode(EC_EN,OUTPUT);

  digitalWrite(PH_EN,1);
  digitalWrite(ORP_EN,1);
  digitalWrite(DO_EN,1);
  digitalWrite(EC_EN,1);
  digitalWrite(SD_CS,1);

  pinMode(26,OUTPUT);
  digitalWrite(26,1);


  Wire.begin();

  #if DATALOGGER
   initRTC();
   uSD_card.uSD_init();
   uSD_card.newFile(FILE_NAME);
  #endif

  //INIT LORA
  LoRa.setPins(RFM_CS,RFM_RST,RFM_DIO0);
  while(!LoRa.begin(915E6))
  {
      printlnd(".");
      delay(500);
  }
  LoRa.setSyncWord(0xF3);
  printlnd("Setup Ready");

  mainTask();
}

void loop(){}

void mainTask()
{
  printlnd("uploading data");
  #if DATALOGGER
    DateTime now = rtc.now();
    time_datalogger = now.unixtime();
  #else
    time_datalogger = millis();
  #endif
  readSensors();
  output = pubSensors();
  writeDatalogger(output);
  LoRa.beginPacket();
  LoRa.print(output);
  LoRa.endPacket();
  //WIFI SETUP
  printlnd("going to sleep");

  //mostrarparametros();

  //WiFi.disconnect( true );


  //Ponemos a dormir a la ESP32
  esp_sleep_enable_timer_wakeup(SLEEP_TIME*TIME_FACTOR);
  esp_deep_sleep_start();
}

void setupWiFi()
{
    delay(10);
    printd("Connecting to ");
    printlnd(ssid);
    WiFi.begin(ssid, password);
    uint8_t try_wifi = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        digitalWrite(LED,HIGH);
        delay(500);
        digitalWrite(LED,LOW);

        printd(".");
        try_wifi += 1;
        if (try_wifi ==30)
        {
          wifi_status = false;
          return;
        }
    }
    printlnd("WiFi connected");
    wifi_status = true;
    printd("IP address: ");
    printlnd(WiFi.localIP());
    digitalWrite(LED,HIGH);
}

// Funcion para reconectar a la red en caso que se pierda la conexion
bool reconnect()
{
    while (!mqtt.connected())
    {
        printd("Attempting MQTT connection...");
        if (mqtt.connect(ID, MQTT_USER , MQTT_PASSWORD))
        {
            printlnd("connected");
            mqtt.subscribe(MQTT_RECEIVER_CH);
            //mqtt.publish(MQTT_PUBLISH_CH, "test");

            mqtt_try = 0;
            return true;
        }
        else
        {
            printd("failed, rc=");
            printlnd(mqtt.state());
            printlnd(" try again in 5 seconds");
            delay(5000);
            mqtt_try += 1;
            if(mqtt_try>=3)
            {
              mqtt_try = 0;
              return false;
            }
        }
    }

    return false;
}


void readAtlasSensor( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s)
{
  digitalWrite(LED,1);
  if(temp_s)
  {
    temp_sensor.send_read_cmd();
    delay(1000);
    temp_sensor.receive_read_cmd();
    if ((temp_sensor.get_error() == Ezo_board::SUCCESS )&& (temp_sensor.get_last_received_reading() > -1000.0)&& temp_s)
    {
      temp =  temp_sensor.get_last_received_reading();
      if(ph_s)ph_sensor.send_cmd_with_num("T,",temp);
      if(orp_s)orp_sensor.send_cmd_with_num("T,",temp);
      if(ec_s)ec_sensor.send_cmd_with_num("T,",temp);
      if(od_s)do_sensor.send_cmd_with_num("T,",temp);
    }
    else
    {
      temp = -90.0;
      if(ph_s)ph_sensor.send_cmd_with_num("T,",25.0);
      if(orp_s)orp_sensor.send_cmd_with_num("T,",25.0);
      if(ec_s)ec_sensor.send_cmd_with_num("T,",25.0);
      if(od_s)do_sensor.send_cmd_with_num("T,",25.0);
    }
  }
  else
  {
    if(ph_s)ph_sensor.send_cmd_with_num("T,",25.0);
    if(orp_s)orp_sensor.send_cmd_with_num("T,",25.0);
    if(ec_s)ec_sensor.send_cmd_with_num("T,",25.0);
    if(od_s)do_sensor.send_cmd_with_num("T,",25.0);
  }
  delay(300);
  if(ph_s)ph_sensor.send_read_cmd();
  if(orp_s)orp_sensor.send_read_cmd();
  if(ec_s)ec_sensor.send_read_cmd();
  if(od_s)do_sensor.send_read_cmd();
  delay(1000);
  if(ph_s)
  {
    ph_sensor.receive_read_cmd();
    if (ph_sensor.get_error() == Ezo_board::SUCCESS)
    {
      ph =  ph_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
    }
    else{ ph = -25.0;}
  }
  if(orp_s)
  {
    orp_sensor.receive_read_cmd();
    if (orp_sensor.get_error() == Ezo_board::SUCCESS)
    {
      orp =  orp_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
    }
    else{ orp = -25.0;}
  }
  if(od_s)
  {
    do_sensor.receive_read_cmd();
    if (do_sensor.get_error() == Ezo_board::SUCCESS)
    {
      od =  do_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
    }
    else{ od = -25.0;}
  }
  if(ec_s)
  {
    ec_sensor.receive_read_cmd();
    if (ec_sensor.get_error() == Ezo_board::SUCCESS)
    {
      ec =  ec_sensor.get_last_received_reading();                    //if the PH reading was successful (back in step 1)    //Serial.println(String(OD));
    }
    else{ ec = -25.0;}
  }
  digitalWrite(LED,0);
}

void readDummySensors( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s)
{
  if(ph_s)  { ph    =  random(2,14)+ random(0,99)/100.0;}
  if(orp_s) { orp   =  random(0,300)+ random(0,99)/100.0;}
  if(temp_s){ temp  =  random(15,30)+ random(0,99)/100.0;}
  if(od_s)  { od    =  random(0,300)+ random(0,99)/100.0;}
  if(ec_s)  { ec    =  random(0,1000)+ random(0,99)/100.0;}
}

void readSensors()
{
  if(state == TEST)
  {
    readDummySensors( /*ph */   1,
                      /*orp */  0,
                      /*temp*/  1,
                      /*od */   0,
                      /*ec */   0);
  }
  else
  {
    readAtlasSensor( /*ph */    1,
                      /*orp */  1,
                      /*temp*/  1,
                      /*od */   0,
                      /*ec */   0);
  }
}

#if DATALOGGER
  void initRTC(void)
  {
    // Inicio del módulo DS3231
    if (!rtc.begin())
    {
      printlnd("¡No se encontró el módulo RTC (DS3231)! Revise las conexiones.");
      delay(1500);
    }
    // Setear hora, solo la primera vez (en caso de ya haberlo seteado, definir INIT_CLOCK_BOOL = false)
    if (INIT_CLOCK_BOOL) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
#else
  void initRTC()
  {
    printlnd("initialized RTC module");
  }
#endif

void writeDatalogger(String msg)
{
  #if DATALOGGER
    uSD_card.appendFile(SD,FILE_NAME,msg.c_str());
    uSD_card.appendFile(SD,FILE_NAME,"\n");
  #else
    printd("saving data test: ");
    printlnd(msg);
  #endif
}


String pubSensors(void)
{
  StaticJsonDocument<128> doc_tx;


  doc_tx["id"]    = ID;
  doc_tx["time"]  = time_datalogger;
  doc_tx["temp"]  = temp;
  doc_tx["ph"]    = ph;
  doc_tx["orp"]   = orp;
  //doc_tx["od"]    = od;
  //doc_tx["ec"]    = ec;

  String json;
  serializeJson(doc_tx, json);
  printlnd(json);
  return json;
}


String status(void)
{
  if (SD_ini and SD_status and wifi_status)
  {
    return("OK");
  }
  else if(!wifi_status)
 {
  if(!SD_ini or !SD_status)
  {
    return("SD WIFI");
  }
  return("WIFI");
 }
 else
  {
    return("SD");
  }
}

void mostrarparametros(void)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("pH:");
  display.setTextSize(3);
  display.setCursor(30, 22);
  display.print(ph);
  display.display();

  display.setCursor(0, 57);
  display.setTextSize(1);
  display.println("Status:");
  display.display();
  display.setCursor(45, 57);
  display.println(status());
  display.display();
  delay(5000);

  display.setTextColor(SH110X_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("pH:");
  display.setTextSize(3);
  display.setCursor(30, 22);
  display.print(ph);
  display.display();

  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("Temp:");
  display.setTextSize(3);
  display.setCursor(30, 22);
  display.print(temp);
  display.display();
  delay(5000);

  display.setTextColor(SH110X_BLACK);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("Temp:");
  display.setTextSize(3);
  display.setCursor(30, 22);
  display.print(temp);
  display.display();
  display.setCursor(0, 57);
  display.setTextSize(1);
  display.println("Status:");
  display.display();
  display.setCursor(45, 57);
  display.println(status());
  display.display();
  display.clearDisplay();
}