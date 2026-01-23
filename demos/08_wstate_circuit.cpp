/**
 * This example is another circuit example, but with partial observation.
 */
#include "solace/solace.hpp"
#include "solace/circuit.hpp"

int main() {
    Solace::QuantumCircuit qc {};

    // W state is a three-qubit state
    auto q { qc.createQubits(3) };

    // Mark for partially observing with bitmask 0b101
    auto q02Observe { qc.markForObservation(q, 0b101) };
    auto q02 { q02Observe.first };
    auto q1Unobserved { q02Observe.second };
    auto q1 { qc.markForObservation(q1Unobserved) };

    // End of circuit creation.

    // W state: 1/sqrt(3) (|001> + |010> + |100>)
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
    qc.bindQubits(q, w);

    std::unordered_map<Solace::QuantumCircuit::QubitsRef, Solace::ObservedQubitState> results {};
    qc.run(results);

    std::cout << "Measuring first and last qubits (Either 0, 1, 4, or 5): " << results[q02] << std::endl;
    switch (results[q02]) {
        case 0:
            std::cout << "Middle qubit: Must be 1" << std::endl;
            break;
        case 1:
        case 4:
            std::cout << "Middle qubit: Must be 0" << std::endl;
            break;
        case 5:
        default:
            throw std::runtime_error("Partial reading is not well implemented... Contact developer!");
    }
    
    std::cout << "Fully measure the last (middle) qubit: " << results[q1] << std::endl;

    return 0;
}
