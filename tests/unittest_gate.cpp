#include <gtest/gtest.h>
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"

TEST(QuantumGate, ValidityOK) {
    Solace::QubitStateVector q1 { 2/3, std::complex<double>(2,1)/3.0 };
    Solace::QubitStateVector q2 { std::complex<double>(-2,1)/3.0, 2/3 };

    Solace::QuantumGate H { q1, q2 };
}

TEST(QuantumGate, ValidityFail) {
    Solace::QubitStateVector q1 { 1, 2 };
    Solace::QubitStateVector q2 { 3, 4 };

    ASSERT_ANY_THROW(Solace::QuantumGate H ( q1, q2 ));
}

TEST(QuantumGate, Application_x) {
    // q with (1, 0) as state vector
    Solace::Qubit q;

    Solace::QubitStateVector q1 { 2/3, std::complex<double>(2,1)/3.0 };
    Solace::QubitStateVector q2 { std::complex<double>(-2,1)/3.0, 2/3 };

    const auto q1len { std::sqrt(std::norm(q1.first) + std::norm(q1.second)) };
    Solace::QubitStateVector q1_normalized { q1.first / q1len, q1.second / q1len };

    Solace::QuantumGate H { q1, q2 };

    H.apply(q);
    const auto sv { q.viewStateVector() };
    ASSERT_TRUE(std::abs(q1_normalized.first - sv.first) < 0.001);
    ASSERT_TRUE(std::abs(q1_normalized.second - sv.second) < 0.001);
}

TEST(QuantumGate, Application_y) {
    // q with (0, 1) as state vector
    Solace::Qubit q { 0, 1 };

    Solace::QubitStateVector q1 { 2/3, std::complex<double>(2,1)/3.0 };
    Solace::QubitStateVector q2 { std::complex<double>(-2,1)/3.0, 2/3 };

    const auto q2len { std::sqrt(std::norm(q2.first) + std::norm(q2.second)) };
    Solace::QubitStateVector q2_normalized { q2.first / q2len, q2.second / q2len };

    Solace::QuantumGate H { q1, q2 };

    H.apply(q);
    const auto sv { q.viewStateVector() };
    ASSERT_TRUE(std::abs(q2_normalized.first - sv.first) < 0.001);
    ASSERT_TRUE(std::abs(q2_normalized.second - sv.second) < 0.001);
}
