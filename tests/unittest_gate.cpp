#include <gtest/gtest.h>
#include "solace/solace.hpp"

TEST(QuantumGate, ValidityOK) {
    Solace::QubitStateVector q1 { 2/3, std::complex<double>(2,1)/3.0 };
    Solace::QubitStateVector q2 { std::complex<double>(-2,1)/3.0, 2/3 };

    Solace::QuantumGate H { q1, q2 };
}

TEST(QuantumGate, ValidityFail) {
    Solace::QubitStateVector q1 { 1, 2 };
    Solace::QubitStateVector q2 { 3, 4 };

    ASSERT_ANY_THROW(Solace::QuantumGate H ( q1, q2 ));
}
