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
             ///< epoch, 315964800s (minus leap seconds) behind of GPS epoch)
  Y2K = 1,   ///< Use an epoch starting 1/1/2000, as some RTC's and Arduinos do
             ///< (946684800s ahead of UNIX epoch, 630720000s (minus leap
             ///< seconds) ahead of GPS epoch)
  GPS = 2    ///< Use the GPS epoch starting Jan 5, 1980 (315964800s (plus leap
             ///< seconds) ahead of UNIX epoch, 630720000s behind of Y2K epoch)
};

#ifndef NUMBER_LEAP_SECONDS
/**
 * @brief The number of announced leap seconds as of February 24, 2025
 */
#define NUMBER_LEAP_SECONDS 18
#endif
#ifndef LEAP_SECONDS
/**
 * @brief The GPS epoch equivalent for each of the announced leap seconds as of
 * February 24, 2025
 */
#define LEAP_SECONDS                                                   \
  {46828800,  78364801,  109900802, 173059203,  252028804,  315187205, \
   346723206, 393984007, 425520008, 457056009,  504489610,  551750411, \
   599184012, 820108813, 914803214, 1025136015, 1119744016, 1167264017}
#endif

/**
 * @brief Functions for converting between GSP and Unix epoch, taking leap
 * seconds into account
 *
 * Taken from https://www.andrews.edu/~tzs/timeconv/timealgorithm.html
 *
 * > [W]hile there was an offset of 315964800 seconds between Unix and GPS
 * > time when GPS time began, that offset changes each time there is a leap
 * > second. GPS time labels each second uniquely including leap seconds while
 * > Unix time does not, preferring to count a constant number of seconds a
 * > day including those containing leap seconds.
 */
class GPSTimeConversion {
 public:
  // Convert Unix Time to GPS Time
  static uint32_t unix2gps(uint32_t unixTime) {
    // Add offset in seconds
    bool isLeap;
    if (fmod(unixTime, 1) != 0) {
      unixTime = unixTime - 0.5;
      isLeap   = 1;
    } else {
      isLeap = 0;
    }
    uint32_t gpsTime = unixTime - 315964800;
    int8_t   nLeaps  = countLeaps(gpsTime, true);
    gpsTime          = gpsTime + nLeaps + isLeap;
    return gpsTime;
  }

  // Convert GPS Time to Unix Time
  static uint32_t gps2unix(uint32_t gpsTime) {
    // Add offset in seconds
    uint32_t unixTime = gpsTime + 315964800;
    int8_t   nLeaps   = countLeaps(gpsTime, false);
    unixTime          = unixTime - nLeaps;
    if (isLeap(gpsTime)) { unixTime = unixTime + 0.5; }
    return unixTime;
  }
  GPSTimeConversion()            = default;
  explicit operator bool() const = delete;

 private:
  // Define GPS leap seconds
  static const uint32_t leapSeconds[NUMBER_LEAP_SECONDS];

  // Test to see if a GPS second is a leap second
  static bool isLeap(uint32_t gpsTime) {
    bool isLeap = false;
    for (int8_t i = 0; i < NUMBER_LEAP_SECONDS; i++) {
      if (gpsTime == leapSeconds[i]) { isLeap = true; }
    }
    return isLeap;
  }

  // Count number of leap seconds that have passed
  static int8_t countLeaps(uint32_t gpsTime, bool unix2gps) {
    int8_t nLeaps = 0;  // number of leap seconds prior to gpsTime
    for (int8_t i = 0; i < NUMBER_LEAP_SECONDS; i++) {
      if (unix2gps) {
        if (gpsTime >= leapSeconds[i] - i) { nLeaps++; }
      } else {
        if (gpsTime >= leapSeconds[i]) { nLeaps++; }
      }
    }
    return nLeaps;
  }
};
const uint32_t GPSTimeConversion::leapSeconds[NUMBER_LEAP_SECONDS] =
    LEAP_SECONDS;

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
   *
   * @note This epoch time will *probably* be in UTC.
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
