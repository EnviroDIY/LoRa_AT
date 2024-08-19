#!/bin/bash


set -e # Exit with nonzero exit code if anything fails
if [ "$RUNNER_DEBUG" = "1" ]; then
    echo "Enabling debugging!"
    set -v # Prints shell input lines as they are read.
    set -x # Print command traces before executing command.
fi

echo "\e[32mCurrent Arduino CLI version:\e[0m"
arduino-cli version

echo "\e[32mUpdating the library index\e[0m"
arduino-cli --config-file arduino_cli.yaml lib update-index

echo "\e[32mInstalling StreamDebugger\e[0m"
arduino-cli --config-file /home/runner/work/LoRa_AT/LoRa_AT/continuous_integration/arduino_cli.yaml lib install StreamDebugger --no-deps


echo "::group::Current globally installed libraries"
echo "\e[32mCurrently installed libraries:\e[0m"
arduino-cli --config-file arduino_cli.yaml lib update-index
arduino-cli --config-file arduino_cli.yaml lib list
echo "::endgroup::"
