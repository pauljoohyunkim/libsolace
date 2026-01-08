#include <random>
#include <numbers>
#include <cmath>
#include "solace.hpp"

namespace Solace {

ObservedQubitState Qubit::observe(const bool cheat) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<double> weights { std::norm(stateVector.first), std::norm(stateVector.second) };
    std::discrete_distribution<> dist (weights.begin(), weights.end());

    const auto observedState { (ObservedQubitState) dist(gen) };
    if (cheat == false) {
        // State collapse
        std::uniform_real_distribution<double> phaseDist(0, M_PI);
        const auto phase { phaseDist(gen) };
        const auto nonzero { std::exp(std::complex<double>(0, phase)) };
        const auto complexzero { std::complex<double>(0, 0) };
        switch (observedState) {
            case ObservedQubitState::ZERO:
                stateVector = { nonzero, complexzero };
                break;
            case ObservedQubitState::ONE:
                stateVector = { complexzero, nonzero };
                break;
            default:
                throw std::runtime_error("Unexpected state!");
        }
    }
    return observedState;
}

void Qubit::normalizeStateVector() {
    const auto len { std::sqrt(std::norm(stateVector.first) + std::norm(stateVector.second)) };
    QubitStateVector sv { stateVector.first / len, stateVector.second / len };
    stateVector = sv;
}

}
