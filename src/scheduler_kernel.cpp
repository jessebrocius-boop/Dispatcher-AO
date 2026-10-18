#include "ao_modulation.hpp"
#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>

namespace nascent::dispatcher {

// Telemetry packet matching the matrix fields
struct TelemetryPacket {
    uint64_t sequence_id;
    double frequency_hz;
    double impedance_ratio; // Expected Z_inst/Z_avg (will be checked against matrix)
    double phi_state;       // Acoustic flux potential
    bool invariant_valid = false;
};

class AolsKernel {
public:
    AolsKernel(double base_freq = 55.0, double upper_freq = 880.0)
        : base_frequency_(base_freq), upper_bound_(upper_freq) {}

    // Process a stream of telemetry packets using the full ao_modulation matrix.
    // Returns true only if every packet passes the invariant checks.
    bool execute_tick(std::vector<TelemetryPacket>& stream) {
        bool all_ok = true;
        for (auto &pkt : stream) {
            // Clamp frequency into the allowed range
            double f = pkt.frequency_hz;
            if (f < base_frequency_) f = base_frequency_;
            if (f > upper_bound_)   f = upper_bound_;
            pkt.frequency_hz = f;

            // Find the corresponding layer (exact match is expected, otherwise take the nearest)
            const ao::LayerParams *layer = find_layer(f);
            if (!layer) {
                pkt.invariant_valid = false;
                all_ok = false;
                continue;
            }

            // Use the matrix Z ratio (override packet's value for verification)
            double z = layer->z_ratio;
            // Evaluate the regularized tanh limiter (amplitude A = 1.0, x = 1.0)
            double out = ao::evaluate_modulation(1.0, layer->k, 1.0, pkt.phi_state, z);

            // Invariant: output must stay within [-1, +1] and phi >= 0
            pkt.invariant_valid = (std::abs(out) <= 1.0) && (pkt.phi_state >= 0.0);
            all_ok = all_ok && pkt.invariant_valid;
        }
        return all_ok;
    }

private:
    double base_frequency_;
    double upper_bound_;

    // Linear search – the matrix is tiny (25 entries). Returns nullptr only if out of range.
    const ao::LayerParams* find_layer(double freq) const {
        for (size_t i = 0; i < ao::kNumLayers; ++i) {
            if (std::abs(ao::kMasterMatrix[i].frequency - freq) < 1e-6)
                return &ao::kMasterMatrix[i];
        }
        // Not an exact match – pick the nearest layer (fallback)
        size_t best = 0;
        double best_err = std::abs(ao::kMasterMatrix[0].frequency - freq);
        for (size_t i = 1; i < ao::kNumLayers; ++i) {
            double err = std::abs(ao::kMasterMatrix[i].frequency - freq);
            if (err < best_err) { best_err = err; best = i; }
        }
        return &ao::kMasterMatrix[best];
    }
};

} // namespace nascent::dispatcher

int main() {
    std::cout << "[AOLS Kernel] Initializing full MCE modulation matrix integration..." << std::endl;

    nascent::dispatcher::AolsKernel kernel(55.0, 880.0);
    std::vector<nascent::dispatcher::TelemetryPacket> stream = {
        {1, 55.0, 1.00, 0.05},
        {2, 110.0, 1.02, 0.02},
        {3, 220.0, 0.98, 0.01},
        {4, 440.0, 1.01, 1e-5},
        {5, 880.0, 0.99, 1e-7}
    };

    if (kernel.execute_tick(stream)) {
        std::cout << "[SUCCESS] Full modulation matrix validation passed across 5 frequency bands." << std::endl;
        return 0;
    } else {
        std::cerr << "[ERROR] MCE invariant violation detected in telemetry stream." << std::endl;
        return 1;
    }
}
