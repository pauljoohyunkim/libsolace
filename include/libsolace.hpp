#ifndef __LIBSOLACE_HPP__
#define __LIBSOLACE_HPP__

#include <complex>
#include <utility>

namespace Solace {
    enum ObservedQubitState {
        ZERO = 0,
        ONE = 1
    };

    using QubitStateVector = std::pair<std::complex<double>, std::complex<double>>;

    class Qubit {
        public:
            Qubit(const std::complex<double>& c0, const std::complex<double>& c1) : stateVector({c0, c1}) { normalizeStateVector(); }
            Qubit(const QubitStateVector& sv) : stateVector(sv) { normalizeStateVector(); }

            ObservedQubitState observe(const bool cheat=false);

#if defined(BE_A_QUANTUM_CHEATER)
            QubitStateVector viewStateVector() const { return stateVector; }
#endif
            
        private:
            QubitStateVector stateVector;

            void normalizeStateVector();
    };
}

#endif
