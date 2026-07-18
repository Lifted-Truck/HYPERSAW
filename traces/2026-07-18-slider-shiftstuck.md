# 2026-07-18 — Sliders crawl: shiftHeld stuck true in Live

Report: "dragging the sliders is very slow — drag all the way across the
screen to move halfway." Diagnosis: the fine-drag (20x, 0.05 multiplier)
gated on a globally-tracked `shiftHeld`. In Live the webview receives the
shift KEYDOWN but not always the KEYUP (same host-focus gap as text
entry), so shiftHeld sticks true and EVERY drag runs the fine path — a
full-screen drag = ~half the range (math: (screenWidth/trackWidth)*0.05
~ 0.6*span). Fix: read e.shiftKey off the pointerdown event (always
exact at event time); deleted the shiftHeld tracking entirely (its only
consumer). Headless check: normal drag leaves value to the native slider
(JS writes nothing); shift-drag moves 0.006 per 20px as designed.
