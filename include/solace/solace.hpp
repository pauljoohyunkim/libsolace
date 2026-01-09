#ifndef __SOLACE_HPP__
#define __SOLACE_HPP__

#include <complex>
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
            Qubits(const int n=1) : stateVector(StateVector::Zero(1<<n)) { stateVector(0) = 1.0; }
            Qubits(const std::complex<double>& c0, const std::complex<double>& c1) : stateVector(2) { stateVector << c0, c1; normalizeStateVector(); }
            Qubits(const StateVector& sv) : stateVector(sv) { normalizeStateVector(); }

            ObservedQubitState observe(const bool cheat=false);

#if defined(BE_A_QUANTUM_CHEATER)
            StateVector viewStateVector() const { return stateVector; }
#endif
            
        private:
            friend class QuantumGate;
            StateVector stateVector;

            void normalizeStateVector() { stateVector.normalize(); }
    };

    class QuantumGate {
        public:
            QuantumGate() = default;
            QuantumGate(const StateVector& q0, const StateVector& q1);

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
