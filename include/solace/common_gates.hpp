#ifndef __SOLACE_COMMON_GATES_HPP__
#define __SOLACE_COMMON_GATES_HPP__

#include "solace.hpp"

constexpr static std::complex<double> i { 0.0, 1.0 };

namespace Solace::Gate {
    class Identity : public Solace::QuantumGate {
        public:
            Identity(const int n=1) : Solace::QuantumGate() {
                const auto dim { 1 << n };
                transformer = QuantumGateTransformer::Identity(dim, dim);
                validate();
            }
    };

    class PauliX : public Solace::QuantumGate {
        public:
            PauliX() : Solace::QuantumGate() {
                transformer = QuantumGateTransformer(2,2);
                transformer << 0.0, 1.0,
                               1.0, 0.0;
                validate();
            }
    };

    class PauliY : public Solace::QuantumGate {
        public:
            PauliY() : Solace::QuantumGate() {
                transformer = QuantumGateTransformer(2,2);
                transformer << 0.0, -i,
                               i, 0.0;
                validate();
            }
    };

    class PauliZ : public Solace::QuantumGate {
        public:
            PauliZ() : Solace::QuantumGate() {
                transformer = QuantumGateTransformer(2,2);
                transformer << 1.0, 0.0,
                               0.0, -1.0;
                validate();
            }
    };

    // Hadamard Gate is defined by
    // 1/sqrt(2) * 
    // 1  1
    // 1 -1
    class Hadamard : public Solace::QuantumGate {
        public:
            Hadamard() : Solace::QuantumGate() {
                transformer = QuantumGateTransformer(2,2);
                Solace::StateVector q1(2);
                q1 << 1, 1;
                Solace::StateVector q2(2);
                q2 << 1, -1;

                q1.normalize();
                q2.normalize();

                transformer << q1, q2;
                validate();
            }
    };
}

#endif  // __SOLACE_COMMON_GATES_HPP__
