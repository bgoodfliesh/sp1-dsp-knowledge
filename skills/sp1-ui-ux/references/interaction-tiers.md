# Interaction Tiers

## The three tiers

- **Tier 1** — immediate, playable without any mode knowledge. What the
  player touches constantly.
- **Tier 2** — a related secondary dimension, reached via FN.
- **Tier 3** — deeper / configuration behavior, reached however the
  instrument surfaces deep settings (setup mode, page-hold, etc.).

Frequently touched controls should migrate toward Tier 1 over time. If
players (or you, testing the instrument) keep reaching for something on FN,
that's a signal it may belong one tier up.

## FN reveals relationships, not hidden menus

FN should expose *the other side of an existing idea* — not become "here
are eight arbitrary extra parameters that didn't fit anywhere else."

**The one-sentence test:** if you can't explain the relationship between a
primary control and its FN pairing in one sentence, the pairing is
suspect. Rework it or don't ship it as a pairing.

Good pairing shape: "FN on this control reveals the [complementary /
functional] side of the same idea — where the primary controls X, FN
controls the Y that X's relationship depends on."

Bad pairing shape: "FN on this control does something unrelated that we
needed to put somewhere."

## Using this file

When assigning a control to FN, write down the one-sentence relationship
explicitly before finalizing it. If you can't write it, the control likely
belongs either at Tier 1 (if it's actually used often) or Tier 3 (if it's
genuinely configuration, not performance).
