#ifndef __SOLACE_COMMON_GATES_HPP__
#define __SOLACE_COMMON_GATES_HPP__

#include "solace.hpp"

namespace Solace::Gate {
    class Identity : public Solace::QuantumGate {
        public:
            Identity() : Solace::QuantumGate({1, 0}, {0, 1}) {}
    };

    class PauliX : public Solace::QuantumGate {
        public:
            PauliX() : Solace::QuantumGate({0, 1}, {1, 0}) {}
    };

    class PauliY : public Solace::QuantumGate {
        public:
            PauliY() : Solace::QuantumGate({0, std::complex<double>(0, 1)}, {std::complex<double>(0, -1), 0}) {}
    };

    class PauliZ : public Solace::QuantumGate {
        public:
            PauliZ() : Solace::QuantumGate({1, 0}, {0, -1}) {}
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
