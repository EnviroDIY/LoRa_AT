/**
 * @file       TinyLoRaModem.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYLORAMODEM_H_
#define SRC_TINYLORAMODEM_H_

#include "TinyLoRaCommon.h"

#ifndef GSM_NL
#define GSM_NL "\r\n"
#endif

#ifndef DEFAULT_JOIN_TIMEOUT
#define DEFAULT_JOIN_TIMEOUT 60000L
#endif

static const char GSM_OK[] TINY_LORA_PROGMEM    = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_LORA_PROGMEM = "ERROR" GSM_NL;

#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_LORA_PROGMEM = GSM_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_LORA_PROGMEM = GSM_NL "+CMS ERROR:";
#endif

// Enums taken from:

/**
 * @brief LoRa Frequency bands - use the correct one for your module and country
 *
 * @enum _lora_band
 */
typedef enum {
  AS923 = 0,
  AU915,
  CN470,
  CN779,
  EU433,
  EU868 = 5,
  KR920,
  IN865,
  US915,
  US915_HYBRID,
} _lora_band;

/**
 * @brief LoRa activation modes
 *
 * @see https://www.thethingsindustries.com/docs/devices/concepts/abp-vs-otaa/
 *
 * @enum _lora_mode
 */
typedef enum {
  ABP = 0,  ///< Activation By Personalization - Manual configuration
  OTAA,     ///< Over-The-Air Activation
} _lora_mode;

typedef enum {
  APP_EUI = 0,
  APP_KEY,
  DEV_EUI,
  DEV_ADDR,
  NWKS_KEY,
  APPS_KEY,
  NWK_ID,
} _lora_property;

/**
 * @brief LoRaWAN Device Classes
 *
 * @see https://lora.readthedocs.io/en/latest/#lorawan-device-classes
 */
typedef enum {
  CLASS_A =
      'A', /*!< A(ll) Battery powered devices. Each device uplink to the gateway
              and is followed by two short downlink receive windows. */
  CLASS_B, /*!< B(eacon) Same as class A but these devices also opens extra
              receive windows at scheduled times. */
  CLASS_C, /*!< C(ontinuous) Same as A but these devices are continuously
              listening. Hence these devices uses more power and are often mains
              powered. */
} _lora_class;

template <class modemType>
class TinyLoRaModem {
 public:
  /**
   * @anchor basic_functions
   * @name Basic functions
   */
  /**@{*/

  /**
   * @brief Sets up the LoRa module
   *
   * @param pin A pin used to wake up the module, if needed
   * @return *true* The module was set up as expected
   * @return *false* Something failed in module set up
   */
  bool begin(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }
  /**
   * @copydoc TinyLoRaModem::begin()
   */
  bool init(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }

  /**
   * @brief Recursive variadic template to send AT commands
   *
   * @tparam Args
   * @param cmd The commands to send
   */
  template <typename... Args>
  inline void sendAT(Args... cmd) {
    thisModem().streamWrite("AT", cmd..., GSM_NL);
    thisModem().stream.flush();
    TINY_LORA_YIELD(); /* DBG("### AT:", cmd...); */
  }

  /**
   * @brief Set the module baud rate
   *
   * @param baud The baud rate the use
   *
   * @note After setting and applying the new baud rate, you will have to end()
   * and begin() the serial object.
   */
  void setBaud(uint32_t baud) {
    thisModem().setBaudImpl(baud);
  }

  /**
   * @brief Test response to AT commands
   *
   * @param timeout_ms The the amount of time to test for; optional with a
   * default value of 10s.
   * @return *true*  The module responeded to AT commands
   * @return *false*  The module failed to respond
   */
  bool testAT(uint32_t timeout_ms = 10000L) {
    return thisModem().testATImpl(timeout_ms);
  }

  /**
   * @brief Listen for responses to commands and handle URCs
   *
   * @param timeout_ms The time to wait for a response
   * @param data A string of data to fill in with response results
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of NULL
   * @param r4 The fourth output to test against, optional with a default value
   * of NULL
   * @param r5 The fifth output to test against, optional with a default value
   * of NULL
   * @param r6 The sixth output to test against, optional with a default value
   * of NULL
   * @param r7 The seventh output to test against, optional with a default value
   * of NULL
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR), GsmConstStr r3 = NULL,
                      GsmConstStr r4 = NULL, GsmConstStr r5 = NULL,
                      GsmConstStr r6 = NULL, GsmConstStr r7 = NULL) {
    return thisModem().waitResponseImpl(timeout_ms, data, r1, r2, r3, r4, r5,
                                        r6, r7);
  }

  /**
   * @brief Listen for responses to commands and handle URCs
   *
   * @param timeout_ms The time to wait for a response
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of NULL
   * @param r4 The fourth output to test against, optional with a default value
   * of NULL
   * @param r5 The fifth output to test against, optional with a default value
   * of NULL
   * @param r6 The sixth output to test against, optional with a default value
   * of NULL
   * @param r7 The seventh output to test against, optional with a default value
   * of NULL
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR), GsmConstStr r3 = NULL,
                      GsmConstStr r4 = NULL, GsmConstStr r5 = NULL,
                      GsmConstStr r6 = NULL, GsmConstStr r7 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5, r6, r7);
  }

  /**
   * @brief Listen for responses to commands and handle URCs; listening for 1
   * second.
   *
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of NULL
   * @param r4 The fourth output to test against, optional with a default value
   * of NULL
   * @param r5 The fifth output to test against, optional with a default value
   * of NULL
   * @param r6 The sixth output to test against, optional with a default value
   * of NULL
   * @param r7 The seventh output to test against, optional with a default value
   * of NULL
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR), GsmConstStr r3 = NULL,
                      GsmConstStr r4 = NULL, GsmConstStr r5 = NULL,
                      GsmConstStr r6 = NULL, GsmConstStr r7 = NULL) {
    return waitResponse(1000L, r1, r2, r3, r4, r5, r6, r7);
  }

  /**
   * @brief Get the LoRa module's device EUI - this is unique and set at the
   * factory.
   *
   * @return *String*  The device EUI
   */
  String getDevEUI() {
    return thisModem().getDevEUIImpl();
  }

  /**
   * @brief Get information about the LoRa module
   *
   * @note  The actual value and style of the response is quite varied
   * @return *String* Some info about the LoRa module.
   */
  String getModuleInfo() {
    return thisModem().getModuleInfoImpl();
  }

  /**
   * @brief Get the LoRa module name (as it calls itself)
   *
   * @return *String* The module name
   */
  String getModuleName() {
    return thisModem().getModuleNameImpl();
  }

  /**
   * @brief Reset the module to factory defaults.
   *
   * This generally restarts the module as well.
   *
   * @return *true* The module successfully reset to default.
   * @return *false* The module failed to reset to default.
   */
  bool factoryDefault() {
    return thisModem().factoryDefaultImpl();
  }
  /**@}*/

  /**
   * @anchor network_functions
   * @name Generic Network Functions
   */
  /**@{*/

  /**
   * @brief Set the module for public or private network mode.
   *
   * @param isPublic **TRUE** for a public network, **FALSE** for private MTS
   * network
   *
   * @return *true* The module accepted the command to set the network mode
   * @return *false* The module failed to set the network mode
   */
  bool setPublicNetwork(bool isPublic) {
    return thisModem().setPublicNetworkImpl(isPublic);
  }
  /**
   * @brief Report whether the module is using public network mode
   *
   * @return *true*  The module is set up to connect to a public network
   * @return *false*  The module is set up to connect to a private MTS network
   */
  bool getPublicNetwork() {
    return thisModem().getPublicNetworkImpl();
  }

  /**
   * @brief Join a network using OTAA (Over The Air Activation)
   *
   * @param appEui The app EUI (Network ID)
   * @param appKey The app key (Network key)
   * @param devEui The device EUI
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinOTAA(const char* appEui, const char* appKey, const char* devEui,
                uint32_t timeout) {
    return thisModem().joinOTAAImpl();
  }

  /**
   * @brief Join a network using OTAA (Over The Air Activation)
   *
   * @param appEui The app EUI (Network ID)
   * @param appKey The app key (Network key)
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinOTAA(String appEui, String appKey,
                uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return joinOTAA(appEui.c_str(), appKey.c_str(), NULL, timeout);
  }


  /**
   * @brief Join a network using OTAA (Over The Air Activation), waiting the
   * default timeout.
   *
   * @param appEui The app EUI (Network ID)
   * @param appKey The app key (Network key)
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinOTAA(String appEui, String appKey, String devEui,
                uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return joinOTAA(appEui.c_str(), appKey.c_str(), devEui.c_str(), timeout);
  }

  /**
   * @brief Join a network using ABP (Activation By Personalization)
   *
   * @param devAddr The device address (network address)
   * @param nwkSKey The network session key
   * @param appSKey The app session key (data session key)
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinABP(const char* devAddr, const char* nwkSKey, const char* appSKey,
               uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return thisModem().joinABPImpl();
  }

  /**
   * @brief Join a network using ABP (Activation By Personalization), waiting
   * the default timeout.
   *
   * @param devAddr The device address (network address)
   * @param nwkSKey The network session key
   * @param appSKey The app session key (data session key)
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinABP(String devAddr, String nwkSKey, String appSKey) {
    return joinABP(devAddr.c_str(), nwkSKey.c_str(), appSKey.c_str());
  }
  /**
   * @brief Confirm whether the module is currently connected to the LoRaWAN
   * network.
   *
   * @return *true* The module is connected to the network
   * @return *false* The module is not connected to the network
   */
  bool isNetworkConnected() {
    return thisModem().isNetworkConnectedImpl();
  }

  /**
   * @brief Get the signal quality report
   *
   * @return *int16_t* A measure of the signal quality (probably the RSSI)
   */
  int16_t getSignalQuality() {
    return thisModem().getSignalQualityImpl();
  }
  /**@}*/

  /**
   * @anchor band_functions
   * @name LoRa Class and Band functions
   */
  /**@{*/

  /**
   * @brief Configure the LoRaWAN device class
   *
   * @see https://lora.readthedocs.io/en/latest/#lorawan-device-classes
   *
   * @param _class The device class; must be A, B, or C
   * @return *true* The device class was successfully configured
   * @return *false* The module did not accept the device class
   */
  bool configureClass(_lora_class _class) {
    return thisModem().configureClassImpl(_class);
  }

  /**
   * @brief Configure the LoRa band.  The band should be appropriate to your
   * module and your location.
   *
   * @param band The band to use, must be one of ::_lora_band
   * @return *true* The device band was successfully configured
   * @return *false* The module did not accept the device band
   */
  bool configureBand(_lora_band band) {
    return thisModem().configureBandImpl(band);
  }

  /**
   * @brief Set the frequency sub-band. This only appolies to US 915 MHz
   * modules.
   *
   * @param subBand An int representing the sub-band, per the module
   * documentation
   * @return *true* The device sub-band was successfully configured
   * @return *false* The module did not accept the device sub-band
   */
  bool setFrequencySubBand(int8_t subBand);
  { return thisModem().setFrequencySubBandImpl(subBand); }
  /**
   * @brief Get the  frequency sub-band the module is operating on. This only
   * appolies to US 915 MHz modules.
   *
   * @return *int8_t* An int representing the sub-band, per the module
   * documentation
   */
  int8_t getFrequencySubBand();
  { return thisModem().getFrequencySubBandImpl(); }

  /**
   * @brief Get the 16 or 72 bit channel mask
   *
   * @return *String* The 16 or 72 bit channel mask
   */
  String getChannelMask() {
    return thisModem().getChannelMaskl();
  }

  /**
   * @brief Check if a particular channel is enabled
   *
   * @param pos The position of the channel in the 16 or 72 bit channel mask.
   * @return The position of the channel checked, if it is enabled.
   */
  int isChannelEnabled(int pos) {
    return thisModem().isChannelEnabledImpl(pos);
  }

  /**
   * @brief Disables a channel
   *
   * @param pos The position of the channel in the 16 or 72 bit channel mask.
   * @return *true* The channel was successfully disabled.
   * @return *false* The channel was not disabled.
   */
  bool disableChannel(int pos) {
    return thisModem().disableChannelImpl(pos);
  }

  /**
   * @brief Enables a channel
   *
   * @param pos The position of the channel in the 16 or 72 bit channel mask.
   * @return *true* The channel was successfully enabled.
   * @return *false* The channel was not enabled.
   */
  bool enableChannel(int pos) {
    return thisModem().enableChannelImpl(pos);
  }

  /**
   * @brief Sends a new channel mask to the device
   *
   * @param newMask A full new channel mask
   * @return *true* The module accepted the new channel mask.
   * @return *false* There was an error in changing the channel mask.
   */
  bool sendMask(String newMask) {
    return thisModem().sendMaskImpl(newMask);
  }
  /**@}*/

  /**
   * @anchor duty_functions
   * @name LoRa Duty Cycle and Data Rate Functions
   */
  /**@{*/

  /**
   * @brief Set the LoRa module's duty cycle
   *
   * @see https://lora.readthedocs.io/en/latest/#duty-cycle-time-on-air-toa
   *
   * @param dutyCycle An int representing the duty cycle, per the module
   * documentation
   * @return *true* The module accepted the new duty cycle.
   * @return *false* There was an error in changing the duty cycle.
   */
  bool setDutyCycle(int8_t dutyCycle) {
    return thisModem().setDutyCycleImpl(dutyCycle);
  }
  /**
   * @brief Get the current duty cycle for the LoRa module
   *
   * @see https://lora.readthedocs.io/en/latest/#duty-cycle-time-on-air-toa
   *
   * @return *int8_t* An int representing the duty cycle, per the module
   * documentation
   */
  int8_t getDutyCycle() {
    return thisModem().getDutyCycleImpl();
  }

  /**
   * @brief Set the LoRa data rate
   *
   * @param dataRateAn int representing the data rate, per the module
   * documentation
   * @return *true* The module accepted the new data rate.
   * @return *false* There was an error in changing the data rate.
   */
  bool setDataRate(uint8_t dataRate) {
    return thisModem().setDataRateImpl(dataRate);
  }
  /**
   * @brief Get the current data rate for the LoRa module
   *
   * @return *int8_t* int representing the data rate, per the module
   * documentation
   */
  int8_t getDataRate() {
    return thisModem().getDataRateImpl();
  }

  /**
   * @brief Enable or diable adaptive data rate for the LoRa module
   *
   * @see
   * https://lora-developers.semtech.com/documentation/tech-papers-and-guides/understanding-adr
   *
   * @param useADR True to enable adaptive data rate, false to disable
   * @return *true* The module accepted the new adaptive data rate setting.
   * @return *false* There was an error in changing the adaptive data rate
   * setting.
   */
  bool setAdaptiveDataRate(bool useADR) {
    return thisModem().setAdaptiveDataRateImpl(useADR);
  }
  /**
   * @brief Checks whether adaptive data rate is currently enabled for the LoRa
   * module
   *
   * @see
   * https://lora-developers.semtech.com/documentation/tech-papers-and-guides/understanding-adr
   *
   * @return *true* Adaptive data rate is enabled
   * @return *false* Adaptive data rate is disabled
   */
  bool getAdaptiveDataRate() {
    return thisModem().getAdaptiveDataRateImpl();
  }
  /**@}*/

  /**
   * @anchor abp_properties
   * @name LoRa ABP Session Properties
   */
  /**@{*/
  /**
   * @brief Get the device address (network address)
   *
   * This only applies when the LoRa module is in ABP mode.
   *
   * @return *String* The device address (network address)
   */
  String getDevAddr() {
    return thisModem().getDevAddrImpl();
  }
  // network session key
  /**
   * @brief Get the network session key
   *
   * This only applies when the LoRa module is in ABP mode.
   *
   * @return *String* The network session key
   */
  String getNwkSKey() {
    return thisModem().getNwkSKeyImpl();
  }
  // aka data session key
  /**
   * @brief Get the App Session Key (data session key)
   *
   * This only applies when the LoRa module is in ABP mode.
   *
   * @return *String* The App Session Key (data session key)
   */
  String AppSKey() {
    return thisModem().AppSKeyImpl();
  }
  /**@}*/

  /**
   * @anchor otaa_properties
   * @name LoRa OTAA Session Properties
   */
  /**@{*/
  /**
   * @brief Get the App EUI (network ID)
   *
   * This only applies when the LoRa module is in OTAA  mode.
   *
   * @return *String* The App EUI
   */
  String getAppEUI() {
    return thisModem().getAppKeyImpl();
  }
  /**
   * @brief Get the App Key (network key)
   *
   * This only applies when the LoRa module is in OTAA  mode.
   *
   * @return *String* The App Key
   */
  String getAppKey() {
    return thisModem().getAppKeyImpl();
  }
  /**@}*/

  /**
   * @anchor power_functions
   * @name Power functions
   */
  /**@{*/

  /**
   * @brief Restart the module
   *
   * @param pin A pin to use to wake/restart the module
   * @return *true* The module was successfully restarted.
   * @return *false* There was an error in restarting the module.
   */
  bool restart(const char* pin = NULL) {
    return thisModem().restartImpl(pin);
  }
  /**
   * @brief Power off the module
   *
   * @return *true* The module was successfully powered down.
   * @return *false* There was an error in powering down module.
   */
  bool poweroff() {
    return thisModem().powerOffImpl();
  }
  /**
   * @brief Turn off the module radio
   *
   * @return *true* The module radio was successfully turned off.
   * @return *false* There was an error in turning off the radio.
   */
  bool radioOff() {
    return thisModem().radioOffImpl();
  }
  /**
   * @brief Enable or disable sleep mode for the module.  What "sleep" means
   * varies by module; check your documentation.
   *
   * @param enable True to enable sleep mode, false to disable it.
   * @return *true* The module accepted the sleep mode setting.
   * @return *false* There was an error in setting the sleep mode.
   */
  bool sleepEnable(bool enable = true) {
    return thisModem().sleepEnableImpl(enable);
  }
  /**@}*/

  /**
   * @anchor crtp_helper
   * @name CRTP Helper
   */
  /**@{*/
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  /**@}*/

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

  String getDevEUI() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  String getModuleInfoImpl() {
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

  String getModuleNameImpl() {
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
    DBG("### LoRa Module:", name);
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
   * Generic network functions
   */
 protected:
  bool setPublicNetwork(bool isPublic) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool getPublicNetwork() TINY_LORA_ATTR_NOT_IMPLEMENTED;

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

  int getChannelMaskSize(_lora_band band) {
    switch (band) {
      case AS923:
      case CN779:
      case EU433:
      case EU868:
      case KR920:
      case IN865: mask_size = 1; break;
      case AU915:
      case CN470:
      case US915:
      case US915_HYBRID: mask_size = 6; break;
      default: break;
    }
    return mask_size;
  }

  String getChannelMaskImp() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  int isChannelEnabledImp(int pos) {
    // Populate channelsMask array
    int max_retry = 3;
    int retry     = 0;
    while (retry < max_retry) {
      String mask = thisModem().getChannelMask();
      if (mask != "0") { break; }
      retry++;
    }

    int      row     = pos / 16;
    int      col     = pos % 16;
    uint16_t channel = (uint16_t)(1 << col);

    channel = ((channelsMask[row] & channel) >> col);

    return channel;
  }

  bool disableChannelImp(int pos) {
    // Populate channelsMask array
    int max_retry = 3;
    int retry     = 0;
    while (retry < max_retry) {
      String mask = thisModem().getChannelMask();
      if (mask != "0") { break; }
      retry++;
    }

    int      row  = pos / 16;
    int      col  = pos % 16;
    uint16_t mask = ~(uint16_t)(1 << col);

    channelsMask[row] = channelsMask[row] & mask;

    return thisModem().sendMask();
  }

  bool enableChannelImp(int pos) {
    // Populate channelsMask array
    int max_retry = 3;
    int retry     = 0;
    while (retry < max_retry) {
      String mask = thisModem().getChannelMask();
      if (mask != "0") { break; }
      retry++;
    }

    int      row  = pos / 16;
    int      col  = pos % 16;
    uint16_t mask = (uint16_t)(1 << col);

    channelsMask[row] = channelsMask[row] | mask;

    return thisModem().sendMask();
  }

  bool sendMaskImp() {
    String newMask;

    /* Convert channel mask into string */
    for (int i = 0; i < 6; i++) {
      char hex[4];
      sprintf(hex, "%04x", channelsMask[i]);
      newMask.concat(hex);
    }

    DBG("Newmask: ", newMask);

    return thisModem().sendMask(newMask);
  }

  bool sendMaskImp(String newMask) TINY_LORA_ATTR_NOT_IMPLEMENTED;

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
   * Power functions
   */
 protected:
  bool radioOffImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool sleepEnableImpl(bool enable = true) TINY_LORA_ATTR_NOT_IMPLEMENTED;


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

#endif  // SRC_TINYLORAMODEM_H_
