#include <gtest/gtest.h>
#include <iostream>
#include "solace/solace.hpp"
#include "solace/circuit.hpp"
#include "solace/common_gates.hpp"

TEST(Compilation, SingleQubit) {
    const std::string filename { "./q.qbit" };
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    auto sv { q.viewStateVector() };
    std::cout << sv << std::endl;
    q.compile(filename);

    Solace::Qubits q_load { filename };
    auto sv_load { q_load.viewStateVector() };
    std::cout << sv_load << std::endl;

    auto diff { sv - sv_load };
    ASSERT_TRUE(diff.norm() < 0.001);
}

TEST(Compilation, DoubleQubit) {
    const std::string filename { "./q2.qbit" };
    Solace::Qubits q { {3, 2},
                      {1, -2.2} };
    Solace::Qubits q2 { q ^ q };
    auto sv { q2.viewStateVector() };
    std::cout << sv << std::endl;
    q2.compile(filename);

    Solace::Qubits q2_load { filename };
    auto sv_load { q2_load.viewStateVector() };
    std::cout << sv_load << std::endl;

    auto diff { sv - sv_load };
    ASSERT_TRUE(diff.norm() < 0.001);
}

TEST(Compilation, QuantumGate1) {
    const std::string filename { "./q.qgate" };
    Solace::StateVector q1(2);
    q1 << 2.0/3.0, std::complex<double>(2,1)/3.0;
    Solace::StateVector q2(2);
    q2 << std::complex<double>(-2,1)/3.0, 2.0/3.0;
    Solace::QuantumGate H { q1, q2 };

    auto t { std::get<Solace::QuantumGateTransformer>(H.viewTransformer()) };
    std::cout << t << std::endl;
    H.compile(filename);

    Solace::QuantumGate H_load { filename };
    auto t_load { std::get<Solace::QuantumGateTransformer>(H_load.viewTransformer()) };
    std::cout << t_load << std::endl;

    auto diff { t - t_load };
    ASSERT_TRUE(diff.norm() < 0.001);
}

TEST(Compilation, Circuit1) {
    // A circuit that prepares the Bell state
    Solace::QuantumCircuit qc;

    // Need two qubits.
    auto q0 { qc.createQubits() };
    auto q1 { qc.createQubits() };

    qc.getQubits(q0).label = "q0";
    qc.getQubits(q1).label = "q1";

    // Need two gates
    auto Hadamard { Solace::Gate::Hadamard() };
    auto H { qc.addQuantumGate(Hadamard) };
    qc.setQuantumGateLabel(H, "Hadamard");
    auto CNOTGate { Solace::Gate::CNOT() };
    auto CNOT { qc.addQuantumGate(CNOTGate) };
    qc.setQuantumGateLabel(CNOT, "CNOT");

    // First, q0 goes through Hadamard
    // TODO: Allow qc to directly take a qubits reference and quantum gates reference.
    qc.applyQuantumGateToQubits(H, q0);
    
    // Entangle the two qubits.
    std::vector<Solace::QuantumCircuit::QubitsRef> q01_vec { q0, q1 };
    auto q01 { qc.entangle(q01_vec) };
    qc.getQubits(q01).label = "q0 ^ q1";

    // Apply CNOT to the entalged state
    qc.applyQuantumGateToQubits(CNOT, q01);

    // Mark for full observation
    qc.markForObservation(q01);

    qc.compile("bell.qc");

    // Load the compiled circuit.
    Solace::QuantumCircuit qc2 { "bell.qc" };
}
