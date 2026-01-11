#include <random>
#include <numbers>
#include <cmath>
#if !defined(AVOID_UNSUPPORTED_EIGEN)
#include <unsupported/Eigen/KroneckerProduct>
#endif
#include "solace/solace.hpp"

namespace Solace {

Qubits::Qubits(const std::vector<std::complex<double>>& cs) : stateVector(cs.size()) {
    validateLength();
    for (size_t i = 0; i < cs.size(); i++) {
        stateVector(i) = cs[i];
    }
    normalizeStateVector();
}

Qubits Qubits::operator^(const Qubits& q) const {
#if !defined(AVOID_UNSUPPORTED_EIGEN)
    // Note that this uses tensor product from "unsupported" Eigen library.
    const StateVector sv { Eigen::KroneckerProduct(stateVector, q.stateVector) };
#else
    // Implementing tensor product manually.
    StateVector sv { StateVector::Zero(stateVector.size() * q.stateVector.size()) };
    for (auto i = 0; i < stateVector.size(); i++) {
        for (auto j = 0; j < q.stateVector.size(); j++) {
            sv(q.stateVector.size() * i + j) = stateVector(i) * q.stateVector(j);
        }
    }

#endif
    return Qubits(sv);
}

#if defined(BE_A_QUANTUM_CHEATER)
ObservedQubitState Qubits::observe(const bool randomphase, const bool cheat) {
#else
ObservedQubitState Qubits::observe(const bool randomphase) {
#endif
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights {};
    for (const auto v : stateVector) {
        weights.push_back(std::norm(v));
    }
    std::discrete_distribution<> dist (weights.begin(), weights.end());

    const auto observedState { (ObservedQubitState) dist(gen) };
#if defined(BE_A_QUANTUM_CHEATER)
    if (cheat == false) {
#endif    
        // State collapse
        stateVector = StateVector::Zero(stateVector.size());
        if (randomphase) {
            std::uniform_real_distribution<double> phaseDist(0, M_PI);
            const auto phase { phaseDist(gen) };
            const auto nonzero { std::exp(std::complex<double>(0, phase)) };
            stateVector(observedState) = nonzero;
        } else {
            stateVector(observedState) = 1;
        }
        
#if defined(BE_A_QUANTUM_CHEATER)
    }
#endif
    return observedState;
}

void Qubits::validateLength() {
    const auto n { stateVector.size() };
    if (n == 0 || ((n & (n-1)) != 0)) {
        throw std::runtime_error("State vector must be of length 2^N");
    }

    nQubit = static_cast<size_t>(std::log2(n));
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

QuantumGate QuantumGate::operator^(const QuantumGate& gate) const {
#if !defined(AVOID_UNSUPPORTED_EIGEN)
    const QuantumGateTransformer t { Eigen::KroneckerProduct(transformer, gate.transformer) };
#else
    QuantumGateTransformer t(transformer.rows() * gate.transformer.rows(), transformer.cols() * gate.transformer.cols());
    for (auto i = 0; i < transformer.rows(); i++) {
        for (auto j = 0; j < transformer.cols(); j++) {
            t.block(i * gate.transformer.rows(),
                    j * gate.transformer.cols(),
                    gate.transformer.rows(),
                    gate.transformer.cols())
                    = transformer(i,j) * gate.transformer;
        }
    }
#endif

    return QuantumGate(t);
}

void QuantumGate::apply(Qubits& q) {
    // TODO: Check the lengths.
    if (!isValidated) {
        throw std::runtime_error("Attempt to use invalid quantum gate.");
    }
    if (transformer.rows() != q.stateVector.size()) {
        throw std::runtime_error("Quantum gate and state vector are not compatible.");
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

    nQubit = static_cast<size_t>(std::log2(m));
}

}
