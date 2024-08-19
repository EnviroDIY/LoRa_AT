/**
 * @file       LoRa_AT_Modem.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORAMODEM_H_
#define SRC_TINYLORAMODEM_H_

#include "LoRa_AT_Common.h"

#ifndef DEFAULT_JOIN_TIMEOUT
#define DEFAULT_JOIN_TIMEOUT 60000L
#endif

#ifndef DEFAULT_MESSAGE_TIMEOUT
#define DEFAULT_MESSAGE_TIMEOUT 10000L
#endif

#ifndef DEFAULT_ACKMESSAGE_TIMEOUT
#define DEFAULT_ACKMESSAGE_TIMEOUT 60000L
#endif

#define MAX_LORA_CHANNELS 72
#define LORA_CHANNEL_BYTES 72 / 8

#ifndef AT_NL
#define AT_NL "\r\n"
#endif

#ifndef AT_OK
#define AT_OK "OK"
#endif

#ifndef AT_ERROR
#define AT_ERROR "ERROR"
#endif

#if defined LORA_AT_DEBUG
#ifndef AT_VERBOSE
#define AT_VERBOSE "+LOG"
#endif
#endif

static char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static const char LORA_OK[] LORA_AT_PROGMEM    = AT_OK AT_NL;
static const char LORA_ERROR[] LORA_AT_PROGMEM = AT_ERROR AT_NL;

#if defined       LORA_AT_DEBUG
static const char LORA_VERBOSE[] LORA_AT_PROGMEM = AT_VERBOSE;
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
class LoRa_AT_Modem {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
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
   * @copydoc LoRa_AT_Modem::begin()
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
    LORA_AT_YIELD(); /* DBG("### AT:", cmd...); */
  }

  /**
   * @brief Set the module baud rate
   *
   * @param baud The baud rate the use
   *
   * @note After setting and applying the new baud rate, you will have to end()
   * and begin() the serial object.
   */
  bool setBaud(uint32_t baud) {
    return thisModem().setBaudImpl(baud);
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
                      GsmConstStr r1 = GFP(LORA_OK),
                      GsmConstStr r2 = GFP(LORA_ERROR),
                      GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                      GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                      GsmConstStr r7 = nullptr) {
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
  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(LORA_OK),
                      GsmConstStr r2 = GFP(LORA_ERROR),
                      GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                      GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                      GsmConstStr r7 = nullptr) {
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
  int8_t waitResponse(GsmConstStr r1 = GFP(LORA_OK),
                      GsmConstStr r2 = GFP(LORA_ERROR),
                      GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                      GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                      GsmConstStr r7 = nullptr) {
    return waitResponse(5000L, r1, r2, r3, r4, r5, r6, r7);
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
   * @note Requiring acknowledgement of every send can significantly slow down
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
   * @param appEui The App EUI (aka JoinEUI or Network ID). If using a string -
   * and supported by the module - set useHex to false. If using 8 bytes of hex
   * data, set useHex to true.
   * @param appKey The app key (Network key). If using a string, set useHex to
   * false. If using 16 bytes of hex data, set useHex to true.
   * @param devEui The device EUI. This must be 16 bytes of hex data.
   * @param useHex True if the appKey and appEUI are in hex; false for standard
   * strings
   * @param timeout A timeout to wait for successful join
   * @return *true* The network join was successful
   * @return *false* The network join failed
   */
  bool joinOTAA(const char* appEui, const char* appKey, const char* devEui,
                uint32_t timeout = DEFAULT_JOIN_TIMEOUT, bool useHex = true) {
    return thisModem().joinOTAAImpl(appEui, appKey, devEui, timeout, useHex);
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
  bool joinOTAA(String appEui, String appKey,
                uint32_t timeout = DEFAULT_JOIN_TIMEOUT, bool useHex = true) {
    return joinOTAA(appEui.c_str(), appKey.c_str(), nullptr, timeout, useHex);
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
                uint32_t timeout = DEFAULT_JOIN_TIMEOUT, bool useHex = false) {
    return joinOTAA(appEui.c_str(), appKey.c_str(), devEui.c_str(), timeout,
                    useHex);
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
               int uplinkCounter = 1, int downlinkCounter = 0,
               uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return thisModem().joinABPImpl(devAddr, nwkSKey, appSKey, uplinkCounter,
                                   downlinkCounter, timeout);
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
  bool joinABP(String devAddr, String nwkSKey, String appSKey,
               int uplinkCounter = 1, int downlinkCounter = 0,
               uint32_t timeout = DEFAULT_JOIN_TIMEOUT) {
    return joinABP(devAddr.c_str(), nwkSKey.c_str(), appSKey.c_str(),
                   uplinkCounter, downlinkCounter, timeout);
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
   * @brief Set the LoRaWAN outgoing application port
   *
   *  Port 0 is reserved for MAC commands, ports 1-223 are available for
   * application use, and port 233-255 are reserved for future LoRaWAN use.
   *
   * @param uint8_t The outgoing application port, must be a number from 1-255
   * @return *true* The port was successfully configured
   * @return *false* The module did not accept the port
   */
  bool setPort(uint8_t _port) {
    return thisModem().setPortImpl(_port);
  }
  /**
   * @brief Get the LoRaWAN outgoing application port
   *
   * @return *uint8_t* The outgoing application port
   */
  uint8_t getPort() {
    return thisModem().getPortImpl();
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
   * @return *String* The 16 or 72 bit channel mask - most significant byte
   * first (MSB)
   */
  String getChannelMask() {
    return thisModem().getChannelMaskImpl();
  }

  /**
   * @brief Check if a particular channel is enabled
   *
   * @param pos The channel number
   * @return *true* The channel was successfully enabled.
   * @return *false* The channel was not enabled.
   */
  bool isChannelEnabled(int pos) {
    return thisModem().isChannelEnabledImpl(pos);
  }

  /**
   * @brief Enables or disables a channel
   *
   * @param pos The channel number
   * @return *true* The channel was successfully enabled.
   * @return *false* The channel was not enabled.
   */
  bool enableChannel(int pos, bool enable = true) {
    return thisModem().enableChannelImpl(pos, enable);
  }

  /**
   * @brief Disables a channel
   *
   * @param pos The channel number
   * @return *true* The channel was successfully disabled.
   * @return *false* The channel was not disabled.
   */
  bool disableChannel(int pos) {
    return thisModem().enableChannelImpl(pos, false);
  }

  /**
   * @brief Sends a new channel mask to the device
   *
   * @param newMask A full new channel mask; most significant byte first (MSB)
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
   * @brief Enables duty cycle limitations.
   *
   * Set the maximum duty cycle using the setMaxDutyCycle(int8_t maxDutyCycle)
   * function.
   *
   * @param dutyCycle True to enable duty cycle limitations; false to disable
   * @return *true* The module successfully enabled or disabled duty cycle
   * limitations.
   * @return *false* There was an error in enabling or disabling the duty cycle.
   */
  bool enableDutyCycle(bool dutyCycle) {
    return thisModem().enableDutyCycleImpl(dutyCycle);
  }
  /**
   * @brief Checks if duty cycle limitations are enabled
   *
   * @return *true* Duty cycle limitations are enabled
   * @return *false* Duty cycle limitations are not enabled
   */
  bool isDutyCycleEnabled() {
    return thisModem().isDutyCycleEnabledImpl();
  }

  /**
   * @brief Set the LoRa module's maximum duty cycle
   *
   * This only sets the maximum duty cycle. To actually enable (or disable) the
   * duty cycle limits, call enableDutyCycle(bool dutyCycle).
   *
   * @see https://lora.readthedocs.io/en/latest/#duty-cycle-time-on-air-toa
   *
   * @param dutyCycle An int representing the maximum duty cycle, per the module
   * documentation
   * @return *true* The module accepted the new maximum duty cycle.
   * @return *false* There was an error in changing the maxiumum duty cycle.
   */
  bool setMaxDutyCycle(int8_t maxDutyCycle) {
    return thisModem().setMaxDutyCycleImpl(maxDutyCycle);
  }
  /**
   * @brief Get the current duty cycle for the LoRa module
   *
   * @see https://lora.readthedocs.io/en/latest/#duty-cycle-time-on-air-toa
   *
   * @return *int8_t* An int representing the duty cycle, per the module
   * documentation
   */
  int8_t getMaxDutyCycle() {
    return thisModem().getMaxDutyCycleImpl();
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
   * @anchor modem_crtp_helper
   * @name Modem CRTP Helper
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
  ~LoRa_AT_Modem() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Basic functions
   */
 protected:
  bool initImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool setBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+IPR="), baud);
    return thisModem().waitResponse() == 1;
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
                          GsmConstStr r1 = GFP(LORA_OK),
                          GsmConstStr r2 = GFP(LORA_ERROR),
                          GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                          GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                          GsmConstStr r7 = nullptr) {
    data.reserve(LORA_AT_RX_BUFFER);
#ifdef LORA_AT_DEBUG_DEEP
    DBG(GF("r1 <"), r1 ? r1 : GF("NULL"), GF("> r2 <"), r2 ? r2 : GF("NULL"),
        GF("> r3 <"), r3 ? r3 : GF("NULL"), GF("> r4 <"), r4 ? r4 : GF("NULL"),
        GF("> r5 <"), r5 ? r5 : GF("NULL"), GF("> r6 <"), r6 ? r6 : GF("NULL"),
        GF("> r7 <"), r7 ? r7 : GF("NULL"), '>');
#endif
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      LORA_AT_YIELD();
      while (thisModem().stream.available() > 0) {
        LORA_AT_YIELD();
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
        }
#if defined LORA_AT_DEBUG
        else if (data.endsWith(GFP(LORA_VERBOSE))) {
          // check how long the new line is
          // should be either 1 ('\r' or '\n') or 2 ("\r\n"))
          int len_atnl = strnlen(AT_NL, 3);
          // Read out the verbose message, until the last character of the new
          // line
          data += thisModem().stream.readStringUntil(AT_NL[len_atnl]);
#ifdef TINY_GSM_DEBUG_DEEP
          data.trim();
          DBG(GF("Verbose details <<<"), data, GF(">>>"));
#endif
          data = "";
          goto finish;
        }
#endif
        else if (thisModem().handleURCs(data)) {
          data = "";
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
#ifdef LORA_AT_DEBUG_DEEP
    data.replace("\r", "←");
    data.replace("\n", "↓");
#endif
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    } else {
#ifdef LORA_AT_DEBUG_DEEP
      DBG('<', index, '>', data);
#endif
    }
    return index;
  }

  String getDevEUIImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

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
  bool restartImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool powerOffImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool radioOffImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  bool setPublicNetworkImpl(bool isPublic) LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool getPublicNetworkImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool
  setConfirmationRetriesImpl(int8_t numAckRetries) LORA_AT_ATTR_NOT_IMPLEMENTED;
  int8_t getConfirmationRetriesImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool joinOTAAImpl(const char* appEui, const char* appKey, const char* devEui,
                    uint32_t timeout, bool useHex) LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool joinABPImpl(
      const char* devAddr, const char* nwkSKey, const char* appSKey,
      int uplinkCounter = 1, int downlinkCounter = 0,
      uint32_t timeout = DEFAULT_JOIN_TIMEOUT) LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool isNetworkConnectedImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  int8_t getSignalQualityImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa Class and Band functions
   */

  bool        setClassImpl(_lora_class _class) LORA_AT_ATTR_NOT_IMPLEMENTED;
  _lora_class getClassImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool    setPortImpl(uint8_t _port) LORA_AT_ATTR_NOT_IMPLEMENTED;
  uint8_t getPortImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool   setBandImpl(const char* band) LORA_AT_ATTR_NOT_IMPLEMENTED;
  String getBandImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool   setFrequencySubBandImpl(int8_t subBand) LORA_AT_ATTR_NOT_IMPLEMENTED;
  int8_t getFrequencySubBandImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  String getChannelMaskImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool isChannelEnabledImpl(int pos) {
    // Populate channelsMask array
    int    max_retry        = 3;
    int    retry            = 0;
    String channel_mask_str = "";
    while (retry < max_retry) {
      channel_mask_str = thisModem().getChannelMask();
      if (channel_mask_str.length() > 1) { break; }
      retry++;
    }
    uint8_t channelsMask[LORA_CHANNEL_BYTES];
    // parse the hex mask into a bit array
    parseChannelMask(channel_mask_str.c_str(), channelsMask);
    // get the channel possition in the array
    int row = getChannelOffset(pos);
    // convert the channel position into a mask
    uint8_t channel = getChannelBitMask(pos);

    bool channelEnabled = (channelsMask[row] & channel) > 0;

    return channelEnabled;
  }

  bool enableChannelImpl(int pos, bool enable) {
    // Populate channelsMask array
    int    max_retry        = 3;
    int    retry            = 0;
    String channel_mask_str = "";
    while (retry < max_retry) {
      channel_mask_str = thisModem().getChannelMask();
      if (channel_mask_str != "0") { break; }
      retry++;
    }

    uint8_t channelsMask[LORA_CHANNEL_BYTES] = {0};
    // parse the hex mask into a bit array
    parseChannelMask(channel_mask_str.c_str(), channelsMask);

    // get the channel possition in the array
    int row = getChannelOffset(pos);
    // convert the channel position into a mask
    uint8_t mask = getChannelBitMask(pos);
    // DBG("Channel Mask:", dbg_print_bin(mask));
    // DBG("Row Mask    :", dbg_print_bin(channelsMask[row]));
    if (enable) {
      // or with the mask if we're enabling the channel
      channelsMask[row] = channelsMask[row] | mask;
    } else {
      // and with the inverted mask if we're disabling the channel
      channelsMask[row] = channelsMask[row] & ~mask;
    }
    // DBG("Masked Row  :", dbg_print_bin(channelsMask[row]),
    //     enable ? "(enabling)" : "(disabling)");

    return thisModem().setChannelMask(createHexChannelMask(channelsMask));
  }

  bool setChannelMaskImpl(const char* newMask) LORA_AT_ATTR_NOT_IMPLEMENTED;

  /*
   * LoRa Data Rate and Duty Cycle functions
   */
  bool   enableDutyCycleImpl(bool dutyCycle) LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool   isDutyCycleEnabledImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool   setMaxDutyCycleImpl(int8_t maxDutyCycle) LORA_AT_ATTR_NOT_IMPLEMENTED;
  int8_t getMaxDutyCycleImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool   setDataRateImpl(uint8_t dataRate) LORA_AT_ATTR_NOT_IMPLEMENTED;
  int8_t getDataRateImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool   setAdaptiveDataRateImpl(bool useADR) LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool   getAdaptiveDataRateImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa ABP Session Properties
   */
  // aka network address
  String getDevAddrImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  // network session key
  String getNwkSKeyImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  // aka data session key
  String getAppSKeyImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;


  /*
   * LoRa OTAA Session Properties
   */
  // aka network id
  String getAppEUIImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  // aka network key
  String getAppKeyImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

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
      thisModem().waitResponse(50, nullptr, nullptr);
    }
  }

  inline void streamDump() {
    LORA_AT_YIELD();
    while (thisModem().stream.available()) {
      thisModem().stream.read();
      LORA_AT_YIELD();
    }
  }

 protected:
  inline bool streamFind(char target) {
    return thisModem().stream.find(const_cast<char*>(&target), 1);
  }

#if !defined(LORA_AT_SEND_PLAIN)
/**
 * @brief A flag to force data to be sent as characters instead of as hex
 * values.
 *
 * When using AT commands, writing data as charaters can cause the AT command to
 * fail if there's a new line or carriage return in the message. The CR/NL is
 * interpreted as the end of the send command rather than as part of the
 * message. Because LoRa messages are compressed, there may be NL/CR returns in
 * the text.
 */
#define LORA_AT_SEND_HEX
  // A function to convert a character/uint8_t buffer to hex
  void writeHex(const uint8_t* buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
      if (buf[i] < 16) thisModem().stream.print('0');
      thisModem().stream.print(buf[i], HEX);
    }
  }
  // A function to convert a character/uint8_t buffer to hex
  void writeHex(uint8_t* buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
      if (buf[i] < 16) thisModem().stream.print('0');
      thisModem().stream.print(buf[i], HEX);
    }
  }
#else
#define writeHex(...)
#endif

  void parseChannelMask(const char* mask, uint8_t* channelsMask) {
    // the input mask must be MSB - most significant first
    if (strlen(mask) > 0) {
      size_t startByte = 0;
      // If we got a 20 character mask, assume the first two bytes are 00's and
      // skip them. This will be the case for the mDot.
      if (strlen(mask) == 20) { startByte = 2; }
      for (size_t i = startByte; i < strlen(mask); i += 2) {
        char hexBuff[3] = {'\0'};
        hexBuff[0]      = mask[i];
        hexBuff[1]      = mask[i + 1];
        // DBG(strlen(mask), startByte, i, ((strlen(mask) - i) / 2) - 1,
        //     (int8_t)strtol(hexBuff, nullptr, 16),
        //     dbg_print_bin((int8_t)strtol(hexBuff, nullptr, 16)));
        channelsMask[((strlen(mask) - i) / 2) - 1] =
            (int8_t)strtol(hexBuff, nullptr, 16);
      }
      // #ifdef LORA_AT_DEBUG
      //       DBG(GF("Input mask:"), mask);
      //       LORA_AT_DEBUG.print(GF("Input mask in binary: "));
      //       for (int8_t j = 0; j < LORA_CHANNEL_BYTES; j++) {
      //         LORA_AT_DEBUG.print(dbg_print_bin(channelsMask[j]));
      //         LORA_AT_DEBUG.print(',');
      //       }
      //       LORA_AT_DEBUG.println();
      // #endif
    }
  }

  String createHexChannelMask(uint8_t* channelsMask) {
    // #ifdef LORA_AT_DEBUG
    //     LORA_AT_DEBUG.print(GF("Mask array in binary:"));
    //     for (int8_t j = 0; j < LORA_CHANNEL_BYTES; j++) {
    //       LORA_AT_DEBUG.print(dbg_print_bin(channelsMask[j]));
    //       LORA_AT_DEBUG.print(',');
    //     }
    //     LORA_AT_DEBUG.println();
    // #endif
    // convert the completed channel mask to a string, switching endian-ness
    String resp = "";
    for (int8_t i = LORA_CHANNEL_BYTES - 1; i > -1; i--) {
      if (channelsMask[i] < 16) { resp += '0'; }  // must zero pad!
      resp += String(channelsMask[i], HEX);
      // DBG(i, channelsMask[i], dbg_print_bin(channelsMask[i]),
      //     String(channelsMask[i], HEX));
    }
    // DBG(GF("Mask array in HEX:"), resp);
    return resp;
  }

  uint8_t getChannelBitMask(uint8_t channelNumber) {
    // get the channel possition in the array
    int col = channelNumber % 8;
    // convert the channel position into a mask
    uint8_t channel = (uint8_t)(1 << col);
    // #ifdef LORA_AT_DEBUG
    //     int row = channelNumber / 8;
    //     DBG(GF("\nChannel Number:"), channelNumber, GF("Row:"), row,
    //     GF("Col:"),
    //         col);
    //     LORA_AT_DEBUG.print(GF("Channel Mask:         "));
    //     for (int8_t j = 0; j < LORA_CHANNEL_BYTES; j++) {
    //       if (j == row) {
    //         LORA_AT_DEBUG.print(dbg_print_bin(channel));
    //       } else {
    //         LORA_AT_DEBUG.print("00000000");
    //       }
    //       LORA_AT_DEBUG.print(',');
    //     }
    //     LORA_AT_DEBUG.println();
    // #endif
    return channel;
  }

  uint8_t getChannelOffset(uint8_t channelNumber) {
    // get the channel possition in the array
    int row = channelNumber / 8;
    return row;
  }

  //   String dbg_print_bin(uint8_t num) {
  // #ifdef LORA_AT_DEBUG
  //     String binPrint = "";
  //     int8_t bin_len  = String(num, BIN).length();
  //     for (int8_t z = 0; z < 8 - bin_len; z++) { binPrint += "0"; }
  //     binPrint += String(num, BIN);
  //     return binPrint;
  // #endif
  // }

  bool _networkConnected;
};

#endif  // SRC_TINYLORAMODEM_H_
