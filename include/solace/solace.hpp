#ifndef __SOLACE_HPP__
#define __SOLACE_HPP__

#include <complex>
#include <vector>
#include <Eigen/Dense>

namespace Solace {
    // Forward Declaration
    class Qubits;
    class QuantumGate;
    
    enum ObservedQubitState {
        ZERO = 0,
        ONE = 1
    };

    using StateVector = Eigen::VectorXcd;
    using QuantumGateTransformer = Eigen::MatrixXcd;

    class Qubits {
        public:
            Qubits(const int n=1) : stateVector(StateVector::Zero(1<<n)) { validateLength(); stateVector(0) = 1.0; }
            Qubits(const std::vector<std::complex<double>>& cs);
            Qubits(const std::complex<double>& c0, const std::complex<double>& c1) : stateVector(2) { validateLength(); stateVector << c0, c1; normalizeStateVector(); }
            Qubits(const StateVector& sv) : stateVector(sv) { validateLength(); normalizeStateVector(); }

            // Entanglement (Tensor Product of State Vectors)
            Qubits operator^(const Qubits& q) const;

#if defined(BE_A_QUANTUM_CHEATER)
            ObservedQubitState observe(const bool cheat=false);
#else
            ObservedQubitState observe();
#endif

#if defined(BE_A_QUANTUM_CHEATER)
            StateVector viewStateVector() const { return stateVector; }
#endif
            
        private:
            friend class QuantumGate;
            StateVector stateVector;

            void validateLength() const;
            void normalizeStateVector() { stateVector.normalize(); }
    };

    class QuantumGate {
        public:
            QuantumGate() = default;
            // 2x2
            QuantumGate(const StateVector& q0, const StateVector& q1);
            QuantumGate(const QuantumGateTransformer& transformer) : transformer(transformer) { validate(); }

            void apply(Qubits& q);
#if defined(BE_A_QUANTUM_CHEATER)
            QuantumGateTransformer viewTransformer() const { return transformer; }
#endif
        protected:
            bool isValidated { false };
            QuantumGateTransformer transformer;

            void validate();


    };
}

#endif  // __SOLACE_HPP__
