# Demos

Here are demo programs.
Think of them as tutorials.

* [01_hadamard.cpp](01_hadamard.cpp)
    * This example shows how to create qubits and apply a predefined gate.
    * The result should either be 0 or 1 (Probabilistic)
* [02_hadamard.cpp](02_hadamard2.cpp)
    * This example shows how to "entangle" multiple qubits and abstract away many qubits as a single "system". It also shows you analogously how to construct gates that apply to multiple qubits.
    * The result should be one of 0, 1, 2, or 3. (Probabilistic)
* [03_hadamard.cpp](03_hadamard3.cpp)
    * This example shows how to use `Solace::entangle` function to create entangled qubits/gates.
    * The result should be one of 0, ... , 7. (Probabilistic)
* [04_grover.cpp](04_grover.cpp)
    * This exapmle shows how to define custom quantum gates and an actual application of Grover's algorithm to solve a quantum oracle.
    * The result should be 3. (with >99% probability)
* [05_grover2.cpp](05_grover.cpp)
    * This example shows how to "compile"/precompute the qubits and quantum gate so that once it launches subsequently, it can run much faster (and the objects can be reused in other projects.)
    * For reference, there are three files generated: `qubits.qbit` (45KB), `diffuser.qgate` (177MB), and `oracle.qgate` (60KB).
    * Loading takes a lot of time as well. Use precomputation if you know your storage access speed is faster than computation.
        * In Grover's algorithm, you could actually combine the oracle gate and the diffuser gate into one. For optimization, you could compute the combined gate, then compile it to a file. This would involve only one gate reading instead of two.
        * You could also create a gate from other project and import it over to another (if you know the dimensions and all that.)
        * Benchmark (Your results will vary!)

        | Task                              | Execution Time |
        | --------------------------------- | -------------- |
        | 12 Qubit Initialization           | 0.0570ms       |
        | Compiling 12 qubits               | 0.6680ms       |
        | Generating Grover diffusion gate  | 50994.ms       |
        | Compiling Grover diffusion gate   | 2390.5ms       |
        | Generating oracle                 | 0.3266ms       |
        | Compiling oracle                  | 0.7667ms       |
        | Loading 12 qubits                 | 6.5465ms       |
        | Loading Grover diffusion gate     | 55941.ms       |
        | Loading oracle                    | 1.0867ms       |
        * On my machine (on top of WSL2 at the moment) it seems like loading generally takes a little bit longer, possibly due to loading the values happening one entry at a time.

    * For durability of storage, you might consider saving it onto RAM disk. If your gate is sparse, definitely use sparse gate instead of a general gate.
* [06_wstate.cpp](06_wstate.cpp)
    * This example shows how partial observation is done.
* [07_bell.cpp](07_bell.cpp)
    * This example shows how to build a quantum circuit and run it later by binding qubits.
    * Note that at the time of writing, VQE is not supported. (This may change later.)
