/**
 * @file       TinyLoRaClientMDOT.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYLORACLIENTMDOT_H_
#define SRC_TINYLORACLIENTMDOT_H_
// #pragma message("TinyGSM:  TinyLoRaClientMDOT")

// #define TINY_LORA_DEBUG Serial
// #define TINY_LORA_USE_HEX

/**
 * @brief The number of multi-cast sessions possible
 *
 * @note The multicast session number is indexed from 1-8 for the 8 possible
 * multicasts on the mDOT, not 0-7 like C++ would normally use.
 */
#define TINY_LORA_MUX_COUNT 8
/// The new-line used by the LoRa module
#define GSM_NL "\r\n"  // NOTE:  define before including TinyLoRaModem!

#include "TinyLoRaModem.tpp"
#include "TinyLoRaStream.tpp"
#include "TinyLoRaTime.tpp"
#include "TinyLoRaTemperature.tpp"
#include "TinyLoRaBattery.tpp"

class TinyLoRa_mDOT : public TinyLoRaModem<TinyLoRa_mDOT>,
                      public TinyLoRaTime<TinyLoRa_mDOT>,
                      public TinyLoRaStream<TinyLoRa_mDOT, TINY_LORA_MUX_COUNT>,
                      public TinyLoRaTemperature<TinyLoRa_mDOT>,
                      public TinyLoRaBattery<TinyLoRa_mDOT> {
  friend class TinyLoRaModem<TinyLoRa_mDOT>;
  friend class TinyLoRaTime<TinyLoRa_mDOT>;
  friend class TinyLoRaStream<TinyLoRa_mDOT, TINY_LORA_MUX_COUNT>;
  friend class TinyLoRaTemperature<TinyLoRa_mDOT>;
  friend class TinyLoRaBattery<TinyLoRa_mDOT>;

  /*
   * Inner Client
   */
 public:
  class LoRaStream_mDOT : public LoRaStream {
    friend class TinyLoRa_mDOT;

   public:
    LoRaStream_mDOT() {}

    explicit LoRaStream_mDOT(TinyLoRa_mDOT& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyLoRa_mDOT* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      got_data       = false;

      if (mux < TINY_LORA_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_LORA_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

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
  explicit TinyLoRa_mDOT(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl() {
    DBG(GF("### TinyLoRa Version:"), TINY_LORA_VERSION);
    DBG(GF("### TinyLoRa Compiled Module:  TinyLoRa_mDOT"));

    if (!testAT()) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_LORA_DEBUG
    sendAT(GF("V1"));  // turn on verbose error codes
#else
    sendAT(GF("V0"));  // turn off error codes
#endif
    waitResponse();
  }

  String getDevEUIImpl() {
    String eui;
    sendAT(GF("+DI?"));
    if (waitResponse(1000L, eui) != 1) { return "UNKNOWN"; }
    return eui;
  }

  bool factoryDefaultImpl() {
    bool resp = true;
    sendAT(GF("&F"));  // Factory default settings
    resp &= waitResponse() == 1;
    sendAT(GF("&W"));  // Write configurations
    resp &= waitResponse() == 1;
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
    waitResponse() == 1;
    delay(3000);  // mDOT takes about 3 seconds to reset
    return init();
  }

  // NOTE: No way to power off or turn off the radio!
  bool poweroff() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool radioOffImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

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
  bool setPublicNetwork(bool isPublic) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool getPublicNetwork() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool setSendConfirmation(bool isAckRequired) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool getSendConfirmation() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool joinOTAA(const char* appEui, const char* appKey, const char* devEui,
                uint32_t timeout) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool joinABP(const char* devAddr, const char* nwkSKey, const char* appSKey,
               uint32_t timeout = DEFAULT_JOIN_TIMEOUT)
      TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool isNetworkConnectedImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  int8_t getSignalQualityImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa Class and Band functions
   */

  bool configureClassImp(_lora_class _class) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool configureBandImp(_lora_band band) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool   setFrequencySubBand(int8_t subBand) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  int8_t getFrequencySubBand() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  String getChannelMaskImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   sendMaskImp(String newMask) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  /*
   * LoRa Data Rate and Duty Cycle functions
   */
  bool   setDutyCycleImp(int8_t dutyCycle) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  int8_t getDutyCycleImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   setDataRateImp(uint8_t dataRate) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  int8_t getDataRateImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   setAdaptiveDataRateImp(bool useADR) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   getAdaptiveDataRateImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa ABP Session Properties
   */
  // aka network address
  String getDevAddrImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  // network session key
  String getNwkSKeyImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  // aka data session key
  String AppSKeyImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa OTAA Session Properties
   */
  // aka network id
  String getAppEUIImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  // aka network key
  String getAppKeyImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  /*
   * GSM Location functions
   */
 protected:
  // Depending on the exacty model and firmware revision, should return a
  // GSM-based location from CLBS as per the template
  // TODO(?):  Check number of digits in year (2 or 4)

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // No functions of this type supported

  /*
   * Audio functions
   */
 public:
  bool setVolume(uint8_t volume = 50) {
    // Set speaker volume
    sendAT(GF("+CLVL="), volume);
    return waitResponse() == 1;
  }

  uint8_t getVolume() {
    // Get speaker volume
    sendAT(GF("+CLVL?"));
    if (waitResponse(GF(GSM_NL)) != 1) { return 0; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.replace("+CLVL:", "");
    res.trim();
    return res.toInt();
  }

  bool setMicVolume(uint8_t channel, uint8_t level) {
    if (channel > 4) { return 0; }
    sendAT(GF("+CMIC="), level);
    return waitResponse() == 1;
  }

  bool setAudioChannel(uint8_t channel) {
    sendAT(GF("+CHFA="), channel);
    return waitResponse() == 1;
  }

  bool playToolkitTone(uint8_t tone, uint32_t duration) {
    sendAT(GF("STTONE="), 1, tone);
    delay(duration);
    sendAT(GF("STTONE="), 0);
    return waitResponse();
  }

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Battery functions
   */
 protected:
  // Follows all battery functions per template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    int8_t   rsp;
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
#if !defined(TINY_LORA_MODEM_SIM900)
    sendAT(GF("+CIPSSL="), ssl);
    rsp = waitResponse();
    if (ssl && rsp != 1) { return false; }
#ifdef TINY_LORA_SSL_CLIENT_AUTHENTICATION
    // set SSL options
    // +SSLOPT=<opt>,<enable>
    // <opt>
    //    0 (default) ignore invalid certificate
    //    1 client authentication
    // <enable>
    //    0 (default) close
    //    1 open
    sendAT(GF("+CIPSSL=1,1"));
    if (waitResponse() != 1) return false;
#endif
#endif
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    rsp = waitResponse(
        timeout_ms, GF("CONNECT OK" GSM_NL), GF("CONNECT FAIL" GSM_NL),
        GF("ALREADY CONNECT" GSM_NL), GF("ERROR" GSM_NL),
        GF("CLOSE OK" GSM_NL));  // Happens when HTTPS handshake fails
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "DATA ACCEPT:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    return streamGetIntBefore('\n');
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
#ifdef TINY_LORA_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif
    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    streamSkipUntil(',');  // Skip mux
    int16_t len_requested = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int16_t len_confirmed = streamGetIntBefore('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read.
    // SRGD NOTE:  Contrary to above (which is copied from AT command manual)
    // this is actually be the number of bytes that will be remaining in the
    // buffer after the read.
    for (int i = 0; i < len_requested; i++) {
      uint32_t startMillis = millis();
#ifdef TINY_LORA_USE_HEX
      while (stream.available() < 2 &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_LORA_YIELD();
      }
      char buf[4] = {
          0,
      };
      buf[0] = stream.read();
      buf[1] = stream.read();
      char c = strtol(buf, NULL, 16);
#else
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_LORA_YIELD();
      }
      char c = stream.read();
#endif
      sockets[mux]->rx.put(c);
    }
    // DBG("### READ:", len_requested, "from", mux);
    // sockets[mux]->sock_available = modemGetAvailable(mux);
    sockets[mux]->sock_available = len_confirmed;
    waitResponse();
    return len_requested;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');  // Skip mode 4
      streamSkipUntil(',');  // Skip mux
      result = streamGetIntBefore('\n');
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF(GSM_NL "+CIPRXGET:"))) {
      int8_t mode = streamGetIntBefore(',');
      if (mode == 1) {
        int8_t mux = streamGetIntBefore('\n');
        if (mux >= 0 && mux < TINY_LORA_MUX_COUNT && sockets[mux]) {
          sockets[mux]->got_data = true;
        }
        data = "";
        // DBG("### Got Data:", mux);
        return true;
      } else {
        data += mode;
        return false;
      }
    } else if (data.endsWith(GF(GSM_NL "+RECEIVE:"))) {
      int8_t  mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (mux >= 0 && mux < TINY_LORA_MUX_COUNT && sockets[mux]) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      // DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
      int8_t nl   = data.lastIndexOf(GSM_NL, data.length() - 8);
      int8_t coma = data.indexOf(',', nl + 2);
      int8_t mux  = data.substring(nl + 2, coma).toInt();
      if (mux >= 0 && mux < TINY_LORA_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("*PSNWID:"))) {
      streamSkipUntil('\n');  // Refresh network name by network
      data = "";
      DBG("### Network name updated.");
      return true;
    } else if (data.endsWith(GF("*PSUTTZ:"))) {
      streamSkipUntil('\n');  // Refresh time and time zone by network
      data = "";
      DBG("### Network time and time zone updated.");
      return true;
    } else if (data.endsWith(GF("+CTZV:"))) {
      streamSkipUntil('\n');  // Refresh network time zone by network
      data = "";
      DBG("### Network time zone updated.");
      return true;
    } else if (data.endsWith(GF("DST:"))) {
      streamSkipUntil('\n');  // Refresh Network Daylight Saving Time by network
      data = "";
      DBG("### Daylight savings time state updated.");
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  LoRaStream_mDOT* sockets[TINY_LORA_MUX_COUNT];
};

#endif  // SRC_TINYLORACLIENTMDOT_H_
