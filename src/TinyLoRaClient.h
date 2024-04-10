/**
 * @file       TinyLoRaClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TinyLoRaCLIENT_H_
#define SRC_TinyLoRaCLIENT_H_

#if defined(TINY_LORA_MDOT)
#include "TinyLoRaClientSIM800.h"
typedef TinyLoRaSim800                  TinyLoRa;
typedef TinyLoRaSim800::GsmClientSim800 TinyLoRaClient;

#else
#error "Please define LoRa Radio model"
#endif

#endif  // SRC_TinyLoRaCLIENT_H_
