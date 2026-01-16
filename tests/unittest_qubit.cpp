#include <gtest/gtest.h>
#include "solace/solace.hpp"

TEST(Qubit, Normalization) {
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    const auto sv { q.viewStateVector() };

    ASSERT_TRUE(std::abs(std::norm(sv[0]) + std::norm(sv[1]) - 1) < 0.000001);
}

TEST(Qubit, InvalidLength) {
    ASSERT_ANY_THROW(Solace::Qubits q0 { std::vector<std::complex<double>>({}) });
    ASSERT_ANY_THROW(Solace::Qubits q5 { std::vector<std::complex<double>>({1, 2, 3, 4, 5 }) });
}

TEST(Qubit, ObservationWithCheat) {
    const unsigned int testN { 10000 };
    Solace::Qubits q {
        {1,2},
        {-3,1}
    };
    const auto sv { q.viewStateVector() };
    std::vector<double> dist { std::norm(sv[0]), std::norm(sv[1]) };
    std::vector<int> observedCount { 0, 0 };

    for (auto i = 0U; i < testN; i++) {
        const auto observation { q.cheatObserve() };
        if (observation == 0) {
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
    Solace::Qubits q {
        {1,2},
        {-3,1}
    };
    auto ret { q.observe() };
    const auto sv { q.viewStateVector() };
    if (ret == 0) {
        ASSERT_EQ(sv[1], std::complex<double>(0,0));
    }
    if (ret == 1) {
        ASSERT_EQ(sv[0], std::complex<double>(0,0));
    }

    ASSERT_TRUE(std::abs(std::norm(sv[0]) + std::norm(sv[1]) - 1) < 0.000001);
}

TEST(Qubit, EntangledQubits) {
    Solace::Qubits q1 { 1 };
    Solace::Qubits q2 { {3, 2},
                      {1, -2.2} };
    Solace::Qubits q1xq2 { q1 ^ q2 };
}
