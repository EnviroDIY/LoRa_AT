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

#ifndef AT_NL
#define AT_NL "\r\n"
#endif

#ifndef DEFAULT_JOIN_TIMEOUT
#define DEFAULT_JOIN_TIMEOUT 60000L
#endif

static const char GSM_OK[] TINY_LORA_PROGMEM    = "OK" AT_NL;
static const char GSM_ERROR[] TINY_LORA_PROGMEM = "ERROR" AT_NL;

#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_LORA_PROGMEM = AT_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_LORA_PROGMEM = AT_NL "+CMS ERROR:";
#endif

// Enums taken from: https://github.com/arduino-libraries/MKRWAN

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
   * @return *true* The module was set up as expected
   * @return *false* Something failed in module set up
   */
  bool begin() {
    return thisModem().initImpl();
  }
  /**
   * @copydoc TinyLoRaModem::begin()
   */
  bool init() {
    return thisModem().initImpl();
  }

  /**
   * @brief Recursive variadic template to send AT commands
   *
   * @tparam Args
   * @param cmd The commands to send
   */
  template <typename... Args>
  inline void sendAT(Args... cmd) {
    thisModem().streamWrite("AT", cmd..., AT_NL);
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
   * @anchor power_functions
   * @name Power functions
   */
  /**@{*/

  /**
   * @brief Restart the module
   *
   * @return *true* The module was successfully restarted.
   * @return *false* There was an error in restarting the module.
   */
  bool restart() {
    return thisModem().restartImpl();
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
   * @brief Set the number of times to retry sending a message while waiting for
   * an ACK from the recipient.
   *
   * @note Requireing acknowledgement of every send can significantly slow down
   * the send time.
   *
   * @param numAckRetries The number of retries to attempt to
   * get acknowledgement [0-15].
   * @return *true* The module accepting the acknowledgement setting.
   * @return *false* There was an error in setting the acknowledgement setting.
   */
  bool setConfirmationRetries(int8_t numAckRetries) {
    return thisModem().setConfirmationRetriesImpl(numAckRetries);
  }
  /**
   * @brief Check the number of retries to attempt when sending a message and
   * waiting for an ACK from the recipient.
   *
   * @return The number of retries to attempt to get acknowledgement
   * [0-15].
   */
  int8_t getConfirmationRetries() {
    return thisModem().getConfirmationRetriesImpl();
  }

  /**
   * @brief Join a network using OTAA (Over The Air Activation)
   *
   * @note The useHex parameter applies to both the appEUI and the appKey.
   * Either both or neither should be hex.
   *
   * @param appEui The app EUI (Network ID). If using a string, set useHex to
   * false. If using 8 bytes of hex data, set useHex to true.
   * @param appKey The app key (Network key). If using a string, set useHex to
   * false. If using 8 bytes of hex data, set useHex to true.
   * @param devEui The device EUI. This must be 16 bytes of hex data.
   * @param useHex True if the appKey and appEUI are in hex; false for standard
   * strings
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinOTAA(const char* appEui, const char* appKey, const char* devEui,
                bool useHex = false, uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return thisModem().joinOTAAImpl(appEui, appKey, devEui, useHex, timeout);
  }

  /**
   * @brief Join a network using OTAA (Over The Air Activation)
   *
   * @note The useHex parameter applies to both the appEUI and the appKey.
   * Either both or neither should be hex.
   *
   * @param appEui The app EUI (Network ID). If using a string, set useHex to
   * false. If using 8 bytes of hex data, set useHex to true.
   * @param appKey The app key (Network key). If using a string, set useHex to
   * false. If using 8 bytes of hex data, set useHex to true.
   * @param useHex True if the appKey and appEUI are in hex; false for standard
   * strings
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinOTAA(String appEui, String appKey, bool useHex = false,
                uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return joinOTAA(appEui.c_str(), appKey.c_str(), NULL, useHex, timeout);
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
                bool useHex = false, uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return joinOTAA(appEui.c_str(), appKey.c_str(), devEui.c_str(), useHex,
                    timeout);
  }

  /**
   * @brief Join a network using ABP (Activation By Personalization)
   *
   * @param devAddr The device address (network address). This must be 4 bytes
   * of hex data.
   * @param nwkSKey The network session key. This must be 16 bytes of hex data.
   * @param appSKey The app session key (data session key). This must be 16
   * bytes of hex data.
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinABP(const char* devAddr, const char* nwkSKey, const char* appSKey,
               uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return thisModem().joinABPImpl(devAddr, nwkSKey, appSKey, timeout);
  }

  /**
   * @brief Join a network using ABP (Activation By Personalization), waiting
   * the default timeout.
   *
   * @param devAddr The device address (network address). This must be 4 bytes
   * of hex data.
   * @param nwkSKey The network session key. This must be 16 bytes of hex data.
   * @param appSKey The app session key (data session key). This must be 16
   * bytes of hex data.
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
    bool isConnected  = thisModem().isNetworkConnectedImpl();
    _networkConnected = isConnected;
    return isConnected;
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
   * @brief Set the LoRaWAN device class
   *
   * @see https://lora.readthedocs.io/en/latest/#lorawan-device-classes
   *
   * @param _class The device class; must be A, B, or C
   * @return *true* The device class was successfully configured
   * @return *false* The module did not accept the device class
   */
  bool setClass(_lora_class _class) {
    return thisModem().setClassImpl(_class);
  }
  /**
   * @brief Get the modules current LoRaWAN device class
   *
   * @return *_lora_class* The device class from the ::_lora_class enum
   */
  _lora_class getClass() {
    return thisModem().getClassImpl();
  }

  /**
   * @brief Set the LoRa band. The band should be appropriate to your
   * module and your location. This is not configurable on all modules.
   *
   * @param band The band to use, must be one of the bands available for your
   * module. Check your datasheet.
   * @return *true* The device band was successfully configured
   * @return *false* The module did not accept the device band
   */
  bool setBand(const char* band) {
    return thisModem().setBandImpl(band);
  }
  /**
   * @brief Set the LoRa band. The band should be appropriate to your
   * module and your location. This is not configurable on all modules.
   *
   * @param band The band to use, must be one of the bands available for your
   * module. Check your datasheet.
   * @return *true* The device band was successfully configured
   * @return *false* The module did not accept the device band
   */
  bool setBand(String band) {
    return setBand(band.c_str());
  }
  /**
   * @brief Get the current LoRa frequency band.
   *
   * @return The current LoRa frequency band the module is using.
   */
  String getBand() {
    return thisModem().getBandImpl();
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
  bool setFrequencySubBand(int8_t subBand) {
    return thisModem().setFrequencySubBandImpl(subBand);
  }
  /**
   * @brief Get the  frequency sub-band the module is operating on. This only
   * appolies to US 915 MHz modules.
   *
   * @return *int8_t* An int representing the sub-band, per the module
   * documentation
   */
  int8_t getFrequencySubBand() {
    return thisModem().getFrequencySubBandImpl();
  }

  /**
   * @brief Get the 16 or 72 bit channel mask
   *
   * @return *String* The 16 or 72 bit channel mask
   */
  String getChannelMask() {
    return thisModem().getChannelMaskImpl();
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
  bool setChannelMask(const char* newMask) {
    return thisModem().setChannelMaskImpl(newMask);
  }
  /**
   * @brief Sends a new channel mask to the device
   *
   * @param newMask A full new channel mask
   * @return *true* The module accepted the new channel mask.
   * @return *false* There was an error in changing the channel mask.
   */
  bool setChannelMask(String newMask) {
    return setChannelMask(newMask.c_str());
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
   * @brief Set the Tx LoRa data rate
   *
   * @param dataRate An int representing the Tx data rate, per the module
   * documentation
   * @return *true* The module accepted the new data rate.
   * @return *false* There was an error in changing the data rate.
   */
  bool setDataRate(uint8_t dataRate) {
    return thisModem().setDataRateImpl(dataRate);
  }
  /**
   * @brief Get the current Tx data rate for the LoRa module
   *
   * @return *int8_t* int representing the Tx data rate, per the module
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
  String getAppSKey() {
    return thisModem().getAppSKeyImpl();
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
    return thisModem().getAppEUIImpl();
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
  bool initImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

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
    data.reserve(TINY_LORA_RX_BUFFER);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_LORA_YIELD();
      while (thisModem().stream.available() > 0) {
        TINY_LORA_YIELD();
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
    // data.replace("\r", "←");
    // data.replace("\n", "↓");
    // DBG('<', index, '>', data);
    return index;
  }

  String getDevEUIImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

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
  bool restartImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool poweroffImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool radioOffImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool sleepEnableImpl(bool enable = true) TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * Generic network functions
   */
 protected:
  bool setPublicNetworkImpl(bool isPublic) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool getPublicNetworkImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool
  setConfirmationRetriesImpl(bool isAckRequired) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool getConfirmationRetriesImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool joinOTAAImpl(const char* appEui, const char* appKey, const char* devEui,
                    uint32_t timeout) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool joinABPImpl(const char* devAddr, const char* nwkSKey,
                   const char* appSKey, uint32_t timeout = DEFAULT_JOIN_TIMEOUT)
      TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool isNetworkConnectedImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  int8_t getSignalQualityImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa Class and Band functions
   */

  bool        setClassImpl(_lora_class _class) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  _lora_class getClassImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool   setBandImpl(const char* band) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  String getBandImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool   setFrequencySubBandImpl(int8_t subBand) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  int8_t getFrequencySubBandImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  String getChannelMaskImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  int isChannelEnabledImpl(int pos) {
    // Populate channelsMask array
    int    max_retry        = 3;
    int    retry            = 0;
    String channel_mask_str = "";
    while (retry < max_retry) {
      channel_mask_str = thisModem().getChannelMask();
      if (channel_mask_str != "0") { break; }
      retry++;
    }
    uint16_t channelsMask[6];
    if (channel_mask_str.length() > 0) {
      sscanf(channel_mask_str.c_str(), "%04hx%04hx%04hx%04hx%04hx%04hx",
             &channelsMask[0], &channelsMask[1], &channelsMask[2],
             &channelsMask[3], &channelsMask[4], &channelsMask[5]);
    }

    int      row     = pos / 16;
    int      col     = pos % 16;
    uint16_t channel = (uint16_t)(1 << col);

    channel = ((channelsMask[row] & channel) >> col);

    return channel;
  }

  bool disableChannelImpl(int pos) {
    // Populate channelsMask array
    int    max_retry        = 3;
    int    retry            = 0;
    String channel_mask_str = "";
    while (retry < max_retry) {
      channel_mask_str = thisModem().getChannelMask();
      if (channel_mask_str != "0") { break; }
      retry++;
    }
    uint16_t channelsMask[6];
    if (channel_mask_str.length() > 0) {
      sscanf(channel_mask_str.c_str(), "%04hx%04hx%04hx%04hx%04hx%04hx",
             &channelsMask[0], &channelsMask[1], &channelsMask[2],
             &channelsMask[3], &channelsMask[4], &channelsMask[5]);
    }

    int      row  = pos / 16;
    int      col  = pos % 16;
    uint16_t mask = ~(uint16_t)(1 << col);

    channelsMask[row] = channelsMask[row] & mask;

    return thisModem().setChannelMask();
  }

  bool enableChannelImpl(int pos) {
    // Populate channelsMask array
    int    max_retry        = 3;
    int    retry            = 0;
    String channel_mask_str = "";
    while (retry < max_retry) {
      channel_mask_str = thisModem().getChannelMask();
      if (channel_mask_str != "0") { break; }
      retry++;
    }
    uint16_t channelsMask[6];
    if (channel_mask_str.length() > 0) {
      sscanf(channel_mask_str.c_str(), "%04hx%04hx%04hx%04hx%04hx%04hx",
             &channelsMask[0], &channelsMask[1], &channelsMask[2],
             &channelsMask[3], &channelsMask[4], &channelsMask[5]);
    }

    int      row  = pos / 16;
    int      col  = pos % 16;
    uint16_t mask = (uint16_t)(1 << col);

    channelsMask[row] = channelsMask[row] | mask;

    return thisModem().setChannelMask();
  }

  bool setChannelMaskImpl() {
    String   newMask;
    uint16_t channelsMask[6];

    /* Convert channel mask into string */
    for (int i = 0; i < 6; i++) {
      char hex[4];
      sprintf(hex, "%04x", channelsMask[i]);
      newMask.concat(hex);
    }

    DBG("Newmask: ", newMask);

    return thisModem().setChannelMask(newMask);
  }

  bool setChannelMaskImpl(const char* newMask) TINY_LORA_ATTR_NOT_IMPLEMENTED;

  /*
   * LoRa Data Rate and Duty Cycle functions
   */
  bool   setDutyCycleImpl(int8_t dutyCycle) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  int8_t getDutyCycleImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   setDataRateImpl(uint8_t dataRate) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  int8_t getDataRateImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   setAdaptiveDataRateImpl(bool useADR) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool   getAdaptiveDataRateImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa ABP Session Properties
   */
  // aka network address
  String getDevAddrImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  // network session key
  String getNwkSKeyImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  // aka data session key
  String getAppSKeyImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa OTAA Session Properties
   */
  // aka network id
  String getAppEUIImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
  // aka network key
  String getAppKeyImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

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

  bool _requireAck;
  bool _networkConnected;
};

#endif  // SRC_TINYLORAMODEM_H_
