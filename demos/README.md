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
    * For reference, there are three files generated: `qubits.qbit` (44KiB), `diffuser.qgate` (176.0MiB), and `oracle.qgate`.
    * Loading takes a lot of time as well. Use precomputation if you know your storage access speed is faster than computation.
    * For durability of storage, you might consider saving it onto RAM disk.
