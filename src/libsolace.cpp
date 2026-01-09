#include <random>
#include <numbers>
#include <cmath>
#include <iostream>
#include "solace/solace.hpp"

namespace Solace {

// Computes the generalized inner product of two vectors.
static inline std::complex<double> innerProduct(const QubitStateVector& u, const QubitStateVector& v) {
    return u.dot(v);
}

ObservedQubitState Qubit::observe(const bool cheat) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights { std::norm(stateVector[0]), std::norm(stateVector[1]) };
    std::discrete_distribution<> dist (weights.begin(), weights.end());

    const auto observedState { (ObservedQubitState) dist(gen) };
    if (cheat == false) {
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
    }
    return observedState;
}

QuantumGate::QuantumGate(const QubitStateVector& q0, const QubitStateVector& q1) : transformer(QuantumGateTransformer(2, 2)), transformation{q0, q1} {
    transformation[0].normalize();
    transformation[1].normalize();

    std::cout << transformation[0] << std::endl;
    std::cout << transformation[1] << std::endl;

    QubitStateVector q0_cpy = q0;
    QubitStateVector q1_cpy = q1;
    q0_cpy.normalize();
    q1_cpy.normalize();
    transformer << q0_cpy, q1_cpy;

    // Check if [q0, q1] is actually a unitary matrix.
    // Note that within the class, q0 and q1 are already normalized.
    // Only need to check if "orthogonal"
    const auto dot { innerProduct(transformation[0], transformation[1]) };
    if (dot != 0.0 || std::abs(dot-1.0) < tolerance) {
        throw std::runtime_error("Invalid quantum gate.");
    }
}

void QuantumGate::apply(Qubit& q) {
    // Multiply the transformer matrix within the class.
    QubitStateVector qTemp { transformer * q.stateVector };
    q.stateVector = qTemp;
    q.stateVector.normalize();
}

}
