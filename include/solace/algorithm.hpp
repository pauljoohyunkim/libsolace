#ifndef __SOLACE_CIRCUIT_HPP__
#define __SOLACE_CIRCUIT_HPP__

#include "solace.hpp"

namespace Solace {
namespace Algorithm {

ObservedQubitState quantum_phase_estimation(const QuantumGate& gate, const Qubits& q, unsigned int n);

}
}

#endif  // __SOLACE_CIRCUIT_HPP__
