/**
 * This example introduces
 * 1. How to create entangled set of qubits.
 * 2. How to create "entanged quantum gates" for those qubits.
 */
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"
#include <iostream>

int main() {
    // Create a single qubit. (q = |0> by default)
    Solace::Qubits q {};
    // Compute tensor product and create entangled qubits.
    Solace::Qubits q2 { q ^ q };

    // Create Hadamard gate.
    Solace::Gate::Hadamard H;
    // Create modified Hadamard gate H2 for two qubits.
    Solace::QuantumGate H2 { H ^ H };
    // Apply Hadamard gate to the qubit.
    H2.apply(q2);

    // Measure the two qubits. The result will be 0, 1, 2, or 3 with 25% probability respectively.
    std::cout << (int) q2.observe() << std::endl;
    return 0;
}