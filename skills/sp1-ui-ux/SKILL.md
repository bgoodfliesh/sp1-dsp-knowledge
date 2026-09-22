---
name: sp1-ui-ux
description: Relational Control Grammar for designing SP-1 instrument interfaces — how to derive a coherent physical control vocabulary (WHERE/WHAT/WHEN/HOW, functional vs. opposing relationships, FN tiers, experiential naming) for a new SP-1 instrument, rather than copying an existing instrument's controls. Use whenever designing, reviewing, or naming controls for a new SP-1 instrument, evaluating whether a proposed control layout is coherent, deciding what goes on FN, or naming a parameter. Not for DSP implementation or firmware architecture; see sp1-dsp-engineering and sp1-firmware for those.
---

# SP-1 UI/UX: Relational Control Grammar

This skill teaches how to *derive* a coherent control vocabulary for a new
SP-1 instrument — not how to copy the controls of an instrument that already
exists. Existing instruments (their PITCH/VELOCITY/SPACE/MOD,
BODY/SHAPE/AGITATOR/MIX, etc.) are *instances* of this grammar, not the
grammar itself. Treat them as worked examples, never as the template to
paste into a new design.

The central idea: an SP-1 instrument should behave less like a device with
N independent knobs and more like a small instrument-design language, where
a stable physical grammar carries different musical meaning per page.

## The constitution

When designing (or reviewing) an SP-1 instrument interface:

1. Preserve stable physical semantics — same position, same *kind* of role, across pages.
2. Think in relationships before parameters.
3. Classify each secondary control as functional or opposing (never neither).
4. Use FN to expose the other side of an existing relationship, not a menu of extras.
5. Prefer experiential vocabulary over implementation vocabulary.
6. Put frequently performed actions in the primary (Tier 1) interaction layer.
7. Put complexity into relationships rather than navigation/submenus.
8. Treat tactile learnability as a functional requirement, not a nicety.
9. Avoid arbitrary independent parameters — ask whether it's really a new
   variable or a property/function of one you already have.
10. Preserve intentional weirdness while eliminating accidental friction.

If a proposed control layout violates one of these without a stated reason,
treat that as a design smell worth surfacing before implementation.

## How to use this skill

For a **new instrument design**, work through the references roughly in
this order:

1. `references/control-grammar.md` — the WHERE/WHAT/WHEN/HOW physical
   grammar and why topology should stay hidden.
2. `references/relationships.md` — functional vs. opposing relationships;
   how to avoid inventing an independent parameter by accident.
3. `references/interaction-tiers.md` — what belongs on the primary
   surface vs. FN vs. deeper config, and how FN pairings are validated.
4. `references/naming.md` — turning an internal DSP concept into an
   experiential control name.
5. `references/physical-layout.md` — position, grouping, and muscle memory.
6. `references/modes-and-pages.md` — what's allowed to change between pages
   and what must not.
7. `references/validation.md` — a checklist to run a finished (or
   candidate) control layout against before calling it done.
8. `references/feedback.md` — status: not yet established. Read it before
   assuming an answer here; don't invent one.

For a **review** of an existing proposal, skip straight to
`references/validation.md` and work backward into whichever reference file
explains a failing check.

## What this skill deliberately does not cover

- The concrete control set of any specific instrument (that's instrument
  design, downstream of this grammar — capture it in the instrument's own
  doc, not here).
- DSP implementation. See `sp1-dsp-engineering`.
- Firmware and hardware implementation. See `sp1-firmware`.
- Visual/LED feedback language — see `references/feedback.md` for why this
  is intentionally left open.

## Keeping this skill in sync

This grammar is expected to evolve as new instruments are designed against
it. When a session discovers a new principle, a needed exception, or a
sharper example, propose an update to the relevant reference file (or this
file, for constitution-level changes) rather than letting the insight stay
scattered in chat — that's exactly the failure mode this skill was
extracted to prevent.
