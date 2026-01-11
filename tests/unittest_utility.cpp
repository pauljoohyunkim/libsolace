#include <gtest/gtest.h>
#include "solace/utility.hpp"

TEST(Utility, Entangle) {
    Solace::Qubits q0 { 2 };
    Solace::Qubits q1 { 3 };

    std::vector<Solace::Qubits> qubitSets { q0, q1 };
    auto entangledState { Solace::entangle(qubitSets) };
    auto sv { entangledState.viewStateVector() };

    ASSERT_EQ(sv.size(), 1 << (2 + 3));
}

TEST(Utility, Entangle2) {
    Solace::Qubits q0 { 2 };
    auto entangledState { Solace::entangle(q0, 3) };
    auto sv { entangledState.viewStateVector() };

    ASSERT_EQ(sv.size(), 1 << (2 * 3));
}

TEST(Utility, Entangle3) {
    const double alpha { 0.2 };
    const double beta { std::sqrt(1-alpha*alpha) };
    const double phi { 0.3 };
    const std::complex<double> j { 0, 1 };
    const auto phaseShift { std::exp(j * phi) };
    Solace::Qubits q0 { alpha, -phaseShift * std::conj(beta) };
    Solace::Qubits q1 { beta, phaseShift * std::conj(alpha) };

    Solace::Qubits q2 { q0 ^ q1 };

    // Expected probability of the entangled states.
    Solace::StateVector expectedSv(4);
    expectedSv << alpha * beta,
                  alpha * phaseShift * std::conj(alpha),
                  -phaseShift * std::conj(beta) * beta,
                  -std::conj(alpha * beta) * phaseShift * phaseShift;
    Solace::StateVector sv { q2.viewStateVector() };
    Solace::StateVector diff = sv - expectedSv;

    ASSERT_TRUE(std::abs(diff.norm()) < 0.0001);
}