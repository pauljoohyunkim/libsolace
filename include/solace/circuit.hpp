#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include <vector>
#include "solace.hpp"

namespace Solace {
    class QuantumCircuit {
        public:
            QuantumCircuit() = default;

            std::shared_ptr<Qubits> addQubits(const Qubits& q);
            std::shared_ptr<QuantumGate> addQuantumGate(const QuantumGate& gate);
        private:
            // TODO: Possibly change so that it stores the tuples (labelString, std::shared_ptr<Qubits>)
            std::vector<std::shared_ptr<Qubits>> qubitSets {};
            std::vector<std::shared_ptr<QuantumGate>> gates {};
        

    };
}

#endif  // __SOLACE_CIRCUIT_HPP__
