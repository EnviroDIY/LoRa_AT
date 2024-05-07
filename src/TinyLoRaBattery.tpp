/**
 * @file       TinyLoRaBattery.tpp
 * @author     Sara Damiano
 * @license    LGPL-3.0
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORABATTERY_H_
#define SRC_TINYLORABATTERY_H_

#include "TinyLoRaCommon.h"

#define TINY_LORA_HAS_BATTERY

template <class modemType>
class TinyLoRaBattery {
 public:
  /*
   * Battery functions
   */
  uint16_t getBattVoltage() {
    return thisModem().getBattVoltageImpl();
  }
  int8_t getBattPercent() {
    return thisModem().getBattPercentImpl();
  }
  uint8_t getBattChargeState() {
    return thisModem().getBattChargeStateImpl();
  }
  bool getBattStats(int8_t& chargeState, int8_t& percent, int16_t& milliVolts) {
    return thisModem().getBattStatsImpl(chargeState, percent, milliVolts);
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
   * Battery functions
   */
 protected:
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltageImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  int8_t getBattPercentImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  uint8_t getBattChargeStateImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) TINY_LORA_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYLORABATTERY_H_
