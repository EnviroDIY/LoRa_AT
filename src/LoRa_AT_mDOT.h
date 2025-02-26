/**
 * @file       LoRa_AT_MDOT.h
 * @author     Sara Damiano
 * @copyright  Copyright (c) 2024 Sara Damiano
 * @date       April 2024
 */

#ifndef SRC_LORA_AT_MDOT_H_
#define SRC_LORA_AT_MDOT_H_
// #pragma message("LoRa_AT:  LoRa_AT_mDOT")

// #define LORA_AT_DEBUG Serial

/// The new-line used by the LoRa module
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"  // NOTE:  define before including LoRa_AT_Modem!

#include "LoRa_AT_Modem.tpp"
#include "LoRa_AT_Radio.tpp"
#include "LoRa_AT_Time.tpp"
#include "LoRa_AT_Battery.tpp"
#include "LoRa_AT_Sleep.tpp"

class LoRa_AT_mDOT : public LoRa_AT_Modem<LoRa_AT_mDOT>,
                     public LoRa_AT_Time<LoRa_AT_mDOT>,
                     public LoRa_AT_Radio<LoRa_AT_mDOT>,
                     public LoRa_AT_Battery<LoRa_AT_mDOT>,
                     public LoRa_AT_Sleep<LoRa_AT_mDOT> {
  friend class LoRa_AT_Modem<LoRa_AT_mDOT>;
  friend class LoRa_AT_Time<LoRa_AT_mDOT>;
  friend class LoRa_AT_Radio<LoRa_AT_mDOT>;
  friend class LoRa_AT_Battery<LoRa_AT_mDOT>;
  friend class LoRa_AT_Sleep<LoRa_AT_mDOT>;

  /*
   * Inner Client
   */
 public:
  class LoRaStream_mDOT : public LoRaStream {
    friend class LoRa_AT_mDOT;

   public:
    LoRaStream_mDOT() {}

    explicit LoRaStream_mDOT(LoRa_AT_mDOT& modem) {
      init(&modem);
    }

    bool init(LoRa_AT_mDOT* modem) {
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
  explicit LoRa_AT_mDOT(Stream& stream) : stream(stream) {
    prev_dl_check        = 0;
    _requireConfirmation = false;
    _networkConnected    = false;
  }


  /*
   * Basic functions
   */
 protected:
  bool initImpl() {
    DBG(GF("### LoRa_AT Version:"), LORA_AT_VERSION);
    DBG(GF("### LoRa_AT Compiled Module:  LoRa_AT_mDOT"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef LORA_AT_DEBUG
    sendAT(GF("V1"));  // turn on verbose error codes
    waitResponse();
    sendAT(GF("+LOG=5"));  // set debug port logging level to debug
#else
    sendAT(GF("V0"));  // turn off verbose error codes
    waitResponse();
    sendAT(GF("+LOG=0"));  // turn off debug port logging
#endif
    waitResponse();
    sendAT(GF("+RXO=1"));  // set the receive output format to raw/unprocessed
                           // data (ie, ascii, not hex)
    waitResponse();
    return true;
  }

  String getDevEUIImpl() {
    return sendATGetString(GF("+DI?"));
  }

  bool factoryDefaultImpl() {
    bool resp = true;
    sendAT(GF("&F"));  // Factory default settings
    resp &= waitResponse() == 1;
    resp &= commitSettings();
    sendAT(GF("Z"));  // Reset (restart) the CPU
    resp &= waitResponse() == 1;
    delay(3000);  // mDOT takes about 3 seconds to reset
    return resp;
    // NOTE: Don't attempt to init or check AT after factory resetting!
    // Chances are it won't work because the baud rate or echo settings failed.
  }


  /*
   * Power functions
   */
 protected:
  bool restartImpl() {
    if (!testAT()) { return false; }
    sendAT(GF("&W"));  // Write any pending configuration changes
    waitResponse();
    sendAT(GF("Z"));  // Reset (restart) the CPU
    waitResponse();
    delay(3000);  // mDOT takes about 3 seconds to reset
    return init();
  }

  // NOTE: No way to power off or turn off the radio!

  /*
   * Sleep functions
   */
 public:
  /**
   * @brief Set a pin to wake the module from interrupt sleep.
   *
   * @note THIS IS THE PIN ON THE MODULE, NOT THE CONTROLLER!
   *
   * @param pin The pin number
   * @param pullupMode The pin's pullup mode (0=NOPULL, 1=PULLUP, 2=PULLDOWN)
   * @param trigger The wake trigger (ie, 0=ANY, 1=RISE, 2=FALL)
   * @return True if the module accepted the wake pin setting; false if there
   * was an error in setting the wake.
   */
  bool setWakePin(int8_t pin, int8_t pullupMode, int8_t trigger) {
    if (pullupMode == -1) {
      pullupMode = 0;
    }  // use no pull up if not specified
    if (trigger == -1) {
      trigger = 0;
    }  // use either rising or falling if not specified
    sendAT(GF("+WP="), pin, ',', pullupMode, ',',
           trigger);  // set make mode to wake on interrupt
    return waitResponse() == 1;
  }

  // Puts the end device in sleep mode. The end device wakes on interrupt or
  // interval based on AT+WM setting. Once awakened, use AT+SLEEP again to
  // return to sleep mode. Note: Deep sleep is not available for mDot devices.
  // Deep Sleep (ST Micro standby mode) is the lowest power mode. All RAM is
  // lost and peripherals are off. You can use backup registers to retain the
  // state over sleep. The dot library keeps the running state in the backup
  // registers to be reloaded automatically to maintain the session. Execution
  // begins at the start of the program as it would from power up. Sleep (ST
  // Micro stop mode) maintains RAM and keeps peripherals on. Execution resumes
  // from the call to sleep. If you need to wake the device at a specific 1 msec
  // timing, take the following values into account.
  //  - Waking from Deep Sleep takes 314-407 usec
  //  - Waking from Sleep takes 13-14 usec
  //  - RTC period is 30.5 use

 protected:
  // puts the device into sleep mode with a pin interrupt wake
  bool pinSleepImpl(int8_t pin, int8_t pullupMode, int8_t trigger) {
    bool success = true;
    sendAT(GF("+WM="), 1);  // set make mode to wake on interrupt
    success &= waitResponse() == 1;
    success &= setWakePin(pin, pullupMode, trigger);
    sendAT(GF("+SLEEP=0"));
    waitResponse(GF("OK"),
                 GF("ERROR"));  // doesn't always send the OK or the new line
    return success;
  }

  // puts the device into sleep mode with a UART interrupt wake
  bool uartSleepImpl() {
    bool success = true;
    sendAT(GF("+WM="), 1);  // set make mode to wake on interrupt
    success &= waitResponse() == 1;
    // Set the wake pin to be the UART
    // Wake pin "1" is data in (ie, UART communication)
    // 0 = No pullup
    // 0 = any (rising or falling edge)
    success &= setWakePin(1, 0, 0);
    sendAT(GF("+SLEEP=0"));
    waitResponse(GF("OK"),
                 GF("ERROR"));  // doesn't always send the OK or the new line
    return success;
  }

  bool sleepImpl(uint32_t sleepTimer) {
    bool success = true;
    sendAT(GF("+WM="), 0);  // set make mode to wake on interval
    success &= waitResponse() == 1;
    sendAT(GF("+WI="), sleepTimer / 1000L);  // set the wake interval
    success &= waitResponse() == 1;
    sendAT(GF("+SLEEP="), 0);
    waitResponse(GF("OK"),
                 GF("ERROR"));  // doesn't always send the OK or the new line
    return success;
  }

  bool enableAutoSleepImpl(bool enable = true) {
    sendAT(GF("+AS="), enable);
    return waitResponse() == 1;
  }


  /*
   * Generic network functions
   */
 protected:
  bool setPublicNetworkImpl(bool isPublic) {
    sendAT(GF("+PN="), isPublic);
    return waitResponse() == 1;
  }
  bool getPublicNetworkImpl() {
    sendAT(GF("+PN?"));
    bool resp = waitResponse(GF("1"), GF("0")) == 1;
    waitResponse();  // returns an "OK" after the number
    return resp;
  }

  bool setConfirmationRetriesImpl(int8_t numAckRetries) {
    sendAT(GF("+ACK="), numAckRetries);
    return waitResponse() == 1;
  }
  int8_t getConfirmationRetriesImpl() {
    sendAT(GF("+ACK?"));
    int8_t resp = stream.parseInt();
    waitResponse();  // returns an "OK" after the number
    return resp;
  }

  bool joinOTAAImpl(const char* appEui, const char* appKey, const char* devEui,
                    int8_t attempts, uint32_t initialBackoff, bool useHex) {
    sendAT(GF("+NJM=1"));  // Configure mDot for OTAA join mode (default)
    waitResponse();
    sendAT(GF("+NI="), !useHex, ',', appEui);  // set the app EUI (network id)
    waitResponse();
    sendAT(GF("+NK="), !useHex, ',', appKey);  // set the app key (network key)
    waitResponse();
    if (devEui != nullptr) {
      sendAT(GF("+DI="), devEui);  // set the device EUI
      waitResponse();
    }
    commitSettings();                // save configuration changes
    join(attempts, initialBackoff);  // join the network
    return isNetworkConnected();     // verify that we're connected
  }

  bool joinABPImpl(String devAddr, String nwkSKey, String appSKey,
                   int uplinkCounter, int downlinkCounter, int8_t attempts,
                   uint32_t initialBackoff) {
    sendAT(GF("+NJM=0"));  // Configure mDot for manual provisioning (ABP)
    waitResponse();
    sendAT(GF("+NA="), devAddr);  // set the network address (device address)
    waitResponse();
    sendAT(GF("+DSK="), appSKey);  // set the data session key (app session key)
    waitResponse();
    sendAT(GF("+NSK="), nwkSKey);  // set the network session key
    waitResponse();
    // set the uplink and downlink counters, if we need to
    if (uplinkCounter != 1) {
      sendAT(GF("+ULC="), uplinkCounter);  // set the uplink counter
      waitResponse();
    }
    if (downlinkCounter != 0) {
      sendAT(GF("+DLC="), downlinkCounter);  // set the downlink counter
      waitResponse();
    }
    commitSettings();  // save configuration changes
    return isNetworkConnected(attempts,
                              initialBackoff);  // verify that we're connected
  }

  bool isNetworkConnectedImpl(int8_t attempts, uint32_t initialBackoff) {
    // NOTE: The network join status depends on the link check count and the
    // link check threshold to look for previous ACK's or network link checks
    // and decide if the module is on the network or not. It may not be
    // accurate RIGHT NOW, so we'll do a network link check to get the status
    // NOW.

    // sendAT(GF("+NJS?"));
    // bool resp = waitResponse(GF("1"), GF("0")) == 1;
    // waitResponse();  // returns an "OK" after the number
    // _networkConnected = resp;
    // if (!resp) {

    int8_t tries_remaining = attempts;
    int8_t attempts_made   = 0;
    int    _link_margin    = 255;
    while (_link_margin == 255 && tries_remaining) {
      DBG(GF("Sending LinkCheckReq"), tries_remaining, GF("tries remaining"));
      sendAT(GF("+NLC"));
      tries_remaining--;
      attempts_made++;

      // If there is downlink data available, it will be returned before the
      // "OK" from the NLC command.  Unfortunately, there will be no warning
      // if the downlink data is going to be present or not.  If no data is
      // present, a extra blank line is sent before the "OK."

      String nlc_resp_str = "";  // to hold any downlink data
      nlc_resp_str.reserve(LORA_AT_RX_BUFFER);
      int8_t resp = waitResponse(10000L, nlc_resp_str, GFP(LORA_OK),
                                 GFP(LORA_ERROR), GF("Network Not Joined"));
      if (resp == 1) {
        // The first number in the response is the dBm level above the
        // demodulation floor (not to be confused with the noise floor). This
        // value is from the perspective of the signal sent from the end
        // device and received by the gateway. The second number is the count
        // of gateways reporting the link-check request to the network server.
        int firstComma   = nlc_resp_str.indexOf(',');
        int afterCommaCR = nlc_resp_str.indexOf('\r', firstComma + 1);
        int afterCommaLF = nlc_resp_str.indexOf('\n', afterCommaCR + 1);

        _link_margin = nlc_resp_str.substring(0, firstComma).toInt();

#ifdef LORA_AT_DEBUG
        int gatewayCount =
            nlc_resp_str.substring(firstComma + 1, afterCommaCR).toInt();
        DBG("## NLC link margin in dBm:", _link_margin,
            "gatewayCount:", gatewayCount);
#endif
        // the rest of the string should be the downlink data and the OK
        String downlinkData = "";
        downlinkData.reserve(LORA_AT_RX_BUFFER);
        downlinkData = nlc_resp_str.substring(afterCommaLF + 1);

        prev_dl_check = millis();          // mark that we checked for downlink
        readDownlinkToFifo(downlinkData);  // deal with the downlink data
      } else if (resp == 3) {              // if we got a not joined error, stop
        tries_remaining = 0;
        waitResponse();  // catch the "ERROR"
      } else {
        // delay before the next attempt
        uint32_t backoff = calculateBackoff(attempts_made, initialBackoff);
        delay(backoff);
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
    // Displays signal strength information for received packets: last, min,
    // max, avg
    sendAT(GF("+RSSI"));
    int8_t resp = stream.parseInt();  // only keep the last packet's RSSI (the
                                      // first number returned)
    waitResponse();                   // wait for ending ok
    return resp;
  }


  /*
   * LoRa Class and Band functions
   */
 protected:
  bool setClassImpl(_lora_class _class) {
    sendAT(GF("+DC="), (char)_class);
    return waitResponse() == 1;
  }
  _lora_class getClassImpl() {
    sendAT(GF("+DC?"));
    int8_t devClass = waitResponse(GF("A"), GF("B"), GF("C"));
    waitResponse();  // wait for ending ok
    return (_lora_class)(devClass - 1 + 'A');
  }

  bool setPortImpl(uint8_t _port) {
    sendAT(GF("+AP="), _port);
    return waitResponse() == 1;
  }
  uint8_t getPortImpl() {
    sendAT(GF("+AP?"));
    int8_t resp = stream.parseInt();
    waitResponse();  // wait for ending ok
    return resp;
  }

  // This is not configurable on the mDOT.  It's set at the factory based on the
  // module type.
  bool setBandImpl(const char*) {
    return false;
  }
  String getBandImpl() {
    return sendATGetString(GF("+FREQ?"));
  }

  bool setFrequencySubBandImpl(int8_t subBand) {
    sendAT(GF("+FSB="),
           subBand);  // Enter a value from 1-8 to configure the end device to
                      // use one set of eight channels out of 64 possible. This
                      // must match the gateway settings.
    return waitResponse() == 1;
  }
  int8_t getFrequencySubBandImpl() {
    sendAT(GF("+FSB?"));
    int8_t resp = stream.parseInt();
    waitResponse();  // wait for ending ok
    return resp;
  }

  String getChannelMaskImpl() {
    // NOTE: This will return **20** characters.
    // The first 2 will always be 0.
    String resp = sendATGetString(GF("+CHM?"));
    resp.replace("Channel Mask: ", "");
    resp.trim();
    return resp;
  }
  bool setChannelMaskImpl(const char* newMask) {
    // The mDOT wants the channel mask to be sent as an offset from the first
    // byte and then the next 4 bytes of the mask.
    // For masks longer than 4 bytes (ie AU915, CN470, US915, US915_HYBRID), the
    // command is sent multiple times.

    size_t maskLen = strlen(newMask);
    char*  maskPtr = const_cast<char*>(
        newMask);  // Pointer to where in the buffer we're up to
    uint8_t maskOffset = 4;
    size_t  bytesSent  = 0;

    // The mask must be 16 bits (4 characters) or 72 bits (18 characters)
    // Because the mask is sent in chunks of 4 characters, the input mask must
    // actually be 20 characters for a 72 bit mask. The first two characters
    // will always be zero to make up for this.
    // For US915 with all channels enabled, the mask is '00FFFFFFFFFFFFFFFFFF'
    if (maskLen != 4 && maskLen != 18 && maskLen != 20) {
      DBG("### Incorrect mask length:", maskLen);
      return false;
    }
    while (bytesSent < maskLen) {
      uint8_t sendLength = 4;  // Send 4 bytes at a time

      // start the send command
      stream.write("AT+CHM=");
      stream.print(maskOffset);
      stream.write(',');
      // zero pad the mask if someone only gave 18 characters
      if (maskLen == 18 && maskOffset == 4) {
        stream.print("00");
        sendLength = 2;
      }
      // write out the 4 bytes of the mask
      stream.write(reinterpret_cast<const uint8_t*>(maskPtr), sendLength);
      // finish with a new line
      stream.println();
      stream.flush();

      if (waitResponse() == 1) {
        bytesSent += sendLength;  // bump up number of bytes sent
        maskPtr += sendLength;    // bump up the pointer
        maskOffset--;
      } else {
        break;
      }
    }
    return bytesSent == maskLen;
  }


  /*
   * LoRa Data Rate and Duty Cycle functions
   */
 protected:
  // NOTE: There is no way to directly enable duty cycle limitations. If a max
  // duty cycle is set, it's enforced. To not enforce it, set the max to 0.
  bool enableDutyCycleImpl(bool dutyCycle) {
    if (!dutyCycle) { return setMaxDutyCycle(0); }
    return false;
  }
  bool isDutyCycleEnabledImpl() {
    return getMaxDutyCycleImpl() > 0;
  }

  bool setMaxDutyCycleImpl(int8_t maxDutyCycle) {
    sendAT(GF("+DUTY="), maxDutyCycle);
    return waitResponse() == 1;
  }
  int8_t getMaxDutyCycleImpl() {
    // This gives a detailed response, we're only going to return the int for
    // the max duty value.
    // AT+DUTY?
    // Max 1
    // Index Freq Low Freq High Off Ratio
    // 0 915000000 928000000 100
    // OK
    sendAT(GF("+DUTY?"));
    streamFind(' ');
    int8_t resp = stream.parseInt();
    waitResponse();  // wait for ending ok
    return resp;
  }

  bool setDataRateImpl(uint8_t dataRate) {
    sendAT(GF("+TXDR="), dataRate);
    return waitResponse() == 1;
  }
  int8_t getDataRateImpl() {
    sendAT(GF("+TXDR?"));
    // returns a longer response like "DR0 - SF12BW125"
    // We're only going to keep the DR number
    streamFind('R');
    int8_t resp = stream.parseInt();
    waitResponse();  // wait for ending ok
    return resp;
  }

  bool setAdaptiveDataRateImpl(bool useADR) {
    sendAT(GF("+ADR="), useADR);
    return waitResponse() == 1;
  }
  bool getAdaptiveDataRateImpl() {
    sendAT(GF("+ADR?"));
    bool resp = waitResponse(GF("1"), GF("0")) == 1;
    waitResponse();  // returns an "OK" after the number
    return resp;
  }


  /*
   * LoRa ABP Session Properties
   */
 protected:
  // aka network address
  String getDevAddrImpl() {
    return sendATGetString(GF("+NA?"));
  }

  // network session key
  String getNwkSKeyImpl() {
    return sendATGetString(GF("+NSK?"));
  }

  // aka data session key
  String getAppSKeyImpl() {
    return sendATGetString(GF("+DSK?"));
  }


  /*
   * LoRa OTAA Session Properties
   */
 protected:
  // aka network id
  String getAppEUIImpl() {
    return sendATGetString(GF("+NI?"));
  }
  // aka network key
  String getAppKeyImpl() {
    return sendATGetString(GF("+NK?"));
  }


  /*
   * Time functions
   */
 protected:
  String getDateTimeStringImpl(LoRa_AT_DateTimeFormat format)
      LORA_AT_ATTR_NOT_AVAILABLE;

  bool getDateTimePartsImpl(int* year, int* month, int* day, int* hour,
                            int* minute, int* second,
                            float* timezone) LORA_AT_ATTR_NOT_AVAILABLE;

  // NOTE: This module only returns epoch time! If you need to convert from
  // Epoch time to a human readable time, use a date library.
  uint32_t getDateTimeEpochImpl(LoRa_AT_EpochStart epoch = UNIX) {
    uint32_t gps_time        = 0;
    int8_t   tries_remaining = 5;
    while (gps_time == 0 && tries_remaining) {
      sendAT(GF("+GPSTIME"));  // Use this to retrieve GPC synchronized time in
                               // milliseconds.
      tries_remaining--;

      // NOTE: We can't use parseInt or sendATGetString here because the return
      // is slow!
      String gps_time_str = "";
      gps_time_str.reserve(16);
      // Wait for final OK
      bool got_ok = waitResponse(15000L, gps_time_str) == 1;
      if (got_ok) {
        gps_time_str.replace(AT_NL "OK" AT_NL, "");
        gps_time_str.trim();

        // the epoch time is returned in MILLISECONDS which is too large for a
        // 32-bit unsigned into.  I'm chopping off the milliseconds and only
        // keeping the seconds.
        int endSeconds = gps_time_str.length() - 3;
        // subset text before the first carriage return, if it's there
        int firstCR = gps_time_str.indexOf('\r');
        if (firstCR > 0) { endSeconds = firstCR - 3; }
        gps_time_str = gps_time_str.substring(0, endSeconds);
        gps_time_str.trim();
        gps_time = gps_time_str.toInt();
      } else {
        // delay before the next attempt
        DBG(GF("Delay 10s before next time request attempt"));
        delay(10000L);
        // dump out anything, in case the time came in after the ok
        loraStream->dumpModemBuffer(10000L);
      }
    }

    // The epoch date/time returned by the mDOT uses the GPS epoch - with
    // accounting for leap seconds!
    if (gps_time != 0) {
      switch (epoch) {
        case UNIX: return GPSTimeConversion::gps2unix(gps_time);
        case Y2K: return GPSTimeConversion::gps2unix(gps_time) + 946684800;
        case GPS: return gps_time;
      }
    }

    return gps_time;
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
  // NOTE: The mDOT can only return the battery as a number between 0 and 255,
  // not as a voltage. It also doesn't return the charge state.

  int8_t getBattPercentImpl() {
    sendAT(GF("+BAT"));
    // Read battery charge level
    // returns a number between 0 and 255
    float resp = stream.parseFloat();
    // Wait for final OK
    waitResponse();
    return (int8_t)((resp / 255.) * 100.);
  }

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    sendAT(GF("+BAT"));
    // Read battery charge level
    // returns a number between 0 and 255
    float resp = stream.parseFloat();
    // Wait for final OK
    bool wasOk = waitResponse() == 1;
    percent    = (int8_t)((resp / 255.) * 100.);
    // Charge state and millivolts aren't returned by this module
    chargeState = -1;
    milliVolts  = -9999;
    return wasOk;
  }


  /*
   * Temperature functions
   */
 protected:
  // No functions of this type


  /*
   * Stream related functions
   */
 protected:
  int16_t modemSend(const uint8_t* buff, size_t len) {
    // Pointer to where in the buffer we're up to
    // A const cast is need to cast-away the constant-ness of the buffer (ie,
    // modify it).
    uint8_t* txPtr     = const_cast<uint8_t*>(buff);
    size_t   bytesSent = 0;

    // NOTE: There's no way to require or not require confirmation for an
    // individual message!
    int8_t prev_ack_retries    = getConfirmationRetries();
    bool   changed_ack_retries = false;
    // If a number of retries isn't set, and we want confirmation, set the
    // retries to 3. On the other hand, if the number of retries isn't 0 but we
    // don't want confirmation, reset the retry number to 0.
    if (_requireConfirmation && prev_ack_retries == 0) {
      setConfirmationRetries(3);
      DBG(GF("Set confirmation retries to 3 because ACK is on but no retries "
             "were set"));
      changed_ack_retries = true;
    } else if (!_requireConfirmation && prev_ack_retries != 0) {
      setConfirmationRetries(0);
      DBG(GF("Set confirmation retries to 0 to disable ACK"));
      changed_ack_retries = true;
    }

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
          sendAT(GF("+TXS?"));
          uplinkAvailable = stream.parseInt();
          DBG(uplinkAvailable, GF("bytes available for uplink."),
              !uplinkAvailable ? GF("Flush the MAC buffer with empty message.")
                               : GF(" "));
          waitResponse();  // get the OK after the byte response
        }

        // Ensure the program doesn't read past the allocated memory
        sendLength = uplinkAvailable;
        if (txPtr + uplinkAvailable > const_cast<uint8_t*>(buff) + len) {
          sendLength = const_cast<uint8_t*>(buff) + len - txPtr;
        }
        // if there's no space available to send data, the queue is full of MAC
        // commands and we need to send empty messages to flush them out
        if (uplinkAvailable == 0 || len == 0) {
          DBG(GF("Sending empty request"));
          sendAT(GF("+SEND"));
        } else {
#ifdef LORA_AT_SEND_HEX
          // start the send command
          stream.write("AT+SENDB=");
          // write everything as hex characters
          writeHex(txPtr, sendLength);
#else
          // start the send command
          stream.write("AT+SEND=");
          // write out the number of bytes that are available for this uplink
          stream.write(reinterpret_cast<const uint8_t*>(txPtr), sendLength);
#endif
          // finish with a new line
          stream.println();
          stream.flush();
        }
        // Notes from datasheet:
        // +SEND commands response is significantly impacted by the AT+ACK
        // setting.
        // With +ACK=[1-15], there is a random 1-3s delay before a retransmit if
        // an ACK has not been received.  The theoretical max time with 8
        // re-transmits requiring acknowledgements is 42.2 seconds.
        // With +ACK=0 and no response from the network server, the time has
        // been observed at about 2.5 seconds.
        uint32_t sendTimeout;
        if (_requireConfirmation) {
          sendTimeout = DEFAULT_ACKMESSAGE_TIMEOUT;
        } else {
          sendTimeout = DEFAULT_MESSAGE_TIMEOUT;
        }

        // If there is downlink data available, it will be returned before the
        // "OK" from the send command.  Unfortunately, there will be no warning
        // if the downlink data is going to be present or not.  If no data is
        // present, a extra blank line is sent before the "OK."

        String downlinkData = "";  // to hold any downlink data
        downlinkData.reserve(LORA_AT_RX_BUFFER);
        if (waitResponse(sendTimeout, downlinkData) == 1) {
          bytesSent += sendLength;   // bump up number of bytes sent
          txPtr += sendLength;       // bump up the pointer
          prev_dl_check = millis();  // mark that we checked for downlink
          send_success  = true;
          readDownlinkToFifo(downlinkData);  // deal with the downlink data
        }
        send_attempts++;
      }
      // if we completely failed after 5 attempts, bail from the whole thing
      if (!send_success) { break; }
    } while (bytesSent < len && _networkConnected);

    // undo changes to ack retries
    if (changed_ack_retries) {
      setConfirmationRetries(prev_ack_retries);
      DBG(GF("Re-set confirmation retry number to"), prev_ack_retries);
    }
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
    if (data.endsWith(GF("Network Not Joined" AT_NL)) ||
        data.endsWith(GF("Failed to join network" AT_NL))) {
      _networkConnected = false;
      DBG("### Network disconnected, please re-join!");
      return true;
    }
    return false;
  }


  bool commitSettings() {
    sendAT(GF("&W"));  // Write configurations
    return waitResponse() == 1;
  }

  uint32_t getNextTransmit() {
    sendAT(GF("+TXN?"));
    uint32_t resp = stream.parseInt();
    waitResponse();  // returns an "OK" after the number
    return resp;
  }

  bool join(uint8_t attempts, uint32_t initialBackoff, bool force = false) {
    // try multiple times to join
    bool    success            = false;
    uint8_t attempts_remaining = attempts;
    int8_t  attempts_made      = 0;
    while (!success && attempts_remaining) {
#ifdef LORA_AT_DEBUG
      uint32_t start = millis();
#endif
      sendAT(force ? GF("+JOIN=1") : GF("+JOIN"));
      attempts_remaining--;
      attempts_made++;
      // The maximum settable join delay for non-default connections is 15s, so
      // hopefully this will return before that.
      int8_t join_resp = waitResponse(
          15000L, GF("Successfully joined network" AT_NL),
          GF("Failed to join network" AT_NL), GF("Join backoff" AT_NL));
      waitResponse();        // returns an ok or error after the join message
      if (join_resp == 1) {  // if we succeeded
        success           = true;
        _networkConnected = true;
        DBG(GF("Successfully joined network after"), millis() - start,
            GF("ms"));
      } else {
        DBG(GF("Join attempted failed after"), millis() - start, GF("ms with"),
            attempts_remaining, GF("attempts remaining"));
        // check how long we need to wait for a free channel before next attempt
        uint32_t transmit_wait = getNextTransmit();
        // calculate backoff
        uint32_t backoff = calculateBackoff(attempts_made, initialBackoff);
        // Delay at least until the next channel is free, or to the calculated
        // backoff, whichever is larger.
        DBG(GF("Waiting"), max(transmit_wait + 100L, backoff),
            GF("ms for a free channel before next join attempt."));
        delay(max(transmit_wait + static_cast<uint32_t>(100), backoff));
      }
    }
    return success;
  }

  size_t readDownlinkToFifo(String& downlink) {
    // String dl_marked = downlink;
    // dl_marked.replace("\r", "←");
    // dl_marked.replace("\n", "↓");
    // DBG("## downlinkData: <<", dl_marked, ">>");
    // strip the initial "\r\r\n\r\r\n" pattern out
    downlink.replace("\r\r\n\r\r\n", "");
    // strip out the OK, sometimes with an extra trailing new line
    downlink.replace(AT_NL AT_NL "OK" AT_NL, "");
    downlink.replace(AT_NL "OK" AT_NL, "");
    // dl_marked = downlink;
    // dl_marked.replace("\r", "←");
    // dl_marked.replace("\n", "↓");
    // DBG("## downlinkData cleaned: <<", dl_marked, ">>");
    // if all we have is a various types of returns, it's nothing
    if (downlink == "\r\r\n\r\r\n" || downlink == "\r\n" || downlink == "\r" ||
        downlink == "\n") {
      return 0;
    }

    // if there's data in the downlink, we're connected
    _networkConnected = true;

    // deal with the downlink data
    int downlinkedBytes = downlink.length();

    // if we got data, move it into the FiFo
    if (downlinkedBytes > 0) {
      DBG("## Got", downlinkedBytes, "bytes of downlink data");
      int putBuffLen = downlinkedBytes;
      if (downlinkedBytes > loraStream->rx.free()) {
        DBG("### Buffer overflow: ", downlinkedBytes, "->",
            loraStream->rx.free());
        // reset amount to put into the buffer to the free space available
        putBuffLen = loraStream->rx.free();
      }
      loraStream->rx.put(
          reinterpret_cast<uint8_t*>(const_cast<char*>(downlink.c_str())),
          putBuffLen, false);
      loraStream->sock_available = putBuffLen;
      return putBuffLen;
    }
    return 0;
  }

  String sendATGetString(GsmConstStr cmd) {
    String resp;
    sendAT(cmd);
    if (waitResponse(1000L, resp) != 1) { return "UNKNOWN"; }
    resp.replace(AT_NL "OK" AT_NL, "");
    resp.trim();

    // subset text before the first carriage return
    int firstCR = resp.indexOf('\r');
    return resp.substring(0, firstCR);
  }


 public:
  Stream& stream;

 protected:
  LoRaStream_mDOT* loraStream;
};

#endif  // SRC_LORA_AT_MDOT_H_
