#include <random>
#include <fstream>
#include <sstream>
#include <numbers>
#include <cmath>
#if !defined(AVOID_UNSUPPORTED_EIGEN)
#include <unsupported/Eigen/KroneckerProduct>
#endif
#include "solace/solace.hpp"
#include "solace.pb.h"

namespace Solace {

Qubits::Qubits(const std::vector<std::complex<double>>& cs) : stateVector(cs.size()) {
    validateLength();
    for (size_t i = 0; i < cs.size(); i++) {
        stateVector(i) = cs[i];
    }
    normalizeStateVector();
}

Qubits::Qubits(const std::filesystem::path& filepath) {
    std::ifstream infile { filepath, std::ios::binary };
    std::stringstream filecontentStream;
    filecontentStream << infile.rdbuf();
    
    Compiled::QuantumObject obj;
    if (!obj.ParseFromString(filecontentStream.str())) {
        throw std::runtime_error("Could not read quantum object.");
    }

    if (obj.type() != Compiled::ObjectType::QUBITS) {
        throw std::runtime_error("Wrong type of object read.");
    }

    stateVector = StateVector(1 << obj.nqubit());
    validateLength();
    for (auto i = 0; i < obj.qubits().vector().entry_size(); i++) {
        auto entry { obj.qubits().vector().entry(i) };
        std::complex<double> val { entry.real(), entry.imag() };
        stateVector(i) = val;
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

void Qubits::compile(const std::filesystem::path& filepath) const {
    std::ofstream outfile { filepath, std::ios::binary };
    Compiled::QuantumObject quantumObj;

    quantumObj.set_type(Compiled::ObjectType::QUBITS);
    quantumObj.set_nqubit(nQubit);
    
    const auto qubitsV { quantumObj.mutable_qubits()->mutable_vector() };
    for (auto cs : stateVector) {
        auto entry { qubitsV->add_entry() };
        entry->set_real(cs.real());
        entry->set_imag(cs.imag());
    }

    outfile << quantumObj.SerializeAsString();
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

    validate();
}

QuantumGate::QuantumGate(const std::filesystem::path& filepath) {
    std::ifstream infile { filepath, std::ios::binary };
    std::stringstream filecontentStream;
    filecontentStream << infile.rdbuf();
    
    Compiled::QuantumObject obj;
    if (!obj.ParseFromString(filecontentStream.str())) {
        throw std::runtime_error("Could not read quantum object.");
    }

    if (obj.type() != Compiled::ObjectType::QUANTUM_GATE) {
        throw std::runtime_error("Wrong type of object read.");
    }

    const auto dim { 1 << obj.nqubit() };
    transformer = QuantumGateTransformer(dim, dim);
    for (auto i = 0; i < obj.quantumgate().matrix_size(); i++) {
        for (auto j = 0; j < obj.quantumgate().matrix(i).entry_size(); j++) {
            auto entry { obj.quantumgate().matrix(i).entry(j) };
            std::complex<double> val { entry.real(), entry.imag() };
            transformer(i, j) = val;
        }
    }
    validate();
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

void QuantumGate::compile(const std::filesystem::path& filepath) const {
    std::ofstream outfile { filepath, std::ios::binary };
    Compiled::QuantumObject quantumObj;

    quantumObj.set_type(Compiled::ObjectType::QUANTUM_GATE);
    quantumObj.set_nqubit(nQubit);
    
    auto quantumGateM { quantumObj.mutable_quantumgate() };
    for (auto i = 0; i < transformer.rows(); i++) {
        auto row { quantumGateM->add_matrix() };
        for (auto j = 0; j < transformer.cols(); j++) {
            auto entry { row->add_entry() };
            entry->set_real(transformer(i, j).real());
            entry->set_imag(transformer(i, j).imag());
        }
    }

    outfile << quantumObj.SerializeAsString();
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
