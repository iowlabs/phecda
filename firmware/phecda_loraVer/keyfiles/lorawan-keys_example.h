/*******************************************************************************
 *
 *  File:          lorawan-keys_example.h
 *
 *  Function:      Example for lorawan-keys.h required by LMIC-node.
 *
 *  Copyright:     Copyright (c) 2021 Leonel Lopes Parente
 *
 *  Important      ██ DO NOT EDIT THIS EXAMPLE FILE (see instructions below) ██
 *
 *  Decription:    lorawan-keys.h defines LoRaWAN keys needed by the LMIC library.
 *                 It can contain keys for both OTAA and for ABP activation.
 *                 Only the keys for the used activation type need to be specified.
 *
 *                 It is essential that each key is specified in the correct format.
 *                 lsb: least-significant-byte first, msb: most-significant-byte first.
 *
 *                 For security reasons all files in the keyfiles folder (except file
 *                 lorawan-keys_example.h) are excluded from the Git(Hub) repository.
 *                 Also excluded are all files matching the pattern *lorawan-keys.h.
 *                 This way they cannot be accidentally committed to a public repository.
 *
 *  Instructions:  1. Copy this file lorawan-keys_example.h to file lorawan-keys.h
 *                    in the same folder (keyfiles).
 *                 2. Place/edit required LoRaWAN keys in the new lorawan-keys.h file.
 *
 ******************************************************************************/

#pragma once

#ifndef LORAWAN_KEYS_H_
#define LORAWAN_KEYS_H_

// Optional: If DEVICEID is defined it will be used instead of the default defined in the BSF.
// #define DEVICEID "<deviceid>"

// Keys required for OTAA activation:

///////////////////////// End-device Identifier (u1_t[8]) in lsb format
//Nodo1
//#define OTAA_DEVEUI 0xA7, 0x2F, 0x8B, 0x5E, 0xC4, 0x08, 0x9C, 0x66

////////////////// Application Identifier (u1_t[8]) in lsb format
//Nodo1
//#define OTAA_APPEUI 0xA7, 0x2F, 0x8B, 0x5E, 0xC4, 0x08, 0x9C, 0x66

///////////////////////// Application Key (u1_t[16]) in msb format
//Nodo1
//#define OTAA_APPKEY 0x39, 0x4C, 0x7F, 0x18, 0x71, 0x04, 0x77, 0x8B, 0xD1, 0x2D, 0x27, 0xA5, 0xA9, 0x80, 0xEF, 0xE8



// -----------------------------------------------------------------------------

// Optional: If ABP_DEVICEID is defined it will be used for ABP instead of the default defined in the BSF.
// #define ABP_DEVICEID "<deviceid>"

// Keys required for ABP activation:

// End-device Address (u4_t) in uint32_t format.
// Note: The value must start with 0x (current version of TTN Console does not provide this).

// Nodo 01
//#define ABP_DEVADDR 0x008b2165

// Network Session Key (u1_t[16]) in msb format
//#define ABP_NWKSKEY 0Xe4, 0X3b, 0X4d, 0X90, 0X6b, 0X71, 0X61, 0X0c, 0Xba, 0X71, 0Xc4, 0X3f, 0X37, 0X31, 0X90, 0Xfb

// Application Session K (u1_t[16]) in msb format
//#define ABP_APPSKEY 0X52, 0Xc5, 0Xe5, 0X8f, 0X49, 0Xc6, 0X69, 0Xa3, 0X8f, 0X9b, 0X78, 0X55, 0X58, 0X52, 0X94, 0X5b



// Nodo 02
//#define ABP_DEVADDR 0x01caed9f

// Network Session Key (u1_t[16]) in msb format f3cf07fc0d3ff707c9f97d89c6a1d8c0
//#define ABP_NWKSKEY 0xf3, 0xcf, 0x07, 0xfc, 0x0d, 0x3f, 0xf7, 0x07, 0xc9, 0xf9, 0x7d, 0x89, 0xc6, 0xa1, 0xd8, 0xc0

// Application Session K (u1_t[16]) in msb format dc67779cb73f7d69ef500b14bc9405d6
//#define ABP_APPSKEY 0xdc, 0x67, 0x77, 0x9c, 0xb7, 0x3f, 0x7d, 0x69, 0xef, 0x50, 0x0b, 0x14, 0xbc, 0x94, 0x05, 0xd6


// Nodo 03
//#define ABP_DEVADDR 0x00975d9e

// Network Session Key (u1_t[16]) in msb format 483f06785154c8a3e06836f2a0a8c43a
//#define ABP_NWKSKEY 0x48, 0x3f, 0x06, 0x78, 0x51, 0x54, 0xc8, 0xa3, 0xe0, 0x68, 0x36, 0xf2, 0xa0, 0xa8, 0xc4, 0x3a

// Application Session K (u1_t[16]) in msb format 25ea0e2288459df80e75adb09050de93
//#define ABP_APPSKEY 0x25, 0xea, 0x0e, 0x22, 0x88, 0x45, 0x9d, 0xf8, 0x0e, 0x75, 0xad, 0xb0, 0x90, 0x50, 0xde, 0x93


// Nodo 04
#define ABP_DEVADDR 0x00bf7504

// Network Session Key (u1_t[16]) in msb format 3f7731c9b632dbcb450c0b7fe6f03b59
#define ABP_NWKSKEY 0x3f, 0x77, 0x31, 0xc9, 0xb6, 0x32, 0xdb, 0xcb, 0x45, 0x0c, 0x0b, 0x7f, 0xe6, 0xf0, 0x3b, 0x59
// Application Session K (u1_t[16]) in msb format d4f60d45ed8073ae976a1dc506480fbe
#define ABP_APPSKEY 0x7e, 0x2b, 0x73, 0x35, 0xf0, 0x9f, 0x16, 0x5d, 0x8b, 0xfd, 0xba, 0x3a, 0x88, 0xb7, 0xbc, 0xd5


// Nodo 05
//#define ABP_DEVADDR 0x016e7331

// Network Session Key (u1_t[16]) in msb format a39343e6f2426e79a41ee9fafc9129b5
//#define ABP_NWKSKEY 0xa3, 0x93, 0x43, 0xe6, 0xf2, 0x42, 0x6e, 0x79, 0xa4, 0x1e, 0xe9, 0xfa, 0xfc, 0x91, 0x29, 0xb5

// Application Session K (u1_t[16]) in msb format 7e2b7335f09f165d8bfdba3a88b7bcd5
//#define ABP_APPSKEY 0x7e, 0x2b, 0x73, 0x35, 0xf0, 0x9f, 0x16, 0x5d, 0x8b, 0xfd, 0xba, 0x3a, 0x88, 0xb7, 0xbc, 0xd5

#endif  // LORAWAN_KEYS_H_
