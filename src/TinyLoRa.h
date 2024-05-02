/**
 * @file       TinyLoRa.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYLORA_H_
#define SRC_TINYLORA_H_

#if defined(TINY_LORA_MDOT)
#include "TinyLoRa_MDOT.h"
typedef TinyLoRa_mDOT                  TinyLoRa;
typedef TinyLoRa_mDOT::LoRaStream_mDOT TinyLoRaStream;

#elif defined(TINY_LORA_LORAE5)
#include "TinyLoRa_LoRaE5.h"
typedef TinyLoRa_LoRaE5                    TinyLoRa;
typedef TinyLoRa_LoRaE5::LoRaStream_LoRaE5 TinyLoRaStream;

#else
#error "Please define LoRa Radio model"
#endif

#endif  // SRC_TINYLORA_H_
