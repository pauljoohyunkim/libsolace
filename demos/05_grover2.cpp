/**
 * This example introduces
 * 1. How to compile quantum gates and qubits so that they can be reused in other projects.
 */
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"
#include "solace/utility.hpp"
#include "executionTimeMeasurement.hpp"
#include <iostream>
#include <filesystem>

#define QUBITS_FILE "qubits.qbit"
#define DIFFUSER_FILE "diffuser.qgate"
#define ORACLE_FILE "oracle.qgate"

#define SHOW_EXECUTION_TIME() do { std::cout << "Took " << END_TIMER() << "ms of execution" << std::endl; } while (0)
    

/**
 * To define a custom quantum gate, you need to allocate an appropriate matrix to transformer.
 * then call validate function.
 * Note that the transformer has to be an N x N matrix where N is some power of 2,
 * and it has to be a unitary matrix.
 */
class GroverDiffusionGate : public Solace::QuantumGate {
    public:
        GroverDiffusionGate(const Solace::StateVector& s) : Solace::QuantumGate() {
            const auto dim { s.size() };
            Solace::QuantumGateTransformer identity = Solace::QuantumGateTransformer::Identity(dim, dim);
            transformer = 2 * s * s.transpose() - identity;
            validate();
        }
};

class QuantumOracle : public Solace::QuantumGate {
    public:
        QuantumOracle(const Solace::ObservedQubitState sol, const unsigned int nQubits) : Solace::QuantumGate() {
            const unsigned int dim { 1U << nQubits };
            if (sol >= dim) {
                throw std::invalid_argument("Solution must be representable with nQubit qubits.");
            }
            transformer = Solace::SparseQuantumGateTransformer(dim, dim);
            auto& t { std::get<Solace::SparseQuantumGateTransformer>(transformer) };
            t.setIdentity();
            t.coeffRef(sol, sol) = -1.0;
            validate();
        }
};

int main() {
    constexpr int nQubits { 12 };       // Number of qubits increased from previous example.
    const int nIter { (int) (M_PI_4 * (std::exp2((float) nQubits / 2))) };

    // Initializing vector for Grover's algorithm.
    // Note that technically one should do diligence of initializing by applying Hadamard gate to all nQubits qubits,
    // but the initial state vector is also required for Grover diffusion gate.
    // I will be "quasi-cheating" here.
    std::cout << "Creating " << nQubits << " qubits and the required gates..." << std::endl;
    START_TIMER();
    Solace::StateVector s { Solace::StateVector::Ones(1<<nQubits) };
    s.normalize();
    SHOW_EXECUTION_TIME();

    Solace::Qubits system {};
    Solace::QuantumGate us {};
    Solace::QuantumGate uw {};
    if (!std::filesystem::exists(QUBITS_FILE)) {
        std::cout << "Creating initial vector s" << std::endl;
        START_TIMER();
        system = Solace::Qubits(s);
        SHOW_EXECUTION_TIME();
        std::cout << "Compiling..." << std::endl;
        START_TIMER();
        system.compile(QUBITS_FILE);
        SHOW_EXECUTION_TIME();
    } else {
        std::cout << "Loading initial qubit system file." << std::endl;
        START_TIMER();
        system = Solace::Qubits(QUBITS_FILE);
        SHOW_EXECUTION_TIME();
    }
    if (!std::filesystem::exists(DIFFUSER_FILE)) {
        std::cout << "Creating Grover diffusion gate" << std::endl;
        START_TIMER();
        us = GroverDiffusionGate(s);
        SHOW_EXECUTION_TIME();
        std::cout << "Compiling..." << std::endl;
        START_TIMER();
        us.compile(DIFFUSER_FILE);
        SHOW_EXECUTION_TIME();
    } else {
        std::cout << "Loading Grover diffusion gate" << std::endl;
        START_TIMER();
        us = Solace::QuantumGate(DIFFUSER_FILE);
        SHOW_EXECUTION_TIME();
    }
    if (!std::filesystem::exists(ORACLE_FILE)) {
        std::cout << "Creating oracle gate" << std::endl;
        START_TIMER();
        uw = QuantumOracle(3, nQubits);
        SHOW_EXECUTION_TIME();
        std::cout << "Compiling..." << std::endl;
        START_TIMER();
        uw.compile(ORACLE_FILE);
        SHOW_EXECUTION_TIME();
    } else {
        std::cout << "Loading oracle gate" << std::endl;
        START_TIMER();
        uw = Solace::QuantumGate(ORACLE_FILE);
        SHOW_EXECUTION_TIME();
    }

    std::cout << "Starting Grover algorithm" << std::endl;
    for (auto n = 0; n < nIter; n++) {
        std::cout << "Iter num: " << n << "/" << nIter << std::endl;
        uw.apply(system);
        us.apply(system);
    }

    std::cout << system.observe() << std::endl;

    return 0;
}
