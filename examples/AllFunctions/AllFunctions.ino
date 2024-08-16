/**************************************************************
 * NOTE:
 * Some of the functions may be unavailable for your modem.
 * Just comment them out.
 *
 **************************************************************/

// Select your modem:
// #define LORA_AT_MDOT
// #define LORA_AT_LORAE5

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
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
// #define LORA_AT_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define LORA_AUTOBAUD_MIN 9600
#define LORA_AUTOBAUD_MAX 115200

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define LORA_AT_YIELD() { delay(2); }

/*
 * Tests enabled
 */
#define LORA_AT_TEST_ABP false
#define LORA_AT_TEST_OTAA true
#define LORA_AT_TEST_SETTINGS true
#define LORA_AT_TEST_UPLINK true
#define LORA_AT_TEST_BATTERY true
#define LORA_AT_TEST_TEMPERATURE true
#define LORA_AT_TEST_TIME true
#define LORA_AT_TEST_SLEEP true

// Your ABP credentials, if applicable
// The device address (network address).
// This must be 4 bytes of hex data (8 hex characters).
const char devAddr[] = "4ByteDevAddr";
// The network session key.
// This must be 16 bytes of hex data (32 hex characters).
const char nwkSKey[] = "16ByteNwkSKey";
// The app session key (data session key).
// This must be 16 bytes of hex data (32 hex characters).
const char appSKey[] = "16ByteAppSKey";
// The last server uplink counter
int uplinkCounter = 1;
// The last server downlink counter
int downlinkCounter = 0;


// Your OTAA connection credentials, if applicable
// The App EUI (also called the Join EUI or the Network ID)
// This must be exactly 16 hex characters (8 bytes = 64 bits)
const char appEui[] = "8ByteAppEui";
// The App Key (also called the network key)
// This must be exactly 32 hex characters (16 bytes = 128 bits)
const char appKey[] = "16ByteAppKey";

// the pin on your Arduino that will turn on power to the module
int8_t power_pin_for_module = 18;
// the pin on your Arduino that will wake the module from pin sleep
int8_t arduino_wake_pin = 23;
// the pin on the LoRa module that will listen for pin wakes
int8_t lora_wake_pin = 8;  // NDTR_SLEEPRQ_DI8 (Default)
// The LoRa module's wake pin's pullup mode (0=NOPULL, 1=PULLUP, 2=PULLDOWN)
int8_t lora_wake_pullup = 1;
// The LoRa module's wake trigger mode (ie, 0=ANY, 1=RISE, 2=FALL)
int8_t lora_wake_edge = 0;


#include <LoRa_AT.h>

#if LORA_AT_TEST_ABP
#undef LORA_AT_TEST_OTAA
#define LORA_AT_TEST_OTAA false
#endif
#if LORA_AT_TEST_OTAA
#undef LORA_AT_TEST_ABP
#define LORA_AT_TEST_ABP false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
LoRa_AT        modem(debugger);
#else
LoRa_AT        modem(SerialAT);
#endif

LoRaStream loraStream(modem);

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  if (power_pin_for_module >= 0) {
    SerialMon.print("Powering LoRa module with pin ");
    SerialMon.println(power_pin_for_module);
    pinMode(power_pin_for_module, OUTPUT);
    digitalWrite(power_pin_for_module, HIGH);
  }
  if (arduino_wake_pin >= 0) {
    SerialMon.print("Waking LoRa module with pin ");
    SerialMon.println(arduino_wake_pin);
    pinMode(arduino_wake_pin, OUTPUT);
    digitalWrite(arduino_wake_pin, HIGH);
  }
  // !!!!!!!!!!!

  SerialMon.println(F("Wait..."));
  delay(6000);

  // Set GSM module baud rate
  // LoRa_AT_AutoBaud(SerialAT, LORA_AUTOBAUD_MIN, LORA_AUTOBAUD_MAX);
  SerialAT.begin(9600);
}

void loop() {
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  if (!modem.restart()) {
    // if (!modem.init()) {
    SerialMon.println(
        F("--Failed to restart modem, delaying 10s and retrying"));
    // restart autobaud in case GSM just rebooted
    LoRa_AT_AutoBaud(SerialAT, LORA_AUTOBAUD_MIN, LORA_AUTOBAUD_MAX);
    return;
  }

  String name = modem.getDevEUI();
  SerialMon.print(F("Device EUI: "));
  SerialMon.println(name);
  delay(2000L);

  String modemInfo = modem.getModuleInfo();
  SerialMon.print(F("Module Info: "));
  SerialMon.println(modemInfo);
  delay(2000L);

#if LORA_AT_TEST_SETTINGS

  // get and set the device class to test functionality
  _lora_class currClass = modem.getClass();
  SerialMon.print(F("Device is set to use LoRa device class "));
  SerialMon.println((char)currClass);
  delay(1000L);
  currClass = CLASS_A;
  if (modem.setClass(currClass)) {
    SerialMon.print(F("  Set LoRa device class to "));
    SerialMon.println((char)currClass);
  } else {
    SerialMon.println(F("--Failed to set LoRa device class"));
  }
  delay(2000L);

  // get and set the public network status to test functionality
  bool isPublic = modem.getPublicNetwork();
  SerialMon.print(F("Currently set in "));
  SerialMon.println(isPublic ? "public network mode" : "private network mode");
  delay(1000L);
  if (modem.setPublicNetwork(isPublic)) {
    SerialMon.print(F("  Reset public network mode to "));
    SerialMon.println(isPublic ? "public network mode"
                               : "private network mode");
  } else {
    SerialMon.println(F("--Failed to set public network mode"));
  }
  delay(2000L);

  // get and set the application port
  uint8_t currPort = modem.getPort();
  SerialMon.print(F("Device is set to use LoRa device port "));
  SerialMon.println(currPort);
  delay(1000L);
  if (modem.setPort(currPort)) {
    SerialMon.print(F("  Set LoRa device port to "));
    SerialMon.println(currPort);
  } else {
    SerialMon.println(F("--Failed to set LoRa device port"));
  }
  delay(2000L);
#endif

  // get and set the current band to test functionality
  String currBand = modem.getBand();
  SerialMon.print(F("Device is currently using LoRa band "));
  SerialMon.println(currBand);
  delay(1000L);

#if LORA_AT_TEST_SETTINGS
  // doesn't work with all modules
  currBand = "US915";
  if (modem.setBand(currBand)) {
    SerialMon.print(F("  Set LoRa device band to "));
    SerialMon.println(currBand);
  } else {
    SerialMon.println(F("--Failed to set LoRa device band"));
  }
  delay(2000L);
// get and set the sub-band to test functionality
#ifndef LORA_AT_LORAE5  // not supported
  int8_t currSubBand = modem.getFrequencySubBand();
  SerialMon.print(F("Device is currently using LoRa sub-band "));
  SerialMon.println(currSubBand);
  delay(1000L);
  if (modem.setFrequencySubBand(currSubBand)) {
    SerialMon.print(F("  Set LoRa device sub-band to "));
    SerialMon.println(currSubBand);
  } else {
    SerialMon.println(F("--Failed to set LoRa device sub-band"));
  }
  delay(2000L);
#endif

  // get and set the channel mask to test functionality
  String channelMask = modem.getChannelMask();
  SerialMon.print(F("Current channel mask "));
  SerialMon.println(channelMask);
  delay(1000L);
  if (modem.setChannelMask(channelMask)) {
    SerialMon.print(F("  Set LoRa channel mask to "));
    SerialMon.println(channelMask);
  } else {
    SerialMon.println(F("--Failed to set LoRa channel mask"));
  }
  delay(2000L);
  // Itterate through the 72 channels to check single channel enable
  // functionality
  bool channel_success = true;
  for (uint8_t i = 0; i < MAX_LORA_CHANNELS; i += 9) {
    bool channelEnabled = modem.isChannelEnabled(i);
    DBG(GF("  Channel"), i, GF("was previously"),
        channelEnabled ? GF("enabled") : GF("disabled"));
    bool resp = true;
    if (channelEnabled) {
      resp &= modem.disableChannel(i);
      resp &= modem.isChannelEnabled(i) == false;
      resp &= modem.enableChannel(i);
      resp &= modem.isChannelEnabled(i) == true;
      if (resp) {
        DBG(GF("    Channel"), i, GF("successfully disabled then re-enabled"));
      } else {
        DBG(GF("----Channel"), i, GF("enable failed!"));
      }
    } else {
      resp = modem.enableChannel(i);
      resp &= modem.isChannelEnabled(i) == true;
      resp = modem.disableChannel(i);
      resp &= modem.isChannelEnabled(i) == false;
      if (resp) {
        DBG(GF("    Channel"), i, GF("successfully enabled then disabled"));
      } else {
        DBG(GF("----Channel"), i, GF("disable failed!"));
      }
    }
    channel_success &= resp;
  }
  if (channel_success) {
    SerialMon.println(
        F("  Successfully tested enabling and disabling individual channels."));
  } else {
    SerialMon.println(F("--Failed to change individual channels."));
  }

  // get and set the ack status to test functionality
  int8_t ackRetries = modem.getConfirmationRetries();
  SerialMon.print(F("Device will retry "));
  SerialMon.print(ackRetries);
  SerialMon.println(F(" times waiting for ACK"));
  delay(1000L);
  if (modem.setConfirmationRetries(ackRetries)) {
    SerialMon.print(F("  Set ACK retry count to "));
    SerialMon.println(ackRetries);
  } else {
    SerialMon.println(F("--Failed to set ACK retry count"));
  }
  delay(2000L);

  // get and set the duty cycle to test functionality
  int8_t currDuty = modem.getMaxDutyCycle();
  SerialMon.print(F("Current duty cycle: "));
  SerialMon.println(currDuty);
  delay(1000L);
  if (modem.setMaxDutyCycle(currDuty)) {
    SerialMon.print(F("  Set LoRa duty cycle to "));
    SerialMon.println(currDuty);
  } else {
    SerialMon.println(F("--Failed to set LoRa duty cycle"));
  }
  delay(2000L);

  // get and set the data rate to test functionality
  int8_t currDR = modem.getDataRate();
  SerialMon.print(F("Current LoRa data rate: "));
  SerialMon.println(currDR);
  delay(1000L);
  currDR = 2;
  if (modem.setDataRate(currDR)) {
    SerialMon.print(F("  Set LoRa data rate to "));
    SerialMon.println(currDR);
  } else {
    SerialMon.println(F("--Failed to set LoRa data rate"));
  }
  delay(2000L);

  // get and set the ADR to test functionality
  bool useADR = modem.getAdaptiveDataRate();
  SerialMon.print(F("Currently set to "));
  SerialMon.print(useADR ? "use " : "not use ");
  SerialMon.println(F("adaptive data rate"));
  delay(1000L);
  if (modem.setAdaptiveDataRate(useADR)) {
    SerialMon.print(F("  Reset to "));
    SerialMon.print(useADR ? "use" : "not use");
    SerialMon.println(F(" adaptive data rate"));
  } else {
    SerialMon.println(F("--Failed to set adaptive data rate"));
  }
  delay(2000L);
#endif

#if LORA_AT_TEST_OTAA
  SerialMon.println(F("Attempting to join with OTAA..."));
  if (!modem.joinOTAA(appEui, appKey)) {
    SerialMon.println(F("--fail\n"));
    // power cycle
    if (power_pin_for_module >= 0) { digitalWrite(power_pin_for_module, LOW); }
    delay(10000);
    if (power_pin_for_module >= 0) {
      digitalWrite(power_pin_for_module, HIGH);
      delay(5000);
    }
    return;
  }
  SerialMon.println(F("  success"));
  delay(2000L);

  String devAddr = modem.getDevAddr();
  SerialMon.print(F("Device (Network) Address: "));
  SerialMon.println(devAddr);
  delay(2000L);

  String appEUI = modem.getAppEUI();
  SerialMon.print(F("App EUI (network ID): "));
  SerialMon.println(appEUI);
  delay(2000L);

  String appKey = modem.getAppKey();
  SerialMon.print(F("App Key (network key): "));
  SerialMon.println(appKey);
  delay(2000L);
#endif

#if LORA_AT_TEST_ABP
  SerialMon.println(F("Attempting to join with ABP..."));
  if (!modem.joinABP(devAddr, nwkSKey, appSKey, uplinkCounter,
                     downlinkCounter)) {
    SerialMon.print(F("--fail\n"));
    // power cycle
    if (power_pin_for_module >= 0) { digitalWrite(power_pin_for_module, LOW); }
    delay(10000);
    if (power_pin_for_module >= 0) {
      digitalWrite(power_pin_for_module, HIGH);
      delay(5000);
    }
    return;
  }
  SerialMon.println(F("  success"));
  delay(2000L);

  String devAddr = modem.getDevAddr();
  SerialMon.print(F("Device (Network) Address: "));
  SerialMon.println(devAddr);
  delay(2000L);

  String nwkSKey = modem.getNwkSKey();
  SerialMon.print(F("Network Session Key: "));
  SerialMon.println(nwkSKey);
  delay(2000L);

  String appSKey = modem.getAppSKey();
  SerialMon.print(F("App Session Key: "));
  SerialMon.println(appSKey);
  delay(2000L);
#endif

  bool res = modem.isNetworkConnected();
  SerialMon.print(F("Network status: "));
  SerialMon.println(res ? "connected" : "not connected");
  delay(2000L);
  // we may get downlink from the network link check in the isNetworkConnected
  // fxn
  if (loraStream.available()) {
    SerialMon.print(F("Got downlink data!\n<<<"));
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
    SerialMon.println(F(">>>"));
  }
  delay(2000L);

#if LORA_AT_TEST_UPLINK
  // Send out some data, without confirmation
  String short_message = "hello";
  SerialMon.println(F("Sending a short message without confirmation"));
  modem.requireConfirmation(false);
  if (loraStream.print(short_message) == short_message.length()) {
    SerialMon.println(F("  Successfully sent unconfirmed data"));
  } else {
    SerialMon.println(F("--Failed to send unconfirmed data!"));
    res = modem.isNetworkConnected();
    SerialMon.print(F("Network status: "));
    SerialMon.println(res ? "connected" : "not connected");
  }
  if (loraStream.available()) {
    SerialMon.print(F("Got downlink data!\n<<<"));
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
    SerialMon.println(F(">>>"));
  }
  delay(2000L);
  // Send out some data, requiring confirmation
  modem.requireConfirmation(true);
  SerialMon.println(F("Sending a short message with confirmation"));
  if (loraStream.print(short_message) == short_message.length()) {
    SerialMon.println(F("  Successfully sent acknowledged data"));
  } else {
    SerialMon.println(F("--Failed to send acknowledged data!"));
    res = modem.isNetworkConnected();
    SerialMon.print(F("Network status: "));
    SerialMon.println(res ? "connected" : "not connected");
  }
  if (loraStream.available()) {
    SerialMon.print(F("Got downlink data!\n<<<"));
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
    SerialMon.println(F(">>>"));
  }
  delay(2000L);
  // Send out some longer data, without confirmation
  modem.requireConfirmation(false);
  String longer_message = "This is a longer message";
  SerialMon.println(F("Sending a longer message without confirmation"));
  if (loraStream.print(longer_message) == longer_message.length()) {
    SerialMon.println(F("  Successfully sent longer unconfirmed message"));
  } else {
    SerialMon.println(F("--Failed to send longer unconfirmed message!"));
    res = modem.isNetworkConnected();
    SerialMon.print(F("Network status: "));
    SerialMon.println(res ? "connected" : "not connected");
  }
  if (loraStream.available()) {
    SerialMon.print(F("Got downlink data!\n<<<"));
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
    SerialMon.println(F(">>>"));
  }
  delay(2000L);
  String super_long_message =
      "This is a really long message that I'm using to test and ensure that "
      "packets are being broken up correctly by the send function.  Lorem "
      "ipsum dolor sit amet, consectetur adipiscing elit. Quisque vulputate "
      "dolor vitae ante vehicula molestie. Duis in quam nec dolor varius "
      "lobortis. Proin eget malesuada odio. Etiam condimentum sodales "
      "hendrerit. Curabitur molestie sem vel sagittis commodo. Proin ut tortor "
      "sodales, molestie nisl eget, ultricies dolor. Donec bibendum, dui nec "
      "pharetra ornare, ex velit ullamcorper mi, eu facilisis purus turpis "
      "quis dolor. Suspendisse rhoncus nisl non justo tempor vulputate. Duis "
      "sit amet metus sit amet leo tristique venenatis nec in libero. Maecenas "
      "pharetra enim quis ornare aliquet. Aenean pretium cursus magna, "
      "fringilla auctor metus faucibus non. Nulla blandit mauris a quam "
      "tincidunt commodo. Duis dapibus lorem eget augue ornare, id lobortis "
      "quam volutpat.";
  SerialMon.println(F("Sending a super long message without confirmation"));
  if (loraStream.print(super_long_message) == super_long_message.length()) {
    SerialMon.println(F("  Successfully sent super long message"));
  } else {
    SerialMon.println(F("--Failed to send super long message!"));
    res = modem.isNetworkConnected();
    SerialMon.print(F("Network status: "));
    SerialMon.println(res ? "connected" : "not connected");
  }
  if (loraStream.available()) {
    SerialMon.print(F("Got downlink data!\n<<<"));
    while (loraStream.available()) { SerialMon.write(loraStream.read()); }
    SerialMon.println(F(">>>"));
  }
  delay(2000L);
#endif

  int rssi = modem.getSignalQuality();
  SerialMon.print(F("Signal quality: "));
  SerialMon.println(rssi);
  delay(2000L);


#if LORA_AT_TEST_TIME && defined LORA_AT_HAS_TIME
#ifndef LORA_AT_MDOT  // mDot only does epoch time
  int   year3    = 0;
  int   month3   = 0;
  int   day3     = 0;
  int   hour3    = 0;
  int   min3     = 0;
  int   sec3     = 0;
  float timezone = 0;
  for (int8_t i = 5; i; i--) {
    SerialMon.println(F("Requesting current network time"));
    if (modem.getDateTimeParts(&year3, &month3, &day3, &hour3, &min3, &sec3,
                               &timezone)) {
      SerialMon.print(F("  Year:"));
      SerialMon.println(year3);
      SerialMon.print(F("  Month:"));
      SerialMon.println(month3);
      SerialMon.print(F("  Day:"));
      SerialMon.println(day3);
      SerialMon.print(F("  Hour:"));
      SerialMon.println(hour3);
      SerialMon.print(F("  Minute:"));
      SerialMon.println(min3);
      SerialMon.print(F("  Second:"));
      SerialMon.println(sec3);
      SerialMon.print(F("  Timezone:"));
      SerialMon.println(timezone);
      break;
    } else {
      SerialMon.println(F("--Couldn't get network time, retrying in 15s."));
      delay(15000L);
    }
  }

  SerialMon.println(F("Retrieving time as a string"));
  String time = modem.getDateTimeString(DATE_FULL);
  SerialMon.print(F("  Current Network Time: "));
  SerialMon.println(time);
#endif  // mDot only does epoch time

  SerialMon.println(F("Retrieving time as an offset from the epoch"));
  uint32_t epochTime = modem.getDateTimeEpoch();
  SerialMon.print(F("  Current Epoch Time: "));
  SerialMon.println(epochTime);
#endif

#if LORA_AT_TEST_BATTERY && defined LORA_AT_HAS_BATTERY
  int8_t  chargeState = -99;
  int8_t  percent     = -99;
  int16_t milliVolts  = -9999;
  modem.getBattStats(chargeState, percent, milliVolts);
  SerialMon.print(F("Battery charge state: "));
  SerialMon.println(chargeState);
  SerialMon.print(F("Battery charge 'percent': "));
  SerialMon.println(percent);
  SerialMon.print(F("Battery voltage: "));
  SerialMon.println(milliVolts / 1000.0F);
  delay(2000L);
#endif

#if LORA_AT_TEST_TEMPERATURE && defined LORA_AT_HAS_TEMPERATURE
  float temp = modem.getTemperature();
  SerialMon.print(F("Chip temperature: "));
  SerialMon.println(temp);
#endif

#if LORA_AT_TEST_SLEEP && defined LORA_AT_HAS_SLEEP_MODE
  // test sleeping and waking with the UART
  SerialMon.println(F("Testing basic sleep mode with UART wake after 5s"));
  if (modem.uartSleep()) {  // could alo use sleep();
    SerialMon.println(F("  Put LoRa modem to sleep"));
  } else {
    SerialMon.println(F("--Failed to put LoRa modem to sleep"));
  }
  delay(5000L);
  if (modem.testAT()) {
    SerialMon.println(F("  Woke up LoRa modem"));
  } else {
    SerialMon.println(F("--Failed to wake LoRa modem"));
  }


#ifndef LORA_AT_LORAE5  // LoRaE5 doesn't do pin sleep
  if (arduino_wake_pin >= 0) {
    // test sleeping and waking with the an interrupt pin
    SerialMon.println(
        F("Testing basic sleep mode with pin interrupt wake after 5s"));
    if (modem.pinSleep(lora_wake_pin, lora_wake_pullup, lora_wake_edge)) {
      SerialMon.println(F("  Put LoRa modem to sleep"));
    } else {
      SerialMon.println(F("--Failed to put LoRa modem to sleep"));
    }
    delay(5000L);
    digitalWrite(arduino_wake_pin, LOW);
    delay(50L);
    digitalWrite(arduino_wake_pin, HIGH);
    if (modem.testAT()) {
      SerialMon.println(F("  Woke up LoRa modem"));
    } else {
      SerialMon.println(F("--Failed to wake LoRa modem"));
    }
  }
#endif

  // test sleeping and waking for a set amount of time
  uint32_t sleepTime = 15000L;
  SerialMon.print(F("Testing timed sleep for "));
  SerialMon.print(sleepTime);
  SerialMon.println(F("ms"));
  if (modem.sleep(sleepTime)) {
    SerialMon.print(F("Put LoRa modem to sleep for "));
    SerialMon.print(sleepTime);
    SerialMon.println(F("ms"));
  } else {
    SerialMon.println(F("--Failed to put LoRa modem to sleep"));
  }
  // wait for the modem to wake back up
  delay(sleepTime + 1000L);
  if (modem.testAT()) {
    SerialMon.println(F("  Modem woke after timed sleep"));
  }

  // test automatic sleep functionality
  SerialMon.println(F("Testing automatic sleep mode"));
  if (modem.enableAutoSleep(true)) {
    SerialMon.println(F("  Enabled auto-sleep mode"));
    // wait a bit, see if the modem wakes on contact, and then see if it goes
    // back to sleep again
    delay(10000L);
    modem.testAT();
    delay(10000L);
  } else {
    SerialMon.println(F("--Failed to enable auto-sleep mode"));
  }
  if (modem.enableAutoSleep(false) && modem.testAT()) {
    SerialMon.println(F("  Disabled auto-sleep mode"));
  } else {
    SerialMon.println(F("--Failed to disable auto-sleep mode"));
  }
#endif

  SerialMon.println(F("End of tests.\n"));
  SerialMon.println(F("----------------------------------------"));
  SerialMon.println(F("----------------------------------------\n"));

  // Just listen forevermore
  while (true) {
    modem.maintain();
    if (loraStream.available()) {
      SerialMon.print(F("Got downlink data!\n<<<"));
      while (loraStream.available()) { SerialMon.write(loraStream.read()); }
      SerialMon.println(F(">>>"));
    }
  }
}
