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

TEST(CircuitTest, EntangleQubits) {
    Solace::QuantumCircuit qc;
    auto H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    auto q0 { qc.createQubits(1) };
    auto q1 { qc.createQubits(2) };
    q0->applyQuantumGate(H);

    std::vector<std::shared_ptr<Solace::QuantumCircuitComponent::Qubits>> qbts { q0, q1 };
    auto q0q1 { qc.entangle(qbts) };

    ASSERT_EQ(q0q1->getEntangleTo(), nullptr);
    auto entangledFrom { q0q1->getEntangledFrom() };
    ASSERT_EQ(entangledFrom.size(), 2);
    ASSERT_EQ(entangledFrom.at(0) , q0);
    ASSERT_EQ(entangledFrom.at(1) , q1);
    ASSERT_EQ(q0->getEntangleTo(), q0q1);
    ASSERT_EQ(q1->getEntangleTo(), q0q1);
    ASSERT_EQ(q0->getEntangledFrom().size(), 0);
    ASSERT_EQ(q1->getEntangledFrom().size(), 0);
}
