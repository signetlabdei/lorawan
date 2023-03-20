# Using the module #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") module that can be used
to perform simulations of a [LoRaWAN](http://www.lora-alliance.org/technology
"LoRa Alliance") network.

This version has been modified to implement a traffic emulator for the [Chirpstack server stack](https://www.chirpstack.io/ "ChirpStack, open-source LoRaWAN® Network Server").

[Original repository](https://github.com/signetlabdei/lorawan "LoRaWAN ns-3 module").

[API documentation](https://signetlabdei.github.io/lorawan-docs/html/index.html) (not up to date with this fork).

[Module Documentation](https://signetlabdei.github.io/lorawan-docs/models/build/html/lorawan.html) (currently not up to date with the original module).

## Getting started ##

### Prerequisites ###

If not already, install the `libcurl` development library in your linux distribution (`libcurl4-gnutls-dev` on Ubuntu, `curl-dev` on Alpine).

To run simulations using this module, you will need to install ns-3, and clone
this repository inside the `contrib` or `src` directory:

```bash
git clone https://gitlab.com/non-det-alle/ns-3-dev
git clone https://github.com/non-det-alle/elora ns-3-dev/contrib/lorawan
```

Check out the chirpstack branch:

```bash
cd ns-3-dev/contrib/lorawan
git checkout chirpstack
```

### Compilation ###

To compile, move to the `ns-3-dev` folder, configure and then build ns-3:

```bash
./ns3 configure -d debug --enable-examples
./ns3 build
```
## Usage examples ##

The module includes the following example:

- `chirpstack-example`

Examples can be run via the `./ns3 run --enable-sudo "chirpstack-example [options]"` command.

Options can be retrived with `./ns3 run "chirpstack-example --help"`.

## Documentation ##

For a complete description of the module, refer to `doc/lorawan.rst` (currently not up to date).

- [ns-3 tutorial](https://www.nsnam.org/docs/tutorial/html "ns-3 Tutorial")
- [ns-3 manual](https://www.nsnam.org/docs/manual/html "ns-3 Manual")
- The LoRaWAN specification can be found on the [LoRa Alliance
  website](http://www.lora-alliance.org)

## License ##

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.