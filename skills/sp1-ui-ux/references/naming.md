# Naming

## Describe experience, not implementation

The name on the control should describe what the player experiences, not
what the DSP is doing underneath.

- `AGITATOR` is better than "OSC2 FM amount."
- `FLUX` is potentially better than "LFO depth."
- `MELD` is potentially better than "cross-modulation mode."

The implementation underneath a control can change completely without
breaking the player's mental model, as long as the name still describes the
*experience* the control produces.

## Nouns vs. relationship words

- **Nouns** describe things or properties: `BODY`, `SHAPE`, `PITCH`.
- **Relationship / operation words** describe what happens *between*
  things: `FLUX`, `MELD`.

This distinction matters when naming a new control: decide first whether
the control is naming a *thing* (use a noun) or an *action between two
things* (use a relationship word). A relationship-type control (see
`relationships.md`) usually wants a relationship-word name; a control that
sets a domain or property usually wants a noun.

## Using this file

When naming a new control:
1. Say out loud what the player will *feel* happen when they move it.
2. Check whether that's a property of the sound (→ noun) or an interaction
   between two existing controls (→ relationship word).
3. Reject any name that only makes sense with reference to the DSP
   implementation (e.g. anything with "LFO," "FM," "mode," "algorithm" in
   the player-facing label).
