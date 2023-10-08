/*
  Project     : Ph/ORP/TEMP. sensor
  Description : This version of the code read up to 4 (ph/ORP/EC/OD) altas
  scientific ezo boards and 1 RTD ezo sensor board and Update local data waiting
  until the gateway ask for publish  action then send data in json format.
  The comunication between the gateway and the node is by LoRa Rf protocol.


  Authors:
    WAC@IOWLAB

  Connections:
    Atlas sensor board
    -----------------------------
    VIN(5V)     ---    V+
    GND         ---    GND

    SDA
    SCL
    PH_EN
    ORP_EN
    EC_EN
    OD_EN
    Temp_EN



  Comunicationmunication:
    - Json structure:
      RCVED MSG:
      {"id":"NODE_ID" "cmd":"CMD_RX","arg":1}
      SENDED MSG:
      {"id":"ain_01", "time" : 456739, "ph": 11.23 ,"orp":224.5, "od":230.34,"ec":123.123 ,"t": 11.23,"resp":"ok"}
    - List of cmds
      - RESET: reset the micro
      - led:  turn of the on bord LED. used for debugging.
          arg = 0  LED off
          arg = 1  LED on
      - start: Start the sampling.
          arg = 0 STOP sampling
          arg = 1 START sampling.

  LIBS:
  the code need the following libraries (all of them availables in the official
  arduino repo):
  - ArduinoJson  bblanchon/ArduinoJson @ ^6.18.0
  - EZO I2C Sensors mulmer89/EZO I2C Sensors @ 1.0.0+32e1eda
*/

//-------------
//  LIBS
//-------------

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Ezo_i2c.h>
#include "Ezo_i2c_util.h"
#include <ArduinoJson.h>
#include <SparkFun_RV8803.h>
#include "uSD_iow.h"


//-------------
//  DEFINES
//-------------
// PINS
#define LED       2
#define PH_EN     33
#define ORP_EN    32
#define DO_EN     12
#define EC_EN     13
#define RFM_RST   4
#define RFM_DIO0  14
#define RFM_CS    15
#define RFM_INT   17
#define SD_CS     5
#define BAT_LVL   35



// Version (mayor, minor, patch, build)
#define VERSION    "Phecda-v.0.1.0.b.1" //Atlas sensor board
#define ID         "p01"

// SAMPLING
#define TIMER_F_SCLK        10000   // TIMER SOURCE CLOCK. IN kHz
#define TIMER_DEFAULT_TS    60      // TIMER DEFAULT PERIOD. IN SECONDS
#define N_SAMPLES           1      // NUMBER OF SAMPLES TO CONSIDERAR IN THE AVERAGE OF SAMPLES

#define BR_DEBUG  9600

#define IDLE      0
#define RUNNING   1
#define SLAVE     2
#define TEST      3

#define DEBUG     1
#if DEBUG
#define printd(s) {Serial.print((s));}
#define printlnd(s) {Serial.println((s));}
#else
#define printd(s)
#define printlnd(s)
#endif

#define PH_ADDRESS    100
#define ORP_ADDRESS   101
#define TEMP_ADDRESS  102
#define DO_ADDRESS    103
#define EC_ADDRESS    104

#define RESPONSE_OK     "OK"
#define RESPONSE_ERROR  "ERROR"
#define FILE_NAME       "/log.txt"

//MQTT
#define MQTT_PORT         1883
#define MQTT_USER         "iowlabs"
#define MQTT_PASSWORD     "!iow_woi!"
#define MQTT_PUBLISH_CH   "phecda/tx"
#define MQTT_RECEIVER_CH  "phecda/rx"

//-------------
// INSTANCES
//-------------
//JSON to SEND
char json_tx[1024];

//OD sensor reader EZO board
Ezo_board ph_sensor   = Ezo_board(PH_ADDRESS,"PH");
Ezo_board orp_sensor  = Ezo_board(ORP_ADDRESS,"ORP");
Ezo_board temp_sensor = Ezo_board(TEMP_ADDRESS,"RTD");
Ezo_board do_sensor   = Ezo_board(DO_ADDRESS,"DO");
Ezo_board ec_sensor   = Ezo_board(EC_ADDRESS,"EC");

//timers
hw_timer_t * timer = NULL;    //timer to adquisition of sensors
//RTC
RV8803 rtc;
//SD
USD_IOW uSD_card(SD_CS);
//Wifi and mqtt client
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

//-------------
// VARIABLES
//-------------
// CMD recived
const char* rcvd_id;
const char* cmd;
int arg = 0;
const char* response = RESPONSE_OK;

int mode        = RUNNING;
int sample_time = TIMER_DEFAULT_TS;

//Timer variables
int   timer_counter = int(sample_time * TIMER_F_SCLK); //VARIABLE TO SET THE MAX COUNT OF THE TIMER COUNTER
bool  timer_flag    = false;
bool  publish_flag  = false;
int   count         = 0;

// variables
float ph    = 0.0;
float orp   = 0.0;
float temp  = 0.0;
float od    = 0.0;
float ec    = 0.0;
//RTC unix stamp
unsigned long timestamp = 0;
//SD
bool SD_ini;
bool SD_status;
//wifi
const char* ssid      = "iownwater";
const char* password  = "temp3_NL156$";
const char* mqtt_server = "35.223.234.244";

uint8_t mqtt_try = 0;

//-------------
// FUNCTIONS
//-------------
void processCmd();
void publishData();
void publishResponse();
void publishMqtt(char *data);
void readAtlasSensors( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s);
void readSensors();
void setupWiFi();
bool reconnect();
void IRAM_ATTR timerISR();

void setup()
{
  pinMode(LED,OUTPUT);
  pinMode(PH_EN,OUTPUT);
  pinMode(ORP_EN,OUTPUT);
  pinMode(SD_CS, OUTPUT);

  digitalWrite(PH_EN  ,HIGH);
  digitalWrite(ORP_EN ,HIGH);
  digitalWrite(SD_CS  ,HIGH);

  Wire.begin();
  Serial.begin(BR_DEBUG);
  delay(500);

  //setup Timer
  timer = timerBegin(3, 8000, true);          // timer 3, prescaler 8000, counting up
  timerAttachInterrupt(timer,&timerISR,true); // params: timer object, pointer to ISR function anddress, mode edge (if false: level mode)
  timerAlarmWrite(timer, timer_counter,true); // params: timer object, counter_limit, restart counter on top.// to get T= 1s, n=T*f_{timer source clock}
  timerAlarmEnable(timer);                    // enable CTC mode
  //timerStop(timer);                           // timer start stoped

  if (rtc.begin() == false)
  {
    printlnd("Device not found. Please check wiring.");
  }

  // un comment for setup time sec, min, hour, dayof the week, date, month, year
  /*
  if (rtc.setTime(0, 38, 18, 6, 30, 6, 2023) == false)
  {
    Serial.println("Something went wrong setting the time");
  }
  */

  uSD_card.uSD_init();
  delay(200);

  printd("setup ready");

}

void loop()
{
  if(timer_flag)
  {
    timer_flag = 0;
    readSensors();
    count +=  1;
    if(count >= N_SAMPLES)
    {
      publish_flag = 1;
    }
  }
  if(publish_flag && mode == RUNNING)
  {
    publish_flag = 0;
    if (rtc.updateTime() == true) //Updates the time variables from RTC
    {
      timestamp = rtc.getEpoch(); //Get the time
    }
    publishData();
    count = 0;
  }
  if (Serial.available() > 0)
  {
    printlnd("msg recived by lora");
    processCmd();
  }

}


void setupWiFi()
{
    delay(10);
    printd("Connecting to ");
    printlnd(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        digitalWrite(LED,HIGH);
        delay(500);
        digitalWrite(LED,LOW);

        printd(".");
    }

    printlnd("WiFi connected");
    printd("IP address: ");
    printlnd(WiFi.localIP());
    digitalWrite(LED,HIGH);
}

// Function to reconnect to the MQTT broker
bool reconnect()
{
    while (!mqtt.connected())
    {
        printd("Attempting MQTT connection...");
        if (mqtt.connect(ID, MQTT_USER , MQTT_PASSWORD))
        {
            printlnd("connected");
            mqtt.subscribe(MQTT_RECEIVER_CH);
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

void publishMqtt(char *payload)
{
  setupWiFi();
  mqtt.setServer(mqtt_server,MQTT_PORT);

    if(!mqtt.connected())
    {
        reconnect();
    }

    mqtt.publish(MQTT_PUBLISH_CH, payload);

  WiFi.disconnect(true);
}

void readSensors()
{
  readAtlasSensors(1,1,1,0,0);
  if (rtc.updateTime() == true) //Updates the time variables from RTC
  {
    timestamp = rtc.getEpoch(); //Get the time in UNIX
  }
}

/*
  Read all sensors and update the respectives buffers
*/
void readAtlasSensors( bool ph_s,bool orp_s,bool temp_s,bool od_s,bool ec_s)
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

/*
  get the avg of each buff and then Publish the json data structure by Lora.
  (For debug, also send the json msg on usb serial)
*/
void publishData()
{
  StaticJsonDocument<256> doc_tx;

  doc_tx["id"]    = ID;
  doc_tx["t"]     = timestamp;//millis();
  doc_tx["ph"]    = ph;
  doc_tx["orp"]   = orp;
  doc_tx["temp"]  = temp;
  doc_tx["do"]    = od;
  doc_tx["ec"]    = ec;
  doc_tx["resp"]  = RESPONSE_OK;

  String json;
  serializeJson(doc_tx, json);
  uSD_card.appendFile(SD,FILE_NAME,json.c_str());
  uSD_card.appendFile(SD,FILE_NAME,"\n");
  publishMqtt((char*)json.c_str());
  printlnd(json);
}


/*
  this function only send a respone type of data.
  used for communication.
*/
void publishResponse()
{
  StaticJsonDocument<256> doc_tx;

  doc_tx["id"]    = ID;
  doc_tx["cmds"]  = "r";
  doc_tx["arg"]   = 0;
  doc_tx["resp"]  = response;

  String json;
  serializeJson(doc_tx, json);
  printlnd(json);
}

/*
  Parse the received json message and procces the recived commands
*/
void processCmd()
{
    StaticJsonDocument<256> doc_rx;
    //const char* json_rx = "{\"id\":\"anw00\",\"cmd\":\"gps\",\"arg\":1}";
    DeserializationError error_rx;
    //check for error
    error_rx = deserializeJson(doc_rx, Serial);
    if (error_rx)
    {
      printd(F("deserializeJson() failed: "));
      printlnd(error_rx.c_str());
    }

    //parsing incoming msg

    rcvd_id = doc_rx["id"];
    if( strcmp(rcvd_id,ID)==0)
    {
      cmd = doc_rx["cmd"];
      arg = doc_rx["arg"];

      //prossesing incoming command
      if(strcmp(cmd,"mode")==0)
      {
        if(arg == IDLE )
        {
          printlnd("stop sampling");
          mode =  IDLE;
          timerStop(timer);
        }
        if(arg == RUNNING)
        {
          printlnd("start sampling");
          mode = RUNNING;
          timerRestart(timer);
        }
        if(arg == SLAVE)
        {
          printlnd("start sampling");
          mode = SLAVE;
          timerRestart(timer);
        }
        response = RESPONSE_OK;
        publishResponse();
      }
      else if(strcmp(cmd,"led")==0)
      {
        if(arg==1){digitalWrite(LED,HIGH);}
        else{digitalWrite(LED,LOW);}
        response = RESPONSE_OK;
        publishResponse();
      }
      else if(strcmp(cmd,"fs")==0)
      {
        sample_time = int(arg);
        if(sample_time <3){sample_time = 3;} // min sample time can be 3s
        response = RESPONSE_OK;
        publishResponse();
      }
      else if(strcmp(cmd,"pub")==0)
      {
        if(mode == SLAVE)
        {
            publishData();
        }
        else
        {
          response = RESPONSE_ERROR;
          publishResponse();
        }

      }
      else
      {
        printlnd("Command not valid");
      }
      cmd = "";
      arg = 0;

    }
    else
    {
      printlnd("msg not for me");
    }
}

void timerISR()
{
  timer_flag = true;
}
