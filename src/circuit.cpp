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
    auto pQ { std::make_shared<QuantumCircuitComponent::Qubits>(nQubit) };
    qubitSets.push_back(pQ);

    return pQ;
}

std::shared_ptr<QuantumGate> QuantumCircuit::addQuantumGate(const QuantumGate& gate) {
    auto pG { std::make_shared<QuantumGate>(gate) };
    gates.push_back(pG);

    return pG;
}

}
