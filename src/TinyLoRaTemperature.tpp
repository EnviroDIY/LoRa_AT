/**
 * @file       TinyLoRaTemperature.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYLORATEMPERATURE_H_
#define SRC_TINYLORATEMPERATURE_H_

#include "TinyLoRaCommon.h"

#define TINY_LORA_HAS_TEMPERATURE

template <class modemType>
class TinyLoRaTemperature {
 public:
  /*
   * Temperature functions
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

  float getTemperatureImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYLORATEMPERATURE_H_
