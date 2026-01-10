#include <random>
#include <numbers>
#include <cmath>
#include "solace/solace.hpp"

namespace Solace {

Qubits::Qubits(const std::vector<std::complex<double>>& cs) : stateVector(cs.size()) {
    validateLength();
    for (size_t i = 0; i < cs.size(); i++) {
        stateVector(i) = cs[i];
    }
    normalizeStateVector();
}

#if defined(BE_A_QUANTUM_CHEATER)
ObservedQubitState Qubits::observe(const bool cheat) {
#else
ObservedQubitState Qubits::observe() {
#endif
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights { std::norm(stateVector[0]), std::norm(stateVector[1]) };
    std::discrete_distribution<> dist (weights.begin(), weights.end());

    const auto observedState { (ObservedQubitState) dist(gen) };
#if defined(BE_A_QUANTUM_CHEATER)
    if (cheat == false) {
#endif    
        // State collapse
        std::uniform_real_distribution<double> phaseDist(0, M_PI);
        const auto phase { phaseDist(gen) };
        const auto nonzero { std::exp(std::complex<double>(0, phase)) };
        const auto complexzero { std::complex<double>(0, 0) };
        stateVector = Eigen::VectorXcd(2);
        switch (observedState) {
            case ObservedQubitState::ZERO:
                stateVector << nonzero, complexzero;
                break;
            case ObservedQubitState::ONE:
                stateVector << complexzero, nonzero;
                break;
            default:
                throw std::runtime_error("Unexpected state!");
        }
#if defined(BE_A_QUANTUM_CHEATER)
    }
#endif
    return observedState;
}

void Qubits::validateLength() const {
    const auto n { stateVector.size() };
    if (n == 0 || ((n & (n-1)) != 0)) {
        throw std::runtime_error("State vector must be of length 2^N");
    }
}

QuantumGate::QuantumGate(const StateVector& q0, const StateVector& q1) : transformer(QuantumGateTransformer(2, 2)) {
    if (q0.size() != 2 || q1.size() != 2) {
        throw std::runtime_error("Invalid quantum gate.");
    }
    StateVector q0_cpy = q0;
    StateVector q1_cpy = q1;
    q0_cpy.normalize();
    q1_cpy.normalize();
    transformer << q0_cpy, q1_cpy;

    // Check if [q0, q1] is actually a unitary matrix.
    if (!transformer.isUnitary()) {
        throw std::runtime_error("Invalid quantum gate.");
    }

    isValidated = true;
}

void QuantumGate::apply(Qubits& q) {
    if (!isValidated) {
        throw std::runtime_error("Attempt to use invalid quantum gate.");
    }

    // Multiply the transformer matrix within the class.
    StateVector qTemp { transformer * q.stateVector };
    q.stateVector = qTemp;
    q.stateVector.normalize();
}

void QuantumGate::validate() {
    if (transformer.isUnitary()) {
        isValidated = true;
    } else {
        throw std::runtime_error("Invalid quantum gate.");
    }

    const auto m { transformer.rows() };
    const auto n { transformer.cols() };
    if (m == 0 || m != n || ((m & (m-1)) != 0)) {
        throw std::runtime_error("Invalid quantum gate.");
    }
}

}
