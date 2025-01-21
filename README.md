# LoRa_AT<!--!{#mainpage}-->

A small Arduino library for AT command based LoRa modules, that just works.

This is a modification of the excellent TinyGSM library to use with LoRa modules.
I decided to create a new fork rather than modifying the original library to handle LoRa because LoRa radios do not work as internet clients directly.
In the LoRa case, I'm using a stream, not a client, without mux or direct connection.
It seemed to be different enough to create a fork.
I may merge it in later.

__________

## Download and Install<!--!{#mainpage_download}-->

This library is available from both the PlatformIO and Arduino library registries.

The current version can be viewed on the PlatformIO registry at https://registry.platformio.org/libraries/envirodiy/LoRa_AT.

The logs from ingestion into the Arduino library registry can be viewed at http://downloads.arduino.cc/libraries/logs/github.com/EnviroDIY/LoRa_AT/.

## License<!--!{#mainpage_license}-->

This project is released under
The GNU Lesser General Public License (LGPL-3.0)
