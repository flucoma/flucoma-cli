# Fluid Corpus Manipulation: Command line interface

This repository hosts code for generating the command line executables and documentation resources for the Fluid Corpus Manipulation Project. Much of the actual code that does the exciting stuff lives in this repository's principal dependency,  the [Fluid Corpus Manipulation Library](https://github.com/flucoma/flucoma-core).

* A wrapper from our code that allows us to generate command-line executables from a generic class.
* Stubs for producing an executable for each 'client' in the Fluid Corpus Manipulation Library.
* CMake code for managing dependencies, building and packaging.

## How to Build

```
mkdir -p build && cd build
cmake ..
make install
```

Alternatively, flucoma-cli is now on the [AUR / Arch User Repository](https://aur.archlinux.org/packages/flucoma-cli-git/) and can now be compiled and downloaded by executing:
```bash
yay -S flucoma-cli-git
```
on Arch Linux and Manjaro (with thanks to @madskjeldgaard)

Please see the [extensive compilation guide](https://github.com/flucoma/flucoma-cli/wiki/Compiling) for instructions on building this package.

## Credits 
#### FluCoMa core development team (in alphabetical order)
Owen Green, Gerard Roma, Pierre Alexandre Tremblay

#### Other contributors:
Alex Harker, Francesco Cameli

> This project has received funding from the European Research Council (ERC) under the European Union's Horizon 2020 research and innovation programme (grant agreement No 725899).
