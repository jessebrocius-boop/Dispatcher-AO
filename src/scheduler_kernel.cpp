// scheduler_kernel.cpp – placeholder for Dispatcher AO kernel integration
// This file will eventually contain the real-time scheduler loop that
// consumes the 25‑layer modulation matrix (ao_modulation.hpp) and drives
// the acoustic‑to‑electrical transduction hardware.
//
// For now it only includes the header and a dummy function so the
// source tree compiles. Replace the stub with the actual implementation
// when the firmware repository is available.

#include "../include/ao_modulation.hpp"

namespace DispatcherAO {

// Example stub: computes the modulation output for a single sample.
inline double process_sample(double amplitude, double phi, double x, size_t layerIdx) {
    const auto& cfg = kMasterMatrix[layerIdx];
    double wavenumber = cfg.frequency * 2.0 * 3.14159265358979323846 / 565.0; // k = 2πf / c (c = 565 m/s)
    return evaluate_modulation(amplitude, wavenumber, x, phi, cfg.z_ratio);
}

} // namespace DispatcherAO
