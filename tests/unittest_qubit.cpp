#include <gtest/gtest.h>
#include "libsolace.hpp"

TEST(Qubit, Normalization) {
    Solace::Qubit q { {3, 2},
                      {1, -2.2} };
    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv.first) + std::norm(sv.second) - 1) < 0.000001);
}

TEST(Qubit, ObservationWithCheat) {
    const unsigned int testN { 10000 };
    Solace::Qubit q {
        {1,2},
        {-3,1}
    };
    const auto sv { q.viewStateVector() };
    std::vector<double> dist { std::norm(sv.first), std::norm(sv.second) };
    std::vector<int> observedCount { 0, 0 };

    for (auto i = 0U; i < testN; i++) {
        const auto observation { q.observe(true) };
        if (observation == Solace::ObservedQubitState::ZERO) {
            observedCount[0]++;
        } else {
            observedCount[1]++;
        }
    }

    ASSERT_TRUE((dist[0] - (double) observedCount[0]/testN) < 0.05);
    ASSERT_TRUE((dist[1] - (double) observedCount[1]/testN) < 0.05);
    std::cout << dist[0] << std::endl;
    std::cout << observedCount[0] << "/" << testN << std::endl;
    std::cout << dist[1] << std::endl;
    std::cout << observedCount[1] << "/" << testN << std::endl;
}

TEST(Qubit, ObservationCollapse) {
    Solace::Qubit q {
        {1,2},
        {-3,1}
    };
    const auto ret { q.observe(false) };
    const auto sv { q.viewStateVector() };
    if (ret == Solace::ObservedQubitState::ZERO) {
        ASSERT_EQ(sv.second, std::complex<double>(0,0));
    }
    if (ret == Solace::ObservedQubitState::ONE) {
        ASSERT_EQ(sv.first, std::complex<double>(0,0));
    }

    ASSERT_TRUE(std::abs(std::norm(sv.first) + std::norm(sv.second) - 1) < 0.000001);
}
