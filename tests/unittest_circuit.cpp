#include <iostream>
#include <gtest/gtest.h>
#include "solace/circuit.hpp"
#include "solace/common_gates.hpp"

TEST(CircuitTest, CreateQubits) {
    Solace::QuantumCircuit qc;
    auto q0 { qc.createQubits() };
    std::cout << "Qubit Reference: " << (int) q0 << std::endl;
}

TEST(CircuitTest, AddQuantumGate) {
    Solace::QuantumCircuit qc;
    Solace::Gate::Hadamard H {};
    auto H0 { qc.addQuantumGate(H) };
    std::cout << "QuantumGate Reference: " << (int) H0 << std::endl;
    //auto tMaybe { H0->viewTransformer() };
    //auto t { std::get<Solace::QuantumGateTransformer>(tMaybe) };
}

TEST(CircuitTest, ApplyQuantumGateToQubits) {
    Solace::QuantumCircuit qc;
    auto H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    //auto tMaybe { H->viewTransformer() };
    //auto t { std::get<Solace::QuantumGateTransformer>(tMaybe) };

    // Create a single qubit on the circuit.
    auto q { qc.createQubits() };
    
    // Apply Hadamard twice.
    qc.getQubits(q).applyQuantumGate(H);
    qc.getQubits(q).applyQuantumGate(H);

    auto appliedGates { qc.getQubits(q).getAppliedGates() };

    ASSERT_EQ(appliedGates.size(), 2);
    ASSERT_EQ(appliedGates.at(0), H);
    ASSERT_EQ(appliedGates.at(1), H);
}

TEST(CircuitTest, ApplyWrongNQubitsQuantumGateToQubits) {
    Solace::QuantumCircuit qc;
    auto H { qc.addQuantumGate(Solace::Gate::Swap()) };
    auto tMaybe { qc.getGates().at(H).viewTransformer() };
    //auto t { std::get<Solace::QuantumGateTransformer>(tMaybe) };

    // Create a single qubit on the circuit.
    auto q { qc.createQubits() };
    
    // Apply Swap twice. Expect failure
    ASSERT_ANY_THROW(qc.getQubits(q).applyQuantumGate(H));

}

TEST(CircuitTest, EntangleQubits) {
    Solace::QuantumCircuit qc;
    auto H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    auto q0 { qc.createQubits(1) };
    auto q1 { qc.createQubits(2) };
    qc.getQubits(q0).applyQuantumGate(H);

    std::vector<Solace::QuantumCircuit::QubitsRef> qbts { q0, q1 };
    auto q0q1 { qc.entangle(qbts) };

    ASSERT_EQ(qc.getQubits(q0q1).getEntangleTo(), 0);
    auto entangledFrom { qc.getQubits(q0q1).getEntangledFrom() };
    ASSERT_EQ(entangledFrom.size(), 2);
    ASSERT_EQ(entangledFrom.at(0) , q0);
    ASSERT_EQ(entangledFrom.at(1) , q1);
    ASSERT_EQ(qc.getQubits(q0).getEntangleTo(), q0q1);
    ASSERT_EQ(qc.getQubits(q1).getEntangleTo(), q0q1);
    ASSERT_EQ(qc.getQubits(q0).getEntangledFrom().size(), 0);
    ASSERT_EQ(qc.getQubits(q1).getEntangledFrom().size(), 0);
}

TEST(CircuitTest, IllegalEntanglement_AlreadyEntangled) {
    Solace::QuantumCircuit qc;
    auto q0 { qc.createQubits(1) };
    auto q1 { qc.createQubits(2) };
    std::vector<Solace::QuantumCircuit::QubitsRef> qbts {q0, q1};
    auto q0q1 { qc.entangle(qbts) };
    
    // q0 and q1 cannot be used again, so it should throw error.
    std::vector<Solace::QuantumCircuit::QubitsRef> qbts2 {q0q1, q0};
    ASSERT_ANY_THROW(qc.entangle(qbts2));
}

TEST(CircuitTest, IllegalEntanglement_Duplicate) {
    Solace::QuantumCircuit qc;
    auto q0 { qc.createQubits(1) };
    auto q1 { q0 };
    std::vector<Solace::QuantumCircuit::QubitsRef> qbts {q0, q1};
    ASSERT_ANY_THROW(qc.entangle(qbts));
}

TEST(CircuitTest, RunBellStateCircuit) {
    Solace::QuantumCircuit qc;

    // Create two qubits.
    auto q0 { qc.createQubits() };
    auto q1 { qc.createQubits() };

    // Hadamard gate and CNOT gate
    auto H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    auto CNOT { qc.addQuantumGate(Solace::Gate::CNOT()) };

    // q0 goes through Hadamard
    qc.applyQuantumGateToQubits(H, q0);

    // Entangle two qubits.
    std::vector<Solace::QuantumCircuit::QubitsRef> q01_vec { q0, q1 };
    auto q01 { qc.entangle(q01_vec) };
    
    // Apply CNOT gate to the entangled pair of qubits.
    qc.applyQuantumGateToQubits(CNOT, q01);

    // End of circuit
    // Now bind q0 and q1 with newly created Solace::Qubits
    Solace::Qubits actualQubit {};
    
    qc.bindQubit(q0, actualQubit);
    qc.bindQubit(q1, actualQubit);

    qc.run();
    
}