#include <memory>
#include "solace/solace.hpp"
#include "solace/circuit.hpp"

namespace Solace {
    // Internal structure that keeps track of quantum gates as nodes.
    struct QuantumCircuitGateNode {
        std::shared_ptr<QuantumCircuitGateNode> prevNode;
        std::shared_ptr<QuantumGate> gate;
    };


}