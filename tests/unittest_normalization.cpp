#include <gtest/gtest.h>
#include "libsolace.hpp"

TEST(Qubit, Normalization) {
    Solace::Qubit q { {3, 2},
                      {1, -2.2} };
    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv.first) + std::norm(sv.second) - 1) < 0.000001);
}
