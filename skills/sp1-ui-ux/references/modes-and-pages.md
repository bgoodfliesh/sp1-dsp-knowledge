# Modes & Pages

## What's allowed to change between pages

Changing pages changes *what is being controlled* — the musical meaning
behind a control. It must never change the *fundamental physical language*
of the interface (see `control-grammar.md`).

Concretely: if column three is WHEN on page 1, it should still be
recognizably a timing/triggering role on page 2, even though the specific
parameter it drives is different. A page redefining a column's role from
scratch is a violation of the grammar, not a new use of it.

## Learnability comes from consistency, not simplicity

The interface doesn't need to expose *fewer* concepts to be learnable — it
needs to expose them through a *stable grammar*. A genuinely complex
instrument can still be learnable if the same physical relationships recur
across its pages, because the player is learning the grammar once and
reapplying it, rather than learning each page from scratch.

## Using this file

When adding a new page to an instrument, write down what each column means
on the new page and check it against what that column has meant on every
other page. If the *role* (not the specific parameter) has drifted,
reconsider before shipping the page.
