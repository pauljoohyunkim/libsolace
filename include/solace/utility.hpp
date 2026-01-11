#ifndef __SOLACE_UTILITY_HPP__
#define __SOLACE_UTILITY_HPP__

#include <vector>
#include "solace.hpp"
namespace Solace {
    /**
     * @brief Generate "entangled qubits" from a set of qubits.
     * @param[in] qubitSets a list of set of qubits to be entangled. Should not be empty.
     */
    Qubits entangle(const std::vector<Qubits>& qubitSets);

    /**
     * @brief Generate "entangled qubits" from a set of qubits with (n-1) other copies.
     * @param[in] q the set of qubits
     * @param[in] n the number of copies to be entangled with. Default is 2.
     */
    Qubits entangle(const Qubits& q, size_t n=2);
}

#endif  // __SOLACE_UTILITY_HPP__