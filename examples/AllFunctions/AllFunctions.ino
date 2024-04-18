/**************************************************************
 *
 * TinyLoRa Getting Started guide:
 *   https://tiny.cc/TinyLoRa-readme
 *
 * NOTE:
 * Some of the functions may be unavailable for your modem.
 * Just comment them out.
 *
 **************************************************************/

// Select your modem:
#define TINY_LORA_MDOT

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

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_LORA_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_LORA_YIELD() { delay(2); }

/*
 * Tests enabled
 */
#define TINY_LORA_TEST_ABP false
#define TINY_LORA_TEST_OTAA true
#define TINY_LORA_TEST_BATTERY true
#define TINY_LORA_TEST_TEMPERATURE true
#define TINY_LORA_TEST_TIME false
// disconnect and power down modem after tests
#define TINY_LORA_POWERDOWN false

// Your ABP credentials, if applicable
const char devAddr[] = "YourDeviceAddress";
const char nwkSKey[] = "YourNetworkSessionKey";
const char appSKey[] = "YourAppSessionKey";

// Your OTAA connection credentials, if applicable
const char appEui[] = "YourAppEUI";
const char appKey[] = "YourAppKey";

#include <TinyLoRa.h>

#if TINY_LORA_TEST_ABP
#undef TINY_LORA_TEST_OTAA
#define TINY_LORA_TEST_OTAA false
#endif
#if TINY_LORA_TEST_OTAA
#undef TINY_LORA_TEST_ABP
#define TINY_LORA_TEST_ABP false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyLoRa       modem(debugger);
#else
TinyLoRa       modem(SerialAT);
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
  // !!!!!!!!!!!

  DBG("Wait...");
  delay(6000);

  // Set GSM module baud rate
  // TinyLoRaAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  SerialAT.begin(9600);
}

void loop() {
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.restart()) {
    // if (!modem.init()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    // restart autobaud in case GSM just rebooted
    TinyLoRaAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
    return;
  }

  String name = modem.getDevEUI();
  DBG("Device EUI:", name);
  delay(2000L);

  String modemInfo = modem.getModuleInfo();
  DBG("Module Info:", modemInfo);
  delay(2000L);

  // get and set the public network status to test functionality
  bool isPublic = modem.getPublicNetwork();
  DBG("Currently set in",
      isPublic ? "public network mode" : "private network mode");
  delay(1000L);
  if (modem.setPublicNetwork(isPublic)) {
    DBG("Reset public network mode to",
        isPublic ? "public network mode" : "private network mode");
  } else {
    DBG("Failed to set public network mode");
  }
  delay(2000L);

  // get and set the ack status to test functionality
  int8_t ackRetries = modem.getConfirmationRetries();
  DBG("Device will retry", ackRetries, "waiting for ACK");
  delay(1000L);
  if (modem.setConfirmationRetries(ackRetries)) {
    DBG("Set ACK retry count to", ackRetries);
  } else {
    DBG("Failed to set ACK retry count");
  }
  delay(2000L);

  // get and set the device class to test functionality
  _lora_class currClass = modem.getClass();
  DBG("Device is set to use LoRa device class", (char)currClass);
  delay(1000L);
  if (modem.setClass(currClass)) {
    DBG("Set LoRa device class to", (char)currClass);
  } else {
    DBG("Failed to set LoRa device class");
  }
  delay(2000L);


  // get and set the current band to test functionality
  String currBand = modem.getBand();
  DBG("Device is currently using LoRa band", currBand);
  delay(1000L);
  // doesn't work with all modules
  if (modem.setBand(currBand)) {
    DBG("Set LoRa device band to", currBand);
  } else {
    DBG("Failed to set LoRa device band");
  }
  delay(2000L);

  // get and set the sub-band to test functionality
  int8_t currSubBand = modem.getFrequencySubBand();
  DBG("Device is currently using LoRa sub-band", currSubBand);
  delay(1000L);
  if (modem.setFrequencySubBand(currSubBand)) {
    DBG("Set LoRa device sub-band to", currSubBand);
  } else {
    DBG("Failed to set LoRa device sub-band");
  }
  delay(2000L);

  // get and set the channel mask to test functionality
  String channelMask = modem.getChannelMask();
  DBG("Current channel mask", channelMask);
  delay(1000L);
  if (modem.setChannelMask(channelMask)) {
    DBG("Set LoRa channel mask to", channelMask);
  } else {
    DBG("Failed to set LoRa channel mask");
  }
  delay(2000L);

  // get and set the duty cycle to test functionality
  int8_t currDuty = modem.getDutyCycle();
  DBG("Current duty cycle:", currDuty);
  delay(1000L);
  if (modem.setDutyCycle(currDuty)) {
    DBG("Set LoRa duty cycle to", currDuty);
  } else {
    DBG("Failed to set LoRa duty cycle");
  }
  delay(2000L);

  // get and set the data rate to test functionality
  int8_t currDR = modem.getDataRate();
  DBG("Current LoRa data rate:", currDR);
  delay(1000L);
  if (modem.setDataRate(currDR)) {
    DBG("Set LoRa data rate to", currDR);
  } else {
    DBG("Failed to set LoRa data rate");
  }
  delay(2000L);

  // get and set the ADR to test functionality
  bool useADR = modem.getAdaptiveDataRate();
  DBG("Currently set to", useADR ? "use" : "not use", "adaptive data rate");
  delay(1000L);
  if (modem.setAdaptiveDataRate(useADR)) {
    DBG("Reset to", useADR ? "use" : "not use", "adaptive data rate");
  } else {
    DBG("Failed to set adaptive data rate");
  }
  delay(2000L);

#if TINY_LORA_TEST_OTAA
  DBG("Attempting to join with OTAA...");
  if (!modem.joinOTAA(appEui, appKey)) {
    DBG(" fail");
    delay(10000L);
    return;
  }
  SerialMon.println(" success");
  delay(2000L);

  String devAddr = modem.getDevAddr();
  DBG("Device (Network) Address:", devAddr);
  delay(2000L);

  String appEUI = modem.getAppEUI();
  DBG("App EUI (network ID):", appEUI);
  delay(2000L);

  String appKey = modem.getAppKey();
  DBG("App Key (network key):", appKey);
  delay(2000L);
#endif

#if TINY_LORA_TEST_ABP
  DBG("Attempting to join with ABP...");
  if (!modem.joinABP(devAddr, nwkSKey, appSKey)) {
    DBG(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
  delay(2000L);

  String devAddr = modem.getDevAddr();
  DBG("Device (Network) Address:", devAddr);
  delay(2000L);

  String nwkSKey = modem.getNwkSKey();
  DBG("Network Session Key:", nwkSKey);
  delay(2000L);

  String appSKey = modem.getAppSKey();
  DBG("App Session Key:", appSKey);
  delay(2000L);
#endif

  bool res = modem.isNetworkConnected();
  DBG("Network status:", res ? "connected" : "not connected");
  delay(2000L);

  int rssi = modem.getSignalQuality();
  DBG("Signal quality:", rssi);
  delay(2000L);

  TinyLoRaStream loraStream(modem);
  // Send out some data:
  loraStream.print(String("hello"));
  if (loraStream.available()) {
    DBG("Got downlink data!");
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
  }
  delay(2000L);
  loraStream.print(String("This is a longer message"));
  if (loraStream.available()) {
    DBG("Got downlink data!");
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
  }
  delay(2000L);
  loraStream.print(String(
      "This is a really long message that I'm using to test and ensure that "
      "packets are being broken up correctly by the send function.Lorem ipsum "
      "dolor sit amet, consectetur adipiscing elit. Quisque vulputate dolor "
      "vitae ante vehicula molestie. Duis in quam nec dolor varius lobortis. "
      "Proin eget malesuada odio. Etiam condimentum sodales hendrerit. "
      "Curabitur molestie sem vel sagittis commodo. Proin ut tortor sodales, "
      "molestie nisl eget, ultricies dolor. Donec bibendum, dui nec pharetra "
      "ornare, ex velit ullamcorper mi, eu facilisis purus turpis quis dolor. "
      "Suspendisse rhoncus nisl non justo tempor vulputate. Duis sit amet "
      "metus sit amet leo tristique venenatis nec in libero. Maecenas pharetra "
      "enim quis ornare aliquet. Aenean pretium cursus magna, fringilla auctor "
      "metus faucibus non. Nulla blandit mauris a quam tincidunt commodo. Duis "
      "dapibus lorem eget augue ornare, id lobortis quam volutpat."));
  if (loraStream.available()) {
    DBG("Got downlink data!");
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
  }
  delay(2000L);


#if TINY_LORA_TEST_TIME && defined TINY_LORA_HAS_TIME
  int   year3    = 0;
  int   month3   = 0;
  int   day3     = 0;
  int   hour3    = 0;
  int   min3     = 0;
  int   sec3     = 0;
  float timezone = 0;
  for (int8_t i = 5; i; i--) {
    DBG("Requesting current network time");
    if (modem.getDateTimeParts(&year3, &month3, &day3, &hour3, &min3, &sec3,
                               &timezone)) {
      DBG("Year:", year3, "\tMonth:", month3, "\tDay:", day3);
      DBG("Hour:", hour3, "\tMinute:", min3, "\tSecond:", sec3);
      DBG("Timezone:", timezone);
      break;
    } else {
      DBG("Couldn't get network time, retrying in 15s.");
      delay(15000L);
    }
  }

  DBG("Retrieving time again as a string");
  String time = modem.getDateTimeString(DATE_FULL);
  DBG("Current Network Time:", time);

  DBG("Retrieving time again as an offset from the epoch");
  uint32_t epochTime = modem.getDateTimeEpoch();
  DBG("Current Epoch Time:", epochTime);
#endif

#if TINY_LORA_TEST_BATTERY && defined TINY_LORA_HAS_BATTERY
  int8_t  chargeState = -99;
  int8_t  percent     = -99;
  int16_t milliVolts  = -9999;
  DBG("pre Battery charge state:", chargeState);
  DBG("pre Battery charge 'percent':", percent);
  DBG("pre Battery voltage:", milliVolts / 1000.0F);
  modem.getBattStats(chargeState, percent, milliVolts);
  DBG("Battery charge state:", chargeState);
  DBG("Battery charge 'percent':", percent);
  DBG("Battery voltage:", milliVolts / 1000.0F);
  delay(2000L);
#endif

#if TINY_LORA_TEST_TEMPERATURE && defined TINY_LORA_HAS_TEMPERATURE
  float temp = modem.getTemperature();
  DBG("Chip temperature:", temp);
#endif

#if TINY_LORA_POWERDOWN
  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.poweroff();
  DBG("Poweroff.");
#endif

  DBG("End of tests.");

  // Just listen forevermore
  while (true) {
    modem.maintain();
    if (loraStream.available()) {
      DBG("Got downlink data!");
      while (loraStream.available()) { SerialMon.write(loraStream.read()); }
    }
  }
}
