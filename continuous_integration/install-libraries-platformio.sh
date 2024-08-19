#!/bin/bash


set -e # Exit with nonzero exit code if anything fails
if [ "$RUNNER_DEBUG" = "1" ]; then
    echo "Enabling debugging!"
    set -v # Prints shell input lines as they are read.
    set -x # Print command traces before executing command.
fi

echo "\e[32mCurrent PlatformIO version:\e[0m"
pio --version

echo "\e[32mCurrently installed libraries:\e[0m"
pio pkg list -g -v --only-libraries


echo "\e[32mInstalling StreamDebugger\e[0m"
pio pkg install --skip-dependencies -g --library vshymanskyy/StreamDebugger@~1.0.1


echo "::group::Current globally installed libraries"
echo "\e[32mCurrently installed packages:\e[0m"
pio pkg list -g -v --only-libraries
echo "::endgroup::"
