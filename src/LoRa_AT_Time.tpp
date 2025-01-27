/**
 * @file       LoRa_AT_Time.tpp
 * @author     Sara Damiano
 * @copyright  Stroud Water Research Center
 * @date       May 2024
 */

#ifndef SRC_TINYLORATIME_H_
#define SRC_TINYLORATIME_H_

#include "LoRa_AT_Common.h"

#define LORA_AT_HAS_TIME

enum LoRa_AT_DateTimeFormat { DATE_FULL = 0, DATE_TIME = 1, DATE_DATE = 2 };
/**
 * @brief Set the epoch start value.
 */
enum LoRa_AT_EpochStart {
  UNIX = 0,  ///< Use a Unix epoch, starting 1/1/1970 (946684800s behind of Y2K
             ///< epoch, 315878400ss behind of GPS epoch)
  Y2K = 1,   ///< Use an epoch starting 1/1/2000, as some RTC's and Arduinos do
            ///< (946684800s ahead of UNIX epoch, 630806400s ahead of GPS epoch)
  GPS = 2  ///< Use the GPS epoch starting Jan 5, 1980 (315878400s ahead of UNIX
           ///< epoch, 630806400s behind of Y2K epoch)
};

template <class modemType>
class LoRa_AT_Time {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Time functions
   */

  /**
   * @brief Get the Date Time as a String
   *
   * @param format The date or time part to get: DATE_FULL,
   * DATE_TIME, or DATE_DATE
   * @return The date and/or time from the module
   */
  String getDateTimeString(LoRa_AT_DateTimeFormat format) {
    return thisModem().getDateTimeStringImpl(format);
  }

  /**
   * @brief Get the date and time as parts
   *
   * @param year Reference to an int for the year
   * @param month Reference to an int for the month
   * @param day Reference to an int for the day
   * @param hour Reference to an int for the hour
   * @param minute Reference to an int for the minute
   * @param second Reference to an int for the second
   * @param timezone Reference to a float for the timezone
   * @return True if the references have been filled with valid values from the
   * LoRa module.; false if there was a problem getting the time from the
   * module.
   */
  bool getDateTimeParts(int* year, int* month, int* day, int* hour, int* minute,
                        int* second, float* timezone) {
    return thisModem().getDateTimePartsImpl(year, month, day, hour, minute,
                                            second, timezone);
  }

  /**
   * @brief Get the Date Time as an epoch value
   * @param epoch The epoch start to use.
   * @return The offset from the start of the epoch
   */
  uint32_t getDateTimeEpoch(LoRa_AT_EpochStart epoch = UNIX) {
    return thisModem().getDateTimeEpochImpl(epoch);
  }

  /**
   * @anchor time_crtp_helper
   * @name Time CRTP Helper
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
  ~LoRa_AT_Time() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Time functions
   */
 protected:
  String getDateTimeStringImpl(LoRa_AT_DateTimeFormat format)
      LORA_AT_ATTR_NOT_IMPLEMENTED;

  bool getDateTimePartsImpl(int* year, int* month, int* day, int* hour,
                            int* minute, int* second,
                            float* timezone) LORA_AT_ATTR_NOT_IMPLEMENTED;

  uint32_t getDateTimeEpochImpl(LoRa_AT_EpochStart epoch = UNIX)
      LORA_AT_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYLORATIME_H_
