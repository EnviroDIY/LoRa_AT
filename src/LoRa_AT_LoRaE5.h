/**
 * @file       LoRa_AT_LoRaE5.h
 * @author     Sara Damiano
 * @copyright  Copyright (c) 2024 Sara Damiano
 * @date       April 2024
 */

#ifndef SRC_LORA_AT_LORAE5_H_
#define SRC_LORA_AT_LORAE5_H_
// #pragma message("LoRa_AT:  LoRa_AT_LoRaE5")

// #define LORA_AT_DEBUG Serial

/// The new-line used by the LoRa module
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"  // NOTE:  define before including LoRa_AT_Modem!

#ifdef AT_OK
#undef AT_OK
#endif
#define AT_OK "+AT: OK"

#ifdef AT_ERROR
#undef AT_ERROR
#endif
#define AT_ERROR "+AT: ERROR"

#ifdef AT_VERBOSE
#undef AT_VERBOSE
#endif
#define AT_VERBOSE "+LOG: "

#include "LoRa_AT_Modem.tpp"
#include "LoRa_AT_Radio.tpp"
#include "LoRa_AT_Time.tpp"
#include "LoRa_AT_Battery.tpp"
#include "LoRa_AT_Temperature.tpp"
#include "LoRa_AT_Sleep.tpp"

class LoRa_AT_LoRaE5 : public LoRa_AT_Modem<LoRa_AT_LoRaE5>,
                       public LoRa_AT_Time<LoRa_AT_LoRaE5>,
                       public LoRa_AT_Radio<LoRa_AT_LoRaE5>,
                       public LoRa_AT_Battery<LoRa_AT_LoRaE5>,
                       public LoRa_AT_Temperature<LoRa_AT_LoRaE5>,
                       public LoRa_AT_Sleep<LoRa_AT_LoRaE5> {
  friend class LoRa_AT_Modem<LoRa_AT_LoRaE5>;
  friend class LoRa_AT_Time<LoRa_AT_LoRaE5>;
  friend class LoRa_AT_Radio<LoRa_AT_LoRaE5>;
  friend class LoRa_AT_Battery<LoRa_AT_LoRaE5>;
  friend class LoRa_AT_Temperature<LoRa_AT_LoRaE5>;
  friend class LoRa_AT_Sleep<LoRa_AT_LoRaE5>;

  /*
   * Inner Client
   */
 public:
  class LoRaStream_LoRaE5 : public LoRaStream {
    friend class LoRa_AT_LoRaE5;

   public:
    LoRaStream_LoRaE5() {}

    explicit LoRaStream_LoRaE5(LoRa_AT_LoRaE5& modem) {
      init(&modem);
    }

    bool init(LoRa_AT_LoRaE5* modem) {
      this->at       = modem;
      sock_available = 0;
      at->loraStream = this;

      return true;
    }

    /*
     * Extended API
     */
  };

  /*
   * Constructor
   */
 public:
  explicit LoRa_AT_LoRaE5(Stream& stream) : stream(stream) {
    prev_dl_check        = 0;
    inLowestPowerMode    = false;
    _requireConfirmation = false;
    _msg_quality         = 0;
    _link_margin         = 255;
    _networkConnected    = false;
  }


  /*
   * Basic functions
   */
 public:
  /**
   * @brief Recursive variadic template to send AT commands
   *
   * This is re-written for the LoRa E5 to handle lowest power mode
   *
   * @tparam Args
   * @param cmd The commands to send
   */
  template <typename... Args>
  inline void sendAT(Args... cmd) {
    if (inLowestPowerMode) {
      // If extremely low power mode is enabled, when sending commands to
      // modem, at least four 0xFFs need to be added to the start of each AT
      // command.
      for (uint8_t i = 0; i < 4; i++) { stream.write(0xFF); }
    }
    streamWrite("AT", cmd..., AT_NL);
    stream.flush();
    LORA_AT_YIELD(); /* DBG("### AT:", cmd...); */
  }

  bool testATImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF(""));
      if (waitResponse(200) == 1) { return true; }
      delay(100);
    }
    // if it fails, try testing again with the extra 0xFF wake-ups for low
    // power mode
    DBG(GF("Trying low-power test!"));
    inLowestPowerMode = true;
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF(""));
      if (waitResponse(200) == 1) {
        DBG("### LoRa E5 is in auto low power mode.");
        return true;
      }
      delay(100);
    }
    inLowestPowerMode = false;
    return false;
  }

 protected:
  bool initImpl() {
    DBG(GF("### LoRa_AT Version:"), LORA_AT_VERSION);
    DBG(GF("### LoRa_AT Compiled Module:  LoRa_AT_LoRaE5"));

    if (!testAT()) { return false; }

#ifdef LORA_AT_DEBUG
    sendAT(GF("+LOG=DEBUG"));  // turn on verbose error codes
    waitResponse(GF("+LOG: DEBUG"));
    streamFind('\n');  // throw away the new line
#else
    sendAT(GF("+LOG=QUIET"));  // turn off verbose error codes
    waitResponse(GF("+LOG: QUIET"));
    streamFind('\n');  // throw away the new line
#endif
    return true;
  }

  bool setBaudImpl(uint32_t baud) {
    sendAT(GF("+UART=BR, "), baud);
    bool resp = waitResponse(GF("+UART=BR, "));
    resp &= (uint32_t)stream.parseInt() == baud;
    streamFind('\n');  // throw away the new line
    return resp;
  }

  String getDevEUIImpl() {
    String resp;
    sendAT(GF("+ID=DevEUI"));
    if (waitResponse(GF("+ID: DevEui, ")) != 1) { return "UNKNOWN"; }
    resp = stream.readString();
    resp.trim();
    return resp;
  }

  String getModuleInfoImpl() {
    String info = "Firmware: ";
    info += sendATGetString(GF("+VER"));
    info += " LoRaWan: ";
    String info2;
    sendAT(GF("+LW=VER"));
    if (waitResponse(GF("+LW: VER, ")) != 1) {
      info2 = "UNKNOWN";
    } else {
      info2 = stream.readStringUntil('\r');
    }
    return info + info2;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("+FDEFAULT"));  // Factory default settings
    return waitResponse() == 1;
  }


  /*
   * Power functions
   */
 protected:
  bool restartImpl() {
    if (!testAT()) { return false; }
    sendAT(GF("+RESET"));  // Reset (restart) the CPU
    bool resp = waitResponse(GF("+RESET: OK")) == 1;
    if (resp) { return init(); };
    return false;
  }


  /*
   * Sleep functions
   */
 protected:
  bool pinSleepImpl(int8_t pin, int8_t pullupMode,
                    int8_t trigger) LORA_AT_ATTR_NOT_AVAILABLE;

  bool uartSleepImpl() {
    // enable sleep, will wake with the next UART TX command
    sendAT(GF("+LOWPOWER"));
    // inLowestPowerMode = enable;
    // ^^ this is not the lowest power mode using the extra 0xFF characters
    bool resp = waitResponse(GF("+LOWPOWER:")) == 1;
    resp &= waitResponse(GF("SLEEP")) == 1;
    streamFind('\n');  // throw away the new line
    // There will be a +LOWPOWER: WAKEUP message at the next UART communication
    return true;
  }

  bool sleepImpl(uint32_t sleepTimer) {
    sendAT(GF("+LOWPOWER="), sleepTimer);
    bool resp = waitResponse(GF("+LOWPOWER:")) == 1;
    resp &= waitResponse(GF("SLEEP")) == 1;
    streamFind('\n');  // throw away the new line
    return resp;
  }

  bool enableAutoSleepImpl(bool enable = true) {
    sendAT(GF("+LOWPOWER="), enable ? GF("AUTOON") : GF("AUTOOFF"));
    bool resp = waitResponse(GF("+LOWPOWER:")) == 1;
    resp &= waitResponse(GF("AUTOOFF"), GF("AUTOON")) - 1 == enable;
    if (resp) { inLowestPowerMode = enable; }
    streamFind('\n');  // throw away the new line
    return resp;
  }


  /*
   * Generic network functions
   */
 protected:
  bool setPublicNetworkImpl(bool isPublic) {
    sendAT(GF("+LW=NET, "), isPublic ? GF("ON") : GF("OFF"));
    bool resp = waitResponse(GF("LW: NET, "));
    resp &= waitResponse(isPublic ? GF("ON") : GF("OFF")) == 1;
    streamFind('\n');  // throw away the new line
    return resp;
  }
  bool getPublicNetworkImpl() {
    sendAT(GF("+LW=NET"));
    bool resp = waitResponse(GF("ON"), GF("OFF")) == 1;
    streamFind('\n');  // throw away the new line
    return resp;
  }

  bool setConfirmationRetriesImpl(int8_t numAckRetries) {
    sendAT(GF("+RETRY="), numAckRetries);
    bool resp = waitResponse(GF("+RETRY: "));
    resp &= stream.parseInt() == numAckRetries;
    streamFind('\n');  // throw away the new line
    return resp;
  }
  int8_t getConfirmationRetriesImpl() {
    sendAT(GF("+RETRY"));
    waitResponse(GF("+RETRY: "));
    int8_t resp = stream.parseInt();
    streamFind('\n');  // throw away the new line
    return resp;
  }

  bool joinOTAAImpl(const char* appEui, const char* appKey, const char* devEui,
                    uint32_t timeout, bool) {
    // The App EUI must be a hex value
    sendAT(GF("+ID=AppEui, \""), appEui, '"');
    waitResponse(GF("+ID: AppEui"));  // echos the set command
    streamFind('\n');                 // throw away the echoed App EUI
    // The App Key must also be a hex value
    sendAT(GF("+KEY=APPKEY, \""), appKey, '"');
    waitResponse(GF("+KEY: APPKEY"));  // echos the set command
    streamFind('\n');                  // throw away the echoed App Key
    if (devEui != nullptr) {
      sendAT(GF("+ID=DevEui, \""), devEui, '"');  // set the device EUI
      waitResponse(GF("+ID: DevEui"));            // echos the set command
      streamFind('\n');  // throw away the echoed Device EUI
    }
    changeModes(OTAA);
    return join(5, timeout);  // join the network
  }

  bool joinABPImpl(const char* devAddr, const char* nwkSKey,
                   const char* appSKey, int uplinkCounter = 1,
                   int      downlinkCounter = 0,
                   uint32_t timeout         = DEFAULT_JOIN_TIMEOUT) {
    sendAT(GF("+ID=DevAddr, \""), devAddr, '"');  // set the device address
    waitResponse(GF("+ID: DevAddr"));             // echos the set command
    streamFind('\n');  // throw away the echoed Device Address
    sendAT(GF("+KEY=APPSKEY,\""), appSKey,
           '"');  // set the data session key (app session key)
    waitResponse(GF("+KEY: APPSKEY"));  // echos the set command
    streamFind('\n');                   // throw away the echoed App Session Key
    sendAT(GF("+KEY=NWKSKEY,\""), nwkSKey,
           '"');                        // set the network session key
    waitResponse(GF("+KEY: NWKSKEY"));  // echos the set command
    streamFind('\n');  // throw away the echoed network session key
    // set the uplink and downlink counters, if we need to
    if (uplinkCounter != 1 || downlinkCounter != 0) {
      sendAT(GF("+LW=ULDL, "), uplinkCounter, ',',
             downlinkCounter);        // set the network session key
      waitResponse(GF("+LW: ULDL"));  // echos the set command
      streamFind('\n');               // throw away the echoed counters
    }
    changeModes(ABP);
    return isNetworkConnected();  // verify that we're connected
  }

  bool isNetworkConnectedImpl() {
    int8_t tries_remaining = 10;
    _link_margin           = 255;
    while (_link_margin == 255 && tries_remaining) {
      sendAT(GF("+LW=LCR"));
      waitResponse(GF("+LW: LCR"));
      streamFind('\n');  // throw away the new line
      DBG(GF("Sending empty message to carry LinkCheckReq"), tries_remaining,
          GF("tries remaining"));
      modemSend(nullptr, 0);
      tries_remaining--;
      // delay before the next attempt
      if (_link_margin == 255) {
        DBG(GF("Delay 5s before next LinkCheckReq attempt"));
        delay(5000L);
      }
    }
    if (_link_margin != 255) {
      _networkConnected = true;
    } else {
      _networkConnected = false;
    }
    return _link_margin != 255;
  }

  int8_t getSignalQualityImpl() {
    // the RSSI is returned when reading a message
    // you can also get the RSSI by entering test mode, but this causes you to
    // drop off the network and requires you to rejoin
    int8_t tries_remaining = 5;
    while (_msg_quality == 0 && tries_remaining) {
      DBG(GF("Sending empty message to get RSSI"), tries_remaining,
          GF("tries remaining"));
      modemSend(nullptr, 0);
      tries_remaining--;
    }
    return _msg_quality;
  }


  /*
   * LoRa Class and Band functions
   */
 protected:
  bool setClassImpl(_lora_class _class) {
    sendAT(GF("+CLASS="), (char)_class);
    bool   resp     = waitResponse(GF("+CLASS: "));
    int8_t devClass = waitResponse(GF("A"), GF("B"), GF("C"));
    resp &= (_lora_class)(devClass - 1 + 'A') == _class;
    streamFind('\n');  // throw away the new line
    return resp;
  }
  _lora_class getClassImpl() {
    sendAT(GF("+CLASS"));
    waitResponse(GF("+CLASS: "));
    int8_t devClass = waitResponse(GF("A"), GF("B"), GF("C"));
    streamFind('\n');  // throw away the new line
    return (_lora_class)(devClass - 1 + 'A');
  }

  bool setPortImpl(uint8_t _port) {
    sendAT(GF("+PORT="), _port);
    bool resp = waitResponse(GF("+PORT: "));  // always echos
    resp &= stream.parseInt() == _port;
    streamFind('\n');  // throw away the new line
    return resp;
  }
  uint8_t getPortImpl() {
    sendAT(GF("+PORT"));
    waitResponse(GF("+PORT: "));  // always echos
    int8_t resp = stream.parseInt();
    streamFind('\n');  // throw away the new line
    return resp;
  }

  bool setBandImpl(const char* band) {
    sendAT(GF("+DR="), band);
    bool resp = waitResponse(GF("+DR: "));
    streamFind('\n');  // throw away the returned band
    return resp;
  }
  String getBandImpl() {
    sendAT(GF("+DR=SCHEME"));
    waitResponse(GF("+DR: "));
    String resp = stream.readStringUntil('\r');
    streamDump();  // throw away all the details about the band data rates
    return resp;
  }

  // these don't seem to exist
  bool   setFrequencySubBandImpl(int8_t subBand) LORA_AT_ATTR_NOT_AVAILABLE;
  int8_t getFrequencySubBandImpl() LORA_AT_ATTR_NOT_AVAILABLE;

  // There isn't a simple way to get the whole mask!
  String getChannelMaskImpl() {
    // start with an empty mask
    uint8_t channelsMask[LORA_CHANNEL_BYTES] = {0};
    sendAT(GF("+CH"));  // request all channel type_info
    waitResponse(GF("+CH: "));
    int8_t num_active_channels = 0;
    // bool   all_inactive = waitResponse(50, GF("No channel is activated"))
    // == 1;
    num_active_channels = stream.parseInt();
    DBG(GF("\nTotal Active Channels:"), num_active_channels);
    if (num_active_channels > 0) {
      streamFind(';');  // skip the ;
    }
    // first returns the number of enabled channels, then metadata about the
    // enabled channels
    for (int8_t i = 0; i < num_active_channels; i++) {
      int8_t active_channel_num = stream.parseInt();
      streamFind(',');  // skip the ,
      streamFind(';');  // skip the channel data rates

      // get the channel possition in the array
      int row = getChannelOffset(active_channel_num);
      // convert the channel position into a mask
      uint8_t mask = getChannelBitMask(active_channel_num);

      // add this channel mask to the full mask
      channelsMask[row] = channelsMask[row] | mask;
      // #ifdef LORA_AT_DEBUG
      //       String binPrint = "";
      //       int8_t bin_len  = String(channelsMask[row], BIN).length();
      //       for (int8_t z = 0; z < 8 - bin_len; z++) { binPrint += "0"; }
      //       binPrint += String(channelsMask[row], BIN);
      //       DBG(GF("Current Row Mask:"), binPrint);
      // #endif
    }

    // convert the completed channel mask to a string
    return createHexChannelMask(channelsMask);
  }

  bool isChannelEnabledImpl(int pos) {
    sendAT(GF("+CH="), pos);  // request all channel type_info
    waitResponse(GF("+CH: "));
    // if enabled, it will return the frequency and data rate
    // +CH: 1,902500000,DR0:DR3
    // if disabled, the retur frequency will be 0
    // +CH: 1,0,DR0:DR0
    uint8_t ret_channel = stream.parseInt();
    // DBG(GF("Returned channel:"), ret_channel);
    streamFind(',');  // skip the , after the channel number
    uint32_t ret_freq = stream.parseInt();
    // DBG(GF("Channel frequency:"), ret_freq);
    streamFind(',');   // skip the , after the frequency
    streamFind('\n');  // dump the data rates
    return ret_channel == pos && ret_freq > 0;
  }

  bool enableChannelImpl(int pos, bool enable) {
    sendAT(GF("+CH="), pos, ',', enable ? GF("ON") : GF("OFF"));
    bool resp = waitResponse(GF("+CH: CH"));
    // NOTE:  may be caps or not
    int8_t resp2 = waitResponse(GF("ON"), GF("on"), GF("OFF"), GF("off"));
    resp &= ((resp2 == 1 || resp2 == 2) && enable) ||
        ((resp2 == 3 || resp2 == 4) && !enable);
    streamFind('\n');  // throw away the new line
    return resp;
  }

  // There isn't a simple way to set the whole mask!
  bool setChannelMaskImpl(const char* newMask) {
    bool success = true;

    uint8_t channelsMask[LORA_CHANNEL_BYTES] = {0};
    // parse the hex mask into a bit array
    parseChannelMask(newMask, channelsMask);

    // iterate through max of 72 channels
    for (uint8_t i = 0; i < 72; i++) {
      // get the channel possition in the array
      int row = getChannelOffset(i);
      // convert the channel position into a mask
      uint8_t channel = getChannelBitMask(i);

      bool channelEnabled = (channelsMask[row] & channel) > 0;
      // DBG(GF("Channel"), i, GF("should be"),
      //     channelEnabled ? GF("enabled") : GF("disabled"));

      // enable or disable the channel
      if (channelEnabled) {
        success &= enableChannel(i);
      } else {
        success &= disableChannel(i);
      }
    }
    return success;
  }


  /*
   * LoRa Data Rate and Duty Cycle functions
   */
 protected:
  bool enableDutyCycleImpl(bool dutyCycle) {
    sendAT(GF("+LW=DC, "), dutyCycle ? GF("ON") : GF("OFF"));
    bool resp = waitResponse(GF("+LW: DC"));  // echos your command
    resp &= waitResponse(GF("OFF"),
                         GF("ON")) -
            1 ==
        dutyCycle;     // returns on/off for the duty cycle limit
    streamFind('\n');  // throw away the max duty cycle setting
    return resp;
  }
  bool isDutyCycleEnabledImpl() {
    sendAT(GF("+LW=DC"));
    waitResponse(GF("+LW: DC"));  // echos your command
    bool resp = waitResponse(GF("ON"), GF("OFF")) == 1;
    streamFind('\n');  // throw away the new line
    return resp;
  }

  bool setMaxDutyCycleImpl(int8_t maxDutyCycle) {
    sendAT(GF("+LW=DC, "), maxDutyCycle);
    bool resp = waitResponse(GF("+LW: DC"));  // echos your command
    waitResponse(GF("ON"),
                 GF("OFF"));  // returns on/off for the duty cycle limit
    resp &= stream.parseInt() == maxDutyCycle;  // then returns the limit value
    streamFind('\n');                           // throw away the new line
    return resp;
  }
  int8_t getMaxDutyCycleImpl() {
    sendAT(GF("+LW=DC"));
    waitResponse(GF("+LW: DC"));  // echos your command
    waitResponse(GF("ON"),
                 GF("OFF"));          // returns on/off for the duty cycle limit
    int8_t resp = stream.parseInt();  // then returns the limit value
    streamFind('\n');                 // throw away the new line
    return resp;
  }

  bool setDataRateImpl(uint8_t dataRate) {
    sendAT(GF("+DR="), dataRate);
    bool resp = waitResponse(GF("+DR: DR"));
    resp &= stream.parseInt() == dataRate;
    streamFind('\n');  // throw away the new line
    return resp;
  }
  int8_t getDataRateImpl() {
    sendAT(GF("+DR"));
    waitResponse(GF("+DR: DR"));  // always echos
    int8_t resp = stream.parseInt();
    // throw away anything else in the long response
    streamDump();
    return resp;
  }

  bool setAdaptiveDataRateImpl(bool useADR) {
    sendAT(GF("+ADR="), useADR ? GF("ON") : GF("OFF"));
    bool resp = waitResponse(GF("+ADR:"));
    resp &= waitResponse(useADR ? GF("ON") : GF("OFF")) == 1;
    streamFind('\n');  // throw away the new line
    return resp;
  }
  bool getAdaptiveDataRateImpl() {
    sendAT(GF("+ADR?"));
    bool resp = waitResponse(GF("ON"), GF("OFF")) == 1;
    streamFind('\n');  // throw away the new line
    return resp;
  }


  /*
   * LoRa ABP Session Properties
   */
 protected:
  // aka network address
  String getDevAddrImpl() {
    String resp;
    sendAT(GF("+ID=DevAddr"));
    if (waitResponse(GF("+ID: DevAddr, ")) != 1) { return "UNKNOWN"; }
    return stream.readStringUntil('\r');
  }

  // network session key
  String getNwkSKeyImpl() {
    return "NOT READABLE";
  }

  // app session Key (data session key)
  String getAppSKeyImpl() {
    return "NOT READABLE";
  }


  /*
   * LoRa OTAA Session Properties
   */
 protected:
  // aka network id
  String getAppEUIImpl() {
    String resp;
    sendAT(GF("+ID=AppEui"));
    if (waitResponse(GF("+ID: AppEui, ")) != 1) { return "UNKNOWN"; }
    return stream.readStringUntil('\r');
  }
  // aka network key
  String getAppKeyImpl() {
    return "NOT READABLE";
  }


  /*
   * Time functions
   */
 protected:
  String getDateTimeStringImpl(LoRa_AT_DateTimeFormat format) {
    deviceTimeRequest();
    sendAT(GF("+RTC=FULL"));
    if (waitResponse(2000L, GF("+RTC: ")) != 1) { return ""; }

    String res;

    switch (format) {
      case DATE_FULL: res = stream.readStringUntil(','); break;
      case DATE_TIME:
        streamFind(' ');  // skip until the next blank space
        res = stream.readStringUntil(' ');
        break;
      case DATE_DATE: res = stream.readStringUntil(' '); break;
    }
    streamFind('\n');  // throw away epoch and age
    return res;
  }

  bool getDateTimePartsImpl(int* year, int* month, int* day, int* hour,
                            int* minute, int* second, float* timezone) {
    deviceTimeRequest();
    sendAT(GF("+RTC=FULL"));
    if (waitResponse(2000L, GF("+RTC: ")) != 1) { return false; }

    int iyear     = 0;
    int imonth    = 0;
    int iday      = 0;
    int ihour     = 0;
    int imin      = 0;
    int isec      = 0;
    int itimezone = 0;

    // Date & Time
    iyear = stream.parseInt();
    streamFind('-');  // skip the - after the year
    imonth = stream.parseInt();
    streamFind('-');  // skip the - after the month
    iday = stream.parseInt();
    streamFind(' ');  // skip the space after the day
    ihour = stream.parseInt();
    streamFind(':');  // skip the : after the hour
    imin = stream.parseInt();
    streamFind(':');  // skip the : after the minute
    isec        = stream.parseInt();
    char tzSign = stream.read();
    itimezone   = stream.parseInt();
    streamFind(':');  // skip the : after the time zone hour
    if (tzSign == '-') { itimezone = itimezone * -1; }
    streamFind('\n');  // throw away epoch and age

    // Set pointers
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = isec;
    if (timezone != nullptr) *timezone = itimezone;
    return true;
  }

  uint32_t getDateTimeEpochImpl(LoRa_AT_EpochStart epoch = UNIX) {
    deviceTimeRequest();
    sendAT(GF("+RTC=FULL"));
    if (waitResponse(2000L, GF("+RTC: ")) != 1) { return 0; }
    streamFind(',');  // Skip the text string

    uint32_t resp = stream.parseInt();
    streamFind('\n');  // throw away age

    if (resp != 0) {
      switch (epoch) {
        case UNIX: resp += 315878400; break;
        case Y2K: resp -= 630806400; break;
        case GPS: resp += 0; break;
      }
    }

    return resp;
  }


  /*
   * NTP server functions
   */
 protected:
  // No functions of this type

  /*
   * Battery functions
   */
 protected:
  int16_t getBattVoltageImpl() {
    sendAT(GF("+VDD"));
    waitResponse(GF("+VDD: "));
    float resp = stream.parseFloat();
    streamFind('\n');  // Throw away the "V" if it's there
    // convert V to mV
    return (int16_t)(resp * 1000.);
  }

  int8_t getBattPercentImpl() {
    sendAT(GF("+LW=BAT"));
    waitResponse(GF("+LW: BAT,"));
    // Read battery charge level
    // returns a number between 0 and 255
    float resp = stream.parseFloat();
    streamFind('\n');  // throw away the new line
    return (int8_t)((resp / 255.) * 100.);
  }

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    sendAT(GF("+LW=BAT"));
    bool wasOk = waitResponse(GF("+LW: BAT,")) == 1;
    // Read battery charge level
    // returns a number between 0 and 255
    float resp = stream.parseFloat();
    streamFind('\n');  // throw away the new line
    percent = (int8_t)((resp / 255.) * 100.);

    sendAT(GF("+VDD"));
    wasOk &= waitResponse(GF("+VDD: "));
    resp = stream.parseFloat();
    streamFind('\n');  // throw away up to the new line
    // convert V to mV
    milliVolts = (int16_t)(resp * 1000.);
    return wasOk;
  }


  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    sendAT(GF("+TEMP"));
    waitResponse(GF("+TEMP: "));
    float resp = stream.parseFloat();
    streamFind('\n');  // throw away the new line
    return resp;
  }


  /*
   * Stream related functions
   */
 protected:
  int16_t modemSend(const uint8_t* buff, size_t len) {
    // Pointer to where in the buffer we're up to
    // A const cast is need to cast-away the constantness of the buffer (ie,
    // modify it).
    uint8_t* txPtr     = const_cast<uint8_t*>(buff);
    size_t bytesSent = 0;

    GsmConstStr at_msg_cmd;
#ifdef LORA_AT_SEND_HEX
    at_msg_cmd = GF("+MSGHEX");
#else
    if (_requireConfirmation && len > 0) {
      at_msg_cmd = GF("+CMSG");  // cannot carry 0 payload
    } else if (_requireConfirmation) {
      // must use CMSGHEX to have confirmation and 0 payload
      at_msg_cmd = GF("+CMSGHEX");
    } else {
      at_msg_cmd = GF("+MSG");
    }
#endif

    do {
      // make no more than 5 attempts at the single send command
      int8_t send_attempts = 0;
      bool   send_success  = false;
      while (send_attempts < 5 && !send_success) {
        uint8_t sendLength =
            0;  // Number of bytes to send from buffer in this command

        // check how many bytes are available for the next uplink
        uint8_t uplinkAvailable = 0;
        if (len != 0) {
          sendAT(GF("+LW=LEN"));
          waitResponse(GF("+LW: LEN,"));  // echo
          uplinkAvailable = stream.parseInt();
          streamFind('\n');  // throw away the new line
          DBG(uplinkAvailable, GF("bytes available for uplink."),
              !uplinkAvailable ? GF("Flush the MAC buffer with empty message.")
                               : GF(" "));
        }

        // Ensure the program doesn't read past the allocated memory
        sendLength = uplinkAvailable;
        if (txPtr + uplinkAvailable > const_cast<uint8_t*>(buff) + len) {
          sendLength = const_cast<uint8_t*>(buff) + len - txPtr;
        }
        // if there's no space available to send data, the queue is full of MAC
        // commands and we need to send empty messages to flush them out
        if (uplinkAvailable == 0 || len == 0) {
          sendAT(at_msg_cmd);
        } else {
          // start the send command
          if (inLowestPowerMode) {
            // If extremely low power mode is enabled, when sending commands
            // to modem, at least four 0xFFs need to be added to the start of
            // each AT command.
            for (uint8_t i = 0; i < 4; i++) { stream.write(0xFF); }
          }
          stream.write("AT");
          stream.print(at_msg_cmd);
          stream.write("=\"");
#ifdef LORA_AT_SEND_HEX
          // write everything as hex characters
          writeHex(txPtr, sendLength);
#else
          // write out the number of bytes that are available for this uplink
          stream.write(reinterpret_cast<const uint8_t*>(txPtr), sendLength);
#endif
          stream.write('"');
          // finish with a new line
          stream.println();
          stream.flush();
        }

        // always sends a start notice
        waitResponse(GF(": Start"));
        // the downlink should be processed by handle URCs
        uint32_t sendTimeout;
        if (_requireConfirmation) {
          sendTimeout = DEFAULT_ACKMESSAGE_TIMEOUT;
        } else {
          sendTimeout = DEFAULT_MESSAGE_TIMEOUT;
        }
        if (_requireConfirmation) {
          if (waitResponse(sendTimeout, GF(": ACK Received"), GF(": Done")) ==
              1) {
            bytesSent += sendLength;   // bump up number of bytes sent
            txPtr += sendLength;       // bump up the pointer
            prev_dl_check = millis();  // mark that we checked for downlink
            send_success  = true;
            streamFind('\n');  // throw away the new line
            waitResponse(sendTimeout, GF(": Done"));
          } else {
            DBG(GF("No ACK received on ACK message!"));
            break;
          }
        } else {
          if (waitResponse(sendTimeout, GF(": Done")) == 1) {
            bytesSent += sendLength;   // bump up number of bytes sent
            txPtr += sendLength;       // bump up the pointer
            prev_dl_check = millis();  // mark that we checked for downlink
            send_success  = true;
            streamFind('\n');  // throw away the new line
          }
        }
        DBG(send_attempts, send_success);
        send_attempts++;
      }
      // if we completely failed after 5 attempts, bail from the whole thing
      if (!send_success) { break; }
    } while (bytesSent < len && _networkConnected);
    return bytesSent;
  }

  size_t modemRead() {
    size_t totalBytesRead  = 0;
    int    downlinkedBytes = 1;
    if (loraStream->rx.free() == 0) {
      DBG("Buffer is full! Not requesting downlink data!");
    }
    if (!_networkConnected) {
      DBG("Not joined to network! Can't request downlink data!");
    }
    while (downlinkedBytes > 0 && loraStream->rx.free() > 0 &&
           _networkConnected) {
      size_t prev_size = loraStream->rx.size();
      // Check for new downlink data using the by issuing an empty send command.
      modemSend(nullptr, 0);
      size_t curr_size = loraStream->rx.size();
      downlinkedBytes  = curr_size - prev_size;
      totalBytesRead += downlinkedBytes;
    }
    return totalBytesRead;
  }


  /*
   * Utilities
   */
 private:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+LOWPOWER: WAKEUP" AT_NL))) {
      // inLowestPowerMode = false;
      // ^^ this is not the lowest power mode usingthe extra 0xFF characters
      return true;
    } else if (data.endsWith(GF(": Please join network first" AT_NL))) {
      _networkConnected = false;
      DBG("### Network disconnected, please re-join!");
      return true;
    } else if (data.endsWith(GF(": PORT: "))) {
      // +MSG: PORT: 8; RX: "12345678"
#ifdef LORA_AT_DEBUG
      int8_t incoming_port = stream.parseInt();
      DBG("## Data received on port", incoming_port);
#endif
      streamFind(';');  // skip the ; after the port
      streamFind('"');  // skip to the "

      // create a temporary buffer for reading
      // the data always comes in as hex
      // we always translate the data into ASCII, with two bytes of hex
      // translating to one character
      uint8_t tempRxBuff[LORA_AT_RX_BUFFER * 2];
      memset(tempRxBuff, '\0', LORA_AT_RX_BUFFER * 2);
      // read bytes until the next '"'
      int downlinkedBytes = stream.readBytesUntil('"', tempRxBuff,
                                                  LORA_AT_RX_BUFFER * 2);
      DBG("## Got", downlinkedBytes, "bytes of downlink data");
      // check for buffer overflow
      int putBuffLen = downlinkedBytes / 2;
      if (putBuffLen > loraStream->rx.free()) {
        DBG("### Buffer overflow: ", putBuffLen, "->", loraStream->rx.free());
        // reset amount to put into the buffer to the free space available
        putBuffLen = loraStream->rx.free();
      }
      // translate the hex data to ascii
      for (int i = 0; i < putBuffLen * 2; i += 2) {
        char buf[4] = {
            0,
        };
        buf[0] = tempRxBuff[i];
        buf[1] = tempRxBuff[i + 1];
        char c = strtol(buf, nullptr, 16);
        loraStream->rx.put(c);
      }
      // reset the available count
      loraStream->sock_available = loraStream->rx.size();
      return true;
    } else if (data.endsWith(GF(": RXWIN"))) {
      // +MSG: RXWIN2, RSSI -106, SNR 4
      streamFind('I');  // skip to the I
      _msg_quality = stream.parseInt();
      DBG(GF("Got RSSI:"), _msg_quality);
      streamFind('\n');  // skip the SNR
      return true;
    } else if (data.endsWith(GF(": Link"))) {
      // +MSG: Link 20, 1
      _link_margin = stream.parseInt();
      streamFind(',');  // skip the , after the link margin
#ifdef LORA_AT_DEBUG
      int8_t gateway_count = stream.parseInt();
      DBG(GF("## LinkCheckAns received. Link Margin:"), _link_margin,
          GF("Number Gateways:"), gateway_count);
#endif
      streamFind('\n');  // skip the SNR
      return true;
    }
    return false;
  }

  bool join(uint8_t attempts, uint32_t timeout, bool force = false) {
    // try multiple times to join
    bool    success            = false;
    uint8_t attempts_remaining = attempts;
    while (!success && attempts_remaining) {
#ifdef LORA_AT_DEBUG
      uint32_t start = millis();
#endif
      sendAT(force ? GF("+JOIN=FORCE") : GF("+JOIN"));
      attempts_remaining--;
      int8_t join_resp = waitResponse(
          timeout, GF("+JOIN: Network joined"), GF("+JOIN: Join failed"),
          GF("+JOIN: Joined already"), GF("+JOIN: LoRaWAN modem is busy"),
          GF("+JOIN: Not in OTAA mode"));
      if (join_resp == 1 || join_resp == 3) {
        success           = true;
        _networkConnected = true;
        if (join_resp == 1) {
          DBG(GF("Successfully joined network after"), millis() - start,
              GF("ms"));
        }
        if (join_resp == 3) {
          DBG(GF("Network already connected"), millis() - start, GF("ms"));
        }
      } else {
        DBG(GF("Join attempted failed after"), millis() - start, GF("ms with"),
            attempts_remaining, GF("attempts remaining"));
        // TODO: Implement join backoff
      }
      if (join_resp != 3 && join_resp != 4) {
        if (waitResponse(timeout, GF("+JOIN: Done")) == 1) {
          DBG(GF("Join finished after"), millis() - start, GF("ms"));
        } else {
          DBG(GF("Join timed out after"), millis() - start, GF("ms"));
        };
      }
      streamFind('\n');  // throw away the new line
    }
    return success;
  }

  bool changeModes(_lora_mode mode) {
    bool success = true;
    if (mode == OTAA) {
      sendAT(
          GF("+MODE=LWOTAA"));  // Configure for Over the Air Activation (OTAA)
      success &= waitResponse(GF("+MODE: LWOTAA")) == 1;
    } else if (mode == ABP) {
      sendAT(
          GF("+MODE=LWABP"));  // Configure for manual provisioning/Activation
                               // by Personalization (ABP)
      success &= waitResponse(GF("+MODE: LWABP")) == 1;
    }
    streamFind('\n');  // throw away the new line
    return success;
  }

  bool deviceTimeRequest() {
    // Bufferer DeviceTimeReq MAC command for AT modem, the MAC command will
    // be sent in next LoRaWAN transaction controlled by command
    // MSG/CMSG/MSGHEX/CMSGHEX. It is recommended to use MSGHEX and CMSGHEX to
    // carry this command if there is no application payload to send.
    sendAT(GF("+LW=DTR"));
    waitResponse(GF("+LW: DTR"));
    streamFind('\n');  // throw away the new line
    DBG(GF("Sending empty message to carry DeviceTimeReq"));
    modemSend(nullptr, 0);
    return true;
  }

  String sendATGetString(GsmConstStr cmd) {
    String resp;
    sendAT(cmd);
    if (waitResponse(cmd) != 1) { return "UNKNOWN"; }
    streamFind(' ');  // skip until the next blank space
    return stream.readStringUntil('\r');
  }


 public:
  Stream& stream;

 protected:
  LoRaStream_LoRaE5* loraStream;
  bool               inLowestPowerMode;
  int8_t             _msg_quality;
  uint8_t            _link_margin;
};

#endif  // SRC_LORA_AT_LORAE5_H_
