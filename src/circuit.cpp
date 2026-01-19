#include <memory>
#include "solace/solace.hpp"
#include "solace/circuit.hpp"

namespace Solace {

// Internal structure that keeps track of quantum gates as nodes.
struct QuantumCircuitGateNode {
    std::shared_ptr<QuantumCircuitGateNode> prevNode;
    std::shared_ptr<QuantumGate> gate;
};

std::shared_ptr<QuantumCircuitComponent::Qubits> QuantumCircuit::createQubits(const size_t nQubit) {
    //auto pQ { std::make_shared<QuantumCircuitComponent::Qubits>(nQubit) };
    // Due to make_shared requiring the constructor to be public (but QuantumCircuitComponent::Qubits is not expected to be manually constructed), old-fashioned "new" keyword used.
    auto pQ { std::shared_ptr<QuantumCircuitComponent::Qubits>(new QuantumCircuitComponent::Qubits(nQubit)) };
    qubitSets.push_back(pQ);

    return pQ;
}

std::shared_ptr<QuantumGate> QuantumCircuit::addQuantumGate(const QuantumGate& gate) {
    auto pG { std::make_shared<QuantumGate>(gate) };
    gates.push_back(pG);

    return pG;
}

std::shared_ptr<QuantumCircuitComponent::Qubits> QuantumCircuit::entangle(std::vector<std::shared_ptr<QuantumCircuitComponent::Qubits>>& qubits) {
    size_t nQubit { 0 };
    // Find the total number of qubits
    for (const auto& q : qubits) {
        nQubit += q->nQubit;
    }

    auto pQ { std::shared_ptr<QuantumCircuitComponent::Qubits>(new QuantumCircuitComponent::Qubits(nQubit)) };
    for (const auto& q : qubits) {
        pQ->entangledFrom.push_back(q);
        q->entangleTo = pQ;
    }

    return pQ;
}

}
