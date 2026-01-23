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
    qc.applyQuantumGateToQubits(H, q);
    qc.applyQuantumGateToQubits(H, q);

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
    ASSERT_ANY_THROW(qc.applyQuantumGateToQubits(H, q));

}

TEST(CircuitTest, EntangleQubits) {
    Solace::QuantumCircuit qc;
    auto H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    auto q0 { qc.createQubits(1) };
    auto q1 { qc.createQubits(2) };
    qc.applyQuantumGateToQubits(H, q0);

    std::vector<Solace::QuantumCircuit::QubitsRef> qbts { q0, q1 };
    auto q0q1 { qc.entangle(qbts) };

    ASSERT_ANY_THROW(qc.getQubits(q0q1).getEntangleTo());
    auto entangledFrom { qc.getQubits(q0q1).getEntangledFrom() };
    ASSERT_EQ(entangledFrom.size(), 2);
    ASSERT_EQ(entangledFrom.at(0) , q0);
    ASSERT_EQ(entangledFrom.at(1) , q1);
    ASSERT_EQ(qc.getQubits(q0).getEntangleTo(), q0q1);
    ASSERT_EQ(qc.getQubits(q1).getEntangleTo(), q0q1);
    ASSERT_ANY_THROW(qc.getQubits(q0).getEntangledFrom());
    ASSERT_ANY_THROW(qc.getQubits(q1).getEntangledFrom());
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
    
    qc.bindQubits(q0, actualQubit);
    qc.bindQubits(q1, actualQubit);

    qc.markForObservation(q01);

    auto qc2 { qc };

    qc.run();

    std::unordered_map<Solace::QuantumCircuit::QubitsRef, Solace::ObservedQubitState> observationResult {};
    qc2.run(observationResult);
    
}

TEST(CircuitTest, PartialObserve) {
    // Create a simple circuit of simply partial-observing the first qubit. Will be reading W-state

    Solace::QuantumCircuit qc {};

    // W state has three qubits.
    auto q0 { qc.createQubits(3) };

    // Partial read the first qubit.
    auto q0_read { qc.markForObservation(q0, 0b100) };

    // End of circuit.
    // Binding W state.
    Solace::StateVector wSv(8);
    wSv << 0,
           1,
           1,
           0,
           1,
           0,
           0,
           0;
    Solace::Qubits w { wSv };

    // Testing 100 times. The result must be either 0 or 4
    for (auto i = 0; i < 100; i++) {
        qc.bindQubits(q0, w);

        std::unordered_map<Solace::QuantumCircuit::QubitsRef, Solace::ObservedQubitState> observationResult {};

        qc.run(observationResult);

        ASSERT_TRUE(observationResult[q0_read.first] == 0 || observationResult[q0_read.first] == 4);
        // Unbind for the next run
        qc.unbindAllQubits();
    }
}
