#include <gtest/gtest.h>
#include "solace/common_gates.hpp"
#include "solace/algorithm.hpp"

TEST(Algorithm, QPE1) {
    // I * pi = I * (2pi) * phi
    // -> phi = 1/2
    Solace::Gate::PhaseShift P { M_PI };
    Solace::Qubits q { 0, 1 };

    const auto estimation { Solace::Algorithm::quantum_phase_estimation(P, q, 5) };

    ASSERT_EQ(estimation, 0b10000);
}

TEST(Algorithm, QPE2) {
    Solace::Gate::PhaseShift P { M_PI_2 };
    Solace::Qubits q { 0, 1 };

    const auto estimation { Solace::Algorithm::quantum_phase_estimation(P, q, 5) };

    // 2 = 0b010
    ASSERT_EQ(estimation, 0b01000);
}

TEST(Algorithm, QPE3) {
    Solace::Gate::PhaseShift P { M_PI/4 };
    Solace::Qubits q { 0, 1 };

    const auto estimation { Solace::Algorithm::quantum_phase_estimation(P, q, 5) };

    // 1 = 0b001
    ASSERT_EQ(estimation, 0b00100);
}
