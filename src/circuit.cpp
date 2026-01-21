#include <memory>
#include <fstream>
#include <unordered_set>
#include "solace.pb.h"
#include "solace/solace.hpp"
#include "solace/utility.hpp"
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
        // nQubit
        auto index { createQubits(qsProto.nqubit()) };
        auto& q { qubitSets.at(index) };
        // appliedGates
        for (const auto gRef : qsProto.appliedgates()) {
            q.appliedGates.push_back(gRef);
        }
        // inLinkageType & inLinkage
        if (qsProto.inlinkagetype() == Compiled::LinkageType::NONE) {
            q.inLink = std::monostate();
        } else if (qsProto.inlinkagetype() == Compiled::LinkageType::ENTANGLEMENT) {
            q.inLink = std::vector<QuantumCircuit::QubitsRef>();
            auto& entangledFrom { std::get<std::vector<QuantumCircuit::QubitsRef>>(q.inLink) };
            for (const auto eFrom : qsProto.entangledfrom().entangledfrom()) {
                entangledFrom.push_back(eFrom);
            }
        } else if (qsProto.inlinkagetype() == Compiled::LinkageType::OBSERVATION) {
            q.inLink = qsProto.observedfrom();
        } else {
            // Not possible.
            throw std::runtime_error("Invalid inLinkageType from protobuf.");
        }

        // outLinkageType & outLinkage
        if (qsProto.outlinkagetype() == Compiled::LinkageType::NONE) {
            q.outLink = std::monostate();
        } else if (qsProto.outlinkagetype() == Compiled::LinkageType::ENTANGLEMENT) {
            q.outLink = qsProto.entangleto();
        } else if (qsProto.outlinkagetype() == Compiled::LinkageType::OBSERVATION) {
            // Full or Partial
            if (qsProto.observationscheme().bitmask() == 0) {
                // Full
                q.outLink = qsProto.observationscheme().observedto();
            } else {
                // Partial
                q.outLink = QuantumCircuitComponent::Qubits::PartialObservationScheme {
                    .bitmask = qsProto.observationscheme().bitmask(),
                    .observedTo = qsProto.observationscheme().observedto(),
                    .unobservedTo = qsProto.observationscheme().unobservedto()
                };
            }
        } else {
            // Not possible.
            throw std::runtime_error("Invalid outLinkageType from protobuf.");
        }
        
        //label
        q.label = qsProto.label();
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
    Q.inLink = std::vector<QuantumCircuit::QubitsRef>();
    auto& entangledFrom { std::get<std::vector<QuantumCircuit::QubitsRef>>(Q.inLink) };
    for (const auto& qRef : qubits) {
        auto& q { qubitSets.at(qRef) };
        entangledFrom.push_back(qRef);
        q.outLink = ref;
    }
    qubitSets.push_back(Q);

    return ref;
}

QuantumCircuit::QubitsRef QuantumCircuit::markForObservation(const QubitsRef q) {
    auto& qComponent { qubitSets.at(q) };
    if (!qComponent.isTerminal()) {
        throw std::runtime_error("Marking a non-terminal Qubits component!");
    }

    // qPO = q Post-Observation
    QubitsRef qPO { createQubits(qComponent.nQubit) };
    auto& qPOComponent { qubitSets.at(qPO) };

    // Linkage
    qComponent.outLink = qPO;
    qPOComponent.inLink = q;

    return qPO;
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
        // nQubit
        addedQubitset->set_nqubit(q.nQubit);
        // appliedGates
        for (const auto gRef : q.appliedGates) {
            addedQubitset->add_appliedgates(gRef);
        }

        // inLinkageType: NONE, ENTANGLEMENT, OBSERVATION
        if (std::holds_alternative<std::monostate>(q.inLink)) {
            // NONE
            addedQubitset->set_inlinkagetype(Compiled::LinkageType::NONE);
        } else if (std::holds_alternative<std::vector<QuantumCircuit::QubitsRef>>(q.inLink)) {
            // ENTANGLEMENT
            addedQubitset->set_inlinkagetype(Compiled::LinkageType::ENTANGLEMENT);
            const auto& entangleFrom { std::get<std::vector<QuantumCircuit::QubitsRef>>(q.inLink) };
            for (const auto eFrom : entangleFrom) {
                addedQubitset->mutable_entangledfrom()->add_entangledfrom(eFrom);
            }
        } else if (std::holds_alternative<QuantumCircuit::QubitsRef>(q.inLink)) {
            // OBSERVATION
            addedQubitset->set_inlinkagetype(Compiled::LinkageType::OBSERVATION);
            const auto observedFrom { std::get<QuantumCircuit::QubitsRef>(q.inLink) };
            addedQubitset->set_observedfrom(observedFrom);
        } else {
            // Not possible.
            throw std::runtime_error("Unidentified inLink type detected.");
        }
        
        // outLinkageType: NONE, ENTANGLEMENT, OBSERVATION
        if (std::holds_alternative<std::monostate>(q.outLink)) {
            // NONE
            addedQubitset->set_outlinkagetype(Compiled::LinkageType::NONE);
        } else if (std::holds_alternative<QuantumCircuit::QubitsRef>(q.outLink)) {
            // ENTANGLEMENT
            addedQubitset->set_outlinkagetype(Compiled::LinkageType::ENTANGLEMENT);
            const auto entangleTo { std::get<QuantumCircuit::QubitsRef>(q.outLink) };
            addedQubitset->set_entangleto(entangleTo);
        } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservationScheme>(q.outLink)) {
            // OBSERVATION
            addedQubitset->set_outlinkagetype(Compiled::LinkageType::OBSERVATION);
            const auto& observationOut { std::get<QuantumCircuitComponent::Qubits::ObservationScheme>(q.outLink) };
            // Check if full or partial observation.
            if (std::holds_alternative<QuantumCircuit::QubitsRef>(observationOut)) {
                // Full observation
                const auto observeTo { std::get<QuantumCircuit::QubitsRef>(observationOut) };
                addedQubitset->mutable_observationscheme()->set_bitmask(0);
                addedQubitset->mutable_observationscheme()->set_observedto(observeTo);
            } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::PartialObservationScheme>(observationOut)) {
                // Partial observation, unless bitmask is 0 or 0b11...1, in which case, it is full
                const auto& partialObservationScheme { std::get<QuantumCircuitComponent::Qubits::PartialObservationScheme>(observationOut) };
                if (partialObservationScheme.bitmask == 0 || partialObservationScheme.bitmask == (1 << q.nQubit)-1) {
                    // Full observation, but "badly" phrased... Let's correct it for the poor user
                    addedQubitset->mutable_observationscheme()->set_bitmask(0);
                    addedQubitset->mutable_observationscheme()->set_observedto(partialObservationScheme.observedTo);
                } else {
                    // Partial observation scheme. Users are on their own.
                    addedQubitset->mutable_observationscheme()->set_bitmask(partialObservationScheme.bitmask);
                    addedQubitset->mutable_observationscheme()->set_observedto(partialObservationScheme.observedTo);
                    addedQubitset->mutable_observationscheme()->set_unobservedto(partialObservationScheme.unobservedTo);
                }
            } else {
                // Not possible.
                throw std::runtime_error("Unidentified outLink type detected.");
            }
        }
        
        addedQubitset->set_label(q.label);
    }

    quantumObj.PrintDebugString();

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

void QuantumCircuit::setQubitLabel(const QubitsRef qRef, const std::string& labelStr) {
    // Due to C++ freaking out about QuantumCircuitComponent::Qubits not being declared,
    // I decided to set the header, as forward declaring QuantumCircuitComponent::Qubits does not seem to help much.
    qubitSets.at(qRef).label = labelStr;
}

#if 0
void QuantumCircuit::run() {
    // For debugging, this expression for GDB might be useful:
    // p *qComponent.boundQubits.value().stateVector.data()@(1<<qComponent.boundQubits.value().nQubit)
    std::vector<bool> exhausted(qubitSets.size(), false);

    /*
    Follow the following algorithm:
    1. If working on qubits from entangled qubits, check if dependent qubits have been exhausted. (Error checking)
    2. If there are nonexhausted qubit, apply the gates piled up mark that qubits as exhuasted.
    */
    for (QubitsRef i = 0; i < qubitSets.size(); i++) {
        auto& qComponent { qubitSets.at(i) };

        // Check if entangled
        if (!qComponent.entangledFrom.empty()) {
            // If entangled,
            // Check if dependencies all have been bound,
            // while computing entanglement.
            std::vector<Qubits> qbts {};
            for (auto j : qComponent.entangledFrom) {
                if (!qubitSets.at(j).boundQubits.has_value()) {
                    throw std::runtime_error("Dependency qubits is not calculated before.");
                }
                if (qubitSets.at(j).entangleTo != i) {
                    throw std::runtime_error("Dependency qubits does not point to the entangled qubits.");
                }
                if (exhausted.at(j) == false) {
                    throw std::runtime_error("Cannot entangle qubits when previous (potentially intermediate) qubits components have not been visited.");
                }
                qbts.push_back(qubitSets.at(j).boundQubits.value());
            }
            auto entangled { Solace::entangle(qbts) };
            qComponent.bindQubits(entangled);
        } 

        if (!qComponent.boundQubits.has_value()) {
            // Default bind |0...0>
            Qubits q { qComponent.nQubit };
            qComponent.bindQubits(q);
        }

        // Apply gates
        for (auto gRef : qComponent.appliedGates) {
            Qubits q { qComponent.boundQubits.value() };
            //Qubits Gq { gates.at(gRef) * q };
            gates.at(gRef).apply(q);
            qComponent.bindQubits(q);
        }
        
        // Mark as exhausted.
        exhausted.at(i) = true;
    }
}
#endif

}
