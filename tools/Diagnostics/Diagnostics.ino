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
// #define TINY_LORA_SIM808
// #define TINY_LORA_SIM868
// #define TINY_LORA_SIM900
// #define TINY_LORA_SIM7000
// #define TINY_LORA_SIM7000SSL
// #define TINY_LORA_SIM7080
// #define TINY_LORA_SIM5360
// #define TINY_LORA_SIM7600
// #define TINY_LORA_UBLOX
// #define TINY_LORA_SARAR4
// #define TINY_LORA_M95
// #define TINY_LORA_BG96
// #define TINY_LORA_A6
// #define TINY_LORA_A7
// #define TINY_LORA_M590
// #define TINY_LORA_MC60
// #define TINY_LORA_MC60E
// #define TINY_LORA_ESP8266
// #define TINY_LORA_XBEE
// #define TINY_LORA_SEQUANS_MONARCH

// Set serial for debug console (to the Serial Monitor, default speed 115200)
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

// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#ifndef TINY_LORA_RX_BUFFER
#define TINY_LORA_RX_BUFFER 1024
#endif

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_LORA_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Add a reception delay - may be needed for a fast processor at a slow baud
// rate #define TINY_LORA_YIELD() { delay(2); }

// Uncomment this if you want to use SSL
// #define USE_SSL

#define TINY_LORA_USE_GPRS true
#define TINY_LORA_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]      = "YourAPN";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// Server details
const char server[]   = "vsh.pp.ua";
const char resource[] = "/TinyLoRa/logo.txt";

#include <TinyLoRa.h>

// Just in case someone defined the wrong thing..
#if TINY_LORA_USE_GPRS && not defined TINY_LORA_HAS_GPRS
#undef TINY_LORA_USE_GPRS
#undef TINY_LORA_USE_WIFI
#define TINY_LORA_USE_GPRS false
#define TINY_LORA_USE_WIFI true
#endif
#if TINY_LORA_USE_WIFI && not defined TINY_LORA_HAS_WIFI
#undef TINY_LORA_USE_GPRS
#undef TINY_LORA_USE_WIFI
#define TINY_LORA_USE_GPRS true
#define TINY_LORA_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyLoRa       modem(debugger);
#else
TinyLoRa       modem(SerialAT);
#endif

#ifdef USE_SSL&& defined TINY_LORA_HAS_SSL
TinyLoRaClientSecure     client(modem);
const int                port = 443;
#else
TinyLoRaClient client(modem);
const int      port = 80;
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println("Wait...");

  // Set GSM module baud rate
  TinyLoRaAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  // SerialAT.begin(9600);
  delay(6000);
}

void loop() {
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.print("Initializing modem...");
  if (!modem.restart()) {
    // if (!modem.init()) {
    SerialMon.println(F(" [fail]"));
    SerialMon.println(F("************************"));
    SerialMon.println(F(" Is your modem connected properly?"));
    SerialMon.println(F(" Is your serial speed (baud rate) correct?"));
    SerialMon.println(F(" Is your modem powered on?"));
    SerialMon.println(F(" Do you use a good, stable power source?"));
    SerialMon.println(F(" Try using File -> Examples -> TinyLoRa -> tools -> "
                        "AT_Debug to find correct configuration"));
    SerialMon.println(F("************************"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" [OK]"));

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

#if TINY_LORA_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) { modem.simUnlock(GSM_PIN); }
#endif

#if TINY_LORA_USE_WIFI
  // Wifi connection parameters must be set before waiting for the network
  SerialMon.print(F("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
#endif

#if TINY_LORA_USE_GPRS && defined TINY_LORA_XBEE
  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork(
          600000L)) {  // You may need lengthen this in poor service areas
    SerialMon.println(F(" [fail]"));
    SerialMon.println(F("************************"));
    SerialMon.println(F(" Is your sim card locked?"));
    SerialMon.println(F(" Do you have a good signal?"));
    SerialMon.println(F(" Is antenna attached?"));
    SerialMon.println(F(" Does the SIM card work with your phone?"));
    SerialMon.println(F("************************"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" [OK]"));

#if TINY_LORA_USE_GPRS
  // GPRS connection parameters are usually set after network registration
  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(F(" [fail]"));
    SerialMon.println(F("************************"));
    SerialMon.println(F(" Is GPRS enabled by network provider?"));
    SerialMon.println(F(" Try checking your card balance."));
    SerialMon.println(F("************************"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" [OK]"));
#endif

  IPAddress local = modem.localIP();
  SerialMon.print("Local IP: ");
  SerialMon.println(local);

  SerialMon.print(F("Connecting to "));
  SerialMon.print(server);
  if (!client.connect(server, port)) {
    SerialMon.println(F(" [fail]"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" [OK]"));

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  // Wait for data to arrive
  while (client.connected() && !client.available()) {
    delay(100);
    SerialMon.print('.');
  };
  SerialMon.println();

  // Skip all headers
  client.find("\r\n\r\n");

  // Read data
  uint32_t timeout       = millis();
  uint32_t bytesReceived = 0;
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      char c = client.read();
      // SerialMon.print(c);
      bytesReceived += 1;
      timeout = millis();
    }
  }

  client.stop();
  SerialMon.println(F("Server disconnected"));

#if TINY_LORA_USE_WIFI
  modem.networkDisconnect();
  SerialMon.println(F("WiFi disconnected"));
#endif
#if TINY_LORA_USE_GPRS
  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));
#endif

  SerialMon.println();
  SerialMon.println(F("************************"));
  SerialMon.print(F(" Received: "));
  SerialMon.print(bytesReceived);
  SerialMon.println(F(" bytes"));
  SerialMon.print(F(" Test:     "));
  SerialMon.println((bytesReceived == 121) ? "PASSED" : "FAILED");
  SerialMon.println(F("************************"));

  // Do nothing forevermore
  while (true) { delay(1000); }
}
