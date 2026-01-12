#include <gtest/gtest.h>
#include <iostream>
#include "solace/solace.hpp"

TEST(Compilation, SingleQubit) {
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    auto sv { q.viewStateVector() };
    std::cout << sv << std::endl;
    q.compile("./q.qbit");
}

TEST(Compilation, DoubleQubit) {
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    Solace::Qubits q2 { q ^ q };
    auto sv { q2.viewStateVector() };
    std::cout << sv << std::endl;
    q2.compile("./q2.qbit");
}
