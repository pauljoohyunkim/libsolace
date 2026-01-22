/**
 * This example introduces
 * 1. How to create create a circuit using QuantumCircuit API and run the circuit.
 */

 #include "solace/solace.hpp"
 #include "solace/common_gates.hpp"
 #include "solace/circuit.hpp"
 #include <unordered_map>
 #include <iostream>

 int main() {
    // Will be creating a circuit that creates Bell state;
    // a state of 1/sqrt(2) * (|00> + |11>)

    // The outline of the circuit can be described as the following:
    // 1. Two qubits are set to |0>. Call them q0 and q1.
    // 2. Apply Hadamard gate to q0.
    // 3. Apply CNOT gate on the tensor product q0 ^ q1.
    // 4. You have Bell state: You have 50-50 chance of outputting |00> (0) or |11> (3).

    /* Part 1: CIRCUIT CREATION */

    // Create empty circuit.
    Solace::QuantumCircuit qc {};

    // Add two qubits q0 and q1. Note that these are not "assigned" any real Solace::Qubits yet.
    Solace::QuantumCircuit::QubitsRef q0 { qc.createQubits(1) };
    Solace::QuantumCircuit::QubitsRef q1 { qc.createQubits(1) };

    // Load Hadamard gate and CNOT gate to the circuit.
    Solace::QuantumCircuit::QuantumGateRef H { qc.addQuantumGate(Solace::Gate::Hadamard()) };
    Solace::QuantumCircuit::QuantumGateRef CNOT { qc.addQuantumGate(Solace::Gate::CNOT()) };

    // Apply Hadamard to q0
    qc.applyQuantumGateToQubits(H, q0);

    // To apply CNOT gate, you need to turn the q0 ^ q1 into one.
    //std::vector<Solace::QuantumCircuit::QubitsRef>({q0, q1})
    Solace::QuantumCircuit::QubitsRef q0q1 { qc.entangle(std::vector<Solace::QuantumCircuit::QubitsRef>({q0, q1})) };
    // From this point onward, you cannot interact with q0 and q1 (as they are combined to q0q1).
    // Apply CNOT to q0q1
    qc.applyQuantumGateToQubits(CNOT, q0q1);

    // We are interested in the output. Let us observe q0q1.
    Solace::QuantumCircuit::QubitsRef q0q1_observed { qc.markForObservation(q0q1) };
    

    /* Step 2: Running */
    // To run the circuit, you can bind Solace::Qubits to your initially created Qubits components (q0 and q1 in this example),
    // or if they are not bound, it will default to |00...0> (where number of zeros depend on the argument you used for creating Qubits.)

    // For the sake of demonstration, I will manually bind the first qubit with |0> and rely on automatic binding on second qubit.
    // Essentially, q0 = |0> = q1
    qc.bindQubit(q0, Solace::Qubits(1));
    // Skip binding q1 for demonstration. (Even though at runtime, it will be bound to default state vector.)

    // An ordered map to store result.
    std::unordered_map<Solace::QuantumCircuit::QubitsRef, Solace::ObservedQubitState> results;

    // Run circuit.
    qc.run(results);

    // Print result.
    std::cout << results[q0q1_observed] << std::endl;

    // Extra tip: You can compile the quantum circuit as a file as you did with quantum gates by
    // qc.compile(filename)
    // Note that this file will contain definitions of your quantum gates used, so if your quantum gates are very large, the file will be very large!
    // Compiling a circuit DOES NOT retain the bound Solace::Qubits.
 }
