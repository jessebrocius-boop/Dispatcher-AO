#ifndef AO_MODULATION_HPP
#define AO_MODULATION_HPP

#include <cmath>
#include <array>
#include <algorithm>
#include <cstddef>

namespace DispatcherAO {

using std::size_t;

constexpr size_t kNumLayers = 25;
constexpr double kBaseFreq = 55.0;
constexpr double kSoundSpeed = 565.0; // Perfluorinated dielectric fluid substrate (m/s)
constexpr double kEpsilon = 1e-6;     // Regularization floor for Phi^2

struct LayerConfig {
    double frequency;
    double z_ratio;
    double modulation_depth;
};

// Exact geometric progression: f_i = 55.0 * 2^(i / 6), i in [0, 24]
// NOTE: not constexpr — std::pow is not constexpr in C++17 (MSVC/Clang reject it).
inline std::array<LayerConfig, kNumLayers> generate_master_matrix() {
    std::array<LayerConfig, kNumLayers> matrix{};
    for (size_t i = 0; i < kNumLayers; ++i) {
        double f = kBaseFreq * std::pow(2.0, static_cast<double>(i) / 6.0);
        double z_ratio;
        double depth;
        if (i < 5) {
            z_ratio = 0.85 + (0.10 * i / 4.0);
            depth = 0.10 + (0.08 * i / 4.0);
        } else if (i < 10) {
            z_ratio = 0.95 + (0.15 * (i - 5) / 4.0);
            depth = 0.18 + (0.10 * (i - 5) / 4.0);
        } else if (i < 15) {
            z_ratio = 1.10 + (0.25 * (i - 10) / 4.0);
            depth = 0.28 + (0.22 * (i - 10) / 4.0);
        } else if (i < 20) {
            z_ratio = 1.35 - (0.25 * (i - 15) / 4.0);
            depth = 0.50 - (0.22 * (i - 15) / 4.0);
        } else {
            z_ratio = 1.10 - (0.25 * (i - 20) / 4.0);
            depth = 0.28 - (0.18 * (i - 20) / 4.0);
        }
        matrix[i] = LayerConfig{f, z_ratio, depth};
    }
    matrix[24].frequency = 880.0; // Exact terminal lock
    return matrix;
}

inline const auto kMasterMatrix = generate_master_matrix();

// Regularized tanh limiter ensuring absolute output bound: |out| <= A
inline double evaluate_modulation(double amplitude, double wavenumber, double x, double phi, double z_ratio) {
    double denom = (phi * phi) + kEpsilon;
    double arg = (wavenumber * x * z_ratio) / denom;
    return amplitude * std::tanh(arg);
}

// SIMD vectorized batch evaluation (compile with -fopenmp-simd for the pragma to apply)
inline void evaluate_batch(const double* amplitudes, const double* wavenumbers, const double* x_coords,
                           const double* phi_fields, const double* z_ratios, double* outputs, size_t count) {
    #pragma omp simd
    for (size_t i = 0; i < count; ++i) {
        double denom = (phi_fields[i] * phi_fields[i]) + kEpsilon;
        double arg = (wavenumbers[i] * x_coords[i] * z_ratios[i]) / denom;
        outputs[i] = amplitudes[i] * std::tanh(arg);
    }
}

} // namespace DispatcherAO

#endif // AO_MODULATION_HPP
