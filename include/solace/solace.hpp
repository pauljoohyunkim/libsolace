#ifndef __SOLACE_HPP__
#define __SOLACE_HPP__

#include <complex>
#include <Eigen/Dense>

namespace Solace {
    // Forward Declaration
    class Qubit;
    class QuantumGate;
    
    enum ObservedQubitState {
        ZERO = 0,
        ONE = 1
    };

    using QubitStateVector = Eigen::VectorXcd;
    using QuantumGateTransformer = Eigen::MatrixXcd;

    class Qubit {
        public:
            Qubit(const int n=2) : stateVector(QubitStateVector(n)) { stateVector << 1, 0; }
            Qubit(const std::complex<double>& c0, const std::complex<double>& c1) : stateVector(2) { stateVector << c0, c1; normalizeStateVector(); }
            Qubit(const QubitStateVector& sv) : stateVector(sv) { normalizeStateVector(); }

            ObservedQubitState observe(const bool cheat=false);

#if defined(BE_A_QUANTUM_CHEATER)
            QubitStateVector viewStateVector() const { return stateVector; }
#endif
            
        private:
            friend class QuantumGate;
            QubitStateVector stateVector;

            void normalizeStateVector() { stateVector.normalize(); }
    };

    class QuantumGate {
        public:
            QuantumGate() = default;
            QuantumGate(const QubitStateVector& q0, const QubitStateVector& q1);

            void apply(Qubit& q);
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
