#include <Arduino.h>
#include <Ezo_i2c.h>
#include "Ezo_i2c_util.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>
//#include <lmic.h>
//#include <hal/hal.h>
#include <RTClib.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SoftwareSerial.h>

#define GSM_RX        13
#define GSM_TX         4

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800        // Modem is SIM800
#define TINY_GSM_RX_BUFFER    1024  // Set RX buffer to 1Kb


#include <TinyGsmClient.h>

//--------------------
//    Definiciones
//--------------------

#define SS          25
#define LED         2
#define BATTERY_PIN 35
#define RELAY_CH1_PIN 32
#define RELAY_CH2_PIN 33
#define DEVICE_MAC  "XXXXXXXXX"

// GPRS sim credentials
/*
const char apn[] = "m2m.entel.cl";              // APN from PRIMCARDS
const char gprsUser[] = "entelpcs";             // GPRS User
const char gprsPass[] = "entelpcs";             // GPRS Password
*/
const char apn[] = "m2m.movistar.cl"; // APN from PRIMCARDS
const char gprsUser[] = "";           // GPRS User
const char gprsPass[] = "";           // GPRS Password


// Version (mayor, minor, patch, build)
#define VERSION    "AINW_SAPAL-v.0.1.0.b.1" //Atlas sensor board
#define ID         "040200"

//SD
#define FILENAME_TO_SAVE "/NODO2.txt" // Definición del nombre de los archivos
#define TABLE_HEADER "" // Definición del header csv

//Serial definition

#define SerialRS485_1 Serial3
#define SerialRS485_2 Serial2
#define BR_GSM      9600
#define BR_DEBUG    9600
#define BR_RS485_1  9600
#define BR_RS485_2  9600

#define PH_ADDRESS    100
#define ORP_ADDRESS   101
#define TEMP_ADDRESS  102
#define DO_ADDRESS    103
#define EC_ADDRESS    104

#define RESPONSE_OK     "OK"
#define RESPONSE_ERROR  "ERROR"

#define INA_MAX_READ 100

//--------------------
//    Instancias
//--------------------

//serial GSM
SoftwareSerial SerialGSM(GSM_RX, GSM_TX);

//Client and mqtt
TinyGsm       modem(SerialGSM);
TinyGsmClient client(modem);
WiFiClient wifi;



//JSON parameters
StaticJsonDocument<200> doc;
StaticJsonDocument<200> doc2;
const char* response = RESPONSE_OK;

// Contiene los datos a guardar para cada reporte
char report[128];
String timestamp;

RTC_DS3231 rtc;
SPIClass spi(HSPI);
static const int spiClk = 4000000; // 4 MHz


Ezo_board ph_sensor   = Ezo_board(PH_ADDRESS,"PH");
Ezo_board orp_sensor  = Ezo_board(ORP_ADDRESS,"ORP");
Ezo_board temp_sensor = Ezo_board(TEMP_ADDRESS,"RTD");
Ezo_board do_sensor   = Ezo_board(DO_ADDRESS,"DO");
Ezo_board ec_sensor   = Ezo_board(EC_ADDRESS,"EC");

//--------------------
//   Parámetros
//--------------------

//RTC_DATA_ATTR  index = 0;              //Parámetro almaceado en la EEPROM
float time_factor            = 1000000;  //Factor para convertir de segundos a microsegundos
int start_hour               = 9;        //Hora de inicio de operación de la ESP
int stop_hour                = 22;       //Hora de término de operación de la ESP
uint64_t sleep_period_active = 300;        //Tiempo (S) que la ESP duerme luego de realizar una tarea durante el periodo de operación actvio
uint64_t sleep_period_inactive = (24 - (stop_hour - start_hour))*3600; //Tiempo (S) que la ESP duerme en periodo inactivo

int try_connect = 0; // Cantidad de intentos para conectarse

String post_serverName = "15.228.3.15";
String post_serverPath = "/publishFile";
int post_serverPort = 5000;
String getAll;
String getBody;

//--------------------
//    Variables
//--------------------

int year,month,day,hour,minute,second = 0;  //Time variables
float ph,orp,temp,od,ec = 0; //Sensor data
float c = 0.0;
float c_sum ,c_raw= 0.0;
float batt = 0.0;

//////////////////////////////// GSM - MQTT - HTML Functions //////////////////////////////////



//////////////////////////////// Micro SD functions //////////////////////////////////

void writeFile(fs::FS &fs, const char *path, const char *message) {
    // Escribir en la tarjeta SD (NO MODIFICAR ESTA FUNCIÓN)
    Serial.printf("Escribiendo archivo: %s\n", path);
    File file = fs.open(path, FILE_WRITE);

    // Preguntando si existe el archivo
    if (!file) {
        Serial.println("Fallo al abrir el archivo para escribir datos");
        return;
    }

    // Reporte
    if (file.print(message)) Serial.println("File written");
    else Serial.println("Write failed");

    // Cerrando
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
    // Agregando datos a la tarjeta SD
    Serial.printf("Agregando al archivo: %s\n", path);
    File file = fs.open(path, FILE_APPEND);

    // Preguntando si existe el archivo
    if (!file) {
        Serial.println("Fallo al abrir el archivo para agregar datos");
        return;
    }

    // Reporte
    if (file.print(message)) Serial.println("Mensaje agregado");
    else Serial.println("Append failed");

    // Cerrando
    file.close();
}

void sdcard_init(){
  // Asegurarse de que el módulo esté montado
  if(!SD.begin(SS)) {
  Serial.println("¡El montado de la tarjeta ha fallado!");
    return;
  }
  // Asegurarse de que la tarjeta
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("¡No se ha encontrado tarjeta SD!");
    return;
  }

  // Creación del archivo
  File file = SD.open(FILENAME_TO_SAVE);
  if (!file) {
    Serial.println("Archivo no existe");
    Serial.println("Creando archivo...");
    writeFile(SD, FILENAME_TO_SAVE, TABLE_HEADER);

  } else Serial.println("Archivo ya existe");
    file.close();

}

//////////////////////////////// Read sensores ///////////////////////////////////////





void setup(){

  //ESP 32 protocols init
  Wire.begin(21, 22, 400000UL);

  // Set GSM module baud rate and UART pins
  SerialGSM.begin(9600);
  Serial.begin(9600);

  //Lora SS
  pinMode(26,OUTPUT);
  digitalWrite(26,HIGH);
  //SD SS
  pinMode(25,OUTPUT);
  digitalWrite(25,HIGH);
  //Lora reset
  pinMode(27,OUTPUT);
  digitalWrite(27,HIGH);
  //Relay init
  pinMode(32,OUTPUT);
  digitalWrite(32,HIGH);
  pinMode(33,OUTPUT);
  digitalWrite(33,HIGH);


  sdcard_init();


  delay(1000);
  Serial.println("Encendido");



}

void loop () {


}
