#include <iostream>
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

TEST(CircuitTest, ApplyQuantumGateToQubits) {
    Solace::QuantumCircuit qc;
    auto H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    auto tMaybe { H->viewTransformer() };
    auto t { std::get<Solace::QuantumGateTransformer>(tMaybe) };

    // Create a single qubit on the circuit.
    auto q { qc.createQubits() };
    
    // Apply Hadamard twice.
    q->applyQuantumGate(H);
    q->applyQuantumGate(H);

    auto appliedGates { q->getAppliedGates() };

    ASSERT_EQ(appliedGates.size(), 2);
    ASSERT_EQ(appliedGates.at(0), H);
    ASSERT_EQ(appliedGates.at(1), H);
}
