# libsolace
A tiny C++ library for emulating quantum computing.

## Purpose

This project is created as a tool for researching quantum computing and designing quantum algorithms.

I want to stress the fact that this project is not meant to be the winner of "Who can build the best and
most efficient quantum emulator"; Microsoft's QDK seems to be already far ahead in the race at the moment.
Rather this is a **demonstrative tool** with maximum liberty in terms of licenses for education and research.

## Documentation (HIGHLY RECOMMENDED READING)
*The documentation is NOT long at all. You can take ten minutes to read the entire thing!*
(But if you are in a hurry, I would say, generate the documentation and read the [Class List](docs/html/annotated.html))

You can build your documentation using Make and Doxygen.
```
make clean
make docs
```
You should see a new directory called `docs/html` where you can access the Doxygen documentation at `index.html`.

## Quickstart
I provide you four demo codes in `demos` directory.
The examples should serve as a series of short easy-to-follow tutorials.
You can build them by running the following:
```
make clean
make demos
```
You will see `demos/*.bin` files being created.

## Build Guide
First of all, here are the things you need:
* GCC
* Make
* Eigen (C++ header-only library for linear algebra)
    * You could install the package, or simply download/clone it. The only thing that changes is the include path for Eigen library.

Suppose for example, you have the following separate project directory.
```
hadamard/
└── hadamard.cpp
```
where `hadamard.cpp` is your emulation of the usage of the Hadamard gate as the following.
```
// hadamard.cpp
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"
#include <iostream>

int main() {
    Solace::Qubits q {};
    Solace::Gate::Hadamard H;
    H.apply(q);

    std::cout << (int) q.observe() << std::endl;
    return 0;
}
```

Once you have these prerequisites installed, you have several avenues to proceed:

### Option 1: Build the libsolace library, then link with your code.
To build a dynamic library, simply run:
```
make clean
make lib
```
You will see that `libsolace.so` and `libsolace.a` were created at `bin` directory.
You can install the libsolace header at `/usr/include` (or other system path that you want),
then link your `./hadamard.cpp` with `libsolace.so` or `libsolace.a`.

Note that if you have Eigen headers downloaded/installed at different directory, you might get an error.
You can fix that by passing the directory to `EIGEN` variable in make.
```
make clean
make lib EIGEN=/usr/include/eigen3
```

### Option 2: Create standalone binary with your code
Maybe you do not want to compile the library. Maybe you just want emulation for your specific program.
In this case, you can simply compile your code with the source code of this library.

First of all, locate the include path of Eigen library (eg. /usr/include/eigen3).
Also, note the include path of the libsolace library (eg. /home/user/Documents/libsolace/include).
Then you can compile by:
```
g++ -g -Wall -I /usr/include/eigen3 -I /home/user/Documents/libsolace/include /home/user/Documents/libsolace/src/libsolace.cpp ./hadamard.cpp -o hadamard
```

### Build Options

### BE_A_QUANTUM_CHEATER
Note that viewing state vector is not allowed if it were a real quantum computer.
However, since we do not want to pull our hair out every single time we research,
you can enable the getter functions for state vectors by setting
`-DBE_A_QUANTUM_CHEATER` flag during compilation.
(It ain't cheating if you are cheating at quantum level!)

For example, if you are using Option 1, and you want to be a quantum cheater, you can run the following instead:
```
make lib CXXFLAGS=-DBE_A_QUANTUM_CHEATER
```
Rest of the CXXFLAGS will be added.

### AVOID_UNSUPPORTED_EIGEN
Since the library uses tensor products, and Eigen does not officially support tensor product (`KroneckerProduct`),
it can be "broken" in the future update. To mitigate this, I implemented the tensor product manually.
By setting `-DAVOID_UNSUPPORTED_EIGEN`, you use my manual implementation instead of Eigen's.

If you are again following Option 1, you can run the following:
```
make lib CXXFLAGS=-DAVOID_UNSUPPORTED_EIGEN
```

### Optimization
By default, all builds are `-O3` optimzied for speed.
You can change it by passing an alternative optimization level as the following:
```
make lib OPTIMIZATION=-O2
```

## Development Engagement

I welcome constructive feedbacks and contributions (which hopefully aligns with the goal of this project).
I, in fact, highly welcome fixes for bugs that *arise from my erroneous understanding of quantum computing*;
after all, I did not get a doctorate in quantum computing or anything.

