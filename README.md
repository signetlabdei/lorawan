# LoRaWAN ns-3 module #

[![CI](https://github.com/signetlabdei/lorawan/actions/workflows/per-commit.yml/badge.svg)](https://github.com/signetlabdei/lorawan/actions)
[![codecov](https://codecov.io/gh/signetlabdei/lorawan/graph/badge.svg?token=EVBlTb4LgQ)](https://codecov.io/gh/signetlabdei/lorawan)

This is an [ns-3](https://www.nsnam.org "ns-3 Website") module that can be used
to perform simulations of a [LoRaWAN](http://www.lora-alliance.org/technology
"LoRa Alliance") network.

[API documentation](https://signetlabdei.github.io/lorawan-docs/html/index.html).

[Module Documentation](https://signetlabdei.github.io/lorawan-docs/models/build/html/lorawan.html).

## Getting started ##

### Prerequisites ###

To run simulations using this module, you will need to install ns-3. If you are on Ubuntu/Debian/Mint you can install the minimal required packages as follows (note that `ccache` is not strictly necessary; we'll comment on that further below):

```bash
sudo apt install g++ python3 cmake ninja-build git ccache
```
Otherwise please directly refer to the [prerequisites section of the ns-3 installation page](https://www.nsnam.org/wiki/Installation#Prerequisites).

Then you need to clone the ns-3 codebase, clone this repository inside the `src` directory therein, and checkout the latest ns-3 version supported by this module. You can use the following all-in-one command:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git && cd ns-3-dev &&
git clone https://github.com/signetlabdei/lorawan src/lorawan &&
tag=$(< src/lorawan/NS3-VERSION) && tag=${tag#release } && git checkout $tag -b $tag
```

### Compilation ###

Ns-3 adopts a development-oriented philosophy. Thus, before being able to run anything you'll need to compile the ns-3 code. You can either compile ns-3 as a whole (making all simulation modules available), or just focus on the lorawan module to speed up the compilation process which can otherwise take more and 30/40 min depending on your hardware.

* To compile ns-3 as a whole, configure and then build it as follows (make sure you are in the `ns-3-dev` folder!):
```bash
./ns3 configure --enable-tests --enable-examples &&
./ns3 build
```

* Instead, if you are exclusively interested in using the `lorawan` module, you can change the configuration as follows:
```bash
./ns3 clean &&
./ns3 configure --enable-tests --enable-examples --enable-modules lorawan &&
./ns3 build
```
The first line was added to make sure you start from a clean build state. If not already, we highly suggest installing the optional `ccache` package to further improve future compilation times (at an higher diskspace cost of ~5GB, which can be eventually lowered as a setting).

Finally, make sure tests run smoothly with:
```bash
./test.py
```
If the script returns that all tests passed of that just `three-gpp-propagation-loss-model` failed[^1], you are good to go.

Otherwise, if other tests fail or crash, consider filing an issue.

[^1]: That's unfortunately due to [a bug in the current ns-3 version](https://gitlab.com/nsnam/ns-3-dev/-/issues/965) when restricting compilation to the lorawan module and its dependencies. Consider compiling ns-3 as a whole or with the `--enable-modules "lorawan;applications"` option if you need to use the `three-gpp-propagation-loss-model`.

## Usage examples ##

The module includes the following examples:

- `simple-network-example`
- `complete-network-example`
- `network-server-example`

Examples can be run via the `./ns3 run example-name` command.

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
- Martina Capuzzo
- Stefano Romagnolo
- Michele Luvisotto

## License ##

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.

## Acknowledgments and relevant publications ##

The initial version of this code was developed as part of a master's thesis at
the [University of Padova](https://unipd.it "Unipd homepage"), under the
supervision of Prof. Lorenzo Vangelista, Prof. Michele Zorzi and with the help
of Marco Centenaro.

Publications:
- D. Magrin, M. Capuzzo and A. Zanella, "A Thorough Study of LoRaWAN Performance Under Different
  Parameter Settings," in IEEE Internet of Things Journal. 2019.
  [Link](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8863372&isnumber=6702522).
- M. Capuzzo, D. Magrin and A. Zanella, "Confirmed traffic in LoRaWAN: Pitfalls
  and countermeasures," 2018 17th Annual Mediterranean Ad Hoc Networking
  Workshop (Med-Hoc-Net), Capri, 2018. [Link](https://ieeexplore.ieee.org/abstract/document/8407095).
- D. Magrin, M. Centenaro and L. Vangelista, "Performance evaluation of LoRa
  networks in a smart city scenario," 2017 IEEE International Conference On
  Communications (ICC), Paris, 2017. [Link](http://ieeexplore.ieee.org/document/7996384/).
- Network level performances of a LoRa system (Master thesis). [Link](http://tesi.cab.unipd.it/53740/1/dissertation.pdf).
