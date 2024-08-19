#!/bin/bash


set -e # Exit with nonzero exit code if anything fails
if [ "$RUNNER_DEBUG" = "1" ]; then
    echo "Enabling debugging!"
    set -v # Prints shell input lines as they are read.
    set -x # Print command traces before executing command.
fi

echo "\e[32mCurrent PlatformIO version:\e[0m"
pio --version

echo "::group::Atmel AVR"
echo "\e[32mAtmel AVR\e[0m"
pio pkg install -g --platform atmelavr
pio pkg install -g --tool framework-arduino-avr
pio pkg install -g --tool tool-avrdude
pio pkg install -g --tool toolchain-atmelavr
echo "::endgroup::"

echo "::group::Atmel SAM/SAMD"
echo "\e[32mAtmel SAM/SAMD\e[0m"
pio pkg install -g --platform atmelsam
pio pkg install -g --tool framework-arduino-sam
pio pkg install -g --tool framework-arduino-samd
pio pkg install -g --tool framework-arduino-samd-adafruit
pio pkg install -g --tool framework-arduino-samd-sodaq
pio pkg install -g --tool framework-cmsis
pio pkg install -g --tool framework-cmsis-atmel
pio pkg install -g --tool toolchain-gccarmnoneeabi
echo "::endgroup::"

echo "::group::Atmel Mega AVR"
echo "\e[32mAtmel Mega AVR\e[0m"
pio pkg install -g --platform atmelmegaavr
pio pkg install -g --tool framework-arduino-megaavr
pio pkg install -g --tool toolchain-atmelavr
echo "::endgroup::"

echo "::group::Espressif ESP32"
echo "\e[32mEspressif ESP32\e[0m"
pio pkg install -g --platform espressif32
pio pkg install -g --tool tool-esptool
pio pkg install -g --tool tool-esptoolpy
pio pkg install -g --tool toolchain-riscv32-esp
pio pkg install -g --tool toolchain-xtensa-esp32
echo "::endgroup::"

echo "::group::Raspberry Pi Pico"
echo "\e[32mRaspberry Pi Pico\e[0m"
pio pkg install -g --platform raspberrypi
pio pkg install -g --tool framework-arduino-mbed
pio pkg install -g --tool tool-rp2040tools
pio pkg install -g --tool toolchain-gccarmnoneeabi
echo "::endgroup::"

echo "::group::Renesas-RA"
echo "\e[32mRenesas-RA\e[0m"
pio pkg install -g --platform renesas-ra
pio pkg install -g --tool framework-arduinorenesas-uno
pio pkg install -g --tool tool-bossac
pio pkg install -g --tool toolchain-gccarmnoneeabi
echo "::endgroup::"

echo "::group::Nordic NRF52"
echo "\e[32mNordic NRF52\e[0m"
pio pkg install -g --platform nordicnrf52
pio pkg install -g --tool framework-cmsis
pio pkg install -g --tool tool-adafruit-nrfutil
pio pkg install -g --tool tool-sreccat
pio pkg install -g --tool toolchain-gccarmnoneeabi
echo "::endgroup::"

echo "::group::STM32"
echo "\e[32mSTM32\e[0m"
pio pkg install -g --platform ststm32
pio pkg install -g --tool framework-arduino-mbed
pio pkg install -g --tool framework-arduinoststm32
pio pkg install -g --tool framework-cmsis
pio pkg install -g --tool toolchain-gccarmnoneeabi
echo "::endgroup::"

echo "::group::Espressif ESP8266"
echo "\e[32mEspressif ESP8266\e[0m"
pio pkg install -g --platform espressif8266
pio pkg install -g --tool tool-esptool
pio pkg install -g --tool tool-esptoolpy
pio pkg install -g --tool toolchain-xtensa
echo "::endgroup::"

echo "::group::Teensy"
echo "\e[32mTeensy\e[0m"
pio pkg install -g --platform teensy
pio pkg install -g --tool framework-arduinoteensy
pio pkg install -g --tool toolchain-gccarmnoneeabi-teensy
echo "::endgroup::"


echo "::group::Package List"
echo "\e[32mCurrently installed packages:\e[0m"
pio pkg list -g -v
echo "::endgroup::"
