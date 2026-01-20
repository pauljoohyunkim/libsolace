#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include <vector>
#include <string>
#include "solace.hpp"

namespace Solace {

/**
 * @brief A description of the wiring of the input qubits and quantum logic gates, rather than the actual qubits and gates.
 * 
 */
class QuantumCircuit {
    public:
        using QubitsRef = uint32_t;
        using QuantumGateRef = uint32_t;
        /**
         * @brief Construct a Quantum Circuit/Computer instance.
         * 
         */
        QuantumCircuit() = default;

        // TODO: Change so that there is a separation between the circuit and the implementation of quantum gates.
        // Should remove addQubits and replace them with something like createQubits, and later when running, allow "inserting qubit".
        // As for addQuantumGate, I think it will be fine, as the gates do not change according to run-time.
        // TODO: Support classical feedback in the future.

        /**
         * @brief Create a Qubits circuit component
         * 
         * @param[in] nQubit Number of qubits.
         * @return Reference number to the qubits component created.
         */
        QubitsRef createQubits(const size_t nQubit=1);

        /**
         * @brief Add quantum gate definition to circuit. Each gate can be reused. The copy of the gate will be added to the circuit.
         * 
         * @param[in] gate Quantum gate
         * @return Reference number to the gate.
         */
        QuantumGateRef addQuantumGate(const QuantumGate& gate);

        /**
         * @brief Entangle multiple Qubits component into one. Qubits that got entangled should not be used.
         * 
         * @param[in] qubits A vector of pointers to qubits components.
         * @return A new pointer to newly generated qubits.
         */
        QubitsRef entangle(std::vector<QubitsRef>& qubits);

        QuantumCircuitComponent::Qubits& getQubits(const QubitsRef q) { return qubitSets.at(q); }

#ifdef SOLACE_DEV_DEBUG
            std::vector<QuantumCircuitComponent::Qubits> getQubitSets() const { return qubitSets; }
            std::vector<QuantumGate> getGates() const { return gates; }
#endif
    private:
        std::vector<QuantumCircuitComponent::Qubits> qubitSets {};
        std::vector<QuantumGate> gates {};
    

};

namespace QuantumCircuitComponent {
    /**
     * @brief Circuit Component Qubits. Placeholder to qubits for circuit building.
     * 
     */
    class Qubits {
        public:
            /**
             * @brief Label for the qubits. Kept as a public variable as it does not interfere with the computation.
             * 
             */
            std::string label {};

            /**
             * @brief Specify which quantum gate to apply to on the set of qubits. Will not compute until Quantum Circuit's run() method is called.
             * 
             * @param[in] gate Gate to apply to qubits.
             */
            void applyQuantumGate(const QuantumCircuit::QuantumGateRef gate) { 
                appliedGates.push_back(gate);
            }


#ifdef SOLACE_DEV_DEBUG
            std::vector<QuantumCircuit::QuantumGateRef> getAppliedGates() const { return appliedGates; }
            QuantumCircuit::QubitsRef getEntangleTo() const { return entangleTo; }
            std::vector<QuantumCircuit::QubitsRef> getEntangledFrom() const { return entangledFrom; }
#endif
        private:
            friend class Solace::QuantumCircuit;
            Qubits(size_t nQubit=1) : nQubit(nQubit) { 
                if (nQubit == 0) {
                    throw std::runtime_error("Cannot create Qubits component of 0 qubits.");
                }
            }

            const size_t nQubit;
            std::vector<QuantumCircuit::QuantumGateRef> appliedGates {};
            QuantumCircuit::QubitsRef entangleTo { 0 };     // 0 signifies no entangling to next node
            std::vector<QuantumCircuit::QubitsRef> entangledFrom {};
    };
}

}

#endif  // __SOLACE_CIRCUIT_HPP__
