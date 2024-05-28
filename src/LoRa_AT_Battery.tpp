/**
 * @file       LoRa_AT_Battery.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORABATTERY_H_
#define SRC_TINYLORABATTERY_H_

#include "LoRa_AT_Common.h"

#define LORA_AT_HAS_BATTERY

template <class modemType>
class LoRa_AT_Battery {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Battery functions
   */

  /**
   * @brief Get the current battery voltage.
   *
   * @note Unless you have a battery directly connected to your modem module,
   * this will be the input voltage going to the module from your main processor
   * board, not the battery voltage of your main processor.
   *
   * @return *int16_t*  The battery voltage measured by the modem module.
   */
  int16_t getBattVoltage() {
    return thisModem().getBattVoltageImpl();
  }

  /**
   * @brief Get the current battery percent.
   *
   * @note Unless you have a battery directly connected to your modem module,
   * this will be the percent from the input voltage going to the module from
   * your main processor board, not the battery percent of your main processor.
   *
   * @return *int8_t*  The current battery percent.
   */
  int8_t getBattPercent() {
    return thisModem().getBattPercentImpl();
  }

  /**
   * @brief Get the battery charging state.
   *
   * @return *int8_t* The battery charge state.
   */
  int8_t getBattChargeState() {
    return thisModem().getBattChargeStateImpl();
  }

  /**
   * @brief Get the all battery state
   *
   * @param chargeState A reference to an int to set to the battery charge state
   * @param percent A reference to an int to set to the battery percent
   * @param milliVolts A reference to an int to set to the battery voltage
   * @return *true* The battery stats were updated by the module.
   * @return *false* There was a failure in updating the battery stats from the
   * module.
   */
  bool getBattStats(int8_t& chargeState, int8_t& percent, int16_t& milliVolts) {
    return thisModem().getBattStatsImpl(chargeState, percent, milliVolts);
  }

  /**
   * @anchor battery_crtp_helper
   * @name Battery CRTP Helper
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
  ~LoRa_AT_Battery() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Battery functions
   */
 protected:
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  int16_t getBattVoltageImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  int8_t getBattPercentImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  int8_t getBattChargeStateImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) LORA_AT_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYLORABATTERY_H_
