#include <random>
#include <numbers>
#include <cmath>
#include "solace/solace.hpp"

namespace Solace {

// Normalizes the vector to length 1.
// Fails if v is zero vector.
static inline void normalizeVector(QubitStateVector& v) {
    const auto len { std::sqrt(std::norm(v.first) + std::norm(v.second)) };
    if (len == 0) {
        throw std::runtime_error("Zero vector cannot be normalized.");
    }
    QubitStateVector sv { v.first / len, v.second / len };
    v = sv;
}

// Computes the generalized inner product of two vectors.
static inline std::complex<double> innerProduct(const QubitStateVector& u, const QubitStateVector& v) {
    const QubitStateVector uconj { std::conj(u.first), std::conj(u.second) };
    return uconj.first * v.first + uconj.second * v.second;
}

ObservedQubitState Qubit::observe(const bool cheat) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights { std::norm(stateVector.first), std::norm(stateVector.second) };
    std::discrete_distribution<> dist (weights.begin(), weights.end());

    const auto observedState { (ObservedQubitState) dist(gen) };
    if (cheat == false) {
        // State collapse
        std::uniform_real_distribution<double> phaseDist(0, M_PI);
        const auto phase { phaseDist(gen) };
        const auto nonzero { std::exp(std::complex<double>(0, phase)) };
        const auto complexzero { std::complex<double>(0, 0) };
        switch (observedState) {
            case ObservedQubitState::ZERO:
                stateVector = { nonzero, complexzero };
                break;
            case ObservedQubitState::ONE:
                stateVector = { complexzero, nonzero };
                break;
            default:
                throw std::runtime_error("Unexpected state!");
        }
    }
    return observedState;
}

void Qubit::normalizeStateVector() {
    normalizeVector(stateVector);
}

QuantumGate::QuantumGate(const QubitStateVector& q0, const QubitStateVector& q1) : transformation{q0, q1} {
    normalizeVector(transformation[0]);
    normalizeVector(transformation[1]);

    // Check if [q0, q1] is actually a unitary matrix.
    // Note that within the class, q0 and q1 are already normalized.
    // Only need to check if "orthogonal"
    const auto dot { innerProduct(transformation[0], transformation[1]) };
    if (dot != 0.0 || std::abs(dot-1.0) < tolerance) {
        throw std::runtime_error("Invalid quantum gate.");
    }
}

void QuantumGate::apply(Qubit& q) {
    // Multiply the 2x2 matrix within the class.
    const auto state0 { q.stateVector.first * transformation[0].first + q.stateVector.second * transformation[1].first };
    const auto state1 { q.stateVector.first * transformation[0].second + q.stateVector.second * transformation[1].second };
    q.stateVector = { state0, state1 };
    normalizeVector(q.stateVector);
}

}
