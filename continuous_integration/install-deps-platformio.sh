#!/bin/bash

# Makes the bash script print out every command before it is executed, except echo
trap '[[ $BASH_COMMAND != echo* ]] && echo $BASH_COMMAND' DEBUG

# Exit with nonzero exit code if anything fails
set -e

echo "::group::Installing Platforms and Frameworks"
echo "\e[32mInstalling Atmel AVR platforms \e[0m"
pio pkg install -g --platform atmelavr
pio pkg install -g --tool framework-arduino-avr
pio pkg install -g --tool tool-avrdude
pio pkg install -g --tool toolchain-atmelavr

echo "\e[32mInstalling Atmel AVR framework \e[0m"
pio pkg install -g --platform atmelmegaavr
pio pkg install -g --tool framework-arduino-megaavr

echo "\e[32mInstalling Atmel SAM platform \e[0m"
pio pkg install -g --platform atmelsam

echo "\e[32mInstalling Intel ARC 32 platform \e[0m"
pio pkg install -g --platform intel_arc32

echo "\e[32mInstalling Teensy platform \e[0m"
pio pkg install -g --platform teensy

echo "\e[32mInstalling Atmel SAM framework \e[0m"
pio pkg install -g --tool framework-arduino-samd
pio pkg install -g --tool framework-arduino-samd-adafruit
pio pkg install -g --tool framework-cmsis
pio pkg install -g --tool framework-cmsis-atmel
pio pkg install -g --tool tool-bossac
pio pkg install -g --tool toolchain-gccarmnoneeabi
echo "::endgroup::"


echo "::group::Installing Libraries"
echo "\e[32m\nCurrently installed packages:\e[0m"
pio pkg list -g -v

echo "\e[32mInstalling envirodiy/EnviroDIY_DS3231\e[0m"
pio pkg install -g --library envirodiy/EnviroDIY_DS3231

echo "\e[32mInstalling vshymanskyy/StreamDebugger\e[0m"
pio pkg install -g --library vshymanskyy/StreamDebugger

echo "::endgroup::"


echo "::group::Current globally installed packages"
echo "\e[32m\nCurrently installed packages:\e[0m"
pio pkg list -g -v
echo "::endgroup::"
