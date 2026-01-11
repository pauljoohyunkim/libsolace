/**
 * This example introduces
 * 1. How to create a qubit.
 * 2. How to use a predefined quantum gate.
 * 3. How to apply a quantum gate to a qubit.
 * 4. How to observe the qubit.
 */
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"
#include <iostream>

int main() {
    // Create a single qubit. (q = |0> by default)
    Solace::Qubits q {};
    // Create Hadamard gate.
    Solace::Gate::Hadamard H;
    // Apply Hadamard gate to the qubit.
    H.apply(q);

    // Measure the qubit. The result will be 0 or 1 with 50% probability respectively.
    std::cout << (int) q.observe() << std::endl;
    return 0;
}