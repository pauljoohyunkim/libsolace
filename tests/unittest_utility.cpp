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