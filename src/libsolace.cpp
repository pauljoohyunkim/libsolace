#include "libsolace.hpp"

namespace Solace {
ObservedQubitState Qubit::observe() {
    return ObservedQubitState::ZERO;
}

}
