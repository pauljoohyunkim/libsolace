#ifndef __SOLACE_COMMON_GATES_HPP__
#define __SOLACE_COMMON_GATES_HPP__

#include "solace.hpp"

constexpr static std::complex<double> i { 0.0, 1.0 };

namespace Solace::Gate {
    /**
     * @class identity gate. Acts on a set of qubit to do nothing.
     */
    class Identity : public Solace::QuantumGate {
        public:
            /**
             * @brief Constructor for identity gate.
             * @param[in] n The number of qubits to act on.
             */
            Identity(const int n=1) : Solace::QuantumGate() {
                const auto dim { 1 << n };
                transformer = QuantumGateTransformer::Identity(dim, dim);
                validate();
            }
    };

    /**
     * @class Pauli-X gate. Acts on a qubit.
     */
    class PauliX : public Solace::QuantumGate {
        public:
            /**
             * @brief Constructor for Pauli-X gate.
             */
            PauliX() : Solace::QuantumGate() {
                transformer = QuantumGateTransformer(2,2);
                transformer << 0.0, 1.0,
                               1.0, 0.0;
                validate();
            }
    };

    /**
     * @class Pauli-Y gate. Acts on a qubit.
     */
    class PauliY : public Solace::QuantumGate {
        public:
            /**
             * @brief Constructor for Pauli-Y gate.
             */
            PauliY() : Solace::QuantumGate() {
                transformer = QuantumGateTransformer(2,2);
                transformer << 0.0, -i,
                               i, 0.0;
                validate();
            }
    };

    /**
     * @class Pauli-Z gate. Acts on a qubit.
     */
    class PauliZ : public Solace::QuantumGate {
        public:
            /**
             * @brief Constructor for Pauli-Z gate.
             */
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
    /**
     * @class Hadamard gate. Acts on a qubit.
     */
    class Hadamard : public Solace::QuantumGate {
        public:
            /**
             * @brief Constructor for Hadamard gate.
             */
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
