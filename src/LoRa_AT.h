/**
 * @file       LoRa_AT.h
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_LORA_AT_H_
#define SRC_LORA_AT_H_

#if defined(LORA_AT_MDOT)
#include "LoRa_AT_mDOT.h"
typedef LoRa_AT_mDOT                  LoRa_AT;
typedef LoRa_AT_mDOT::LoRaStream_mDOT LoRaStream;

#elif defined(LORA_AT_WIOE5) || defined(LORA_AT_LORAE5)
#include "LoRa_AT_WioE5.h"
typedef LoRa_AT_WioE5                   LoRa_AT;
typedef LoRa_AT_WioE5::LoRaStream_WioE5 LoRaStream;

#else
#error "Please define LoRa Radio model"
#endif

#endif  // SRC_LORA_AT_H_
