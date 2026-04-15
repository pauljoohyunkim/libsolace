#include "solace/algorithm.hpp"

namespace Solace {
namespace Algorithm {

ObservedQubitState quantum_phase_estimation(const QuantumGate& gate, const Qubits& q, unsigned int n) {
    if (n == 0) throw std::invalid_argument("Estimation cannot happen with 0 qubits for estimation.");

    const auto m { q.getNQubit() };
    const auto n_target_states { 1U << m };
    const auto n_capture_vals { 1U << n };

    Qubits capture { std::vector<std::complex<double>>(1U<<n, 1.0) };
    Qubits augmented { capture ^ q };

    QuantumGate U2l { gate };

    for (unsigned int l = 0; l < n; l++) {
        if (l > 0) U2l = U2l * U2l;

        for (unsigned int c = 0; c < n_capture_vals; c++) {
            if ((c >> l) & 1) {
                StateVector cache = augmented.stateVector.segment(c<<m, n_target_states);
                const auto U2l_t { U2l.getTransformer() };
                if (std::holds_alternative<QuantumGateTransformer>(U2l_t)) {
                    cache = (std::get<QuantumGateTransformer>(U2l_t) * cache).eval();
                } else if (std::holds_alternative<SparseQuantumGateTransformer>(U2l_t)) {
                    cache = (std::get<SparseQuantumGateTransformer>(U2l_t) * cache).eval();
                } else {
                    throw std::invalid_argument("Gate has invalid transformer");
                }
                augmented.stateVector.segment(c<<m, n_target_states) = cache;
            }
        }
    }
    augmented.normalizeStateVector();

    // iQFT here.

    throw std::runtime_error("Currently not done implementing iQFT.");
    return 0U;
}

}
}
