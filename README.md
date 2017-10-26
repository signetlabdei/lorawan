# LoRaWAN ns-3 module #

[![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/ns-3-lorawan)
[![Build Status](https://travis-ci.org/DvdMgr/lorawan.svg?branch=master)](https://travis-ci.org/DvdMgr/lorawan)

This is an [ns-3](https://www.nsnam.org "ns-3 Website") module that can be used
to perform simulations of a [LoRaWAN](http://www.lora-alliance.org/technology
"LoRa Alliance") network.

## Getting started ##

### Prerequisites ###

To run simulations using this module, you will need to install ns-3, and clone
this repository inside the `src` directory. Required dependencies include
mercurial and a build environment.

#### Installing dependencies ####

On Ubuntu, run the following to install all the required packets:

```bash
sudo apt install git mercurial build-essential
```

On macOS, you can install the command line tools with:

```bash
xcode-select --install
```

while Mercurial and git can be installed via [Homebrew](https://brew.sh/ "Homebrew
homepage"):

```bash
brew install mercurial git
```

#### Downloading #####

Assuming you have mercurial installed, the procedure to get both ns-3 and the
`lorawan` module is the following:

```bash
hg clone http://code.nsnam.org/ns-3-dev
cd ns-3-dev/src
git clone git@github.com:DvdMgr/lorawan
```

### Compilation ###

If you are interested in only compiling the `lorawan` module and its
dependencies, copy the `.ns3rc` file from `ns-3-dev/utils` to `ns-3-dev`, and
only enable the desired module by making sure the file contains the following
line:

```python
modules_enabled = ['lorawan']
```

To compile, move to the `ns-3-dev` folder, configure and then build ns-3:

```bash
./waf configure --enable-tests --enable-examples
./waf build
```

Finally, make sure tests run smoothly with:

```bash
./test.py -s lorawan
```

If the script returns that the lorawan test suite passed, you are good to go.
Otherwise, if tests fail or crash, consider filing an issue.

## Usage examples ##

The module includes the following examples:

- `simple-lorawan-network-example`
- `complete-lorawan-network-example`
- `network-server-example`

Examples can be run via the `./waf --run example-name` command.

## Contributing ##

Refer to the [contribution guidelines](.github/CONTRIBUTING.md) for information
about how to contribute to this module.

## Documentation ##

For a complete description of the module, refer to `doc/lorawan.rst`.

- [ns-3 tutorial](https://www.nsnam.org/docs/tutorial/html "ns-3 Tutorial")
- [ns-3 manual](https://www.nsnam.org/docs/manual/html "ns-3 Manual")
- The LoRaWAN specification can be requested at the [LoRa Alliance
  website](http://www.lora-alliance.org)

## Getting help ##

To discuss and get help on how to use this module, you can write to us on [our
gitter chat](https://gitter.im/ns-3-lorawan "lorawan Gitter chat").

## Authors ##

- Davide Magrin
- Stefano Romagnolo
- Michele Luvisotto
- Martina Capuzzo

## License ##

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.

## Acknowledgments and relevant publications ##

The initial version of this code was developed as part of a master's thesis at
the [University of Padova](https://unipd.it "Unipd homepage"), under the
supervision of Prof. Lorenzo Vangelista, Prof. Michele Zorzi and with the help
of Marco Centenaro.

Publications:
- D. Magrin, M. Centenaro and L. Vangelista, "Performance evaluation of LoRa
  networks in a smart city scenario," 2017 IEEE International Conference On
  Communications (ICC), Paris, 2017. Available:
  http://ieeexplore.ieee.org/document/7996384/
- Network level performances of a LoRa system (Master thesis). Available:
  http://tesi.cab.unipd.it/53740/1/dissertation.pdf
