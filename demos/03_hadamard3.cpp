/**
 * This example introduces
 * 1. How to use "helper" function to create entangled qubits in a more elegant way.
 */
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"
#include "solace/utility.hpp"
#include <iostream>

int main() {
    // Create a single qubit. (q = |0> by default)
    Solace::Qubits q {};
    // Do q x q
    Solace::Qubits q3 { Solace::entangle(q, 3) };

    // Create Hadamard gate.
    Solace::Gate::Hadamard H;
    // Create modified Hadamard gate H3 for three qubits.
    Solace::QuantumGate H3 { Solace::entangle(H, 3) };
    // Apply Hadamard gate to the qubit.
    H3.apply(q3);

    // Measure the three qubits. The result will be 0, 1, 2, 3, 4, 5, 6, or 7 with 12.5% probability respectively.
    std::cout << (int) q3.observe() << std::endl;
    return 0;
}