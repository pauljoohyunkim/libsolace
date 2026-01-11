#include <algorithm>
#include "solace/utility.hpp"

namespace Solace {

Qubits entangle(const std::vector<Qubits>& qubitSets) {
    if (qubitSets.empty()) {
        throw std::runtime_error("Input vector of qubits cannot be empty");
    }

    if (qubitSets.size() == 1) {
        return qubitSets.at(0);
    }

    // If more, compute tensor product.
    Qubits result { qubitSets.at(0) };
    for (size_t i = 1; i < qubitSets.size(); i++) {
        result = result ^ qubitSets.at(i);
    }

    return result;
}

Qubits entangle(const Qubits& q, size_t n) {
    if (n == 0) {
        throw std::runtime_error("Cannot output 0 qubit system.");
    }

    if (n == 1) {
        return q;
    }

    // If more, compute tensor product.
    Qubits result { q };
    for (size_t i = 1; i < n; i++) {
        result = result ^ q;
    }

    return result;

}

QuantumGate entangle(const std::vector<QuantumGate>& quantumGates) {
    if (quantumGates.empty()) {
        throw std::runtime_error("Input vector of quantum gates cannot be empty");
    }

    if (quantumGates.size() == 1) {
        return quantumGates.at(0);
    }

    // If more, compute tensor product.
    QuantumGate result { quantumGates.at(0) };
    for (size_t i = 1; i < quantumGates.size(); i++) {
        result = result ^ quantumGates.at(i);
    }

    return result;
}

QuantumGate entangle(const QuantumGate& g, size_t n) {
    if (n == 0) {
        throw std::runtime_error("Cannot output 0 qubit system gate.");
    }

    if (n == 1) {
        return g;
    }

    // If more, compute tensor product.
    QuantumGate result { g };
    for (size_t i = 1; i < n; i++) {
        result = result ^ g;
    }

    return result;

}

}