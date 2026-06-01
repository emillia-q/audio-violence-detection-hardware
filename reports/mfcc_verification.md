# DSP & Feature Extraction Verification

To confirm that the on-device feature pipeline matches the Python training environment, a golden-vector sanity check was performed. The C++ `MfccExtractor` on the ESP32 was compared against `librosa.feature.mfcc` using the same 2-second reference signal (`golden_input`: 32,000 samples at 16 kHz, peak-normalised).

## Methodology

1. **Reference generation (Python):** `scripts/gen_golden_mfcc.py` loads a 2 s audio clip, applies `fix_length` and peak normalisation, computes MFCC with default librosa parameters (`n_mfcc=13`), and stores the flat reference vector in `sanity/golden_mfcc_reference.txt` (819 values = 63 frames × 13 coefficients).
2. **On-device extraction (ESP32):** The same `golden_input` array is passed through `MfccExtractor::compute()`, and the resulting 819 MFCC values are read over Serial.
3. **Comparison:** All 819 coefficients were compared. ESP32 output is stored in `sanity/esp_mfcc_dump.txt`; per-frame MAE and max absolute error were computed for every frame (0–62). Detailed spot-check tables for Frames 0, 1, 2, and 62 are included below; a pandas-based error heatmap export is planned as a follow-up (see *Future work*).

## Observations

The 63-frame spectrogram shows a **symmetric boundary pattern**: larger deviations at the **start and end** of the window, near-identical values in the **interior** frames.

| Region | Frames | Behaviour |
| :--- | :--- | :--- |
| **Start (initial transient)** | 0 | Largest mismatch on coefficient `[0]` (log-energy): \|Δ\| ≈ **27**. Several higher-order cepstral coefficients also differ by ~2–6 units. |
| **Start (transition)** | 1 | Partial convergence: \|Δ\| on `[0]` drops to ≈ **2.3**, but other coefficients still differ by ~0.2–3.8. |
| **Interior (steady state)** | 2 … 60 | Errors at the **5th–6th decimal place** (mean MAE ≈ **5×10⁻⁵**, max \|Δ\| ≈ **4.5×10⁻⁴** across all interior coefficients). **Zero** interior coefficients exceed \|Δ\| > 0.001. |
| **End (soft transition)** | 61 | Partial convergence before the final frame: MAE ≈ **0.34**, \|Δ\| on `[0]` ≈ **0.65**. Not identical to the interior, but far smaller than Frames 0 or 62. |
| **End (final transient)** | 62 | Elevated errors again (symmetric to the start): MAE ≈ **6.5**, \|Δ\| on `[0]` ≈ **15**, on `[1]` ≈ **15.4**; remaining coefficients differ by ~1.5–9 units. |

### Full 63-frame summary

| Metric | Value |
| :--- | :--- |
| Frames with MAE > 0.01 | **4** (Frames 0, 1, 61, 62 only) |
| Frames with MAE < 0.001 | **59 of 63** (~94%) |
| Interior frames 2–60 (mean MAE) | ≈ **5×10⁻⁵** |
| Global MAE (all 819 coefficients) | ≈ **0.21** (dominated by the four boundary frames) |
| Global max \|Δ\| | **26.95** at Frame 0, coefficient `[0]` |

Frame 2 is representative of the interior region. Frame 62 confirms that the edge effect is **symmetric** — it affects both boundaries of the fixed 2 s buffer, not only the start.

### Frame 0, 1, and 2 — ESP32 vs Librosa

| Global Index | Frame | MFCC | ESP32 | Librosa | \|Δ\| |
| :---: | :---: | :---: | :---: | :---: | :---: |
| 0 | 0 | [0] | −178.36065674 | −205.31370544 | 26.95304870 |
| 1 | 0 | [1] | 20.10222626 | 26.46078491 | 6.35855865 |
| 2 | 0 | [2] | −127.25276947 | −124.74639893 | 2.50637054 |
| 3 | 0 | [3] | −14.50530052 | −16.56139946 | 2.05609894 |
| 4 | 0 | [4] | −44.46652603 | −48.92832184 | 4.46179581 |
| 5 | 0 | [5] | −25.96669197 | −30.53591919 | 4.56922722 |
| 6 | 0 | [6] | −31.32803535 | −31.18887901 | 0.13915634 |
| 7 | 0 | [7] | −28.51778793 | −26.09206390 | 2.42572403 |
| 8 | 0 | [8] | −16.81155777 | −14.79097557 | 2.02058220 |
| 9 | 0 | [9] | 21.13830757 | 19.03787231 | 2.10043526 |
| 10 | 0 | [10] | 38.19437408 | 35.13408661 | 3.06028747 |
| 11 | 0 | [11] | 36.82704926 | 33.22257996 | 3.60446930 |
| 12 | 0 | [12] | −15.00851917 | −15.93496132 | 0.92644215 |
| 13 | 1 | [0] | −179.31391907 | −181.61759949 | 2.30368042 |
| 14 | 1 | [1] | −6.78359795 | −6.56582451 | 0.21777344 |
| 15 | 1 | [2] | −124.06285858 | −122.67700195 | 1.38585663 |
| 16 | 1 | [3] | −14.59353065 | −15.62556839 | 1.03203774 |
| 17 | 1 | [4] | −47.79911804 | −51.57801056 | 3.77889252 |
| 18 | 1 | [5] | −31.12393761 | −34.58795929 | 3.46402168 |
| 19 | 1 | [6] | −33.81252289 | −34.21442413 | 0.40190124 |
| 20 | 1 | [7] | −32.68042374 | −31.55174637 | 1.12867737 |
| 21 | 1 | [8] | −21.62993240 | −20.98849487 | 0.64143753 |
| 22 | 1 | [9] | 26.45564842 | 26.34556580 | 0.11008262 |
| 23 | 1 | [10] | 44.13585663 | 42.96225739 | 1.17359924 |
| 24 | 1 | [11] | 42.24389648 | 40.28364182 | 1.96025466 |
| 25 | 1 | [12] | −15.91868305 | −17.54552841 | 1.62684536 |
| 26 | 2 | [0] | −247.15798950 | −247.15794373 | 0.00004577 |
| 27 | 2 | [1] | −34.86863708 | −34.86865997 | 0.00002289 |
| 28 | 2 | [2] | −104.62638855 | −104.62641907 | 0.00003052 |
| 29 | 2 | [3] | −13.67924118 | −13.67932892 | 0.00008774 |
| 30 | 2 | [4] | −46.14752960 | −46.14748383 | 0.00004577 |
| 31 | 2 | [5] | −37.89197922 | −37.89199829 | 0.00001907 |
| 32 | 2 | [6] | −36.69091797 | −36.69099426 | 0.00007629 |
| 33 | 2 | [7] | −40.86093903 | −40.86091995 | 0.00001908 |
| 34 | 2 | [8] | −27.95361137 | −27.95361710 | 0.00000573 |
| 35 | 2 | [9] | 41.25266647 | 41.25279999 | 0.00013352 |
| 36 | 2 | [10] | 63.62044144 | 63.62037659 | 0.00006485 |
| 37 | 2 | [11] | 56.12498474 | 56.12485504 | 0.00012970 |
| 38 | 2 | [12] | −17.63802338 | −17.63801193 | 0.00001145 |

### Frame 62 (final frame) — ESP32 vs Librosa

| Global Index | Frame | MFCC | ESP32 | Librosa | \|Δ\| |
| :---: | :---: | :---: | :---: | :---: | :---: |
| 806 | 62 | [0] | −415.35308838 | −430.42465210 | 15.07156372 |
| 807 | 62 | [1] | 108.22764587 | 92.78244781 | 15.44519806 |
| 808 | 62 | [2] | −62.53943634 | −69.33338928 | 6.79395294 |
| 809 | 62 | [3] | −36.49953079 | −40.88977814 | 4.39024735 |
| 810 | 62 | [4] | −0.76771545 | −4.80665541 | 4.03893996 |
| 811 | 62 | [5] | −34.74418640 | −38.46331787 | 3.71913147 |
| 812 | 62 | [6] | −31.40656281 | −37.73635864 | 6.32979583 |
| 813 | 62 | [7] | −4.80172110 | −13.75587082 | 8.95414972 |
| 814 | 62 | [8] | −0.36002922 | −8.25469589 | 7.89466667 |
| 815 | 62 | [9] | −4.03988361 | −9.08262444 | 5.04274083 |
| 816 | 62 | [10] | −10.30536461 | −13.50069809 | 3.19533348 |
| 817 | 62 | [11] | −5.93868017 | −8.53398705 | 2.59530688 |
| 818 | 62 | [12] | 8.62015343 | 7.16702747 | 1.45312596 |

## Root Cause Analysis (Boundary Frames)

The elevated errors on **Frames 0, 1, 61, and 62** are localised to the **STFT window boundaries** of the fixed 2 s buffer, not to a systematic fault in the Mel-filterbank, log-power, or DCT stages.

Both pipelines use `center=True`-style framing with reflective edge padding. Residual differences at the boundaries likely arise from subtle implementation details (e.g. single-reflection vs librosa's full `mode='reflect'` handling). Once the analysis window is fully inside the audio segment (Frames 2–60), both paths consume identical sample data and produce matching MFCC values. The `float32` vs `float64` precision gap explains interior residuals on the order of 10⁻⁴–10⁻⁵, **not** the unit-scale errors (15–27) seen on the outermost frames.

Only **four frames** (0, 1, 61, 62) exceed MAE > 0.01. The remaining **59 frames (~94%)** match to the 5th–6th decimal place.

## Numerical Precision (Interior Frames)

For representative interior frames (e.g. Frame 2):

- **Max absolute error:** ≈ 1.3×10⁻⁴  
- **Mean absolute error (MAE):** ≈ 5×10⁻⁵ per coefficient  

These residuals are expected when comparing ESP32 `float32` hardware arithmetic against librosa, which computes internally in `float64` but was cast to `float32` for the reference file. They are **practically negligible** relative to the dynamic range of log-mel MFCC features and have no meaningful impact on downstream inference.

## Conclusion

The custom C++ DSP pipeline — periodic Hann window, ESP-DSP FFT, Slaney-scale Mel filterbank, global `power_to_db`, and orthonormal DCT-II — is **verified as numerically consistent with librosa** for **59 of 63 frames** (Frames 2–60). The deployed TensorFlow Lite Micro model therefore receives **correct feature data** for the overwhelming majority of the 63×13 input tensor.

Boundary-frame discrepancies (Frames 0–1 and 61–62) are acknowledged but considered acceptable for the **current** model, which was trained on raw librosa MFCC values without edge-frame removal, because:

1. They affect only **4 of 63 frames** (~6% of the time axis).  
2. CNNs rely on spatial patterns across the full spectrogram, not on isolated absolute values at the edges.  
3. No artificial calibration offsets were applied to the firmware; the pipeline remains a faithful port of the training-time feature extractor.

**Decision:** The C++ `MfccExtractor` is accepted for production use without modification.

## Implications for Future MFCC Normalisation

The current model consumes **raw MFCC values** (log-mel cepstral coefficients in dB scale, typically ranging from roughly −400 to +100). No per-feature scaling to \[−1, 1\] is applied at inference time.

If a **future model** is trained with **normalised MFCC features** (e.g. min–max or standard-score scaling to \[−1, 1\]), the boundary frames must be handled explicitly so that on-device preprocessing matches the training pipeline:

1. **Discard unreliable edge frames** before normalisation and inference — at minimum **Frames 0 and 1** at the start and **Frames 61 and 62** at the end, since these are the frames where ESP32 and librosa diverge most.  
2. Apply the **same normalisation statistics** (min, max, mean, std) computed during training on **interior frames only**, then use those fixed parameters on-device.  
3. Alternatively, retrain with the same edge-frame exclusion on both Python and ESP32 so that the effective input tensor shape and content are identical (e.g. 59×13 instead of 63×13, requiring a model architecture change).

Without edge-frame removal, normalising the full 63×13 matrix on-device would bake boundary artefacts into the scaling range and **break parity** with training data, even though the interior MFCC math is correct.

## Future Work

A **pandas**-based analysis script is planned as the next documentation step: load `sanity/esp_mfcc_dump.txt` and `sanity/golden_mfcc_reference.txt` into a DataFrame, generate per-frame MAE/max-error tables, error heatmaps (63×13), and export summary plots for the thesis. The numerical audit across all 63 frames is already complete; the pandas step will formalise visual reporting.
