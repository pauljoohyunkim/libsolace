/**
 * @file
 * @brief Common quantum gates implementations.
 */
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
                transformer = Solace::SparseQuantumGateTransformer(dim, dim);
                std::get<Solace::SparseQuantumGateTransformer>(transformer).setIdentity();
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
                transformer = Solace::SparseQuantumGateTransformer(2,2);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.insert(0, 1) = 1.0;
                t.insert(1, 0) = 1.0;
                t.makeCompressed();
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
                transformer = Solace::SparseQuantumGateTransformer(2,2);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.insert(0, 1) = -i;
                t.insert(1, 0) = i;
                t.makeCompressed();
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
                transformer = Solace::SparseQuantumGateTransformer(2,2);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.insert(0, 0) = 1.0;
                t.insert(1, 1) = -1.0;
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
                const std::complex<double> v { 1/std::sqrt(2), 0.0 };
                transformer = Solace::QuantumGateTransformer(2,2);
                auto& t { std::get<Solace::QuantumGateTransformer>(transformer) };
                t(0, 0) = v;
                t(0, 1) = v;
                t(1, 0) = v;
                t(1, 1) = -v;

                /*
                // You could define the column vectors that each state maps to as well.
                Solace::StateVector q1(2);
                q1 << 1, 1;
                Solace::StateVector q2(2);
                q2 << 1, -1;

                q1.normalize();
                q2.normalize();
                auto& t { std::get<Solace::QuantumGateTransformer>(transformer) };
                t << q1, q2;
                */
                validate();
            }
    };

    /**
     * @brief CNOT gate. Acts on two-qubit system.
     * 
     */
    class CNOT : public Solace::QuantumGate {
        public:
            /**
             * @brief Construct a CNOT gate
             * 
             */
            CNOT() : Solace::QuantumGate() {
                transformer = Solace::SparseQuantumGateTransformer(4, 4);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.insert(0, 0) = 1.0;
                t.insert(1, 1) = 1.0;
                t.insert(2, 3) = 1.0;
                t.insert(3, 2) = 1.0;
                t.makeCompressed();
                validate();
            }
    };

    /**
     * @brief PhaseShift gate. Acts on one qubit
     * 
     */
    class PhaseShift : public Solace::QuantumGate {
        public:
            /**
             * @brief Construct a Phase Shift gate.
             * 
             * @param[in] phi Phase.
             */
            PhaseShift(const double phi) : Solace::QuantumGate() {
                transformer = Solace::SparseQuantumGateTransformer(2, 2);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.insert(0, 0) = 1.0;
                t.insert(1, 1) = std::exp(i * phi);
                t.makeCompressed();
                validate();
            }
    };

    /**
     * @brief Swap gate. Acts on two-qubit system.
     * 
     */
    class Swap : public Solace::QuantumGate {
        public:
            /**
             * @brief Construct a Swap gate.
             * 
             */
            Swap() : Solace::QuantumGate() {
                transformer = Solace::SparseQuantumGateTransformer(4, 4);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.insert(0, 0) = 1.0;
                t.insert(1, 2) = 1.0;
                t.insert(2, 1) = 1.0;
                t.insert(3, 3) = 1.0;
                t.makeCompressed();
                validate();
            }
    };

    /**
     * @brief Toffoli CCNOT gate. Acts on three-qubit system.
     * 
     */
    class CCNOT : public Solace::QuantumGate {
        public:
            /**
             * @brief Construct a Toffoli CCNOT gate
             * 
             */
            CCNOT() : Solace::QuantumGate() {
                transformer = Solace::SparseQuantumGateTransformer(8, 8);
                auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
                t.setIdentity();
                t.coeffRef(6, 6) = 0.0;
                t.coeffRef(6, 7) = 1.0;
                t.coeffRef(7, 6) = 1.0;
                t.coeffRef(7, 7) = 0.0;
                t.makeCompressed();
                validate();
            }
    };
}

#endif  // __SOLACE_COMMON_GATES_HPP__
