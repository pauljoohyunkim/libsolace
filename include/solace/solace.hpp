#ifndef __SOLACE_HPP__
#define __SOLACE_HPP__

#include <complex>
#include <eigen3/Eigen/Dense>

namespace Solace {
    // Forward Declaration
    class Qubit;
    class QuantumGate;
    
    enum ObservedQubitState {
        ZERO = 0,
        ONE = 1
    };

    //using QubitStateVector = std::pair<std::complex<double>, std::complex<double>>;
    //using QubitStateVector = Eigen::Vector<std::complex<double>, 2>;
    using QubitStateVector = Eigen::VectorXcd;

    class Qubit {
        public:
            Qubit(const int n=2) : stateVector(Eigen::VectorXcd(n)) { stateVector << 1; }
            Qubit(const std::complex<double>& c0, const std::complex<double>& c1) : stateVector(2) { stateVector << c0, c1; normalizeStateVector(); }
            Qubit(const QubitStateVector& sv) : stateVector(sv) { normalizeStateVector(); }

            ObservedQubitState observe(const bool cheat=false);

#if defined(BE_A_QUANTUM_CHEATER)
            QubitStateVector viewStateVector() const { return stateVector; }
#endif
            
        private:
            friend class QuantumGate;
            QubitStateVector stateVector;

            void normalizeStateVector();
    };

    class QuantumGate {
        public:
            QuantumGate(const QubitStateVector& q0, const QubitStateVector& q1);

            void apply(Qubit& q);
        protected:
            const double tolerance { 0.0000000001 };
            QubitStateVector transformation[2];


    };
}

#endif  // __SOLACE_HPP__
