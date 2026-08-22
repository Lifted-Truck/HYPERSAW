// text_distortion.hpp — single-header port of the Text Distortion warp.
// Line-for-line port of the JS reference (Text Distortion.dc.html).
// No dependencies. C++11. RGBA8 in, RGBA8 out.
//
// Usage:
//   td::Params p;                       // defaults = the tool's defaults
//   td::Warp warp(p, /*seed*/ 7);
//   warp.addBlob(...); warp.addSwirl(...);   // optional animated elements
//   warp.render(src, dst, W, H, t);     // t in seconds*speed; loops every 8.0
//
// `src` is the undistorted text bitmap (bake it: see README). The warp is a
// pure backward remap — every output pixel samples the source bilinearly, so
// it is trivially parallelizable (split the y loop across threads) and has no
// allocations after construction.

#pragma once
#include <cstdint>
#include <cmath>
#include <vector>

namespace td {

static const float kTau   = 6.28318530718f;
static const float kCycle = 8.0f;          // seamless loop period in t-units
static const float kW1    = kTau / kCycle; // base angular rate (0.785398...)

struct Params {
    float waveAmp     = 26.0f;  // px
    float waveFreq    = 1.4f;   // cycles across width
    float tremor      = 3.5f;   // px
    float smearAngle  = 0.0f;   // degrees
    float angleJitter = 12.0f;  // degrees
    float waveJitter  = 0.5f;   // 0..1
};

// A drawn/frozen smear stroke (blob=true makes it pulse + relocate per cycle).
struct Stroke {
    float x0, y0;      // origin px (ignored while blob relocates)
    float amp;         // signed px; sign = pull direction
    float rad;         // gaussian half-width px
    float r1, r2;      // 0..1, per-stroke angle / wavelength jitter rolls
    bool  blob = false;
    int   cyc  = 1;    // pulses per 8-unit loop (1 or 2)
    float bph  = 0.0f; // phase offset 0..tau
};

// A pulsing grow/shrink + rotation field ("Swirl" button).
struct Field {
    float rad;         // gaussian radius px
    float sc;          // local scale strength (~0.15..0.55, signed)
    float rot;         // local rotation strength (~0.2..0.7, signed)
    int   cyc  = 1;
    float bph  = 0.0f;
};

// fract(sin()) hash — must match JS for identical relocation sequences.
inline float hash2(float a, float b) {
    float x = std::sin(a * 127.1f + b * 311.7f) * 43758.5453f;
    return x - std::floor(x);
}

class Warp {
public:
    Params params;

    explicit Warp(const Params& p = Params(), uint32_t seed = 7) : params(p) {
        // JS rng: a = (a*9301+49297) % 233280, value a/233280
        uint32_t a = (seed * 9301u + 49297u) % 233280u;
        auto next = [&a]() { a = (a * 9301u + 49297u) % 233280u; return a / 233280.0f; };
        ph1 = next() * kTau; ph2 = next() * kTau; ph3 = next() * kTau;
    }

    void addStroke(const Stroke& s) { strokes.push_back(s); }
    void addBlob(float amp, float rad, float r1, float r2, int cyc, float bph) {
        Stroke s; s.x0 = 0; s.y0 = 0; s.amp = amp; s.rad = rad;
        s.r1 = r1; s.r2 = r2; s.blob = true; s.cyc = cyc; s.bph = bph;
        strokes.push_back(s);
    }
    void addSwirl(float rad, float sc, float rot, int cyc, float bph) {
        fields.push_back({rad, sc, rot, cyc, bph});
    }

    // src/dst: RGBA8, W*H*4 bytes. t: animation clock (wraps at 8.0 for a loop).
    void render(const uint8_t* src, uint8_t* dst, int W, int H, float t) const {
        const Params& P = params;

        // -- per-frame precompute ------------------------------------------
        const float d1 = ph1 + t * kW1 + 1.7f * std::sin(t * kW1 * 2 + 0.5f);
        const float d2 = ph2 - t * kW1 + 1.3f * std::sin(t * kW1 * 3 + 2.0f);
        const float aMod = 1.0f + 0.35f * std::sin(t * kW1 * 2 + 1.0f) * std::sin(t * kW1);

        std::vector<float> wave((size_t)W);
        for (int x = 0; x < W; x++) {
            float fx = (float)x / W;
            wave[x] = P.waveAmp * aMod * std::sin(fx * P.waveFreq * kTau + d1)
                    + P.waveAmp * 0.35f * std::sin(fx * P.waveFreq * 5.1f + d2);
        }

        struct SActive { float amp, x0, y0, tanT, cosT, sinT, rad, wf; };
        std::vector<SActive> sts;
        for (const Stroke& s : strokes) {
            float amp = s.amp, x0 = s.x0, y0 = s.y0;
            if (s.blob) {
                float ph = t * kW1 * 4 * s.cyc + s.bph;
                float env = 0.5f * (1.0f - std::cos(ph));
                amp = s.amp * std::pow(env, 1.6f);
                float k = std::floor(ph / kTau);   // cycle index -> new position
                x0 = W * (0.15f + hash2(s.bph, k) * 0.7f);
                y0 = H * (0.25f + hash2(s.bph + 9.7f, k) * 0.5f);
            }
            if (std::fabs(amp) <= 2.0f) continue;
            float th = (P.smearAngle + (s.r1 * 2 - 1) * P.angleJitter) * kTau / 360.0f;
            SActive q;
            q.amp = amp; q.x0 = x0; q.y0 = y0;
            q.tanT = std::tan(th); q.cosT = std::cos(th); q.sinT = std::sin(th);
            q.rad = s.rad + std::fabs(amp) * 0.25f;
            q.wf  = 0.11f * std::pow(2.5f, (s.r2 * 2 - 1) * P.waveJitter);
            sts.push_back(q);
        }

        struct FActive { float x0, y0, rad, sc, rot; };
        std::vector<FActive> flds;
        for (const Field& f : fields) {
            float ph = t * kW1 * 4 * f.cyc + f.bph;
            float e = std::pow(0.5f * (1.0f - std::cos(ph)), 1.6f);
            if (std::fabs(f.sc * e) <= 0.005f && std::fabs(f.rot * e) <= 0.005f) continue;
            float k = std::floor(ph / kTau);
            flds.push_back({ W * (0.15f + hash2(f.bph + 3.1f, k) * 0.7f),
                             H * (0.25f + hash2(f.bph + 13.9f, k) * 0.5f),
                             f.rad, f.sc * e, f.rot * e });
        }

        // -- per-pixel backward remap --------------------------------------
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                float sy = y + wave[x]
                    + P.tremor * std::sin(y * 0.055f + x * 0.01f + ph3
                                          + t * kW1 * 2 + std::sin(t * kW1) * 2);
                float sx = (float)x;

                for (const FActive& f : flds) {
                    float dx = x - f.x0, dy = y - f.y0;
                    if (dx > f.rad * 3 || dx < -f.rad * 3 ||
                        dy > f.rad * 3 || dy < -f.rad * 3) continue;
                    // -0.0111 = value at the 3-sigma cutoff, so env hits exactly 0
                    float env = std::exp(-(dx * dx + dy * dy) / (2 * f.rad * f.rad)) - 0.0111f;
                    if (env <= 0) continue;
                    sx -= env * (f.sc * dx - f.rot * dy);
                    sy -= env * (f.sc * dy + f.rot * dx);
                }

                for (const SActive& q : sts) {
                    float dxr = x - (q.x0 + (y - q.y0) * q.tanT);
                    if (dxr > q.rad * 3.5f || dxr < -q.rad * 3.5f) continue;
                    float env = std::exp(-(dxr * dxr) / (2 * q.rad * q.rad)) - 0.0022f;
                    if (env <= 0) continue;
                    float ad = std::fabs(q.amp), sg = q.amp > 0 ? 1.0f : -1.0f;
                    float dl = (y - q.y0) * sg;
                    if (dl > -40.0f) {
                        float rise = 1.0f - std::exp(-std::fmax(0.0f, dl) / (ad * 0.5f + 10));
                        float over = std::fmax(0.0f, dl - ad * 1.6f);
                        float fall = std::exp(-(over * over) / (2 * (ad + 60) * (ad + 60)));
                        float shift = q.amp * env * rise * fall
                            + sg * env * std::fmin(ad * 0.45f, 70.0f)
                                * std::sin(dl * q.wf) * rise * fall;
                        sy -= shift * q.cosT;
                        sx -= shift * q.sinT;
                    }
                }

                // clamp + bilinear sample
                if (sy < 0) sy = 0; if (sy > H - 1.01f) sy = H - 1.01f;
                if (sx < 0) sx = 0; if (sx > W - 1.01f) sx = W - 1.01f;
                int x0i = (int)sx, y0i = (int)sy;
                float fx = sx - x0i, fy = sy - y0i;
                size_t i00 = ((size_t)y0i * W + x0i) * 4;
                size_t i10 = i00 + 4, i01 = i00 + (size_t)W * 4, i11 = i01 + 4;
                size_t di  = ((size_t)y * W + x) * 4;
                for (int ch = 0; ch < 3; ch++) {
                    float top = src[i00 + ch] + (src[i10 + ch] - src[i00 + ch]) * fx;
                    float bot = src[i01 + ch] + (src[i11 + ch] - src[i01 + ch]) * fx;
                    dst[di + ch] = (uint8_t)(top + (bot - top) * fy + 0.5f);
                }
                dst[di + 3] = 255;
            }
        }
    }

private:
    float ph1, ph2, ph3;
    std::vector<Stroke> strokes;
    std::vector<Field>  fields;
};

} // namespace td
