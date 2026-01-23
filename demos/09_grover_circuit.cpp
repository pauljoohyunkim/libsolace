/**
 * This example is another circuit example. This time the Grover solver will be implemented in circuit.
 */
#include "solace/solace.hpp"
#include "solace/circuit.hpp"

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
    constexpr size_t nQubits { 10 };
    const int nIter { (int) (M_PI_4 * (std::exp2((float) nQubits / 2))) };
    
    std::cout << "nIter " << nIter << std::endl;
    Solace::StateVector s { Solace::StateVector::Ones(1<<nQubits) };
    s.normalize();
    Solace::Qubits initialS { s };
    GroverDiffusionGate us { s };
    QuantumOracle uw { 3, nQubits };

    // Constructing circuit
    Solace::QuantumCircuit qc {};
    Solace::QuantumCircuit::QubitsRef q { qc.createQubits(nQubits) };
    Solace::QuantumCircuit::QuantumGateRef oracle { qc.addQuantumGate(uw) };
    Solace::QuantumCircuit::QuantumGateRef grover { qc.addQuantumGate(us) };
    for (auto n = 0; n < nIter; n++) {
        std::cout << "Circuit Creation Iter num: " << n << "/" << nIter << std::endl;
        qc.applyQuantumGateToQubits(oracle, q);
        qc.applyQuantumGateToQubits(grover, q);
    }
    Solace::QuantumCircuit::QubitsRef q0_observed { qc.markForObservation(q) };

    // Circuit created. Bind the initial Qubits and run.
    std::unordered_map<Solace::QuantumCircuit::QubitsRef, Solace::ObservedQubitState> result {};
    qc.bindQubits(q, initialS);
    qc.run(result);

    // Output result
    std::cout << "Result: " << result[q0_observed] << std::endl;

    return 0;
}
