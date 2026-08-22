# Text Distortion — C++ port

Header-only port of the warp from `Text Distortion.dc.html`. Same math, same
constants, same hash — a given (seed, strokes, t) produces the same image as
the JS tool, so you can art-direct in the browser tool and freeze the result
here.

## Files
- `text_distortion.hpp` — the whole engine (`td::Warp`), C++11, no deps.
- `example_main.cpp` — smoke test; writes 8 PPM frames.

## Model
Pure backward remap over a static source bitmap:

    srcPos(x,y,t) = (x,y) + wave(x,t) + tremor(x,y,t) + swirls(x,y,t) + smears(x,y,t)

then one bilinear sample. Animation is fully periodic: **t loops every 8.0**,
so `t = fmod(seconds * speed, 8.0)` gives a seamless endless loop. Blobs and
swirls pulse to zero and relocate (hash-derived, deterministic) each cycle.

## Freezing your logo
1. In the browser tool, tune everything, then note: seed (Rewarp rolls it),
   Tweaks values, and canvas size.
2. Bake the *undistorted* text once. Options:
   - **Recommended:** render the text to PNG at build time and embed it
     (`xxd -i logo.png`, decode with stb_image, or embed raw RGBA). Zero font
     code shipped; layout is pixel-identical forever.
   - Or rasterize at startup with `stb_truetype` if you want runtime text.
   Note the JS layout consumes rng values (per-line rotation ±0.085 rad,
   x-offset ±40px) from the same seeded rng *after* the three phases — if you
   bake the bitmap you don't care.
3. Construct `td::Warp` with your Tweaks values + seed, add the frozen
   strokes/blobs/swirls, call `render()` per frame.

## In a VST (JUCE sketch)
```cpp
// prepare: bake src into juce::Image or raw buffer once
td::Warp warp(params, seed);
warp.addSwirl(140, 0.3f, 0.45f, 1, 2.6f);

// timerCallback / paint:
float t = std::fmod(playheadSeconds * speed, td::kCycle); // or LFO-driven
warp.render(srcRGBA, dstRGBA, W, H, t);   // dst -> juce::Image -> g.drawImage
```
Costs ~W*H*(4 + strokes-in-range) flops per frame, single-threaded; a 450×288
logo animates comfortably on a UI thread at 30 fps. For bigger canvases split
the outer `y` loop across threads (rows are independent) or render at half
resolution and upscale — the effect is soft anyway.

Fun VST idea: drive `t` speed, `waveAmp`, or swirl strength from envelope
followers / LFOs so the logo reacts to the audio.

## Parity notes
- Envelope cutoffs (-0.0111 at 3σ, -0.0022 at 3.5σ) are load-bearing: they
  make each field's influence reach exactly zero at its bounding box (no
  visible slices). Keep them if you change radii.
- `hash2` must stay `fract(sin(a*127.1+b*311.7)*43758.5453)` for relocation
  parity with the tool.
- All strengths are in *pixels of the authored canvas*; if you render at a
  different resolution, scale amp/rad/waveAmp/tremor proportionally.
