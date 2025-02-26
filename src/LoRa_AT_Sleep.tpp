/**
 * @file       LoRa_AT_Sleep.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_LORA_AT_SLEEP_H_
#define SRC_LORA_AT_SLEEP_H_

#include "LoRa_AT_Common.h"

#define LORA_AT_HAS_SLEEP_MODE

template <class modemType>
class LoRa_AT_Sleep {
 public:
  /*
   * Sleep functions
   */

  /**
   * @brief Put the module to sleep, starting from the time the command is
   * issued and ending when the module is woken by the designated interrupt pin.
   *
   * What "sleep" means varies by module; check your documentation.
   *
   * @note THIS IS THE PIN ON THE MODULE, NOT THE CONTROLLER!
   *
   * @param pin The pin number
   * @param pullupMode The pin's pullup mode (0=NOPULL, 1=PULLUP, 2=PULLDOWN)
   * @param trigger The wake trigger (ie, 0=ANY, 1=RISE, 2=FALL)
   *
   * @return True if the module accepted the sleep mode setting; false if there
   * was an error in setting the sleep mode.
   */
  bool pinSleep(int8_t pin, int8_t pullupMode = -1, int8_t trigger = -1) {
    return thisModem().pinSleepImpl(pin, pullupMode, trigger);
  }

  /**
   * @brief Put the module to sleep, starting from the time the command is
   * issued and module is woken by the UART .
   *
   * What "sleep" means varies by module; check your documentation.
   *
   * @return True if the module accepted the sleep mode setting; false if there
   * was an error in setting the sleep mode.
   */
  bool uartSleep() {
    return thisModem().uartSleepImpl();
  }

  /**
   * @brief Put the module to sleep, starting from the time the command is
   * issued and ending when the module is woken by either the UART or the
   * designated interrupt pin.
   *
   * What "sleep" means varies by module; check your documentation.
   *
   * @return True if the module accepted the sleep mode setting; false if there
   * was an error in setting the sleep mode.
   */
  bool sleep(bool uart_wake = true) {
    if (uart_wake) { return thisModem().uartSleepImpl(); }
    return thisModem().pinSleepImpl();
  }

  /**
   * @brief Put the module to sleep, starting from the time the command is
   * issued and ending when sleep timer expires.
   *
   * What "sleep" means varies by module; check your documentation.
   *
   * @param sleepTimer The time for the module to sleep, in milliseconds.
   * @return True if the module accepted the sleep mode setting; false if there
   * was an error in setting the sleep mode.
   */
  bool sleep(uint32_t sleepTimer) {
    return thisModem().sleepImpl(sleepTimer);
  }

  /**
   * @brief Enable or disable automatic sleep mode for the module.
   *
   * What "automatic sleep" means varies by module; check your documentation.
   * Usually this means the module will enter lowest power mode until woken by
   * the UART and will go back to sleep as soon as UART communication stops.
   *
   * @param enable True to enable automatic sleep mode, false to disable it.
   * @return True if the module accepted the automatic sleep mode setting; false
   * if there was an error in setting the automatic sleep mode.
   */
  bool enableAutoSleep(bool enable = true) {
    return thisModem().enableAutoSleepImpl(enable);
  }

  /**
   * @brief Disable automatic sleep mode for the module.
   *
   * What "automatic sleep" means varies by module; check your documentation.
   * Usually this means the module will enter lowest power mode until woken by
   * the UART and will go back to sleep as soon as UART communication stops.
   */
  bool disableAutoSleep() {
    return thisModem().enableAutoSleepImpl(false);
  }

  /**
   * @anchor sleep_crtp_helper
   * @name Sleep CRTP Helper
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
  ~LoRa_AT_Sleep() {}

  bool pinSleepImpl(int8_t pin, int8_t pullupMode,
                    int8_t trigger) LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool uartSleepImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool sleepImpl(uint32_t sleepTimer) LORA_AT_ATTR_NOT_IMPLEMENTED;
  bool enableAutoSleepImpl(bool enable = true) LORA_AT_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_LORA_AT_SLEEP_H_
