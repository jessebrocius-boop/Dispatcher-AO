# Section 3 — Dispatcher AO Signal Modulation Specification

> Status: **design specification + software reference model**. No hardware
> validation has been performed yet. Section 3.5 lists the experiments that
> would confirm or reject the claims.

## 3.1 Governing equation

$$out(x,t) = A\tanh\!\left(\frac{k\,x\,(Z_{inst}/Z_{avg})}{\Phi(x,t)^2+\epsilon}\right),\qquad k = 2\pi f_n / c$$

**Proposition (boundedness).** For all real inputs and any ε > 0, |out| ≤ A.
*Proof:* the denominator is ≥ ε > 0, so the argument is finite, and |tanh(u)| < 1 for every finite u. ∎

**Corollary (behavior at nodes).** As Φ → 0, the argument tends to k·x·Z/ε. Because ε ≪ 1,
the output saturates to ±A·sgn(x). In other words, "singularity regulation" works as bounded saturation (soft clipping), and ε sets how sharp the clipping is.

## 3.2 Layer frequencies

$f_n = 55\cdot 2^{n/6}$, n = 0…24, which gives exactly 880 Hz at n = 24. Every third layer lands on an octave of A (55, 110, 220, 440, 880 Hz).

| Phase | Layers | Frequencies (Hz) | Z ratio | Depth |
|---|---|---|---|---|
| 1 | 1–5 | 55.00, 61.74, 69.30, 77.78, 87.31 | 0.85→0.95 | 10→18% |
| 2 | 6–10 | 98.00, 110.00, 123.47, 138.59, 155.56 | 0.95→1.10 | 18→28% |
| 3 | 11–15 | 174.61, 196.00, 220.00, 246.94, 277.18 | 1.10→1.35 | 28→50% |
| 4 | 16–20 | 311.13, 349.23, 392.00, 440.00, 493.88 | 1.35→1.10 | 50→28% |
| 5 | 21–25 | 554.37, 622.25, 698.46, 783.99, 880.00 | 1.10→0.85 | 28→10% |

Z and depth are piecewise-linear across layer index, with knots at layers 1, 6, 11, 16, 21 and 25. Both are continuous.

## 3.3 Physical coupling (hypothesis)

Acoustic streaming in a perfluorinated dielectric layer changes the convective
heat transfer at the GaN/SiC stages. The Stokes layer thickness is δ = √(2ν/ω). With ν ≈ 0.8 mm²/s, δ ≈ 68 µm at 55 Hz and ≈ 17 µm at 880 Hz.

## 3.4 Known limitations

- The acoustic period is 1.1–18 ms, so acoustic effects can't give sub-millisecond
  control. Fast reactive compensation has to come from the GaN gate drive, with
  acoustics acting only as a slower thermal/bias trim.
- Changes in permittivity caused by streaming are expected to be very small. This has to be measured.

## 3.5 Falsification protocol

| Claim | Measurement | Reject if |
|---|---|---|
| Streaming tracks the Z curve | LDV on u_s | correlation outside ±5% |
| No ringing in Phase 3 | VNA / scope | measurable ringing at 174–311 Hz |
| Thermal benefit | IR thermography, T_j | < 25% drop in R_th under load |

## 3.6 Software reference test validation

The C++ reference-model test in `tests/test_modulation.cpp` compiled and ran
successfully with MSVC x64 using C++17, `/O2`, `/openmp:experimental`, and
`/EHsc`. The run passed its assertions for selected geometric-frequency
keypoints (including the 880 Hz terminal lock), adjacent impedance-ratio
continuity, and the modulation output bound for tested values of Φ including
zero. The console output is recorded in `test_build.log`.

This is a host-side software test only. It does not validate hardware behavior,
integrate the header into a firmware scheduler, or establish zero-drift
frequency scaling across every layer. Those claims require separate
implementation and validation.
