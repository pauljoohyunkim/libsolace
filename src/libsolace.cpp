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
    transformer = QuantumGateTransformer(2,2);
    auto& t { std::get<QuantumGateTransformer>(transformer) };
    t << q0_cpy, q1_cpy;

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
    auto& t { std::get<QuantumGateTransformer>(transformer) };
    for (auto i = 0; i < obj.quantumgate().matrix_size(); i++) {
        for (auto j = 0; j < obj.quantumgate().matrix(i).entry_size(); j++) {
            auto entry { obj.quantumgate().matrix(i).entry(j) };
            std::complex<double> val { entry.real(), entry.imag() };
            t(i, j) = val;
        }
    }
    validate();
}

QuantumGate QuantumGate::operator^(const QuantumGate& gate) const {
    if (!isValidated || !gate.isValidated) {
        throw std::runtime_error("Invalidated quantum gates cannot be entangled.");
    }
#if !defined(AVOID_UNSUPPORTED_EIGEN)
    // Check if both are sparse matrix.
    if (std::holds_alternative<SparseQuantumGateTransformer>(transformer) && std::holds_alternative<SparseQuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<SparseQuantumGateTransformer>(gate.transformer) };
        SparseQuantumGateTransformer t(t1.rows()*t2.rows(), t1.cols()*t2.cols());
        t = Eigen::kroneckerProduct(t1, t2);
        return QuantumGate(t);
    } else if (std::holds_alternative<SparseQuantumGateTransformer>(transformer) && std::holds_alternative<QuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<QuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { Eigen::KroneckerProduct(t1, t2) };
        return QuantumGate(t);
    } else if (std::holds_alternative<QuantumGateTransformer>(transformer) && std::holds_alternative<SparseQuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<QuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<SparseQuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { Eigen::KroneckerProduct(t1, t2) };
        return QuantumGate(t);
    } else if (std::holds_alternative<QuantumGateTransformer>(transformer) && std::holds_alternative<QuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<QuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<QuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { Eigen::KroneckerProduct(t1, t2) };
        return QuantumGate(t);
    } else {
        throw std::runtime_error("Unsupported quantum gate");
    }
#else
//#error "Not supported at the moment"
    if (std::holds_alternative<SparseQuantumGateTransformer>(transformer) && std::holds_alternative<SparseQuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<SparseQuantumGateTransformer>(gate.transformer) };
        SparseQuantumGateTransformer t(t1.rows()*t2.rows(), t1.cols()*t2.cols());
        for (auto k1 = 0; k1 < t1.outerSize(); k1++) {
            for (auto k2 = 0; k2 < t2.outerSize(); k2++) {
                for (SparseQuantumGateTransformer::InnerIterator it1(t1, k1); it1; ++it1) {
                    for (SparseQuantumGateTransformer::InnerIterator it2(t2, k2); it2; ++it2) {
                        const auto r { it1.row() };
                        const auto s { it1.col() };
                        const auto v { it2.row() };
                        const auto w { it2.col() };
                        t.insert(t2.rows() * r + v, t2.cols() * s + w) = it1.value() * it2.value();
                    }
                }
            }
        }
        t.makeCompressed();
        return QuantumGate(t);
    } else if (std::holds_alternative<SparseQuantumGateTransformer>(transformer) && std::holds_alternative<QuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<QuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { QuantumGateTransformer::Zero(t1.rows() * t2.rows(), t1.cols() * t2.cols()) };
        for (auto k = 0; k < t1.outerSize(); k++) {
            for (SparseQuantumGateTransformer::InnerIterator it(t1, k); it; ++it) {
                const auto i { it.row() };
                const auto j { it.col() };
                t.block(i * t2.rows(),
                        j * t2.cols(),
                        t2.rows(),
                        t2.cols())
                        = it.value() * t2;
            }
        }
        return QuantumGate(t);
    } else if (std::holds_alternative<QuantumGateTransformer>(transformer) && std::holds_alternative<SparseQuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<QuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<SparseQuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { QuantumGateTransformer::Zero(t1.rows() * t2.rows(), t1.cols() * t2.cols()) };
        for (auto i = 0; i < t1.rows(); i++) {
            for (auto j = 0; j < t1.cols(); j++) {
                t.block(i * t2.rows(),
                        j * t2.cols(),
                        t2.rows(),
                        t2.cols())
                        = t1(i,j) * t2;
            }
        }
        return QuantumGate(t);
    } else if (std::holds_alternative<QuantumGateTransformer>(transformer) && std::holds_alternative<QuantumGateTransformer>(gate.transformer)) {
        // Both are dense matrices.
        const auto& t1 { std::get<QuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<QuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { QuantumGateTransformer::Zero(t1.rows() * t2.rows(), t1.cols() * t2.cols()) };
        for (auto i = 0; i < t1.rows(); i++) {
            for (auto j = 0; j < t1.cols(); j++) {
                t.block(i * t2.rows(),
                        j * t2.cols(),
                        t2.rows(),
                        t2.cols())
                        = t1(i,j) * t2;
            }
        }
        return QuantumGate(t);
    } else {
        throw std::runtime_error("Unsupported quantum gate");
    }
#endif

    //return QuantumGate(t);
}

QuantumGate QuantumGate::operator*(const QuantumGate& gate) const {
    if (!isValidated || !gate.isValidated) {
        throw std::runtime_error("Invalidated quantum gates cannot be entangled.");
    }
    if (nQubit != gate.nQubit) {
        throw std::runtime_error("Mismatch in shape. Check the nQubit variable.");
    }
    if (std::holds_alternative<SparseQuantumGateTransformer>(transformer) && std::holds_alternative<SparseQuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<SparseQuantumGateTransformer>(gate.transformer) };
        SparseQuantumGateTransformer t { t1 * t2 };
        return QuantumGate(t);
    } else if (std::holds_alternative<SparseQuantumGateTransformer>(transformer) && std::holds_alternative<QuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<QuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { t1 * t2 };
        return QuantumGate(t);
    } else if (std::holds_alternative<QuantumGateTransformer>(transformer) && std::holds_alternative<SparseQuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<QuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<SparseQuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { t1 * t2 };
        return QuantumGate(t);
    } else if (std::holds_alternative<QuantumGateTransformer>(transformer) && std::holds_alternative<QuantumGateTransformer>(gate.transformer)) {
        const auto& t1 { std::get<QuantumGateTransformer>(transformer) };
        const auto& t2 { std::get<QuantumGateTransformer>(gate.transformer) };
        QuantumGateTransformer t { t1 * t2 };
        return QuantumGate(t);
    } else {
        throw std::runtime_error("Unsupported quantum gate");
    }
}

void QuantumGate::compile(const std::filesystem::path& filepath) const {
    int nRow;
    int nCol;
    if (!isValidated) {
        throw std::runtime_error("Not validated for compilation");
    }

    std::ofstream outfile { filepath, std::ios::binary };
    Compiled::QuantumObject quantumObj;

    quantumObj.set_type(Compiled::ObjectType::QUANTUM_GATE);
    quantumObj.set_nqubit(nQubit);
    auto quantumGateM { quantumObj.mutable_quantumgate() };

    if (std::holds_alternative<QuantumGateTransformer>(transformer)) {
        const auto& t { std::get<QuantumGateTransformer>(transformer) };
        nRow = t.rows();
        nCol = t.cols();
        for (auto i = 0; i < nRow; i++) {
            auto row { quantumGateM->add_matrix() };
            for (auto j = 0; j < nCol; j++) {
                auto entry { row->add_entry() };
                entry->set_real(t(i, j).real());
                entry->set_imag(t(i, j).imag());
            }
        }
    } else if (std::holds_alternative<SparseQuantumGateTransformer>(transformer)) {
        const auto& t { std::get<SparseQuantumGateTransformer>(transformer) };
        nRow = t.rows();
        nCol = t.cols();
        for (auto i = 0; i < nRow; i++) {
            auto row { quantumGateM->add_matrix() };
            for (auto j = 0; j < nCol; j++) {
                auto entry { row->add_entry() };
                entry->set_real(t.coeff(i, j).real());
                entry->set_imag(t.coeff(i, j).imag());
            }
        }
    } else {
        throw std::runtime_error("Gate is not filled for compilation.");
    }

    outfile << quantumObj.SerializeAsString();
}

void QuantumGate::apply(Qubits& q) {
    // TODO: Check the lengths.
    if (!isValidated) {
        throw std::runtime_error("Attempt to use invalid quantum gate.");
    }
    if (nQubit != q.nQubit) {
        throw std::runtime_error("Quantum gate and state vector are not compatible.");
    }

    // Multiply the transformer matrix within the class.
    if (std::holds_alternative<QuantumGateTransformer>(transformer)) {
        q.stateVector = std::get<QuantumGateTransformer>(transformer) * q.stateVector;
    } else if (std::holds_alternative<SparseQuantumGateTransformer>(transformer)) {
        q.stateVector = std::get<SparseQuantumGateTransformer>(transformer) * q.stateVector;
    }
    q.stateVector.normalize();
}

void QuantumGate::validate() {
    int m;
    int n;
    if (std::holds_alternative<QuantumGateTransformer>(transformer)) {
        const auto& t { std::get<QuantumGateTransformer>(transformer) };
        if (!t.isUnitary()) {
            throw std::runtime_error("Invalid quantum gate: not unitary");
        } else {
            m = t.rows();
            n = t.cols();
        }
    } else if (std::holds_alternative<SparseQuantumGateTransformer>(transformer)) {
        const auto& t { std::get<SparseQuantumGateTransformer>(transformer) };
        const auto iMaybe { t.adjoint() * t };
        SparseQuantumGateTransformer identity(t.rows(), t.cols());
        identity.setIdentity();
        SparseQuantumGateTransformer diff { iMaybe - identity };
        if (diff.squaredNorm() > 1e-6) {
            throw std::runtime_error("Invalid quantum gate: not unitary");
        } else {
          m = t.rows();
          n = t.cols();
        }
    } else {
        // Not filled in; variant is monostate.
        throw std::runtime_error("Invalid quantum gate; unfilled.");
    }

    if (m == 0 || m != n || ((m & (m-1)) != 0)) {
        throw std::runtime_error("Invalid quantum gate.");
    }

    nQubit = static_cast<size_t>(std::log2(m));
    isValidated = true;
}

}
