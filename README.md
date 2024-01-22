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

To run simulations using this module, you first need to install ns-3. If you are on Ubuntu/Debian/Mint, you can install the minimal required packages as follows:

```bash
sudo apt install g++ python3 cmake ninja-build git ccache
```
Otherwise please directly refer to the [prerequisites section of the ns-3 installation page](https://www.nsnam.org/wiki/Installation#Prerequisites).

> Note: While the `ccache` package is not strictly required, it is highly recommended. It can significantly enhance future compilation times by saving tens of minutes, albeit with a higher disk space cost of approximately 5GB. This disk space usage can be eventually reduced through a setting.

Then, you need to:
1. Clone the ns-3 codebase,
2. Clone this repository inside the `src` directory therein, and
3. Checkout the latest ns-3 version supported by this module.

You can use the following all-in-one command:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git && cd ns-3-dev &&
git clone https://github.com/signetlabdei/lorawan src/lorawan &&
tag=$(< src/lorawan/NS3-VERSION) && tag=${tag#release } && git checkout $tag -b $tag
```

### Compilation ###

Ns-3 adopts a development-oriented philosophy. Before you can run anything, you'll need to compile the ns-3 code. You have two options:

1. **Compile ns-3 as a whole:** Make all simulation modules available by configuring and building as follows (ensure you are in the `ns-3-dev` folder!):
   ```bash
   ./ns3 configure --enable-tests --enable-examples &&
   ./ns3 build
   ```

2. **Focus exclusively on the lorawan module:** To expedite the compilation process, as it can take more than 30/40 minutes on slow hardware, change the configuration as follows:
   ```bash
   ./ns3 clean &&
   ./ns3 configure --enable-tests --enable-examples --enable-modules lorawan &&
   ./ns3 build
   ```
   The first line ensures you start from a clean build state.

Finally, ensure tests run smoothly with:
```bash
./test.py
```
If the script reports that all tests passed or that just `three-gpp-propagation-loss-model` failed[^1], you are good to go.

If other tests fail or crash, consider filing an issue.

[^1]: This is due to [a bug in the current ns-3 version](https://gitlab.com/nsnam/ns-3-dev/-/issues/965) when restricting compilation to the lorawan module and its dependencies. If you need to use the `three-gpp-propagation-loss-model`, you can solve this by compiling ns-3 as a whole or with the `--enable-modules "lorawan;applications"` option to reduce compilation time.

## Usage examples ##

The module includes the following examples:

- `simple-network-example`
- `complete-network-example`
- `network-server-example`

Examples can be run via the `./ns3 run example-name` command (refer to `./ns3 run --help` for more options).

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
