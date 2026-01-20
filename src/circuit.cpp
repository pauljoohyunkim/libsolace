#include <memory>
#include <fstream>
#include <unordered_set>
#include "solace.pb.h"
#include "solace/solace.hpp"
#include "solace/circuit.hpp"

namespace Solace {

QuantumCircuit::QuantumCircuit(const std::filesystem::path& filepath) {
    std::ifstream infile { filepath, std::ios::binary };
    std::stringstream filecontentStream;
    filecontentStream << infile.rdbuf();

    Compiled::QuantumObject obj;
    if (!obj.ParseFromString(filecontentStream.str())) {
        throw std::runtime_error("Could not read quantum object.");
    }

    if (obj.type() != Compiled::ObjectType::QUANTUM_CIRCUIT) {
        throw std::runtime_error("Wrong type of object read.");
    }

    auto& qcProto { obj.quantumcircuit() };

    // Load quantum gates.
    for (const auto& gProto : qcProto.gate()) {
        if (gProto.type() == Compiled::ObjectType::QUANTUM_GATE) {
            addQuantumGate(QuantumGate(gProto.quantumgate()));
        } else if (gProto.type() == Compiled::ObjectType::SPARSE_QUANTUM_GATE) {
            addQuantumGate(QuantumGate(gProto.sparsequantumgate()));
        } else {
            throw std::runtime_error("Invalid quantum gate type when loading circuit.");
        }
    }

    // Load Qubits and recover node information
    for (const auto& qsProto : qcProto.qubitset()) {
        auto index { createQubits(qsProto.nqubit()) };
        auto& q { qubitSets.at(index) };
        q.entangleTo = qsProto.entangleto();
        q.label = qsProto.label();
        for (const auto gRef : qsProto.appliedgates()) {
            q.appliedGates.push_back(gRef);
        }
        for (const auto eFrom : qsProto.entangledfrom()) {
            q.entangledFrom.push_back(eFrom);
        }
    }
}

QuantumCircuit::QubitsRef QuantumCircuit::createQubits(const size_t nQubit) {
    //auto pQ { std::make_shared<QuantumCircuitComponent::Qubits>(nQubit) };
    // Due to make_shared requiring the constructor to be public (but QuantumCircuitComponent::Qubits is not expected to be manually constructed), old-fashioned "new" keyword used.
    const auto ref { static_cast<QuantumCircuit::QubitsRef>(qubitSets.size()) };
    qubitSets.push_back(QuantumCircuitComponent::Qubits(*this, nQubit));

    return ref;
}

QuantumCircuit::QuantumGateRef QuantumCircuit::addQuantumGate(const QuantumGate& gate) {
    const auto ref { static_cast<QuantumCircuit::QuantumGateRef>(gates.size()) };
    gates.push_back(gate);

    return ref;
}

void QuantumCircuit::applyQuantumGateToQubits(const QuantumGateRef g, const QubitsRef q) { 
    auto& qubits { qubitSets.at(q) };
    if (!qubits.isTerminal()) {
        throw std::runtime_error("Cannot apply gate when it is not a terminal qubit set.");
    }
    qubits.applyQuantumGate(g);
}

QuantumCircuit::QubitsRef QuantumCircuit::entangle(std::vector<QubitsRef>& qubits) {
    size_t nQubit { 0 };
    // Find the total number of qubits, while checking if any of them have already been entangled.
    {
        std::unordered_set<QuantumCircuit::QubitsRef> seenRefs {};
        for (const auto& qRef : qubits) {
            if (seenRefs.count(qRef)) {
                throw std::runtime_error("Duplicate Qubits component detected.");
            }
            seenRefs.insert(qRef);

            const auto& q { qubitSets.at(qRef) };
            if (!q.isTerminal()) {
                throw std::runtime_error("Already entangled Qubits component passed.");
            }
            nQubit += q.nQubit;
        }
    }

    auto Q { QuantumCircuitComponent::Qubits(*this, nQubit) };
    const auto ref { static_cast<QuantumCircuit::QubitsRef>(qubitSets.size()) };
    for (const auto& qRef : qubits) {
        auto& q { qubitSets.at(qRef) };
        Q.entangledFrom.push_back(qRef);
        q.entangleTo = ref;
    }
    qubitSets.push_back(Q);

    return ref;
}

void QuantumCircuit::compile(const std::filesystem::path& filepath) const {
    std::ofstream outfile { filepath, std::ios::binary };
    Compiled::QuantumObject quantumObj;

    auto protoCircuit { quantumObj.mutable_quantumcircuit() };
    quantumObj.set_type(Compiled::ObjectType::QUANTUM_CIRCUIT);

    // Gates
    for (const auto& gate : gates) {
        // Build proto from Core Solace side, then extract info.
        const auto gateObjProto { gate.buildProto() };
        auto addedGate { protoCircuit->add_gate() };
        if (gateObjProto.type() == Compiled::ObjectType::QUANTUM_GATE) {
            // Dense gate
            addedGate->set_type(Compiled::ObjectType::QUANTUM_GATE);
            *addedGate->mutable_quantumgate() = gateObjProto.quantumgate();
        } else if (gateObjProto.type() == Compiled::ObjectType::SPARSE_QUANTUM_GATE) {
            // Sparse gate
            addedGate->set_type(Compiled::ObjectType::SPARSE_QUANTUM_GATE);
            *addedGate->mutable_sparsequantumgate() = gateObjProto.sparsequantumgate();
        } else {
            // Not a gate. Throw error.
            throw std::runtime_error("Compilation process expected a gate, but did not parse a gate.");
        }
    }

    // Qubits
    for (const auto& q : qubitSets) {
        auto addedQubitset { protoCircuit->add_qubitset() };
        addedQubitset->set_label(q.label);
        addedQubitset->set_nqubit(q.nQubit);
        addedQubitset->set_entangleto(q.entangleTo);
        for (const auto gRef : q.appliedGates) {
            addedQubitset->add_appliedgates(gRef);
        }
        for (const auto eFrom : q.entangledFrom) {
            addedQubitset->add_entangledfrom(eFrom);
        }
    }

    outfile << quantumObj.SerializeAsString();
}

void QuantumCircuit::bindQubit(const QubitsRef qRef, const Qubits& qubits) {
    if (qRef >= qubitSets.size()) {
        throw std::runtime_error("Qubits component of such reference number does not exist.");
    }
    auto& qComponent { qubitSets.at(qRef) };
    if (!qComponent.isInitial()) {
        throw std::runtime_error("Cannot bind to a non-initial Qubits component.");
    }
    qComponent.bindQubits(qubits);
}

}
