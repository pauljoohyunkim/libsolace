#ifndef __SOLACE_HPP__
#define __SOLACE_HPP__

#include <complex>
#include <vector>
#include <Eigen/Dense>

namespace Solace {
    // Forward Declaration
    class Qubits;
    class QuantumGate;
    
    using ObservedQubitState = unsigned int;
    using StateVector = Eigen::VectorXcd;
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
             * @brief Construct combined qubit system by a tensor product.
             * @param[in] q the other set of qubits.
             * @param[out] Qubits a new set of qubits that looks like it is entangled.
             */
            Qubits operator^(const Qubits& q) const;

#if defined(BE_A_QUANTUM_CHEATER)
            /**
             * @brief observe the qubit system. Note that this will collapse the state vector.
             * @param[in] cheat whether or not the observation will collapse the state vector. Setting this true will prevent the collapse.
             * @param[out] ObservedQubitState the result of the measurement.
             */
            ObservedQubitState observe(const bool cheat=false);
#else
            /**
             * @brief observe the qubit system. Note that this will collapse the state vector. Should you wish, compile with -DBE_A_QUANTUM_CHEATER flag for collapse-free version support.
             * @param[out] ObservedQubitState the result of the measurement.
             */
            ObservedQubitState observe();
#endif

#if defined(BE_A_QUANTUM_CHEATER)

            /**
             * @brief get the state vector out of a qubit system. This is useful for debugging purposes and unit testing.
             * @param[out] StateVector the current state vector.
             */
            StateVector viewStateVector() const { return stateVector; }
#endif
            
        private:
            friend class QuantumGate;
            StateVector stateVector;

            /**
             * @brief validate the number of entries of the state vector. Makes sure that the number of entries is a power of 2.
             */
            void validateLength() const;
            /**
             * @brief normalizes the length of the state vector to 1.
             */
            void normalizeStateVector() { stateVector.normalize(); }
    };

    class QuantumGate {
        public:
            QuantumGate() = default;
            // 2x2
            QuantumGate(const StateVector& q0, const StateVector& q1);
            QuantumGate(const QuantumGateTransformer& transformer) : transformer(transformer) { validate(); }

            // Entanglement (Tensor Product of Quantum Gate Matrices)
            QuantumGate operator^(const QuantumGate& gate) const;

            void apply(Qubits& q);
#if defined(BE_A_QUANTUM_CHEATER)
            QuantumGateTransformer viewTransformer() const { return transformer; }
#endif
        protected:
            bool isValidated { false };
            QuantumGateTransformer transformer;

            void validate();


    };
}

#endif  // __SOLACE_HPP__
