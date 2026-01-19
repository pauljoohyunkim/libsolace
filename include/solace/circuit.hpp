#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include <vector>
#include "solace.hpp"

namespace Solace {

namespace QuantumCircuitComponent {
    /**
     * @brief Circuit Component Qubits. Placeholder to qubits for circuit building.
     * 
     */
    class Qubits {
        public:
            /**
             * @brief Specify which quantum gate to apply to on the set of qubits. Will not compute until Quantum Circuit's run() method is called.
             * 
             * @param[in] gate Gate to apply to qubits.
             */
            void applyQuantumGate(const std::shared_ptr<QuantumGate>& gate) { appliedGates.push_back(gate); }


#ifdef SOLACE_DEV_DEBUG
            std::vector<std::shared_ptr<QuantumGate>> getAppliedGates() const { return appliedGates; }
            std::shared_ptr<Qubits> getEntangleTo() const { return entangleTo; }
            std::vector<std::shared_ptr<Qubits>> getEntangledFrom() const { return entangledFrom; }
#endif
        private:
            friend class Solace::QuantumCircuit;
            Qubits(size_t nQubit=1) : nQubit(nQubit) { 
                if (nQubit == 0) {
                    throw std::runtime_error("Cannot create Qubits component of 0 qubits.");
                }
            }

            const size_t nQubit;
            std::vector<std::shared_ptr<QuantumGate>> appliedGates {};
            std::shared_ptr<Qubits> entangleTo { nullptr };
            std::vector<std::shared_ptr<Qubits>> entangledFrom {};
    };
}

/**
 * @brief A description of the wiring of the input qubits and quantum logic gates, rather than the actual qubits and gates.
 * 
 */
class QuantumCircuit {
    public:
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
         * @return Pointer to qubits component created.
         */
        std::shared_ptr<QuantumCircuitComponent::Qubits> createQubits(const size_t nQubit=1);

        /**
         * @brief Add quantum gate definition to circuit. Each gate can be reused. The copy of the gate will be added to the circuit.
         * 
         * @param[in] gate Quantum gate
         * @return Pointer to gate.
         */
        std::shared_ptr<QuantumGate> addQuantumGate(const QuantumGate& gate);

        /**
         * @brief Entangle multiple Qubits component into one. Qubits that got entangled should not be used.
         * 
         * @param[in] qubits A vector of pointers to qubits components.
         * @return A new pointer to newly generated qubits.
         */
        std::shared_ptr<QuantumCircuitComponent::Qubits> entangle(std::vector<std::shared_ptr<QuantumCircuitComponent::Qubits>>& qubits);
    private:
        // TODO: Possibly change so that it stores the tuples (labelString, std::shared_ptr<Qubits>)
        std::vector<std::shared_ptr<QuantumCircuitComponent::Qubits>> qubitSets {};
        std::vector<std::shared_ptr<QuantumGate>> gates {};
    

};

}

#endif  // __SOLACE_CIRCUIT_HPP__
