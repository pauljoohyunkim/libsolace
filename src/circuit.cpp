#include <memory>
#include <fstream>
#include <unordered_set>
#include "solace.pb.h"
#include "solace/solace.hpp"
#include "solace/utility.hpp"
#include "solace/circuit.hpp"

// Kernighan's algorithm
static inline unsigned int countSetBits(unsigned int n) {
    unsigned int count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
}

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
        #define q (qubitSets.at(index))
        // appliedGates
        q.appliedGates.reserve(qsProto.appliedgates_size());
        for (const auto gRef : qsProto.appliedgates()) {
            q.appliedGates.push_back(gRef);
        }
        // inLinkageType & inLinkage
        if (qsProto.inlinkagetype() == Compiled::LinkageType::NONE) {
            q.inLink = std::monostate();
        } else if (qsProto.inlinkagetype() == Compiled::LinkageType::ENTANGLEMENT) {
            q.inLink = std::vector<QuantumCircuit::QubitsRef>();
            auto& entangledFrom { std::get<std::vector<QuantumCircuit::QubitsRef>>(q.inLink) };
            entangledFrom.reserve(qsProto.entangledfrom().entangledfrom_size());
            for (const auto eFrom : qsProto.entangledfrom().entangledfrom()) {
                entangledFrom.push_back(eFrom);
            }
        } else if (qsProto.inlinkagetype() == Compiled::LinkageType::OBSERVATION) {
            const auto observationLinkageType { qsProto.observationfrom().observationfromtype() };
            if (observationLinkageType == Compiled::LINK_BY_OBSERVATION) {
                q.inLink = QuantumCircuitComponent::Qubits::ObservedFrom{qsProto.observationfrom().observationfrom()};
            } else if (observationLinkageType == Compiled::LINK_BY_UNOBSERVATION) {
                q.inLink = QuantumCircuitComponent::Qubits::UnobservedFrom{qsProto.observationfrom().unobservationfrom()};
            }
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
                    qsProto.observationscheme().bitmask(),
                    qsProto.observationscheme().observedto(),
                    qsProto.observationscheme().unobservedto()
                };
            }
        } else {
            // Not possible.
            throw std::runtime_error("Invalid outLinkageType from protobuf.");
        }
        
        //label
        q.label = qsProto.label();
        #undef q
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

QuantumCircuit::QubitsRef QuantumCircuit::entangle(std::vector<QubitsRef>&& qubits) {
    return entangle(qubits);
}

QuantumCircuit::QubitsRef QuantumCircuit::entangle(std::vector<QubitsRef>& qubits) {
    size_t nQubit { 0 };
    // Find the total number of qubits, while checking if any of them have already been entangled.
    {
        std::unordered_set<QuantumCircuit::QubitsRef> seenRefs {};
        for (const auto qRef : qubits) {
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
    entangledFrom.reserve(qubits.size());
    for (const auto& qRef : qubits) {
        auto& q { qubitSets.at(qRef) };
        entangledFrom.push_back(qRef);
        q.outLink = ref;
    }
    qubitSets.push_back(Q);

    return ref;

}

QuantumCircuit::QubitsRef QuantumCircuit::markForObservation(const QubitsRef q) {
    #define qComponent (qubitSets.at(q))

    if (!qComponent.isTerminal()) {
        throw std::runtime_error("Marking a non-terminal Qubits component!");
    }

    // qPO = q Post-Observation
    QubitsRef qPO { createQubits(qComponent.nQubit) };
    #define qPOComponent (qubitSets.at(qPO))

    // Linkage
    qComponent.outLink = QuantumCircuitComponent::Qubits::ObservationToScheme(qPO);
    qPOComponent.inLink = QuantumCircuitComponent::Qubits::ObservedFrom { q };

    return qPO;

    #undef qComponent
    #undef qPOComponent
}

std::pair<QuantumCircuit::QubitsRef, QuantumCircuit::QubitsRef> QuantumCircuit::markForObservation(const QubitsRef q, const unsigned int bitmask) {
    #define qComponent (qubitSets.at(q))

    if (!qComponent.isTerminal()) {
        throw std::runtime_error("Marking a non-terminal Qubits component!");
    }
    if (bitmask >= (1U << qComponent.nQubit)) {
        throw std::runtime_error("Invalid bitmask for the number of bits.");
    }

    // qO = q Observed
    // qU = q Unobserved
    const auto observedCount { countSetBits(bitmask) };
    QubitsRef qO { createQubits(observedCount) };
    QubitsRef qU { createQubits(qComponent.nQubit-observedCount) };
    #define qOComponent (qubitSets.at(qO))
    #define qUComponent (qubitSets.at(qU))

    // Linkage
    qComponent.outLink = QuantumCircuitComponent::Qubits::ObservationToScheme{
        QuantumCircuitComponent::Qubits::PartialObservationScheme{
            bitmask,
            qO,
            qU}};
    qOComponent.inLink = QuantumCircuitComponent::Qubits::ObservedFrom { q };
    qUComponent.inLink = QuantumCircuitComponent::Qubits::UnobservedFrom { q };

    return { qO, qU };

    #undef qComponent
    #undef qOComponent
    #undef qUComponent
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
        } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservationFromScheme>(q.inLink)) {
            // OBSERVATION
            addedQubitset->set_inlinkagetype(Compiled::LinkageType::OBSERVATION);
            const auto observedFrom { std::get<QuantumCircuitComponent::Qubits::ObservationFromScheme>(q.inLink) };

            // Check if inLink is by observation or "unobservation"
            if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservedFrom>(observedFrom)) {
                const auto q { std::get<QuantumCircuitComponent::Qubits::ObservedFrom>(observedFrom).q };
                addedQubitset->mutable_observationfrom()->set_observationfromtype(Compiled::LINK_BY_OBSERVATION);
                addedQubitset->mutable_observationfrom()->set_observationfrom(q);
            } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::UnobservedFrom>(observedFrom)) {
                const auto q { std::get<QuantumCircuitComponent::Qubits::UnobservedFrom>(observedFrom).q };
                addedQubitset->mutable_observationfrom()->set_observationfromtype(Compiled::LINK_BY_UNOBSERVATION);
                addedQubitset->mutable_observationfrom()->set_unobservationfrom(q);
            }
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
        } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservationToScheme>(q.outLink)) {
            // OBSERVATION
            addedQubitset->set_outlinkagetype(Compiled::LinkageType::OBSERVATION);
            const auto& observationOut { std::get<QuantumCircuitComponent::Qubits::ObservationToScheme>(q.outLink) };
            // Check if full or partial observation.
            if (std::holds_alternative<QuantumCircuit::QubitsRef>(observationOut)) {
                // Full observation
                const auto observeTo { std::get<QuantumCircuit::QubitsRef>(observationOut) };
                addedQubitset->mutable_observationscheme()->set_bitmask(0);
                addedQubitset->mutable_observationscheme()->set_observedto(observeTo);
            } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::PartialObservationScheme>(observationOut)) {
                // Partial observation, unless bitmask is 0 or 0b11...1, in which case, it is full
                const auto& partialObservationScheme { std::get<QuantumCircuitComponent::Qubits::PartialObservationScheme>(observationOut) };
                if (partialObservationScheme.bitmask == 0 || partialObservationScheme.bitmask == (1U << q.nQubit)-1) {
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

void QuantumCircuit::bindQubits(const QubitsRef qRef, const Qubits& qubits) {
    if (qRef >= qubitSets.size()) {
        throw std::runtime_error("Qubits component of such reference number does not exist.");
    }
    auto& qComponent { qubitSets.at(qRef) };
    if (!qComponent.isInitial()) {
        throw std::runtime_error("Cannot bind to a non-initial Qubits component.");
    }
    qComponent.bindQubits(qubits);
}

void QuantumCircuit::unbindAllQubits() {
    for (auto& q : qubitSets) {
        q.boundQubits = std::nullopt;
    }
}

void QuantumCircuit::setQubitLabel(const QubitsRef qRef, const std::string& labelStr) {
    // Due to C++ freaking out about QuantumCircuitComponent::Qubits not being declared,
    // I decided to set the header, as forward declaring QuantumCircuitComponent::Qubits does not seem to help much.
    qubitSets.at(qRef).label = labelStr;
}

void QuantumCircuit::check() const {
    // Visit qubit components in a similar fashion as the runInternal function without explicit computation.

    // Tracks which qubits have been visited already.
    // Since runInternal loops over each of the qubitSets, it should be set to true one after the other.
    std::vector<bool> exhausted(qubitSets.size(), false);

    // 1. Check entanglement logic
    // 2. Check gate application
    // 3. Check observation logic
    for (QubitsRef currentQRef = 0; currentQRef < qubitSets.size(); currentQRef++) {
        const auto& qComponent { qubitSets.at(currentQRef) };
        // Entanglement check
        {
            if (std::holds_alternative<std::vector<QubitsRef>>(qComponent.inLink)) {
                // inLink = Entanglement
                const auto& entangledFromQRefs { std::get<std::vector<QubitsRef>>(qComponent.inLink) };

                size_t nQubitFromDependencies { 0 };
                for (const auto dependencyQRef : entangledFromQRefs) {
                    const auto& dependencyComponent { qubitSets.at(dependencyQRef) };
                    // Check if dependency is marked exhausted.
                    if (!exhausted.at(dependencyQRef)) {
                        throw std::runtime_error("Dependency is not computed for entanglement.");
                    }
                    // Check if outLink is entanglement.
                    if (!std::holds_alternative<QubitsRef>(dependencyComponent.outLink)) {
                        throw std::runtime_error("Entanglement expected from dependency.");
                    }
                    // Check if dependency outLink points to currentQRef.
                    const auto dependencyPointToQRef { std::get<QubitsRef>(dependencyComponent.outLink) };
                    if (dependencyPointToQRef != currentQRef) {
                        throw std::runtime_error("Dependency does not entangle to its supposed output.");
                    }
                    nQubitFromDependencies += dependencyComponent.nQubit;
                }
                // Check if current nQubit corresponds to nQubit sum of dependencies.
                if (nQubitFromDependencies != qComponent.nQubit) {
                    throw std::runtime_error("Number of qubits for entangled component must be sum of number of qubits of dependencies.");
                }
            }
        }

        // Gate Application Check
        {
            for (QuantumGateRef gRef : qComponent.appliedGates) {
                const QuantumGate& gate { gates.at(gRef) };
                if (gate.nQubit != qComponent.nQubit) {
                    throw std::runtime_error("The gate is not applicable to this qubits component.");
                }
            }
        }

        // TODO: Observation Check

        exhausted.at(currentQRef) = true;
    }
}

void QuantumCircuit::runInternal(std::unordered_map<QubitsRef, ObservedQubitState>* m) {
    // Run sanity check.
    check();

    // For debugging, this expression for GDB might be useful:
    // p *qComponent.boundQubits.value().stateVector.data()@(1<<qComponent.boundQubits.value().nQubit)
    std::vector<bool> exhausted(qubitSets.size(), false);

    /*
    Follow the following steps (Note the inLink -> applying gates -> outLink)
    1. If working on qubits from entangled qubits, check if dependent qubits have been exhausted. (Error checking)
    2. If there are nonexhausted qubit, apply the gates piled up mark that qubits as exhuasted.
    3. If outLink is set to full observation or partial observation, observe the qubit then assign the values at out qubits. The for loop will eventually parse it as normal
    (as it will have inLink set to observation)
    */
    for (QubitsRef i = 0; i < qubitSets.size(); i++) {
        auto& qComponent { qubitSets.at(i) };

        // Check if entangled (inLink)
        if (std::holds_alternative<std::vector<QuantumCircuit::QubitsRef>>(qComponent.inLink)) {
            // If entangled,
            // Check if dependencies all have been bound,
            // while computing entanglement.
            // TODO: This check must be done after "default binding" of the initial qubits component!!!!!
            const auto& entangledFrom { std::get<std::vector<QuantumCircuit::QubitsRef>>(qComponent.inLink) };
            std::vector<Qubits> qbts {};
            qbts.reserve(entangledFrom.size());
            for (auto j : entangledFrom) {
                if (!qubitSets.at(j).boundQubits.has_value()) {
                    throw std::runtime_error("Dependency qubits is not calculated before.");
                }
                // If either not "entangle to" or entangleto points to a different qubits,
                if (!std::holds_alternative<QubitsRef>(qubitSets.at(j).outLink) || std::get<QubitsRef>(qubitSets.at(j).outLink) != i) {
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

        // Check if marked for observation (outLink)
        // TODO: Check if inLink of observationTo and unobservationTo are correct.
        if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservationToScheme>(qComponent.outLink)) {
            auto& outLink { std::get<QuantumCircuitComponent::Qubits::ObservationToScheme>(qComponent.outLink) };
            // For both full observation and partial observation, take the current Qubits, observe, and assign new Qubits to "observeTo" (and "unobserveTo").
            if (std::holds_alternative<QubitsRef>(outLink)) {
                QubitsRef observedQuibitComponentRef { std::get<QubitsRef>(outLink) };
                // Full observation
                auto qubitsForObservation { qComponent.boundQubits.value() };
                auto observation { qubitsForObservation.observe() };
                if (m) {
                    (*m)[observedQuibitComponentRef] = observation;
                }
                qubitSets.at(observedQuibitComponentRef).bindQubits(qubitsForObservation);

            } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::PartialObservationScheme>(outLink)) {
                // Partial observation
                auto partialObservationScheme { std::get<QuantumCircuitComponent::Qubits::PartialObservationScheme>(outLink) };
                auto qubitsForObservation { qComponent.boundQubits.value() };
                auto pObservationResult { qubitsForObservation.observe(partialObservationScheme.bitmask) };
                
                QubitsRef observedTo { partialObservationScheme.observedTo };
                QubitsRef unobservedTo { partialObservationScheme.unobservedTo };

                if (m) {
                    (*m)[observedTo] = pObservationResult.first;
                }
                qubitSets.at(observedTo).bindQubits(qubitsForObservation);
                qubitSets.at(unobservedTo).bindQubits(pObservationResult.second.value());
            } else {
                // Not possible
                throw std::runtime_error("Cannot determine if full or partial observation.");
            }
        }
        
        /*
        if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservationFromScheme>(qComponent.inLink)) {
            auto& inLink { std::get<QuantumCircuitComponent::Qubits::ObservationFromScheme>(qComponent.inLink) };
            if (std::holds_alternative<QuantumCircuitComponent::Qubits::ObservedFrom>(inLink)) {
                auto& observedQComponent { qubitSets.at(std::get<QuantumCircuitComponent::Qubits::ObservedFrom>(inLink).q) };
                // Extract (make copy) as Solace::Qubits, observe, then put the collapsed state vector into the new component
                // Assert that dependency already has value, as it should have been visited before.
                auto observedQubits { observedQComponent.boundQubits.value() };
                // TODO: For now, throw away the measurement, though the state vector is now modified.
                auto observation { observedQubits.observe() };
                if (m) {
                    // If given map, write to map.
                    (*m)[i] = observation;
                }
                qComponent.bindQubits(observedQubits);
            } else if (std::holds_alternative<QuantumCircuitComponent::Qubits::UnobservedFrom>(inLink)) {
                // TODO: Implement this
                throw std::runtime_error("Partial observation is not yet supported!!!!");
            }
        }
        */

        
        // Mark as exhausted.
        exhausted.at(i) = true;
    }
}

}
