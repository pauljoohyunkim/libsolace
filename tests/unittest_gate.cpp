#include <iostream>
#include <gtest/gtest.h>
#include "solace/solace.hpp"
//#include "solace/common_gates.hpp"

TEST(QuantumGate, ValidityOK) {
    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;

    Solace::QuantumGate H { q1, q2 };
}

TEST(QuantumGate, ValidityFail) {
    Solace::StateVector q1(2);
    q1 << 1, 2;
    Solace::StateVector q2(2);
    q2 << 3, 4;

    ASSERT_ANY_THROW(Solace::QuantumGate H ( q1, q2 ));
}

TEST(QuantumGate, Application_x) {
    // q with (1, 0) as state vector
    Solace::Qubits q;

    Solace::StateVector q1(2); 
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;

    Solace::QuantumGate H { q1, q2 };
    q1.normalize();

    H.apply(q);
    std::cout << H.viewTransformer() << std::endl;
    const auto sv { q.viewStateVector() };
    ASSERT_TRUE(std::abs(q1[0] - sv[0]) < 0.001);
    ASSERT_TRUE(std::abs(q1[1] - sv[1]) < 0.001);
}

TEST(QuantumGate, Application_y) {
    // q with (0, 1) as state vector
    Solace::Qubits q { 0, 1 };

    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;

    Solace::QuantumGate H { q1, q2 };
    q2.normalize();

    H.apply(q);
    const auto sv { q.viewStateVector() };
    ASSERT_TRUE(std::abs(q2[0] - sv[0]) < 0.001);
    ASSERT_TRUE(std::abs(q2[1] - sv[1]) < 0.001);
}
