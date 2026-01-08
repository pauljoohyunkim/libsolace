#ifndef __SOLACE_COMMON_GATES_HPP__
#define __SOLACE_COMMON_GATES_HPP__

#include "solace.hpp"

namespace Solace::Gate {
    class Identity : public Solace::QuantumGate {
        public:
            Identity() : Solace::QuantumGate({1, 0}, {0, 1}) {}
    };

    // Hadamard Gate is defined by
    // 1/sqrt(2) * 
    // 1  1
    // 1 -1
    class Hadamard : public Solace::QuantumGate {
        public:
            Hadamard() : Solace::QuantumGate({1, 1},{1, -1}) {}
    };
}

#endif  // __SOLACE_COMMON_GATES_HPP__
