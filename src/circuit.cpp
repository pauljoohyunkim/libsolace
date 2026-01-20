#include <memory>
#include "solace/solace.hpp"
#include "solace/circuit.hpp"

namespace Solace {

// Internal structure that keeps track of quantum gates as nodes.
struct QuantumCircuitGateNode {
    std::shared_ptr<QuantumCircuitGateNode> prevNode;
    std::shared_ptr<QuantumGate> gate;
};

QuantumCircuit::QubitsRef QuantumCircuit::createQubits(const size_t nQubit) {
    //auto pQ { std::make_shared<QuantumCircuitComponent::Qubits>(nQubit) };
    // Due to make_shared requiring the constructor to be public (but QuantumCircuitComponent::Qubits is not expected to be manually constructed), old-fashioned "new" keyword used.
    const auto ref { static_cast<QuantumCircuit::QubitsRef>(qubitSets.size()) };
    qubitSets.push_back(QuantumCircuitComponent::Qubits(nQubit));

    return ref;
}

QuantumCircuit::QuantumGateRef QuantumCircuit::addQuantumGate(const QuantumGate& gate) {
    const auto ref { static_cast<QuantumCircuit::QuantumGateRef>(gates.size()) };
    gates.push_back(gate);

    return ref;
}

QuantumCircuit::QubitsRef QuantumCircuit::entangle(std::vector<QubitsRef>& qubits) {
    size_t nQubit { 0 };
    // Find the total number of qubits
    for (const auto& qRef : qubits) {
        const auto& q { qubitSets.at(qRef) };
        nQubit += q.nQubit;
    }

    //auto pQ { std::shared_ptr<QuantumCircuitComponent::Qubits>(new QuantumCircuitComponent::Qubits(nQubit)) };
    auto Q { QuantumCircuitComponent::Qubits(nQubit) };
    const auto ref { static_cast<QuantumCircuit::QubitsRef>(qubitSets.size()) };
    for (const auto& qRef : qubits) {
        auto& q { qubitSets.at(qRef) };
        Q.entangledFrom.push_back(qRef);
        q.entangleTo = ref;
    }
    qubitSets.push_back(Q);

    return ref;
}

}
