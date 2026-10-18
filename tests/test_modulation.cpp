#undef NDEBUG  // keep assert() active even in release builds
#include <iostream>
#include <cassert>
#include <cmath>
#include "ao_modulation.hpp"

int main() {
    std::cout << "[Dispatcher AO] Initializing modulation matrix verification...\n";

    auto matrix = DispatcherAO::generate_master_matrix();

    // Verify boundary and octave keypoints
    assert(std::abs(matrix[0].frequency - 55.0) < 1e-4);
    assert(std::abs(matrix[5].frequency - 98.0) < 1e-2);
    assert(std::abs(matrix[10].frequency - 174.61) < 1e-2);
    assert(std::abs(matrix[15].frequency - 311.13) < 1e-2);
    assert(std::abs(matrix[20].frequency - 554.37) < 1e-2);
    assert(std::abs(matrix[24].frequency - 880.0) < 1e-4);
    std::cout << "[PASS] Geometric 2^(1/6) frequency progression verified.\n";

    // Verify continuity of impedance ratios across phases
    for (size_t i = 1; i < DispatcherAO::kNumLayers; ++i) {
        double delta_z = std::abs(matrix[i].z_ratio - matrix[i-1].z_ratio);
        assert(delta_z < 0.1);
    }
    std::cout << "[PASS] Impedance ratio continuity verified.\n";

    // Verify boundedness as Phi -> 0 (Soft Saturation Proof)
    double A = 10.0;
    double k = 1.0;
    double x = 0.5;
    double z = 1.2;
    for (double phi = 1.0; phi >= 0.0; phi -= 0.1) {
        double val = DispatcherAO::evaluate_modulation(A, k, x, phi, z);
        assert(std::abs(val) <= A);
    }
    double zero_phi_val = DispatcherAO::evaluate_modulation(A, k, x, 0.0, z);
    assert(std::abs(zero_phi_val) <= A);
    std::cout << "[PASS] Bounded output invariant (|out| <= A) verified at Phi -> 0.\n";

    std::cout << "All modulation matrix unit tests passed successfully.\n";
    return 0;
}
