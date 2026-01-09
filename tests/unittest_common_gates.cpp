#include <gtest/gtest.h>
#include "solace/common_gates.hpp"

TEST(CommonGate, Identity) {
    Solace::Qubit q { 0.5, std::sqrt(3)/2 };
    Solace::Gate::Identity H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.25) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 0.75) < 0.0001);
}
TEST(CommonGate, PauliX) {
    Solace::Qubit q { 1, 0 };
    Solace::Gate::PauliX H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.0) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 1.0) < 0.0001);
}

TEST(CommonGate, PauliY) {
    Solace::Qubit q { 1, 0 };
    Solace::Gate::PauliY H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.0) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 1.0) < 0.0001);
}

TEST(CommonGate, PauliZ) {
    Solace::Qubit q { 1, 0 };
    Solace::Gate::PauliZ H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 1.0) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 0.0) < 0.0001);
}

TEST(CommonGate, Hadamard) {
    Solace::Qubit q { 1, 0 };
    Solace::Gate::Hadamard H;
    H.apply(q);

    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) - 0.5) < 0.0001);
    ASSERT_TRUE(std::abs(std::norm(sv[1]) - 0.5) < 0.0001);
}
