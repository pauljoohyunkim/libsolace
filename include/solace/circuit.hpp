#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include <vector>
#include "solace.hpp"

namespace Solace {

namespace QuantumCircuitComponent {
    class Qubits {
        public:
            Qubits(size_t nQubit=1) : nQubit(nQubit) { 
                if (nQubit == 0) {
                    throw std::runtime_error("Cannot create Qubits component of 0 qubits.");
                }
            }

        private:
            const size_t nQubit;
    };
}

/**
 * @brief A description of the wiring of the input qubits and quantum logic gates, rather than the actual qubits and gates.
 * 
 */
class QuantumCircuit {
    public:
        QuantumCircuit() = default;

        // TODO: Change so that there is a separation between the circuit and the implementation of quantum gates.
        // Should remove addQubits and replace them with something like createQubits, and later when running, allow "inserting qubit".
        // As for addQuantumGate, I think it will be fine, as the gates do not change according to run-time.
        // TODO: Support classical feedback in the future.
        std::shared_ptr<QuantumCircuitComponent::Qubits> createQubits(const size_t nQubit=1);
        std::shared_ptr<QuantumGate> addQuantumGate(const QuantumGate& gate);
    private:
        // TODO: Possibly change so that it stores the tuples (labelString, std::shared_ptr<Qubits>)
        std::vector<std::shared_ptr<QuantumCircuitComponent::Qubits>> qubitSets {};
        std::vector<std::shared_ptr<QuantumGate>> gates {};
    

};

}

#endif  // __SOLACE_CIRCUIT_HPP__
