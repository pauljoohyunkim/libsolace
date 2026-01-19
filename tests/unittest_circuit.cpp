#include <gtest/gtest.h>
#include "solace/circuit.hpp"
#include "solace/common_gates.hpp"

TEST(CircuitTest, CreateQubits) {
    Solace::QuantumCircuit qc;
    auto q0 { qc.createQubits() };
}

TEST(CircuitTest, AddQuantumGate) {
    Solace::QuantumCircuit qc;
    Solace::Gate::Hadamard H {};
    auto H0 { qc.addQuantumGate(H) };
    auto tMaybe { H0->viewTransformer() };
    auto t { std::get<Solace::QuantumGateTransformer>(tMaybe) };
}
