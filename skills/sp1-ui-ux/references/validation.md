# Validation Checklist

Run a candidate (or finished) control layout against this checklist before
calling the design done. Each item traces back to a constitution point in
`SKILL.md`.

1. **Physical roles** — Does every column/position mean the same *kind* of
   thing on every page it appears on? (→ `control-grammar.md`,
   `modes-and-pages.md`)
2. **Relationships classified** — Is every secondary control classified as
   functional or opposing? If neither, is it actually just an independent
   parameter that snuck in? (→ `relationships.md`)
3. **FN one-sentence test** — For every FN pairing, can you state the
   relationship between primary and FN control in one sentence?
   (→ `interaction-tiers.md`)
4. **Tier placement** — Are frequently-touched controls actually at Tier 1?
   Is anything on Tier 1 that's rarely touched and could move down?
   (→ `interaction-tiers.md`)
5. **Naming** — Does every control name describe the player's experience
   rather than the DSP implementation? Is it a noun or relationship-word as
   appropriate? (→ `naming.md`)
6. **Topology hidden** — Does any control expose signal routing rather than
   a musically meaningful behavior? (→ `control-grammar.md`)
7. **Complexity placement** — Wherever the design got complex, did that
   complexity land in a relationship between controls, or did it leak into
   a submenu/mode-select? (→ `relationships.md`)
8. **Playability blind** — Could a player who knows the grammar but not
   this specific instrument make a reasonable guess at each control's role
   without looking at a label? (→ `physical-layout.md`)
9. **Intentional vs. accidental** — For anything that feels "weird," is the
   weirdness a deliberate expressive choice, or accidental friction that
   should be smoothed out?

A layout that fails several of these isn't automatically wrong — but every
failure should be a conscious exception, not an oversight. State the reason
when overriding a check.
