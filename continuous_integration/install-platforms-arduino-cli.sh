#!/bin/bash

set -e # Exit with nonzero exit code if anything fails
if [ "$RUNNER_DEBUG" = "1" ]; then
    echo "Enabling debugging!"
    set -v # Prints shell input lines as they are read.
    set -x # Print command traces before executing command.
fi

echo "\e[32mCurrent Arduino CLI version:\e[0m"
arduino-cli version

echo "\e[32mUpdating the core index\e[0m"
arduino-cli --config-file arduino_cli.yaml core update-index

echo "::group::Envirodiy Avr"
echo "\e[32mEnvirodiy Avr\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install EnviroDIY:avr
echo "::endgroup::"

echo "::group::Arduino Avr"
echo "\e[32mArduino Avr\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:avr
echo "::endgroup::"

echo "::group::Arduino Sam"
echo "\e[32mArduino Sam\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:sam
echo "::endgroup::"

echo "::group::Arduino Samd"
echo "\e[32mArduino Samd\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:samd
echo "::endgroup::"

echo "::group::Arduino Megaavr"
echo "\e[32mArduino Megaavr\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:megaavr
echo "::endgroup::"

echo "::group::Arduino Esp32"
echo "\e[32mArduino Esp32\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:esp32
echo "::endgroup::"

echo "::group::Arduino Mbed_Rp2040"
echo "\e[32mArduino Mbed_Rp2040\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:mbed_rp2040
echo "::endgroup::"

echo "::group::Arduino Renesas_Uno"
echo "\e[32mArduino Renesas_Uno\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:renesas_uno
echo "::endgroup::"

echo "::group::Arduino Mbed_Nano"
echo "\e[32mArduino Mbed_Nano\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:mbed_nano
echo "::endgroup::"

echo "::group::Arduino Mbed_Portenta"
echo "\e[32mArduino Mbed_Portenta\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:mbed_portenta
echo "::endgroup::"

echo "::group::Adafruit Avr"
echo "\e[32mAdafruit Avr\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install adafruit:avr
echo "::endgroup::"

echo "::group::Adafruit Samd"
echo "\e[32mAdafruit Samd\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install adafruit:samd
echo "::endgroup::"

echo "::group::Adafruit Nrf52"
echo "\e[32mAdafruit Nrf52\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install adafruit:nrf52
echo "::endgroup::"

echo "::group::Esp8266 Esp8266"
echo "\e[32mEsp8266 Esp8266\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install esp8266:esp8266
echo "::endgroup::"

echo "::group::Esp32 Esp32"
echo "\e[32mEsp32 Esp32\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install esp32:esp32
echo "::endgroup::"

echo "::group::Stmicroelectronics Stm32"
echo "\e[32mStmicroelectronics Stm32\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install STMicroelectronics:stm32
echo "::endgroup::"

echo "::group::Teensy Avr"
echo "\e[32mTeensy Avr\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install teensy:avr
echo "::endgroup::"

echo "\e[32mUpdating the core index\e[0m"
arduino-cli --config-file arduino_cli.yaml core update-index

echo "\e[32mUpgrading all cores\e[0m"
arduino-cli --config-file arduino_cli.yaml core upgrade

echo "\e[32mCurrently installed cores:\e[0m"
arduino-cli --config-file arduino_cli.yaml core list
