# REFLECTIONS — dated, prunable at /wakeup

- [2026-08-28] Stacked-PR merge-order trap: #493 (base: increment-4 branch) was
  merged AFTER #492 had already taken that base into main — GitHub does not
  retarget an open PR when its base's PR merges unless the base BRANCH is
  deleted, so the merge landed on a corpse and increment 5 was absent from main
  until rescue PR #494. Merge stacks bottom-up, or delete merged base branches
  immediately. (Now L0044.)
- [2026-08-28] Route persistence is the loudest open gap: right-click and macro
  routes vanish on session reload (param-161's route survives). B72's
  deterministic link IDs — key (source slot, dest id) — are the natural
  serialization identity; build persistence ON that key, not before it.
- [2026-08-28] Three FOUNDATIONS responses to our briefs sit answered and
  unread in their tree: round1-rulings, seam-round2, stage3-doorframes (F2
  extraction scope). Pulled, deliberately not processed mid-build. Read before
  building anything that touches the extraction seams.
