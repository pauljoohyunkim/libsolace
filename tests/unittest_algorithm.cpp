#include <gtest/gtest.h>
#include "solace/common_gates.hpp"
#include "solace/algorithm.hpp"

TEST(Algorithm, QPE) {
    Solace::Gate::Hadamard H;
    Solace::Qubits q { 1, 1 };

    Solace::Algorithm::quantum_phase_estimation(H, q, 2);
}
