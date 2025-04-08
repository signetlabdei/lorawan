# LoRaWAN ns-3 module

[![CI](https://github.com/signetlabdei/lorawan/actions/workflows/per-commit.yml/badge.svg)](https://github.com/signetlabdei/lorawan/actions)
[![codecov](https://codecov.io/gh/signetlabdei/lorawan/graph/badge.svg?token=EVBlTb4LgQ)](https://codecov.io/gh/signetlabdei/lorawan)

This is an [ns-3](https://www.nsnam.org "ns-3 Website") module that can be used
to perform simulations of a [LoRaWAN](https://lora-alliance.org/about-lorawan
"LoRa Alliance") network.

Quick links:

* [Simulation Model Overview](https://signetlabdei.github.io/lorawan/models/build/html/lorawan.html)

* [API Documentation](https://signetlabdei.github.io/lorawan/html/d5/d00/group__lorawan.html)

## Getting started

### Prerequisites

To run simulations using this module, you first need to install ns-3. If you are on Ubuntu/Debian/Mint, you can install the minimal required packages as follows:

```bash
sudo apt install g++ python3 cmake ninja-build git ccache
```

Otherwise please directly refer to the [prerequisites section of the ns-3 installation page](https://www.nsnam.org/wiki/Installation#Prerequisites).

> Note: While the `ccache` package is not strictly required, it is highly recommended. It can significantly enhance future compilation times by saving tens of minutes, albeit with a higher disk space cost of approximately 5GB. This disk space usage can be eventually reduced through a setting.

Then, you need to:

1. Clone the main ns-3 codebase,
1. Clone this repository inside the `src` directory therein, and
1. Checkout the current ns-3 version supported by this module.

To install this module at the latest commit, you can use the following all-in-one command:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git && cd ns-3-dev &&
git clone https://github.com/signetlabdei/lorawan src/lorawan &&
tag=$(< src/lorawan/NS3-VERSION) && tag=${tag#release } && git checkout $tag -b $tag
```

**Note**: When switching to any previous commit, *including the latest release*, always make sure to also checkout ns-3 to the correct version (`NS3-VERSION` file at the root of this repository) supported at that point in time.

### Compilation

Ns-3 adopts a development-oriented philosophy. Before you can run anything, you'll need to compile the ns-3 code. You have two options:

1. **Compile ns-3 as a whole:** Make all simulation modules available by configuring and building as follows (ensure you are in the `ns-3-dev` folder!):

   ```bash
   ./ns3 configure --enable-tests --enable-examples &&
   ./ns3 build
   ```

1. **Focus exclusively on the lorawan module:** To expedite the compilation process, as it can take more than 30/40 minutes on slow hardware, change the configuration as follows:

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

If the script reports that all tests passed you are good to go.

If some tests fail or crash, consider filing an issue.

## Usage examples

The module includes the following examples:

* `simple-network-example`
* `complete-network-example`
* `network-server-example`
* `adr-example`
* `aloha-throughput`
* `frame-counter-update`
* `lora-energy-model-example`
* `parallel-reception-example`

Examples can be run via the `./ns3 run example-name` command (refer to `./ns3 run --help` for more options).

## Documentation

* [Simulation Model Overview](https://signetlabdei.github.io/lorawan/models/build/html/lorawan.html): A description of the foundational models of this module (source file located at `doc/lorawan.rst`).
* [API Documentation](https://signetlabdei.github.io/lorawan/html/d5/d00/group__lorawan.html): documentation of all classes, member functions and variables generated from Doxygen comments in the source code.

Other useful documentation sources:

* [Ns-3 tutorial](https://www.nsnam.org/docs/tutorial/html/ "ns-3 Tutorial"): **Start here if you are new to ns-3!**
* [Ns-3 manual](https://www.nsnam.org/docs/manual/html/ "ns-3 Manual"): Overview of the fundamental tools and abstractions in ns-3.
* The LoRaWAN specification can be downloaded at the [LoRa Alliance
  website](http://www.lora-alliance.org).

## Getting help

To discuss and get help on how to use this module, you can open an issue here.

## Contributing

Refer to the [contribution guidelines](.github/CONTRIBUTING.md) for information
about how to contribute to this module.

## Authors

* Davide Magrin
* Martina Capuzzo
* Stefano Romagnolo
* Michele Luvisotto

## License

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.

## Acknowledgments and relevant publications

The initial version of this code was developed as part of a master's thesis at
the [University of Padova](https://unipd.it "Unipd homepage"), under the
supervision of Prof. Lorenzo Vangelista, Prof. Michele Zorzi and with the help
of Marco Centenaro.

Publications:

* D. Magrin, M. Capuzzo and A. Zanella, "A Thorough Study of LoRaWAN Performance Under Different
  Parameter Settings," in IEEE Internet of Things Journal. 2019.
  [Link](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8863372&isnumber=6702522).
* M. Capuzzo, D. Magrin and A. Zanella, "Confirmed traffic in LoRaWAN: Pitfalls
  and countermeasures," 2018 17th Annual Mediterranean Ad Hoc Networking
  Workshop (Med-Hoc-Net), Capri, 2018. [Link](https://ieeexplore.ieee.org/abstract/document/8407095).
* D. Magrin, M. Centenaro and L. Vangelista, "Performance evaluation of LoRa
  networks in a smart city scenario," 2017 IEEE International Conference On
  Communications (ICC), Paris, 2017. [Link](http://ieeexplore.ieee.org/document/7996384/).
* Network level performances of a LoRa system (Master thesis). [Link](http://tesi.cab.unipd.it/53740/1/dissertation.pdf).
