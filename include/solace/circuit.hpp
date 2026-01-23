#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include "solace.hpp"

namespace Solace {

/**
 * @brief A description of the wiring of the input qubits and quantum logic gates, rather than the actual qubits and gates.
 * 
 */
class QuantumCircuit {
    public:
        /**
         * @brief Reference to a Qubits component.
         * 
         */
        using QubitsRef = uint32_t;

        /**
         * @brief Reference to a quantum gate.
         * 
         */
        using QuantumGateRef = uint32_t;

        /**
         * @brief Construct a Quantum Circuit/Computer instance.
         * 
         */
        QuantumCircuit() = default;

        /**
         * @brief Constructor of quantum circuit. Reads from a previously "compiled" quantum circuit and loads from it.
         * 
         * @param[in] filepath the file path to compiled quantum circuit object.
         */
        QuantumCircuit(const std::filesystem::path& filepath);

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
         * @brief Apply quantum gate to Qubits component by reference number.
         * 
         * @param[in] g Reference to quantum gate in the quantum circuit.
         * @param[in] q Reference to Qubits component in the quantum circuit.
         */
        void applyQuantumGateToQubits(const QuantumGateRef g, const QubitsRef q);

        /**
         * @brief Entangle multiple Qubits component into one. Qubits that got entangled should not be used.
         * 
         * @param[in] qubits A vector of pointers to qubits components.
         * @return A reference number to newly generated qubits.
         */
        QubitsRef entangle(std::vector<QubitsRef>&& qubits);

        /**
         * @brief Entangle multiple Qubits component into one. Qubits that got entangled should not be used.
         * 
         * @param[in] qubits A vector of pointers to qubits components.
         * @return A reference number to newly generated qubits.
         */
        QubitsRef entangle(std::vector<QubitsRef>& qubits);

        /**
         * @brief Mark a qubits component for full observation
         * 
         * @param[in] q Reference to Qubits component in the quantum circuit.
         * @return new reference to Qubits component after observation. Previous q cannot be used again.
         */
        QubitsRef markForObservation(const QubitsRef q);

        /**
         * @brief Mark a qubits component for partial observation
         * 
         * @param[in] q Reference to Qubits component in the quantum circuit.
         * @param[in] bitmask Bitmask for partial observation
         * @return std::pair<QubitsRef, QubitsRef> new references to Qubits components after observation.
         * The former representing the observed and the latter representing the unobserved.
         * Previous q cannot be used again.
         */
        std::pair<QubitsRef, QubitsRef> markForObservation(const QubitsRef q, const unsigned int bitmask);

        /**
         * @brief Get the Qubits object by "QubitsRef" reference number
         * 
         * @param[in] q QubitsRef number for referring to previously created Qubits component.
         * @return Reference to Qubits component.
         */
        QuantumCircuitComponent::Qubits& getQubits(const QubitsRef q) { return qubitSets.at(q); }
        
        /**
         * @brief Get the Gate object by "QuantumGateRef" reference number
         * 
         * @param[in] g QuantumGateRef number for referring to previously added gate.
         * @return Reference to a quantum gate. Note that this is returned as a constant.
         */
        const QuantumGate& getGate(const QuantumGateRef g) { return gates.at(g); }

        /**
         * @brief Compile the quantum circuit into a file. This packages the quantum gates as well.
         * 
         * @param[in] filepath output quantum circuit file. (*.qc)
         */
        void compile(const std::filesystem::path& filepath) const;

        /**
         * @brief Bind Solace::Qubits to Qubits component on circuit before running.
         * 
         * @param[in] qRef reference number to Qubits circuit component
         * @param[in] qubits Solace::Qubits from the core library.
         */
        void bindQubits(const QubitsRef qRef, const Qubits& qubits);

        /**
         * @brief Unbind all Qubits circuit component from Solace::Qubits
         * Essentially, resetting for next run.
         * 
         */
        void unbindAllQubits();

        /**
         * @brief Set the label for Qubits component.
         * 
         * @param[in] qRef reference number to Qubits circuit component
         * @param[in] labelStr label string
         */
        void setQubitLabel(const QubitsRef qRef, const std::string& labelStr);

        /**
         * @brief Set the label for Quantum Gate.
         * 
         * @param[in] gRef reference number to quantum gate circuit component
         * @param[in] labelStr label string
         */
        void setQuantumGateLabel(const QuantumGateRef gRef, const std::string& labelStr) { gates.at(gRef).label = labelStr; }

        /**
         * @brief Check the QuantumCircuit object for (trivial) structural errors.
         * Users might not need this, but this is provided in case there is some sort of memory corruption.
         * Will throw if it detects error.
         * Essentially provides "compile" level check at application level.
         * This check is done automatically at run(), compile(), and loading from file.
         * 
         */
        void check() const;

        /**
         * @brief Run the quantum circuit. If some initial qubits are left unbound, then they will be assigned default state vector |0...0>.
         * None of the observation results will be returned.
         * 
         */
        void run() { runInternal(nullptr); }

        /**
         * @brief Run the quantum circuit. If some initial qubits are left unbound, then they will be assigned default state vector |0...0>.
         * Observation results will be written on the provided unordered_map
         * 
         * @param[in] m Unordered map from reference number to Qubits component to ObservedState.
         */
        void run(std::unordered_map<QubitsRef, ObservedQubitState>& m) { runInternal(&m); }

#ifdef SOLACE_DEV_DEBUG
            std::vector<QuantumCircuitComponent::Qubits> getQubitSets() const { return qubitSets; }
            std::vector<QuantumGate> getGates() const { return gates; }
#endif
    private:
        std::vector<QuantumCircuitComponent::Qubits> qubitSets {};
        std::vector<QuantumGate> gates {};
    
        void runInternal(std::unordered_map<QubitsRef, ObservedQubitState>* m=nullptr);

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

#ifdef SOLACE_DEV_DEBUG
            std::vector<QuantumCircuit::QuantumGateRef> getAppliedGates() const { return appliedGates; }
            QuantumCircuit::QubitsRef getEntangleTo() const { return std::get<QuantumCircuit::QubitsRef>(outLink); }
            std::vector<QuantumCircuit::QubitsRef> getEntangledFrom() const { return std::get<std::vector<QuantumCircuit::QubitsRef>>(inLink); }
#endif
        private:
            friend class Solace::QuantumCircuit;
            /**
             * @brief Construct a new Qubits component for circuit.
             * 
             * @param circuit Reference to Quantum circuit that this component is bound to.
             * @param nQubit Number of qubits this component holds.
             */
            Qubits(QuantumCircuit& circuit, const size_t nQubit=1) : circuit(circuit), nQubit(nQubit) { 
                if (nQubit == 0) {
                    throw std::runtime_error("Cannot create Qubits component of 0 qubits.");
                }
            }

            /**
             * @brief Specify which quantum gate to apply to on the set of qubits. Will not compute until Quantum Circuit's run() method is called.
             * 
             * @param[in] gate Gate to apply to qubits.
             */
            void applyQuantumGate(const QuantumCircuit::QuantumGateRef gate) { 
                if (!isTerminal()) {
                    throw std::runtime_error("This gate is already entangled and you cannot operate on it.");
                }
                if (circuit.getGate(gate).getNQubit() != nQubit) {
                    throw std::runtime_error("Gate size and qubits mismatch.");
                }
                appliedGates.push_back(gate);
            }

            /**
             * @brief Check if the Qubits component is one of the initial components in the circuit.
             * 
             * @return true if it is an initial Qubits component (that is, neither entangled from other qubits nor from being observed.)
             * @return false if it is made from entangling other Qubits components.
             */
            bool isInitial() const { return std::holds_alternative<std::monostate>(inLink); }

            /**
             * @brief Check if the Qubits component is one of the last in the tree, that is, it does not have any other Qubits that are created from it.
             * 
             * @return true if it can be output.
             * @return false if it is used for creating another entangled qubits component. This component should not have been used.
             */
            bool isTerminal() const { return std::holds_alternative<std::monostate>(outLink); }

            void bindQubits(const Solace::Qubits& q) { 
                if (q.getNQubit() != nQubit) {
                    throw std::runtime_error("Cannot bind Qubits with Qubits circuit component of different number of qubits.");
                }
                boundQubits = q;
            }


            QuantumCircuit& circuit;
            const size_t nQubit;
            std::vector<QuantumCircuit::QuantumGateRef> appliedGates {};

            struct PartialObservationScheme {
                unsigned int bitmask;       // If bitmask = 0 or 0b11...1, then it will be forced to act like a full observation.
                QuantumCircuit::QubitsRef observedTo;
                QuantumCircuit::QubitsRef unobservedTo;
            };

            struct ObservedFrom {
                QuantumCircuit::QubitsRef q;
            };

            struct UnobservedFrom {
                QuantumCircuit::QubitsRef q;
            };

            using ObservationFromScheme = std::variant<ObservedFrom, UnobservedFrom>;
            // Full or partial observation
            using ObservationToScheme = std::variant<QuantumCircuit::QubitsRef, PartialObservationScheme>;

            // None, entangledFrom, observedFrom
            std::variant<std::monostate, std::vector<QuantumCircuit::QubitsRef>, ObservationFromScheme> inLink { std::monostate() };
            // None, entangleTo, observe output to
            std::variant<std::monostate, QuantumCircuit::QubitsRef, ObservationToScheme> outLink { std::monostate() };

            std::optional<Solace::Qubits> boundQubits { std::nullopt };
    };
}

}

#endif  // __SOLACE_CIRCUIT_HPP__
