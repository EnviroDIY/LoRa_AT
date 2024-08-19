/**
 * @file       LoRa_AT_Common.h
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORACOMMON_H_
#define SRC_TINYLORACOMMON_H_

// The current library version number
#define LORA_AT_VERSION "0.3.0"

#if defined(SPARK) || defined(PARTICLE)
#include "Particle.h"
#elif defined(ARDUINO)
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#endif

#ifndef LORA_AT_YIELD_MS
/**
 * @brief The length of time in milliseconds to "yield" waiting for a new
 * character to arrive from the modem stream. This may be needed for slow baud
 * rates.
 */
#define LORA_AT_YIELD_MS 0
#endif

#ifndef LORA_AT_YIELD
/**
 * @brief A function to yield by delaying #LORA_AT_YIELD_MS.
 */
#define LORA_AT_YIELD() \
  { delay(LORA_AT_YIELD_MS); }
#endif

#if !defined(LORA_AT_RX_BUFFER)
/**
 * @brief The length of the receiving buffer.
 */
#define LORA_AT_RX_BUFFER 256
#endif

#if !defined(LORA_AT_DL_CHECK)
/**
 * @brief How frequently to check for downlinks in the maintain function.
 *
 * @warning It is NOT good practice in LoRaWAN to check frequently for
 * downlinks.  For this reason, the maintain function should not be regularly
 * used.
 */
#define LORA_AT_DL_CHECK 30000L
#endif

#define LORA_AT_ATTR_NOT_AVAILABLE \
  __attribute__((error("Not available on this modem type")))
#define LORA_AT_ATTR_NOT_IMPLEMENTED __attribute__((error("Not implemented")))

#if defined(__AVR__) && !defined(__AVR_ATmega4809__)
#define LORA_AT_PROGMEM PROGMEM
typedef const __FlashStringHelper* GsmConstStr;
#define GFP(x) (reinterpret_cast<GsmConstStr>(x))
#define GF(x) F(x)
#else
#define LORA_AT_PROGMEM
typedef const char* GsmConstStr;
#define GFP(x) x
#define GF(x) x
#endif

#ifdef LORA_AT_DEBUG
namespace {
template <typename T>
static void DBG_PLAIN(T last) {
  LORA_AT_DEBUG.println(last);
}

template <typename T, typename... Args>
static void DBG_PLAIN(T head, Args... tail) {
  LORA_AT_DEBUG.print(head);
  LORA_AT_DEBUG.print(' ');
  DBG_PLAIN(tail...);
}

template <typename... Args>
static void DBG(Args... args) {
  LORA_AT_DEBUG.print(GF("["));
  LORA_AT_DEBUG.print(millis());
  LORA_AT_DEBUG.print(GF("] "));
  DBG_PLAIN(args...);
}
}  // namespace
#else
#define DBG_PLAIN(...)
#define DBG(...)
#endif

template <class T>
const T& LoRa_AT_Min(const T& a, const T& b) {
  return (b < a) ? b : a;
}

template <class T>
const T& LoRa_AT_Max(const T& a, const T& b) {
  return (b < a) ? a : b;
}


/*
 * Automatically find baud rate
 */
template <class T>
uint32_t LoRa_AT_AutoBaud(T& SerialAT, uint32_t minimum = 9600,
                          uint32_t maximum = 115200) {
  static uint32_t rates[] = {115200, 57600,  38400, 19200, 9600,  74400, 74880,
                             230400, 460800, 2400,  4800,  14400, 28800};

  for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
    uint32_t rate = rates[i];
    if (rate < minimum || rate > maximum) continue;

    DBG("Trying baud rate", rate, "...");
    SerialAT.begin(rate);
    delay(10);
    for (int j = 0; j < 10; j++) {
      SerialAT.print("AT\r\n");
      String input = SerialAT.readString();
      if (input.indexOf("OK") >= 0) {
        DBG("Modem responded at rate", rate);
        return rate;
      }
    }
  }
  SerialAT.begin(minimum);
  return 0;
}

#endif  // SRC_TINYLORACOMMON_H_
