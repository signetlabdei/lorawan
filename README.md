# Ns-3 Chirpstack simulator #

This is a traffic generator for the [Chirpstack server stack](https://www.chirpstack.io/ "ChirpStack, open-source LoRaWAN® Network Server"). 

The code is a direct extension of the ns-3 [LoRaWAN module](https://github.com/signetlabdei/lorawan "LoRaWAN ns-3 module").

This module can be used to simulate in real-time multiple devices and gateways sharing a radio channel with very high flexibility in terms of possible configurations. LoRaWAN traffic is then UDP-encapsulated by gateways and forwarded outside the simulation. If a Chirpstack network server is in place, it will think the traffic is coming from a real network. 

In addition to what is provided by the original LoRaWAN module, the following changes/additions were made:

* A gateway application implementing the [UDP forwarder protocol](https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT "Semtech packet forwarder implementation") running on real gateways
* An helper to register devices and gateways in the server using the included [REST API](https://github.com/chirpstack/chirpstack-rest-api "ChirpStack gRPC to REST API proxy")
* Libraries to compute the Meassage Integrity Code (MIC) of packets for devices to be recognised by the server
* The `chirpstack-example` to show a complete usage of the traffic generator
* Many small improvements or corrections of features of the original module

## Prerequisites ##

To use this simulator you need to know the following:

* The registration helper (`chirpstack-helper` class) uses the cURL library to speak to the server. [Ns-3](https://gitlab.com/nsnam/ns-3-dev "The Network Simulator, Version 3") does not support cURL by default so a minor modification is needed in the ns-3 CMake code. This change has been made in [this fork of the ns-3 repository](https://gitlab.com/non-det-alle/ns-3-dev "Ns-3 fork supporting cURL"), that has been used to test the traffic generator. Use the forked version which is maintaned by us, or reproduce the simple changes in your ns-3 version
* The simulator works as is with the default configuration of Chirpstark v4 on `localhost:8080`. It has been tested with the [docker-compose installation](https://www.chirpstack.io/docs/getting-started/docker.html "Chirpstack docs: Quickstart Docker Compose") of the server. To test a distributed version of the setup, the server address needs to be changed in `chirpstack-example` and `chirpstack-helper`
* An authentification token needs to be generated in the server (API keys section), and needs to be pasted in the constructor of `chirpstack-helper` class
* Ns-3 needs to be configured with the `--enable-sudo` option


For information on how to use the underlying LoRaWAN module refer to the file `USAGE.md`, an updated version of the original module readme.

## Known limitations ##

The simulator is able to receive real downlink messages from the server. Theoretically, the server could be used to reconfigure the radio parameters of simulated devices with MAC commands. Unfortunately, at time of writing, there seems to be fluctuating non-synchronisation (few ms) between reception windows of devices, and gateways sending downlinks on the radio channel. This often causes downlinks to be lost. It could be solved by increasing the duration of reception windows.

## Getting help ##

If you need any help, feel free to open an issue here.
