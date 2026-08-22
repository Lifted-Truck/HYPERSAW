# lab-index-writer — gen_lab_index.py now writes the index it documents

- **Queue item:** unqueued: bug found independently by two agent lanes
  (glitch-lab and modulator-lab reports, 2026-08-22) — docs/design/index.html
  never regenerates when a lab lands.
- **Why:** `tools/gen_lab_index.py` main() only counted labs and printed —
  the docstring and the index's own "Regenerate with python3
  tools/gen_lab_index.py" instruction both promised a writer that did not
  exist. Git history shows the tool and the index were committed together in
  509d538 and the script never had a write step; the committed index was
  produced by other means in that session. Added the render + write to
  main(), matching the committed index byte-for-byte on the unchanged cards
  (verified by diff: the only changes were the two new cards and one stale
  one). Regenerated: glitch-lab and modulator-lab now listed;
  quantum-morph-lab card dropped — that file was never tracked in git
  (`git log --all -- docs/design/quantum-morph-lab.html` is empty), so the
  committed index has always carried a link broken for every clone.
- **Evidence consulted:** tools/gen_lab_index.py, docs/design/index.html,
  commit 509d538 (message + stat), `git log --follow` on both files,
  docs/design/ directory listing, dev-server fetch of all 22 hrefs (0 broken),
  rendered page (20 cards, both new labs present).
- **Alternatives rejected:** templating the static shell from a separate file
  (a second file for one consumer; embedded string keeps the tool
  single-file like the labs it indexes); re-adding a quantum-morph card
  (the lab file does not exist in the repo — the index must describe the
  tree, not one machine's working copy).
- **Verify:** fast, exit 0, git 86c1274 (.harness/last-verify.json
  2026-08-22T21:20:23Z). Idempotency: two consecutive runs produce identical
  md5 (0274b086f17b9aafe1fd532128e9539e).
- **Open questions:** nothing regenerates the index automatically when a lab
  lands — the tool must still be run by hand (or wired into verify/a hook,
  which touches `./verify` and is human-gated, so deferred).
