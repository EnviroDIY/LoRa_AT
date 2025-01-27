/**
 * @file       LoRa_AT_Temperature.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORATEMPERATURE_H_
#define SRC_TINYLORATEMPERATURE_H_

#include "LoRa_AT_Common.h"

#define LORA_AT_HAS_TEMPERATURE

template <class modemType>
class LoRa_AT_Temperature {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Temperature functions
   */

  /**
   * @brief Get the modem chip temperature in degrees celsius.
   *
   * @return The modem chip temperature in degrees celsius.
   */
  float getTemperature() {
    return thisModem().getTemperatureImpl();
  }

  /**
   * @anchor temperature_crtp_helper
   * @name Temperature CRTP Helper
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
  ~LoRa_AT_Temperature() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Temperature functions
   */

  float getTemperatureImpl() LORA_AT_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYLORATEMPERATURE_H_
