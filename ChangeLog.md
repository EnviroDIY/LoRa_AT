# ChangeLog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

- Added faster baud rates to auto-baud function
- Corrected name from LoRa E5 to Wio-E5.
  - For backwards compatibility, you can defined either `LORA_AT_WIOE5` or `LORA_AT_LORAE5` to ensure the module is loaded.

### Added

### Removed

### Fixed

***

## [0.4.3]

### Changed

- Added faster baud rates to auto-baud function
- Corrected name from LoRa E5 to Wio-E5.
  - For backwards compatibility, you can defined either `LORA_AT_WIOE5` or `LORA_AT_LORAE5` to ensure the module is loaded.

### Added

- Implemented function to change the default frequency band on the mDot.
  - While implemented, this should not be used unless you're sure you know what you're doing!

### Fixed

- Correctly handle protected settings on the mDot

***

## [0.4.2]

### Fixed

- Fixed conflicting defines with TinyGSM

***

## [0.4.1] 2025-02-24

### Added

- Added functions for proper conversion between GPS time and Unix time.
  - Functionality taken from [GPS Time Conversion Algorithm](https://www.andrews.edu/~tzs/timeconv/timealgorithm.html)

### Fixed

- Corrected offsets with GPS epoch times and other epoch starts

***

## [0.4.0] 2025-01-27

### Changed

- **Breaking** Removed timeout from join and get connection functions; replaced with a number of join/check attempts and the initial backoff.
  - This changes the inputs for the `joinOTAA(...)`, `joinABP(...)`, and `isNetworkConnected()` functions - although if you were previously accepting defaults for not required arguments, you do not need to change your code.
  - After the initial backoff, further backoff delays will be calculated using exponential calculation with jitter.
  - The default number of attempts is 10, the default initial backoff is 5s, and the default backoff factor is 1.5. If all attempts are used, the total delay will be (approximately) 9.5 minutes. The real delay will vary because of a random jitter of up to 20% added to or subtracted from each delay.
    - The default backoff parameters can be changed with the following defines:
      - `DEFAULT_JOIN_ATTEMPTS`
      - `DEFAULT_INITIAL_BACKOFF`
      - `DEFAULT_BACKOFF_FACTOR`
    - The number of join attempts and the initial backoff can also be changed in all functions that use it, but the backoff factor can only be changed with the define.

### Added

- Added a function to calculate backoff using exponential calculation with jitter.

***

## [0.3.1] 2025-01-21

### Fixed

- Fixed overly greedy consumption of module debug output.
- Fixed some unused var warnings

***

## [0.3.0] 2024-08-19

### Changed

- **Possibly Breaking** Made the hex send the default send method
- **Possibly Breaking** Increase the default response time for all commands from 1s to 5s
- Minor updates to text in examples
- Updated github workflows

### Fixed

- Corrected some unnecessary casting

***

## [0.2.0] 2024-05-28

### Changed

- Renamed from TinyLoRa to LoRa_AT because the library name TinyLoRa was already taken.

***

## [0.1.0] 2024-05-24

### Added

- Initial release of the library, supporting LoRa on the mDOT and the Wio-E5

***

[Unreleased]: https://github.com/EnviroDIY/LoRa_AT/compare/v0.4.3...HEAD
[0.4.3]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.4.3
[0.4.2]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.4.2
[0.4.1]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.4.1
[0.4.0]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.4.0
[0.3.1]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.3.1
[0.3.0]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.3.0
[0.2.0]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.2.0
[0.1.0]: https://github.com/EnviroDIY/LoRa_AT/releases/tag/v0.1.0
