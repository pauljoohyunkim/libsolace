/**
 * @file
 * @brief Main Solace library. Defines Qubits and QuantumGate.
 */
#ifndef __SOLACE_HPP__
#define __SOLACE_HPP__

#include <complex>
#include <vector>
#include <filesystem>
#include <Eigen/Dense>

namespace Solace {
    // Forward Declaration
    class Qubits;
    class QuantumGate;
    
    /**
     * @brief Represents what value can be observed from measurement. (Alias to unsigned int)
     */
    using ObservedQubitState = unsigned int;

    /**
     * @brief Represents a state vector. (Alias to Eigen::VectorXcd from Eigen library)
     */
    using StateVector = Eigen::VectorXcd;

    /**
     * @brief Represents a quantum gate matrix. (Alias to Eigen::MatrixXcd from Eigen library)
     */
    using QuantumGateTransformer = Eigen::MatrixXcd;

    /**
     * @class Qubits
     * @brief Represents a quantum system with multiple qubits.
     */
    class Qubits {
        public:
            /**
             * @brief Constructor of qubits. Initialized to the first entry in state vector being 1.
             * @param[in] n Number of qubits. Default is 1.
             */
            Qubits(const int n=1) : stateVector(StateVector::Zero(1<<n)) { validateLength(); stateVector(0) = 1.0; }

            /**
             * @brief Constructor of qubits. Initialized to the entries given. Will be normalized.
             * @param[in] cs A vector of complex numbers representing the state vector. (Must be size of a power of 2.)
             */
            Qubits(const std::vector<std::complex<double>>& cs);

            /**
             * @brief Constructor of a single qubit. The state vector is initialized from the given values. Will be normalized.
             * @param[in] c0 The state vector entry for |0>
             * @param[in] c1 The state vector entry for |1>
             */
            Qubits(const std::complex<double>& c0, const std::complex<double>& c1) : stateVector(2) { validateLength(); stateVector << c0, c1; normalizeStateVector(); }

            /**
             * @brief Constructor of qubits. Applies the state vector after normalization. The state vector must be size of a power of 2.
             * @param[in] sv The state vector.
             */
            Qubits(const StateVector& sv) : stateVector(sv) { validateLength(); normalizeStateVector(); }

            /**
             * @brief Constructor qubits. Reads from a previously "compiled" qubit and load from it.
             * 
             * @param[in] filepath the file path to compiled qubit object.
             */
            Qubits(const std::filesystem::path& filepath);

            /**
             * @brief Construct combined qubit system by a tensor product.
             * @param[in] q the other set of qubits.
             * @param[out] Qubits a new set of qubits that looks like it is entangled.
             */
            Qubits operator^(const Qubits& q) const;

            /**
             * @brief Get number of qubits.
             * @param[out] nQubit The number of qubits.
             */
            size_t getNQubit() const { return nQubit; }

#if defined(BE_A_QUANTUM_CHEATER)
            /**
             * @brief observe the qubit system. Note that this will collapse the state vector.
             * @param[in] cheat whether or not the observation will collapse the state vector. Setting this true will prevent the collapse.
             * @param[in] randomphase whether or not post-measurement phase should be randomized or not. (Often meaningless.)
             * @param[out] ObservedQubitState the result of the measurement.
             */
            ObservedQubitState observe(const bool randomphase=false, const bool cheat=false);
#else
            /**
             * @brief observe the qubit system. Note that this will collapse the state vector. Should you wish, compile with -DBE_A_QUANTUM_CHEATER flag for collapse-free version support.
             * @param[in] randomphase whether or not post-measurement phase should be randomized or not. (Often meaningless.)
             * @param[out] ObservedQubitState the result of the measurement.
             */
            ObservedQubitState observe(const bool randomphase=false);
#endif

#if defined(BE_A_QUANTUM_CHEATER)

            /**
             * @brief get the state vector out of a qubit system. This is useful for debugging purposes and unit testing.
             * @param[out] StateVector the current state vector.
             */
            StateVector viewStateVector() const { return stateVector; }
#endif

            /**
             * @brief Compile the generated qubits to a file.
             * 
             * @param[in] filepath 
             */
            void compile(const std::filesystem::path& filepath) const;

        private:
            friend class QuantumGate;
            StateVector stateVector;
            size_t nQubit { 0 };

            /**
             * @brief validate the number of entries of the state vector. Makes sure that the number of entries is a power of 2.
             */
            void validateLength();
            /**
             * @brief normalizes the length of the state vector to 1.
             */
            void normalizeStateVector() { stateVector.normalize(); }
    };

    /**
     * @class QuantumGate
     * @brief Represents a quantum gate. Is a wrapper around unitary matrix.
     */
    class QuantumGate {
        public:
            /**
             * @brief Empty quantum gate constructor. Used for defining custom quantum gate. Otherwise, other constructors are recommended.
             */
            QuantumGate() = default;
            
            /**
             * @brief 2x2 quantum gate constructor. Each of the state vector will be normalized. Fails if the two are not "orthogonal" in generalized inner product.
             * @param[in] q0 the state vector that |0> is mapped to.
             * @param[in] q1 the state vector that |1> is mapped to.
             */
            QuantumGate(const StateVector& q0, const StateVector& q1);

            /**
             * @brief Quantum gate constructor for any number of qubits. 
             * @param[in] transformer the unitary matrix that defines the gate. Must be a unitary matrix of N x N where N is a power of 2.
             */
            QuantumGate(const QuantumGateTransformer& transformer) : transformer(transformer) { validate(); }

            // Entanglement (Tensor Product of Quantum Gate Matrices)
            /**
             * @brief create a new quantum gate that is the tensor product of two quantum gates (hence acts on larger set of qubits)
             * @param[in] gate a quantum gate
             * @param[out] QuantumGate a quantum gate that is the tensor product of the two.
             */
            QuantumGate operator^(const QuantumGate& gate) const;

            /**
             * @brief Get number of qubits that the gate can apply to.
             * @param[out] nQubit The number of qubits.
             */
            size_t getNQubit() const { return nQubit; }

            /**
             * @brief Compile the generated quantum gate to a file.
             * 
             * @param[in] filepath 
             */
            void compile(const std::filesystem::path& filepath) const;

            /**
             * @brief apply the quantum gate to a set of qubits
             * @param[in] q the set of qubits to apply the quantum gate on
             */
            void apply(Qubits& q);
#if defined(BE_A_QUANTUM_CHEATER)
            /**
             * @brief get the transformer matrix of the quantum gate for debugging purposes.
             * @param[out] transformer the unitary matrix that defines the gate.
             */
            QuantumGateTransformer viewTransformer() const { return transformer; }
#endif
        protected:
            bool isValidated { false };
            QuantumGateTransformer transformer;
            size_t nQubit { 0 };

            /**
             * @brief validates the quantum gate at initialization.
             */
            void validate();


    };
}

#endif  // __SOLACE_HPP__
