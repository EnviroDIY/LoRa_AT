/**
 * @file       TinyLoRaModem.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TinyLoRaMODEM_H_
#define SRC_TinyLoRaMODEM_H_

#include "TinyLoRaCommon.h"

#ifndef GSM_NL
#define GSM_NL "\r\n"
#endif

static const char GSM_OK[] TINY_GSM_PROGMEM    = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CMS ERROR:";
#endif

template <class modemType>
class TinyLoRaModem {
 public:
  /*
   * Basic functions
   */
  bool begin(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }
  bool init(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }
  template <typename... Args>
  inline void sendAT(Args... cmd) {
    thisModem().streamWrite("AT", cmd..., GSM_NL);
    thisModem().stream.flush();
    TINY_LORA_YIELD(); /* DBG("### AT:", cmd...); */
  }
  void setBaud(uint32_t baud) {
    thisModem().setBaudImpl(baud);
  }
  // Test response to AT commands
  bool testAT(uint32_t timeout_ms = 10000L) {
    return thisModem().testATImpl(timeout_ms);
  }
  // Listen for responses to commands and handle URCs
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL,
                      GsmConstStr r7 = NULL) {
    return thisModem().waitResponseImpl(timeout_ms, data, r1, r2, r3, r4, r5,
                                        r6, r7);
  }

  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL,
                      GsmConstStr r7 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5, r6, r7);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL,
                      GsmConstStr r7 = NULL) {
    return waitResponse(1000L, r1, r2, r3, r4, r5, r6, r7);
  }

  // Asks for modem information via the V.25TER standard ATI command
  // NOTE:  The actual value and style of the response is quite varied
  String getModemInfo() {
    return thisModem().getModemInfoImpl();
  }
  // Gets the modem name (as it calls itself)
  String getModemName() {
    return thisModem().getModemNameImpl();
  }
  bool factoryDefault() {
    return thisModem().factoryDefaultImpl();
  }

  /*
   * Power functions
   */
  bool restart(const char* pin = NULL) {
    return thisModem().restartImpl(pin);
  }
  bool poweroff() {
    return thisModem().powerOffImpl();
  }
  bool radioOff() {
    return thisModem().radioOffImpl();
  }
  bool sleepEnable(bool enable = true) {
    return thisModem().sleepEnableImpl(enable);
  }

  /*
   * Generic network functions
   */
  // RegStatus getRegistrationStatus() {}
  bool isNetworkConnected() {
    return thisModem().isNetworkConnectedImpl();
  }
  // Waits for network attachment
  bool waitForNetwork(uint32_t timeout_ms = 60000L, bool check_signal = false) {
    return thisModem().waitForNetworkImpl(timeout_ms, check_signal);
  }
  // Gets signal quality report
  int16_t getSignalQuality() {
    return thisModem().getSignalQualityImpl();
  }

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /*
   * Basic functions
   */
 protected:
  void setBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+IPR="), baud);
    thisModem().waitResponse();
  }

  bool testATImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      thisModem().sendAT(GF(""));
      if (thisModem().waitResponse(200) == 1) { return true; }
      delay(100);
    }
    return false;
  }


  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponseImpl(uint32_t timeout_ms, String& data,
                          GsmConstStr r1 = GFP(GSM_OK),
                          GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                          GsmConstStr r3 = GFP(GSM_CME_ERROR),
                          GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                          GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                          GsmConstStr r5 = NULL, GsmConstStr r6 = NULL,
                          GsmConstStr r7 = NULL) {
    data.reserve(64);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (thisModem().stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = thisModem().stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
#if defined TINY_GSM_DEBUG
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
#endif
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (r6 && data.endsWith(r6)) {
          index = 6;
          goto finish;
        } else if (r7 && data.endsWith(r7)) {
          index = 7;
          goto finish;
        } else if (thisModem().handleURCs(data)) {
          data = "";
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    }
    // data.replace(AT_NL, "/");
    // DBG('<', index, '>', data);
    return index;
  }


  String getModemInfoImpl() {
    thisModem().sendAT(GF("I"));
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    // Do the replaces twice so we cover both \r and \r\n type endings
    res.replace("\r\nOK\r\n", "");
    res.replace("\rOK\r", "");
    res.replace("\r\n", " ");
    res.replace("\r", " ");
    res.trim();
    return res;
  }

  String getModemNameImpl() {
    thisModem().sendAT(GF("+CGMI"));
    String res1;
    if (thisModem().waitResponse(1000L, res1) != 1) { return "unknown"; }
    res1.replace("\r\nOK\r\n", "");
    res1.replace("\rOK\r", "");
    res1.trim();

    thisModem().sendAT(GF("+GMM"));
    String res2;
    if (thisModem().waitResponse(1000L, res2) != 1) { return "unknown"; }
    res2.replace("\r\nOK\r\n", "");
    res2.replace("\rOK\r", "");
    res2.trim();

    String name = res1 + String(' ') + res2;
    DBG("### Modem:", name);
    return name;
  }

  bool factoryDefaultImpl() {
    thisModem().sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    thisModem().waitResponse();
    thisModem().sendAT(GF("+IPR=0"));  // Auto-baud
    thisModem().waitResponse();
    thisModem().sendAT(GF("&W"));  // Write configuration
    return thisModem().waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool radioOffImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool sleepEnableImpl(bool enable = true) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  // Gets the modem's registration status via CREG/CGREG/CEREG
  // CREG = Generic network registration
  // CGREG = GPRS service registration
  // CEREG = EPS registration for LTE modules
  int8_t getRegistrationStatusXREG(const char* regCommand) {
    thisModem().sendAT('+', regCommand, '?');
    // check for any of the three for simplicity
    int8_t resp = thisModem().waitResponse(GF("+CREG:"), GF("+CGREG:"),
                                           GF("+CEREG:"));
    if (resp != 1 && resp != 2 && resp != 3) { return -1; }
    thisModem().streamSkipUntil(','); /* Skip format (0) */
    int status = thisModem().stream.parseInt();
    thisModem().waitResponse();
    return status;
  }

  bool waitForNetworkImpl(uint32_t timeout_ms   = 60000L,
                          bool     check_signal = false) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (check_signal) { thisModem().getSignalQuality(); }
      if (thisModem().isNetworkConnected()) { return true; }
      delay(250);
    }
    return false;
  }

  // Gets signal quality report according to 3GPP TS command AT+CSQ
  int8_t getSignalQualityImpl() {
    thisModem().sendAT(GF("+CSQ"));
    if (thisModem().waitResponse(GF("+CSQ:")) != 1) { return 99; }
    int8_t res = thisModem().streamGetIntBefore(',');
    thisModem().waitResponse();
    return res;
  }

  String getLocalIPImpl() {
    thisModem().sendAT(GF("+CGPADDR=1"));
    if (thisModem().waitResponse(GF("+CGPADDR:")) != 1) { return ""; }
    thisModem().streamSkipUntil(',');  // Skip context id
    String res = thisModem().stream.readStringUntil('\r');
    if (thisModem().waitResponse() != 1) { return ""; }
    return res;
  }

  /*
   Utilities
   */
 public:
  // Utility templates for writing/skipping characters on a stream
  template <typename T>
  inline void streamWrite(T last) {
    thisModem().stream.print(last);
  }

  template <typename T, typename... Args>
  inline void streamWrite(T head, Args... tail) {
    thisModem().stream.print(head);
    thisModem().streamWrite(tail...);
  }

  inline void streamClear() {
    while (thisModem().stream.available()) {
      thisModem().waitResponse(50, NULL, NULL);
    }
  }

 protected:
  inline bool streamGetLength(char* buf, int8_t numChars,
                              const uint32_t timeout_ms = 1000L) {
    if (!buf) { return false; }

    int8_t   numCharsReady = -1;
    uint32_t startMillis   = millis();
    while (millis() - startMillis < timeout_ms &&
           (numCharsReady = thisModem().stream.available()) < numChars) {
      TINY_LORA_YIELD();
    }

    if (numCharsReady >= numChars) {
      thisModem().stream.readBytes(buf, numChars);
      return true;
    }

    return false;
  }

  inline int16_t streamGetIntLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    char buf[numChars + 1];
    if (streamGetLength(buf, numChars, timeout_ms)) {
      buf[numChars] = '\0';
      return atoi(buf);
    }

    return -9999;
  }

  inline int16_t streamGetIntBefore(char lastChar) {
    char   buf[7];
    size_t bytesRead = thisModem().stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(7));
    // if we read 7 or more bytes, it's an overflow
    if (bytesRead && bytesRead < 7) {
      buf[bytesRead] = '\0';
      int16_t res    = atoi(buf);
      return res;
    }

    return -9999;
  }

  inline float streamGetFloatLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    char buf[numChars + 1];
    if (streamGetLength(buf, numChars, timeout_ms)) {
      buf[numChars] = '\0';
      return atof(buf);
    }

    return -9999.0F;
  }

  inline float streamGetFloatBefore(char lastChar) {
    char   buf[16];
    size_t bytesRead = thisModem().stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(16));
    // if we read 16 or more bytes, it's an overflow
    if (bytesRead && bytesRead < 16) {
      buf[bytesRead] = '\0';
      float res      = atof(buf);
      return res;
    }

    return -9999.0F;
  }

  inline bool streamSkipUntil(const char c, const uint32_t timeout_ms = 1000L) {
    uint32_t startMillis = millis();
    while (millis() - startMillis < timeout_ms) {
      while (millis() - startMillis < timeout_ms &&
             !thisModem().stream.available()) {
        TINY_LORA_YIELD();
      }
      if (thisModem().stream.read() == c) { return true; }
    }
    return false;
  }
};

#endif  // SRC_TinyLoRaMODEM_H_
