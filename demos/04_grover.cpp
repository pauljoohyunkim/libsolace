/**
 * This example introduces
 * 1. How to create custom quantum gates.
 */
#include "solace/solace.hpp"
#include "solace/common_gates.hpp"
#include "solace/utility.hpp"
#include "executionTimeMeasurement.hpp"
#include <iostream>

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
    constexpr int nQubits { 10 };
    constexpr int nIter { (int) (M_PI_4 * (1 << (nQubits >> 1))) };

    // Initializing vector for Grover's algorithm.
    // Note that technically one should do diligence of initializing by applying Hadamard gate to all nQubits qubits,
    // but the initial state vector is also required for Grover diffusion gate.
    // I will be "quasi-cheating" here.
    std::cout << "Creating " << nQubits << " qubits and the required gates..." << std::endl;
    START_TIMER();
    Solace::StateVector s { Solace::StateVector::Ones(1<<nQubits) };
    s.normalize();

    Solace::Qubits system { s };
    GroverDiffusionGate us { s };
    QuantumOracle uw { 3, nQubits };
    auto duration { END_TIMER() };
    std::cout << "Took " << duration << "ms for creating quantum objects." << std::endl;

    std::cout << "Starting Grover algorithm" << std::endl;
    for (auto n = 0; n < nIter; n++) {
        std::cout << "Iter num: " << n << "/" << nIter << std::endl;
        uw.apply(system);
        us.apply(system);
    }

    std::cout << system.observe() << std::endl;

    return 0;
}
