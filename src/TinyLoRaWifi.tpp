/**
 * @file       TinyLoRaWifi.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TinyLoRaWIFI_H_
#define SRC_TinyLoRaWIFI_H_

#include "TinyLoRaCommon.h"

#define TINY_LORA_HAS_WIFI

template <class modemType>
class TinyLoRaWifi {
 public:
  /*
   * WiFi functions
   */
  bool networkConnect(const char* ssid, const char* pwd) {
    return thisModem().networkConnectImpl(ssid, pwd);
  }
  bool networkDisconnect() {
    return thisModem().networkDisconnectImpl();
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
   * WiFi functions
   */

  bool networkConnectImpl(const char* ssid,
                          const char* pwd) TINY_LORA_ATTR_NOT_IMPLEMENTED;
  bool networkDisconnectImpl() TINY_LORA_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TinyLoRaWIFI_H_
