![TinyLoRa logo](https://cdn.rawgit.com/vshymanskyy/TinyLoRa/d18e93dc51fe988a0b175aac647185457ef640b5/extras/logo.svg)

A small Arduino library for AT command based LoRa modules, that just works.

This is a modification of the excellent TinyGSM library to use with LoRa modules.
I decided to create a new fork rather than modifying the original library to handle LoRa because LoRa radios do not work as internet clients directly.
In the LoRa case, I'm using a stream, not a client, without mux or direct connection.
It seemed to be different enough to create a fork.
I may merge it in later.

__________

## License
This project is released under
The GNU Lesser General Public License (LGPL-3.0)
