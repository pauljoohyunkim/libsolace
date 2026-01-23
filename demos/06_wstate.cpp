/**
 * This example introduces
 * 1. How to partially observe Qubits in an entangled state.
 */

#include "solace/solace.hpp"

int main() {
    // W state: 1/sqrt(3) (|001> + |010> + |100>)
    Solace::StateVector wSv(8);
    wSv << 0,
           1,
           1,
           0,
           1,
           0,
           0,
           0;
    Solace::Qubits w { wSv };

    // Observing the first qubit by bitmask 0b100

    auto partialObservation { w.observe(0b100) };

    std::cout << "Partial Observation Result: " << (int) partialObservation.first << std::endl;
    std::cout << "(This should either be 0b000(=0) or 0b100(=4))" << std::endl;

    // 2nd and 3rd qubits are still unobserved and entangled.
    // If first qubit is observed to be 0, the other two qubits have state: 1/sqrt(2) (|01> + |10>)
    // If first qubit is observed to be 1, the other two qubits have the state |00>, so they both must be zero.
    if (partialObservation.first == 0) {
        std::cout << "Since partial observation is 0, the other two qubits should full-observe to 1 or 2 with equal probability" << std::endl;
    } else {
        std::cout << "Since partial observation is 0b100(=4), the other two qubits should full-observe to 0" << std::endl;
    }

    auto unobserved { partialObservation.second.value() };
    auto restObservation { unobserved.observe() };
    std::cout << "Other two observed to be: " << (int) restObservation << std::endl;

    return 0;
}
