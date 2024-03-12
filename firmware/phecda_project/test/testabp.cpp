#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "iowPhecda.h"

// defines para deep sleep
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60//60*60*1        /* Time ESP32 will go to sleep (in seconds) */

bool flag_TXCOMPLETE = false;


// LoRaWAN NwkSKey, network session key
// This should be in big-endian (aka msb).
static const PROGMEM u1_t NWKSKEY[16] = { 0x45, 0x69, 0x86, 0x5A, 0x2D, 0xCC, 0x33, 0x1B, 0xD6, 0x44, 0x30, 0x5E, 0x23, 0xF9, 0xAA, 0xD7 };

// LoRaWAN AppSKey, application session key
// This should also be in big-endian (aka msb).
static const u1_t PROGMEM APPSKEY[16] = { 0xD5, 0xB6, 0x1B, 0x81, 0x69, 0x02, 0x36, 0x1C, 0x9D, 0x02, 0x69, 0xD9, 0xC9, 0x23, 0x6C, 0xEF };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x260C021B; // <-- Change this address for every node!


void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


static uint8_t mydata[] = "data";
static osjob_t sendjob;

iowPhecda phecda = iowPhecda();
String output;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60; //60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 15,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {17, 14, LMIC_UNUSED_PIN},
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void do_send(osjob_t* j)
{
	phecda.readSensors();
	output = phecda.pubData();
	Serial.println(output);
	//phecda.saveData();
	Serial.println("DO SEND");
    if (LMIC.opmode & OP_TXRXPEND)
	{
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
	else
	{
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, (unsigned char*) output.c_str(), sizeof(output)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
                u4_t netid = 0;
                devaddr_t devaddr = 0;
                u1_t nwkKey[16];
                u1_t artKey[16];
                LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
                Serial.print("netid: ");
                Serial.println(netid, DEC);
                Serial.print("devaddr: ");
                Serial.println(devaddr, HEX);
                Serial.print("AppSKey: ");
                for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                    Serial.print("-");
                printHex2(artKey[i]);
                }
                Serial.println("");
                Serial.print("NwkSKey: ");
                for (size_t i=0; i<sizeof(nwkKey); ++i) {
                        if (i != 0)
                                Serial.print("-");
                        printHex2(nwkKey[i]);
                }
                Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            //if (LMIC.txrxFlags & TXRX_ACK)
            //  Serial.println(F("Received ack"));
            //if (LMIC.dataLen) {
            //  Serial.println(F("Received "));
            //  Serial.println(LMIC.dataLen);
            //  Serial.println(F(" bytes of payload"));
            //}
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            //flag_TXCOMPLETE = true;

            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            //ESP.restart();
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void setup()
{
    delay(1000);
    Serial.begin(115200);
    delay(100);
    Serial.println(F("Starting"));

	phecda.activatePH();
	phecda.activateTEMP();
	phecda.activateORP();
    //phecda.activateLoRa();

    delay(100);

	phecda.begin();

    delay(100);

    // LMIC init
	Serial.println("OS init");
    os_init();
    //LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    // Reset the MAC state. Session and pending data transfers will be discarded.
	Serial.println("lmic reset");
	LMIC_reset();

    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.

    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x13, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    LMIC_selectSubBand(1);

	Serial.println("lmic config");
    //LMIC.dn2Dr = US915_DR_SF12CR;
    //LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
    LMIC_setLinkCheckMode(0);
    //LMIC_setDrTxpow(DR_SF7,14);
    LMIC.dn2Dr = DR_SF9;
        LMIC_setDrTxpow(DR_SF7,14);
	
    Serial.println("asing send job");
    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
	Serial.println("ready");    
}

void loop()
{   
    os_runloop_once();
    //if (flag_TXCOMPLETE) {
    //    delay(10000);
    //    // Entrar en el modo de deep sleep
    //    Serial.println("Deep Sleep"); 
    //    Serial.flush();
    //    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    //    esp_deep_sleep_start();
    //}
}
