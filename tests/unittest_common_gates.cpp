#include <gtest/gtest.h>
#include "solace/common_gates.hpp"

TEST(CommonGate, Identity) {
    Solace::Qubits q { 0.5, std::sqrt(3)/2 };
    Solace::Gate::Identity H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.25) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 0.75) < 0.0001);
}
TEST(CommonGate, PauliX) {
    Solace::Qubits q { 1, 0 };
    Solace::Gate::PauliX H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.0) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 1.0) < 0.0001);
}

TEST(CommonGate, PauliY) {
    Solace::Qubits q { 1, 0 };
    Solace::Gate::PauliY H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.0) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 1.0) < 0.0001);
}

TEST(CommonGate, PauliZ) {
    Solace::Qubits q { 1, 0 };
    Solace::Gate::PauliZ H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 1.0) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 0.0) < 0.0001);
}

TEST(CommonGate, Hadamard) {
    Solace::Qubits q { 1, 0 };
    Solace::Gate::Hadamard H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.5) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 0.5) < 0.0001);
}

TEST(CommonGate, Hadamard2) {
    Solace::Qubits q {};
    Solace::Gate::Hadamard H;
    H.apply(q);

    // Create two qubit system
    Solace::Qubits q2 { q ^ q };
    const auto sv { q2.viewStateVector() };
    ASSERT_TRUE(std::abs(std::norm(sv(0)) - 0.25) < 0.001);
    ASSERT_TRUE(std::abs(std::norm(sv(1)) - 0.25) < 0.001);
    ASSERT_TRUE(std::abs(std::norm(sv(2)) - 0.25) < 0.001);
    ASSERT_TRUE(std::abs(std::norm(sv(3)) - 0.25) < 0.001);
}

TEST(CommonGate, Hadamard3) {
    // Create two qubits, zeroed, but entangled.
    Solace::Qubits q { 2 };
    Solace::Gate::Hadamard H;
    Solace::QuantumGate H2 { H ^ H };
    H2.apply(q);

    const auto sv { q.viewStateVector() };
    ASSERT_TRUE(std::abs(std::norm(sv(0)) - 0.25) < 0.001);
    ASSERT_TRUE(std::abs(std::norm(sv(1)) - 0.25) < 0.001);
    ASSERT_TRUE(std::abs(std::norm(sv(2)) - 0.25) < 0.001);
    ASSERT_TRUE(std::abs(std::norm(sv(3)) - 0.25) < 0.001);
}
