/**
 * @file       LoRa_AT_Radio.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_LORA_AT_RADIO_H_
#define SRC_LORA_AT_RADIO_H_

#include "LoRa_AT_Common.h"

#define LORA_AT_HAS_RADIO

#include "TinyGsmFifo.h"

template <class modemType>
class LoRa_AT_Radio {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Basic functions
   */

  /**
   * @brief Checks for new downlinks from the LoRaWAN network at the frequency
   * defined by #LORA_AT_DL_CHECK.
   *
   * @warning It is NOT good practice in LoRaWAN to check frequently for
   * downlinks.  For this reason, the maintain function should not be regularly
   * used.
   *
   */
  void maintain() {
    return thisModem().maintainImpl();
  }

  /**
   * @brief Set the LoRa module to require confirmation (ACK) or messages, or
   * not.
   *
   * @note Requiring acknowledgement of every send can significantly slow down
   * the send time.
   *
   * @param requireConfirmation True to require that uplink messages be
   * confirmed
   */
  void requireConfirmation(bool requireConfirmation) {
    _requireConfirmation = requireConfirmation;
  }

  /**
   * @brief Check whether the module is asking for confirmation of uplink
   * messages or not.
   *
   * @return True if confirmation is being requested for all uplinks; false if
   * confirmation is NOT being requested for all uplinks.
   */
  bool isConfrirmationRequired() {
    return _requireConfirmation;
  }

  /**
   * @anchor radio_crtp_helper
   * @name Radio CRTP Helper
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
  ~LoRa_AT_Radio() {}

  /*
   * Inner Stream
   * NOTE:  This differs from TinyGSM in that it is a **Stream** instance, not a
   * **Client** instance!
   */
 public:
  class LoRaStream : public Stream {
    // Make all classes created from the modem template friends
    friend class LoRa_AT_Radio<modemType>;
    typedef TinyGsmFifo<uint8_t, LORA_AT_RX_BUFFER> RxFifo;

   public:
    // bool init(modemType* modem, uint8_t);

    // Writes data out on the client using the modem send functionality
    size_t write(const uint8_t* buf, size_t size) override {
      return at->modemSend(buf, size);
    }

    size_t write(uint8_t c) override {
      return write(&c, 1);
    }

    size_t write(const char* str) {
      if (str == nullptr) return 0;
      return write(reinterpret_cast<uint8_t*>(str), strlen(str));
    }

    int available() override {
      LORA_AT_YIELD();
      // Returns the combined number of characters available in the LoRa_AT_
      // fifo, doing an extra check-in with the modem to see if anything has
      // arrived that wasn't presented immediately after the last uplink.
      if (!rx.size()) { at->maintain(); }
      return static_cast<uint16_t>(rx.size());
    }

    int read(uint8_t* buf, size_t size) {
      LORA_AT_YIELD();
      size_t cnt = 0;
      // Reads characters out of the TinyGSM fifo
      uint32_t _startMillis = millis();
      while (cnt < size && millis() - _startMillis < _timeout) {
        size_t chunk = LoRa_AT_Min(size - cnt, rx.size());
        if (chunk > 0) {
          rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        } /* TODO: Read directly into user buffer? */
        if (!rx.size() && at->_networkConnected) {
          // this will check for new downlinks if it's been too long since the
          // last downlink check, the network has been joined, and we haven't
          // received and 'not joined' Errors/URCs.
          at->maintain();
        }
      }
      return cnt;
    }

    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

    int peek() override {
      return (uint8_t)rx.peek();
    }

    void flush() override {
      at->stream.flush();
    }

    /*
     * Extended API
     */


   protected:
    // Read and dump anything remaining in the modem's internal buffer.
    // Using this in the client stop() function.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the
    // data that it wants from the socket even if it was closed externally.
    inline void dumpModemBuffer(uint32_t maxWaitMs) {
      LORA_AT_YIELD();
      uint32_t startMillis = millis();
      while (sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
        rx.clear();
        at->modemRead();
      }
      rx.clear();
      at->streamClear();
    }

    modemType* at;
    uint16_t   sock_available;
    RxFifo     rx;
  };

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Basic functions
   */
 protected:
  void maintainImpl() {
    // Check for any new downlinks
    if (millis() - prev_dl_check > LORA_AT_DL_CHECK &&
        thisModem()._networkConnected) {
      thisModem().modemRead();  // modemRead should set prev_dl_check
    }
    // listen for URCs
    while (thisModem().stream.available()) {
      thisModem().waitResponse(15, nullptr, nullptr);
    }
  }

  // Yields up to a time-out period and then reads a character from the stream
  // into the multicast FIFO
  inline void moveCharFromStreamToFifo() {
    if (!thisModem().loraStream) return;
    uint32_t startMillis = millis();
    while (!thisModem().stream.available() &&
           (millis() - startMillis < thisModem().loraStream->_timeout)) {
      LORA_AT_YIELD();
    }
    char c = thisModem().stream.read();
    thisModem().loraStream->rx.put(c);
  }

  uint32_t prev_dl_check;
  bool     _requireConfirmation;
};

#endif  // SRC_LORA_AT_RADIO_H_
