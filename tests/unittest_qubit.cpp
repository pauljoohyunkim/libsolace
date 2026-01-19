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

TEST(Qubit, WState) {
    std::complex<double> v { 1/std::sqrt(3), 0 };
    Solace::StateVector sv(8);
    sv << 0, v, v, 0, v, 0, 0, 0;
    std::cout << sv << std::endl;
    Solace::Qubits q { sv };

    // Observing qubit 0 and 2
    const auto bitmask { 0b101 };
    auto result { q.observe(bitmask) };
    auto measurement { result.first };
    auto unobservedMaybe { result.second };
    ASSERT_TRUE(measurement == 0b000 || measurement == 0b001 || measurement == 0b100 || measurement == 0b101);
    Solace::Qubits unobserved { unobservedMaybe.value() };
    Solace::StateVector unobservedSv { unobserved.viewStateVector() };

    std::cout << "Measurement: " << (int) measurement << std::endl;
    std::cout << "Unobserved state vector" << unobservedSv << std::endl;
    std::cout << std::flush;
}

TEST(Qubit, WState2) {
    std::complex<double> v { 1/std::sqrt(3), 0 };
    Solace::StateVector sv(8);
    sv << 0, v, v, 0, v, 0, 0, 0;
    Solace::Qubits q { sv };

    // Observing qubit 2; qubit 0 and qubit 1 still entangled
    const auto bitmask { 0b001 };
    auto result { q.observe(bitmask) };
    auto measurement { result.first };
    auto entangledMaybe { result.second };
    ASSERT_TRUE(measurement == 0U || measurement == 1U);
    Solace::Qubits entangled { entangledMaybe.value() };
    Solace::StateVector entangledSv { entangled.viewStateVector() };
    ASSERT_EQ(entangledSv.size(), 4);   // 2 qubit system expected.
    std::cout << entangledSv << std::endl;
}
