/**
 * @file
 * @brief Utility functions for Solace library.
 */
#ifndef __SOLACE_UTILITY_HPP__
#define __SOLACE_UTILITY_HPP__

#include <vector>
#include "solace.hpp"
namespace Solace {
    /**
     * @brief Generate "entangled qubits" from a set of qubits.
     * @param[in] qubitSets a list of set of qubits to be entangled. Should not be empty.
     * @param[out] Qubits set of entangled qubits.
     */
    Qubits entangle(const std::vector<Qubits>& qubitSets);

    /**
     * @brief Generate "entangled qubits" from a set of qubits with (n-1) other copies.
     * @param[in] q the set of qubits
     * @param[in] n the number of copies to be entangled with. Default is 2.
     * @param[out] Qubits set of entangled qubits.
     */
    Qubits entangle(const Qubits& q, size_t n=2);

    /**
     * @brief Generate analogous gate for "entangled qubits" from a set of gates.
     * @param[in] quantumGates a list of gates to be entangled. Should not be empty.
     * @param[out] Qubits a larger gate that can act on more qubits.
     */
    QuantumGate entangle(const std::vector<QuantumGate>& quantumGates);

    /**
     * @brief Generate analogous gate for "entangled qubits" from (n-1) other copies.
     * @param[in] q the quantum gate
     * @param[in] n the number of copies to be entangled with. Default is 2.
     * @param[out] Qubits a larger gate that can act on more qubits.
     */
    QuantumGate entangle(const QuantumGate& g, size_t n=2);
}

#endif  // __SOLACE_UTILITY_HPP__