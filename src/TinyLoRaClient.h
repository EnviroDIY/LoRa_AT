/**
 * @file       TinyLoRaClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYLORACLIENT_H_
#define SRC_TINYLORACLIENT_H_

#if defined(TINY_LORA_MDOT)
#include "TinyLoRaClientMDOT.h"
typedef TinyLoRamDOT                 TinyLoRa;
typedef TinyLoRamDOT::LoRaStreammDOT TinyLoRaClient;

#else
#error "Please define LoRa Radio model"
#endif

#endif  // SRC_TINYLORACLIENT_H_
