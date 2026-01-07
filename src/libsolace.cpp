#include "libsolace.hpp"

namespace Solace {

ObservedQubitState Qubit::observe() {
    return ObservedQubitState::ZERO;
}

void Qubit::normalizeStateVector() {
    const auto len { std::sqrt(std::norm(stateVector.first) + std::norm(stateVector.second)) };
    QubitStateVector sv { stateVector.first / len, stateVector.second / len };
    stateVector = sv;
}

}
