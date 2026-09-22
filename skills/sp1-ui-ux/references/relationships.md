# Relationships, Not Parameters

A control doesn't have to mean a bare 0.0 → 1.0 value. It can move through a
meaningful relationship, or represent an axis.

## Two fundamental relationship types

**Functional relationship** — one parameter defines the domain; another
defines behavior *within* that domain.

Example: `PITCH → RANGE`, `RANGE → FLUX`. Moving PITCH doesn't just change
pitch in isolation — it changes what RANGE means, which in turn changes
what FLUX means. The controls are chained, not independent.

**Opposing / complementary relationship** — increasing one meaningfully
gives way to another; the two controls trade off against each other rather
than both climbing independently.

Example: `VELOCITY ↔ SPACE`.

Every secondary control should be classified as one or the other. If it
fits neither, that's a signal the control may actually be an arbitrary
independent parameter in disguise (see below) rather than a real
relationship.

## Avoiding arbitrary independent parameters

Before adding a new control, ask:

> Is this really another independent variable, or is it a property/function
> of something we already have?

This is the parameter-proliferation check. A small physical interface can
express a surprisingly large musical vocabulary specifically *because*
complexity is pushed into relationships between a few controls, rather than
into a growing list of independent ones. When in doubt, look for an
existing control this new idea could become a function *of*, before adding
a new one.

## Complexity belongs in relationships, not navigation

Prefer:

```
one control → another control → emergent behavior
```

over:

```
one control → another control → hidden submenu → algorithm selection
```

If implementing an idea seems to require a submenu, first check whether the
choice being made in that submenu could instead become a relationship
between two existing (or two new, but related) controls.
