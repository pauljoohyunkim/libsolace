#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include <vector>
#include "solace.hpp"

namespace Solace {
    class QuantumCircuit {
        public:
            QuantumCircuit() = default;
        private:
            std::vector<Qubits> qubitSets {};
            std::vector<QuantumGate> gates {};
        

    };
}

#endif  // __SOLACE_CIRCUIT_HPP__
