#include <iostream>
#include <gtest/gtest.h>
#include "solace/common_gates.hpp"
#include "solace/solace.hpp"
//#include "solace/common_gates.hpp"

TEST(QuantumGate, ValidityOK) {
    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;

    Solace::QuantumGate H { q1, q2 };

    q1.normalize();
    q2.normalize();
    Solace::QuantumGateTransformer H_transformer(2,2);
    H_transformer << q1, q2;
    Solace::QuantumGate H2 { H_transformer };
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
    std::cout << std::get<Solace::QuantumGateTransformer>(H.viewTransformer()) << std::endl;
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

// Checks if HG = H * G
TEST(QuantumGate, MergeOperatorDense) {
    // q with normalized (2, 1) as state vector
    Solace::Qubits q { 2, 1 };
    Solace::Qubits q2 { 2, 1 };

    Solace::StateVector Q1(2);
    Q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector Q2(2);
    Q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;
    Solace::QuantumGate G { Q1, Q2 };
    Solace::Gate::Hadamard H;

    // q is now HG(q)
    G.apply(q);
    H.apply(q);

    Solace::QuantumGate HG { H * G };
    HG.apply(q2);

    const auto sv { q.viewStateVector() };
    const auto sv2 { q2.viewStateVector() };
    auto diff { sv - sv2 };
    ASSERT_TRUE(diff.norm() < 0.0001);
}

TEST(QuantumGate, TensorProductSparseSparse) {
    std::complex<double> i { 0, 1 };
    Solace::SparseQuantumGateTransformer g(2,2);
    g.insert(0, 0) = -1;
    g.insert(1, 1) = i;
    g.makeCompressed();
    Solace::QuantumGate G { g };

    Solace::SparseQuantumGateTransformer s(4, 4);
    s.insert(0,0) = 1;
    s.insert(2,1) = 1;
    s.insert(1, 2) = 1;
    s.insert(3, 3) = 1;
    s.makeCompressed();
    Solace::QuantumGate S { s };

    Solace::SparseQuantumGateTransformer gs_expected(8, 8);
    gs_expected.insert(0,0) = -1;
    gs_expected.insert(1,2) = -1;
    gs_expected.insert(2,1) = -1;
    gs_expected.insert(3,3) = -1;
    gs_expected.insert(4,4) = i;
    gs_expected.insert(5,6) = i;
    gs_expected.insert(6,5) = i;
    gs_expected.insert(7,7) = i;
    gs_expected.makeCompressed();

    Solace::QuantumGate GS { G ^ S };
    auto t_gs_maybe { GS.viewTransformer() };
    auto t_gs = std::get<Solace::SparseQuantumGateTransformer>(t_gs_maybe);
    auto diff { t_gs - gs_expected };
    ASSERT_TRUE(diff.norm() < 0.00001);
    
}

TEST(QuantumGate, TensorProductSparseDense) {
    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;
    Solace::QuantumGate H { q1, q2 };
    
    Solace::SparseQuantumGateTransformer t(2,2);
    t.insert(0,0) = 1;
    t.insert(1,1) = -1;
    t.makeCompressed();
    Solace::QuantumGate G { t };

    Solace::QuantumGate GH { G ^ H };
    auto t_gh_maybe { GH.viewTransformer() };
    auto t_gh = std::get<Solace::QuantumGateTransformer>(t_gh_maybe);
    std::cout << t_gh << std::endl;
}

TEST(QuantumGate, TensorProductDenseSparse) {
    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;
    Solace::QuantumGate G { q1, q2 };
    
    Solace::SparseQuantumGateTransformer t(2,2);
    t.insert(0,0) = 1;
    t.insert(1,1) = -1;
    t.makeCompressed();
    Solace::QuantumGate H { t };

    Solace::QuantumGate GH { G ^ H };
    auto t_gh_maybe { GH.viewTransformer() };
    auto t_gh = std::get<Solace::QuantumGateTransformer>(t_gh_maybe);
    std::cout << t_gh << std::endl;
}

TEST(QuantumGate, TensorProductDenseDense) {
    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;
    Solace::QuantumGate H { q1, q2 };
    
    Solace::QuantumGate H2 { H ^ H };
    auto t_h2_maybe { H2.viewTransformer() };
    auto t_h2 = std::get<Solace::QuantumGateTransformer>(t_h2_maybe);
    std::cout << t_h2 << std::endl;
}
