#include <gtest/gtest.h>
#include <iostream>
#include "solace/solace.hpp"

TEST(Compilation, SingleQubit) {
    const std::string filename { "./q.qbit" };
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    auto sv { q.viewStateVector() };
    std::cout << sv << std::endl;
    q.compile(filename);

    Solace::Qubits q_load { filename };
    auto sv_load { q_load.viewStateVector() };
    std::cout << sv_load << std::endl;

    auto diff { sv - sv_load };
    ASSERT_TRUE(diff.norm() < 0.001);
}

TEST(Compilation, DoubleQubit) {
    const std::string filename { "./q2.qbit" };
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    Solace::Qubits q2 { q ^ q };
    auto sv { q2.viewStateVector() };
    std::cout << sv << std::endl;
    q2.compile(filename);

    Solace::Qubits q2_load { filename };
    auto sv_load { q2_load.viewStateVector() };
    std::cout << sv_load << std::endl;

    auto diff { sv - sv_load };
    ASSERT_TRUE(diff.norm() < 0.001);
}
