# ELoRa: An end-to-end LoRaWAN emulator for the ChirpStack network server

>**If you just want to create scenarios using the emulator, and you are not interested in tinkering with it, we refer you to a quicker installation running the [emulator image with Docker Compose](https://github.com/non-det-alle/elora-docker).**

This is a traffic emulator for the [Chirpstack server stack](https://www.chirpstack.io/ "ChirpStack, open-source LoRaWAN® Network Server"). 

This software can be used to simulate in real-time multiple devices and gateways sharing a radio channel with very high flexibility in terms of possible configurations. LoRaWAN traffic is then UDP-encapsulated by gateways and forwarded outside the simulation. If a Chirpstack network server is in place, it will think the traffic is coming from a real network. All Class A MAC primitives used in the UE868 region are supported: radio transmission parameters of simulated devices can be changed by the downlink LoRaWAN traffic of the real server. 

The code is an extension of the ns-3 [LoRaWAN module](https://github.com/signetlabdei/lorawan "LoRaWAN ns-3 module"). In addition to what is provided by the original LoRaWAN module, the following changes/additions were made:

* A gateway application implementing the [UDP forwarder protocol](https://github.com/Lora-net/packet_forwarder/blob/master/PROTOCOL.TXT "Semtech packet forwarder implementation") running on real gateways
* An helper to register devices and gateways in the server using the included [REST API](https://github.com/chirpstack/chirpstack-rest-api "ChirpStack gRPC to REST API proxy")
* Cryptographyc libraries to compute the Meassage Integrity Code (MIC) and encryption of packets for devices to be recognised by the server
* The `chirpstack-example` to show a complete usage of the traffic generator
* Many improvements and corrections of features of the original module, such that traffic could be transparently be accepted by the server

## Prerequisites ##

To use this simulator you need to know the following:

* The ChirpStack server needs to be running somewhere (reachable by the simulation via network)
* The registration helper (`chirpstack-helper` class) uses the cURL library to speak to the server. [Ns-3](https://gitlab.com/nsnam/ns-3-dev "The Network Simulator, Version 3") does not support cURL by default so a minor modification is needed in the ns-3 CMake code. This change has been made in [this fork of the ns-3 repository](https://gitlab.com/non-det-alle/ns-3-dev "Ns-3 fork supporting cURL"), that has been used to test the emulator. Use the forked version which is maintaned by us, or reproduce the simple changes in your ns-3 version
* The simulator works as is with the default configuration of Chirpstark v4 on `localhost:8080`. It has been tested with the [docker-compose installation](https://www.chirpstack.io/docs/getting-started/docker.html "Chirpstack docs: Quickstart Docker Compose") of the server. To test a distributed version of the setup, the server/port address needs to be changed in `chirpstack-example`, and ChirpStack needs to be set up such that a Gateway Bridge container remains co-located on the same machine of the ELoRa process
* An authentification token needs to be generated in the server (API keys section), and needs to be copy-pasted in the constructor of `chirpstack-helper` class
* Ns-3 needs to run with the `--enable-sudo` option

## Usage ##

For detailed information on how to run ELoRa refer to the [USAGE.md](USAGE.md) file.

For more information on how to use the underlying LoRaWAN module refer to the [original module readme](https://github.com/signetlabdei/lorawan/blob/e8f7a21044418e92759d5c7c4bcab147cdaf05b3/README.md "LoRaWAN ns-3 module README").

## Getting help ##

If you need any help, feel free to open an issue here.
