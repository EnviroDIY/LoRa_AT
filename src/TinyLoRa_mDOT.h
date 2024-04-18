/**
 * @file       TinyLoRa_MDOT.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYLORA_MDOT_H_
#define SRC_TINYLORA_MDOT_H_
// #pragma message("TinyGSM:  TinyLoRa_MDOT")

// #define TINY_LORA_DEBUG Serial
// #define TINY_LORA_USE_HEX

/// The new-line used by the LoRa module
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"  // NOTE:  define before including TinyLoRaModem!

#include "TinyLoRaModem.tpp"
#include "TinyLoRaRadio.tpp"
#include "TinyLoRaTime.tpp"
#include "TinyLoRaBattery.tpp"

class TinyLoRa_mDOT : public TinyLoRaModem<TinyLoRa_mDOT>,
                      public TinyLoRaTime<TinyLoRa_mDOT>,
                      public TinyLoRaRadio<TinyLoRa_mDOT>,
                      public TinyLoRaBattery<TinyLoRa_mDOT> {
  friend class TinyLoRaModem<TinyLoRa_mDOT>;
  friend class TinyLoRaTime<TinyLoRa_mDOT>;
  friend class TinyLoRaRadio<TinyLoRa_mDOT>;
  friend class TinyLoRaBattery<TinyLoRa_mDOT>;

  /*
   * Inner Client
   */
 public:
  class LoRaStream_mDOT : public LoRaStream {
    friend class TinyLoRa_mDOT;

   public:
    LoRaStream_mDOT() {}

    explicit LoRaStream_mDOT(TinyLoRa_mDOT& modem) {
      init(&modem);
    }

    bool init(TinyLoRa_mDOT* modem) {
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
  explicit TinyLoRa_mDOT(Stream& stream) : stream(stream) {}

  /*
   * Basic functions
   */
 protected:
  bool initImpl() {
    DBG(GF("### TinyLoRa Version:"), TINY_LORA_VERSION);
    DBG(GF("### TinyLoRa Compiled Module:  TinyLoRa_mDOT"));
    prev_dl_check = 0;

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_LORA_DEBUG
    sendAT(GF("V1"));  // turn on verbose error codes
#else
    sendAT(GF("V0"));      // turn off verbose error codes
#endif
    waitResponse();
#ifdef TINY_LORA_STORE_HEX  // set the receive output format
    sendAT(GF("+RXO=0"));   // Hexadecimal (Default)
#else
    sendAT(GF("+RXO=1"));  // Raw/Unprocessed data (non-hex)
#endif
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
    resp &= committSettings();
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
  bool sleepEnableImpl(bool deepSleep = true) {
    sendAT(GF("+SLEEP="), deepSleep);
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

  bool setConfirmationRetriesImpl(bool isAckRequired) {
    sendAT(GF("+ACK="), isAckRequired);
    return waitResponse() == 1;
  }
  bool getConfirmationRetriesImpl() {
    sendAT(GF("+ACK?"));
    bool resp = waitResponse(GF("1"), GF("0")) == 1;
    waitResponse();  // returns an "OK" after the number
    return resp;
  }

  bool joinOTAAImpl(const char* appEui, const char* appKey, const char* devEui,
                    bool useHex, uint32_t timeout) {
    sendAT(GF("+NJM=1"));  // Configure mDot for OTAA join mode (default)
    waitResponse();
    sendAT(GF("+NI="), !useHex, ',', appEui);  // set the app EUI (network id)
    waitResponse();
    sendAT(GF("+NK="), !useHex, ',', appKey);  // set the app key (network key)
    waitResponse();
    if (devEui != NULL) {
      sendAT(GF("+DI="), devEui);  // set the device EUI
      waitResponse();
    }
    committSettings();            // save configuration changes
    join(timeout);                // join the network
    return isNetworkConnected();  // verify that we're connected
  }

  bool joinABPImpl(const char* devAddr, const char* nwkSKey,
                   const char* appSKey,
                   uint32_t    timeout = DEFAULT_JOIN_TIMEOUT) {
    sendAT(GF("+NJM=1"));  // Configure mDot for manual provisioning (ABP)
    waitResponse();
    sendAT(GF("+NA="), devAddr);  // set the network address (device address)
    waitResponse();
    sendAT(GF("+DSK="), appSKey);  // set the data session key (app session key)
    waitResponse();
    sendAT(GF("+NSK="), nwkSKey);  // set the network session key
    waitResponse();
    committSettings();            // save configuration changes
    join(timeout);                // join the network
    return isNetworkConnected();  // verify that we're connected
  }

  bool isNetworkConnectedImpl() {
    sendAT(GF("+NJS?"));
    bool resp = waitResponse(GF("1"), GF("0")) == 1;
    waitResponse();  // returns an "OK" after the number
    return resp;
  }

  int8_t getSignalQualityImpl() {
    // Displays signal strength information for received packets: last, min,
    // max, avg
    sendAT(GF("+RSSI"));
    int8_t res = stream.parseInt();  // only keep the last packet's RSSI (the
                                     // first number returned)
    waitResponse();                  // wait for ending ok
    return res;
  }


  /*
   * LoRa Class and Band functions
   */

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

  // This is not configurable on the mDOT.  It's set at the factory based on the
  // module type.
  bool setBandImpl(const char* band) {
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
    int8_t res = stream.parseInt();
    waitResponse();  // wait for ending ok
    return res;
  }

  String getChannelMaskImpl() {
    String res = sendATGetString(GF("+CHM?"));
    res.replace("Channel Mask: ", "");
    res.trim();
    return res;
  }
  bool setChannelMaskImpl(const char* newMask) {
    // The mDOT wants the channel mask to be sent as an offset from the first
    // byte and then the next 4 bytes of the mask.
    // For masks longer than 4 bytes (ie AU915, CN470, US915, US915_HYBRID), the
    // command is sent multiple times.

    size_t maskLen = strlen(newMask);
    char*  maskPtr =
        (char*)newMask;  // Pointer to where in the buffer we're up to
    uint8_t maskOffset = 0;
    size_t  bytesSent  = 0;

    // The mask must be 16 bits (4 characters) or 72 bits (18 characters)
    // Because the mask is sent in chunks of 4 characters, the input mask must
    // actually be 20 characters for a 72 bit mask. The first two characters
    // will always be zero to make up for this.
    // For US915 with all channels enabled, the mask is '00FFFFFFFFFFFFFFFFFF'
    if (maskLen != 4 && maskLen != 20) {
      DBG("### Incorrect mask length");
      return false;
    }


    while (bytesSent < maskLen) {
      uint8_t sendLength = 4;  // Send 4 bytes at a time

      // start the send command
      stream.write("AT+CHM=");
      stream.print(maskOffset);
      stream.write(',');
      // write out the number of bytes that are available for this uplink
      stream.write(reinterpret_cast<const uint8_t*>(maskPtr), sendLength);
      // finish with a new line
      stream.println();
      stream.flush();

      if (waitResponse() == 1) {
        bytesSent += sendLength;  // bump up number of bytes sent
        maskPtr += sendLength;    // bump up the pointer
      } else {
        break;
      }
    }
    return bytesSent == maskLen;
  }


  /*
   * LoRa Data Rate and Duty Cycle functions
   */
  bool setDutyCycleImpl(int8_t dutyCycle) {
    sendAT(GF("+DUTY="), dutyCycle);
    return waitResponse() == 1;
  }
  int8_t getDutyCycleImpl() {
    // This gives a detailed response, we're only going to return the int for
    // the max duty value.
    // AT+DUTY?
    // Max 1
    // Index Freq Low Freq High Off Ratio
    // 0 915000000 928000000 100
    // OK
    sendAT(GF("+DUTY?"));
    streamSkipUntil(' ');
    int8_t res = stream.parseInt();
    waitResponse();  // wait for ending ok
    return res;
  }
  bool setDataRateImpl(uint8_t dataRate) {
    sendAT(GF("+TXDR="), dataRate);
    return waitResponse() == 1;
  }
  int8_t getDataRateImpl() {
    sendAT(GF("+TXDR?"));
    // returns a longer response like "DR0 - SF12BW125"
    // We're only going to keep the DR number
    streamSkipUntil('R');
    int8_t res = streamGetIntBefore(' ');
    waitResponse();  // wait for ending ok
    return res;
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
  // aka network address
  String getDevAddrImpl() {
    return sendATGetString(GF("+DI?"));
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
  // NOTE: This module only returns epoch time!
  uint32_t getDateTimeEpochImpl(bool use2000Epoch = false) {
    sendAT(GF("+GPSTIME"));  // Use this to retrieve GPC synchronized time in
                             // milliseconds.

    uint32_t res;
    char     buf[10];
    size_t   bytesRead = stream.readBytesUntil('\r', buf,
                                               static_cast<size_t>(10));
    // if we read 10 or more bytes, it's an overflow
    if (bytesRead && bytesRead < 10) {
      buf[bytesRead] = '\0';
      uint32_t res   = atol(buf);

      if (use2000Epoch) { res += EPOCH_TIME_OFF; }
      return res;
    }

    res = 0;

    // Wait for final OK
    waitResponse();
    return res;
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
    float res = stream.parseFloat();
    // Wait for final OK
    waitResponse();
    return (int8_t)((res / 255.) * 100.);
  }

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    sendAT(GF("+BAT"));
    // Read battery charge level
    // returns a number between 0 and 255
    float res = stream.parseFloat();
    // Wait for final OK
    bool wasOk = waitResponse() == 1;
    percent    = (int8_t)((res / 255.) * 100.);
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
  int16_t modemSend(const void* buff, size_t len) {
    char*  txPtr = (char*)buff;  // Pointer to where in the buffer we're up to
    size_t bytesSent = 0;

    while (bytesSent < len && _networkConnected) {
      uint8_t sendLength =
          0;  // Number of bytes to send from buffer in this command

      // check how many bytes are available for the next uplink
      uint8_t uplinkAvailable = 0;
      sendAT(GF("+TXS?"));
      uplinkAvailable = stream.parseInt();
      DBG(uplinkAvailable, "bytes available for uplink");
      waitResponse();  // get the OK after the byte response

      // Ensure the program doesn't read past the allocated memory
      sendLength = uplinkAvailable;
      if (txPtr + uplinkAvailable > (char*)buff + len) {
        sendLength = (char*)buff + len - txPtr;
      }

      // start the send command
      stream.write("AT+SEND=");
      // write out the number of bytes that are available for this uplink
      stream.write(reinterpret_cast<const uint8_t*>(txPtr), sendLength);
      // finish with a new line
      stream.println();
      stream.flush();
      // Notes from datasheet:
      // +SEND commands response is significantly impacted by the AT+ACK
      // setting.
      // With +ACK=[1-15], there is a random 1-3s delay before a retransmit if
      // an ACK has not been received.  The theoretical max time with 8
      // re-transmitts requiring acknowledgements is 42.2 seconds.
      // With +ACK=0 and no response from the network server, the time has
      // been observed at about 2.5 seconds.

      // If there is downlink data available, it will be returned before the
      // "OK" from the send command.  Unfortunately, there will be no warning
      // if the downlink data is going to be present or not.  If no data is
      // present, a extra blank line is sent before the "OK."

      // deal with the downlink data
      String downlinkData = "";  // to hold any downlink data
      downlinkData.reserve(TINY_LORA_RX_BUFFER);
      if (waitResponse(30000L, downlinkData) == 1) {
        bytesSent += sendLength;   // bump up number of bytes sent
        txPtr += sendLength;       // bump up the pointer
        prev_dl_check = millis();  // mark that we checked for downlink

        // deal with the downlink data
        readDownlinkToFifo(downlinkData);
      }
    }
    return bytesSent == len;
  }

  /** This doesn't work consistentlly
  size_t modemRead() {
    size_t totalBytesRead;
    int    downlinkedBytes = 1;
    if (loraStream->rx.free() == 0) {
      DBG("Buffer is full! Not requesting downlink data!");
    }
    if (!_networkConnected) {
      DBG("Not joined to network! Can't request downlink data!");
    }
    while (downlinkedBytes > 0 && loraStream->rx.free() > 0 &&
           _networkConnected) {
      // Check for new downlink data using the Network Link Check command
      // Do NOT use the RECV command, because it will always return the last
      // data received, even if that data is has already been read out. The
      // NLC command will only return new downlink data. When the network link
      // check is performed, an empty packet is sent to the gateway and the
      // network server may include a downlink payload with the command
      // answer. If a payload is included it displays on the next line.
      sendAT(GF("+NLC"));

      // If there is downlink data available, it will be returned before the
      // "OK" from the NLC command.  Unfortunately, there will be no warning
      // if the downlink data is going to be present or not.  If no data is
      // present, a extra blank line is sent before the "OK."

      // deal with the downlink data
      String nlcResponse = "";  // to hold any downlink data
      nlcResponse.reserve(TINY_LORA_RX_BUFFER);
      if (waitResponse(3000L, nlcResponse) == 1) {
        // The first number in the response is the dBm level above the
        // demodulation floor (not to be confused with the noise floor). This
        // value is from the perspective of the signal sent from the end
        // device and received by the gateway. The second number is the count
        // of gateways reporting the link-check request to the network server.
        int firstComma   = nlcResponse.indexOf(',');
        int afterCommaCR = nlcResponse.indexOf('\r', firstComma + 1);
        int afterCommaLF = nlcResponse.indexOf('\n', afterCommaCR + 1);

        // int dbm = nlcResponse.substring(0, firstComma).toInt();
        // int gatewayCount =
        //     nlcResponse.substring(firstComma + 1, afterCommaCR).toInt();
        // DBG("## NLC dbm:", dbm, "gatewayCount:", gatewayCount);
        // the rest of the string should be the downlink data and the OK
        String downlinkData = nlcResponse.substring(afterCommaLF);

        // deal with the downlink data
        downlinkedBytes = readDownlinkToFifo(downlinkData);
        totalBytesRead += downlinkedBytes;
      }
    }
    return totalBytesRead;
  }**/

  size_t modemRead() {
    size_t totalBytesRead;
    int    downlinkedBytes = 1;
    if (loraStream->rx.free() == 0) {
      DBG("Buffer is full! Not requesting downlink data!");
    }
    if (!_networkConnected) {
      DBG("Not joined to network! Can't request downlink data!");
    }
    while (downlinkedBytes > 0 && loraStream->rx.free() > 0 &&
           _networkConnected) {
      // Check for new downlink data using the by issuing and empty send
      // command.
      sendAT(GF("+SEND"));

      // If there is downlink data available, it will be returned before the
      // "OK" from the SEND command.  Unfortunately, there will be no warning
      // if the downlink data is going to be present or not.  If no data is
      // present, a extra blank line is sent before the "OK."

      // deal with the downlink data
      String downlinkData = "";  // to hold any downlink data
      downlinkData.reserve(TINY_LORA_RX_BUFFER);
      if (waitResponse(30000L, downlinkData) == 1) {
        // deal with the downlink data
        downlinkedBytes = readDownlinkToFifo(downlinkData);
        totalBytesRead += downlinkedBytes;
      }
    }
    return totalBytesRead;
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
      loraStream->rx.put((uint8_t*)(downlink.c_str()), putBuffLen, false);
      loraStream->sock_available = putBuffLen;
      return putBuffLen;
    }
    return 0;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("Network Not Joined" AT_NL)) ||
        data.endsWith(GF("Failed to join network" AT_NL))) {
      _networkConnected = false;
      DBG("### Network disconnected, please re-join!");
      return true;
    }
    return false;
  }


  bool committSettings() {
    sendAT(GF("&W"));  // Write configurations
    return waitResponse() == 1;
  }

  bool join(uint32_t timeout) {
    sendAT(GF("+JOIN"));
    if (waitResponse(timeout, GF("Successfully joined network")) != 1) {
      return false;
    }
    waitResponse();  // returns an ok after the success message
    return true;
  }


  String sendATGetString(GsmConstStr cmd) {
    String res;
    sendAT(cmd);
    if (waitResponse(1000L, res) != 1) { return "UNKNOWN"; }
    res.replace(AT_NL "OK" AT_NL, "");
    res.trim();

    // subset text before the first carriage return
    int firstCR = res.indexOf('\r');
    return res.substring(0, firstCR);
  }


 public:
  Stream& stream;

 protected:
  LoRaStream_mDOT* loraStream;
};

#endif  // SRC_TINYLORA_MDOT_H_
