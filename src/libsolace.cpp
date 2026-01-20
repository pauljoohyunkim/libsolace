#include <random>
#include <fstream>
#include <sstream>
#include <numbers>
#include <algorithm>
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

    stateVector = StateVector(1 << obj.qubits().nqubit());
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
ObservedQubitState Qubits::cheatObserve() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights {};

    // Build Born interpretation probability vector.
    for (const auto v : stateVector) {
        weights.push_back(std::norm(v));
    }
    // Choose based on the probability vector
    std::discrete_distribution<> dist (weights.begin(), weights.end());
    const auto observedState { (ObservedQubitState) dist(gen) };

    return observedState;
}
#endif

std::pair<ObservedQubitState, std::optional<Qubits>> Qubits::observe(const unsigned int bitmask) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights {};

    if (bitmask == 0) {
        // Observe all.

        // Build Born interpretation probability vector.
        for (const auto v : stateVector) {
            weights.push_back(std::norm(v));
        }
        // Choose based on the probability vector
        std::discrete_distribution<> dist (weights.begin(), weights.end());
        const auto observedState { (ObservedQubitState) dist(gen) };

        // State collapse
        stateVector = StateVector::Zero(stateVector.size());
        stateVector(observedState) = 1;
            
        return { observedState, std::nullopt };
    } else {
        // Observing specific qubits.
        // TODO: Check if bitmask is valid with respect to nQubits.
        std::vector<ObservedQubitState> observableStates {};
        //std::vector<ObservedQubitState> unobservableStates {};

        // Build Born interpretation probability vector.
        // Determine which states are observable through the mask, and modify the weight vector.
        const auto stateVectorLen { stateVector.size() };
        weights.resize(stateVectorLen, 0.0);
        for (ObservedQubitState state = 0; state < stateVectorLen; state++) {
            const bool isObservableState { (state & (~bitmask)) == 0 };
            // The potential output state that current state will aggregate to.
            const ObservedQubitState filtered { state & bitmask };

            if (isObservableState) {
                observableStates.push_back(state);
            } else {
                //unobservableStates.push_back(state);
            }
            
            weights.at(filtered) += std::norm(stateVector(state));
        }

        // Choose based on the probability vector
        std::discrete_distribution<> dist (weights.begin(), weights.end());
        const auto observedState { (ObservedQubitState) dist(gen) };
        const auto it { std::find(observableStates.begin(), observableStates.end(), observedState) };
        const auto index { std::distance(observableStates.begin(), it) };

        // Build a new qubits object from unobserved.
        //const size_t unobservedStateVectorLength { stateVectorLen / observableStates.size() };
        //StateVector unobservedStateVector { StateVector::Zero(unobservedStateVectorLength) };
        std::vector<std::complex<double>> unobservedStateVector {};
        const auto nCondition { observedState & bitmask };    // This encodes the necessary condition for the rest of the entangled qubits
        for (ObservedQubitState i = 0; i < stateVectorLen; i++) {
            // Conditional
            if ((nCondition & bitmask) == (i & bitmask)) {
                unobservedStateVector.push_back(stateVector(i));
            }
        }
        if ((size_t) stateVectorLen != observableStates.size() * unobservedStateVector.size()) {
            throw std::runtime_error("Something went wrong when computing the entangled subsystem.");
        }
        // Automatically normalized by the constructor.
        Qubits unobservedQubits { unobservedStateVector };

        // Finally, state vector collapse.
        stateVector = StateVector::Zero(observableStates.size());
        stateVector(index) = 1;
        validateLength();

        return { observedState, unobservedQubits };
    }
}

void Qubits::compile(const std::filesystem::path& filepath) const {
    std::ofstream outfile { filepath, std::ios::binary };
    Compiled::QuantumObject quantumObj;

    quantumObj.set_type(Compiled::ObjectType::QUBITS);
    quantumObj.mutable_qubits()->set_nqubit(nQubit);
    
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

    if (obj.type() != Compiled::ObjectType::QUANTUM_GATE && obj.type() != Compiled::ObjectType::SPARSE_QUANTUM_GATE) {
        throw std::runtime_error("Wrong type of object read.");
    }

    size_t dim;
    if (obj.type() == Compiled::ObjectType::QUANTUM_GATE) {
        dim = 1 << obj.quantumgate().nqubit();
        transformer = QuantumGateTransformer(dim, dim);
        auto& t { std::get<QuantumGateTransformer>(transformer) };
        for (auto i = 0; i < obj.quantumgate().matrix_size(); i++) {
            for (auto j = 0; j < obj.quantumgate().matrix(i).entry_size(); j++) {
                auto entry { obj.quantumgate().matrix(i).entry(j) };
                std::complex<double> val { entry.real(), entry.imag() };
                t(i, j) = val;
            }
        }
    } else {
        // Check if the number of row indices, column indices and nonzero vals are equal.
        dim = 1 << obj.sparsequantumgate().nqubit();
        auto sparseQuantumGate { obj.sparsequantumgate() };
        if (sparseQuantumGate.rowindices_size() != sparseQuantumGate.colindices_size() || sparseQuantumGate.colindices_size() != sparseQuantumGate.nonzerovals().entry_size()) {
            throw std::runtime_error("Malformed sparse quantum gate object.");
        }
        const auto nNonZeroVals { sparseQuantumGate.nonzerovals().entry_size() };
        transformer = SparseQuantumGateTransformer(dim, dim);
        auto& t { std::get<SparseQuantumGateTransformer>(transformer) };
        for (auto i = 0; i < nNonZeroVals; i++) {
            const auto row { sparseQuantumGate.rowindices(i) };
            const auto col { sparseQuantumGate.colindices(i) };
            const auto val { sparseQuantumGate.nonzerovals().entry(i) };
            t.insert(row, col) = std::complex<double>(val.real(), val.imag());
        }
        t.makeCompressed();
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
    if (!isValidated) {
        throw std::runtime_error("Not validated for compilation");
    }

    std::ofstream outfile { filepath, std::ios::binary };
    Compiled::QuantumObject quantumObj { buildProto() };

    outfile << quantumObj.SerializeAsString();
}

void QuantumGate::apply(Qubits& q) {
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

Compiled::QuantumObject QuantumGate::buildProto() const {
    if (!isValidated) {
        throw std::runtime_error("Not validated for proto building");
    }

    Compiled::QuantumObject quantumObj;
    int nRow;
    int nCol;

    if (std::holds_alternative<QuantumGateTransformer>(transformer)) {
        quantumObj.set_type(Compiled::ObjectType::QUANTUM_GATE);
        auto quantumGateM { quantumObj.mutable_quantumgate() };
        quantumGateM->set_nqubit(nQubit);
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
        quantumObj.set_type(Compiled::ObjectType::SPARSE_QUANTUM_GATE);
        auto quantumGateM { quantumObj.mutable_sparsequantumgate() };
        quantumGateM->set_nqubit(nQubit);
        const auto& t { std::get<SparseQuantumGateTransformer>(transformer) };
        for (auto k = 0; k < t.outerSize(); k++) {
            for (SparseQuantumGateTransformer::InnerIterator it(t, k); it; ++it) {
                const auto i { it.row() };
                const auto j { it.col() };
                const auto val { it.value() };
                quantumGateM->add_rowindices(i);
                quantumGateM->add_colindices(j);
                auto entry { quantumGateM->mutable_nonzerovals()->add_entry() };
                entry->set_real(val.real());
                entry->set_imag(val.imag());
            }
        }
    } else {
        throw std::runtime_error("Gate is not filled for proto building / compilation.");
    }

    return quantumObj;
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
        auto& t { std::get<SparseQuantumGateTransformer>(transformer) };
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

        if (!t.isCompressed()) {
            t.makeCompressed();
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
