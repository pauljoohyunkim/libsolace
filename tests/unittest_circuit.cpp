#include <gtest/gtest.h>
#include "solace/circuit.hpp"
#include "solace/common_gates.hpp"

TEST(CircuitTest, AddQubits) {
    Solace::QuantumCircuit qc;
    Solace::Qubits q {};
    auto q0 { qc.addQubits(q) };
    auto sv { q0->viewStateVector() };
    ASSERT_EQ(sv.size(), 2);
    ASSERT_EQ(sv(0), std::complex<double>(1));
    ASSERT_EQ(sv(1), std::complex<double>(0));
}

TEST(CircuitTest, AddQuantumGate) {
    Solace::QuantumCircuit qc;
    Solace::Gate::Hadamard H {};
    auto H0 { qc.addQuantumGate(H) };
    auto tMaybe { H0->viewTransformer() };
    auto t { std::get<Solace::QuantumGateTransformer>(tMaybe) };
}
