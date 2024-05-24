/**
 * @file       TinyLoRaTemperature.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORATEMPERATURE_H_
#define SRC_TINYLORATEMPERATURE_H_

#include "TinyLoRaCommon.h"

#define TINY_LORA_HAS_TEMPERATURE

template <class modemType>
class TinyLoRaTemperature {
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
   * @return *float* The modem chip temperature in degrees celsius.
   */
  float getTemperature() {
    return thisModem().getTemperatureImpl();
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
  ~TinyLoRaTemperature() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Temperature functions
   */

  float getTemperatureImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYLORATEMPERATURE_H_
