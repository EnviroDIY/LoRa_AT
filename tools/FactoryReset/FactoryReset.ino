/**************************************************************
 *
 * To run this tool you need StreamDebugger library:
 *   https://github.com/EnviroDIY/StreamDebugger
 *   or from http://librarymanager/all#StreamDebugger
 *
 * TinyLoRa Getting Started guide:
 *   https://tiny.cc/TinyLoRa-readme
 *
 **************************************************************/

// Select your modem:
#define TINY_LORA_MDOT
// #define TINY_LORA_SIM900
// #define TINY_LORA_SIM808
// #define TINY_LORA_SIM868
// #define TINY_LORA_UBLOX
// #define TINY_LORA_M95
// #define TINY_LORA_BG96
// #define TINY_LORA_A6
// #define TINY_LORA_A7
// #define TINY_LORA_M590
// #define TINY_LORA_MC60
// #define TINY_LORA_MC60E
// #define TINY_LORA_ESP8266
// #define TINY_LORA_XBEE

#include <TinyLoRa.h>

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#ifndef __AVR_ATmega328P__
#define SerialAT Serial1

// or Software Serial on Uno, Nano
#else
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3);  // RX, TX
#endif

#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyLoRa       modem(debugger);

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(6000);

  if (!modem.init()) {
    SerialMon.println(
        F("***********************************************************"));
    SerialMon.println(F(" Cannot initialize modem!"));
    SerialMon.println(
        F("   Use File -> Examples -> TinyLoRa -> tools -> AT_Debug"));
    SerialMon.println(F("   to find correct configuration"));
    SerialMon.println(
        F("***********************************************************"));
    return;
  }

  bool ret = modem.factoryDefault();

  SerialMon.println(
      F("***********************************************************"));
  SerialMon.print(F(" Return settings to Factory Defaults: "));
  SerialMon.println((ret) ? "OK" : "FAIL");
  SerialMon.println(
      F("***********************************************************"));
}

void loop() {}
