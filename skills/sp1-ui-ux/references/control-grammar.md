# Control Grammar

## Persistent semantic roles

The same physical position should mean roughly the same *kind* of thing
across every page of an instrument. This is most important for the four
columns, which currently carry this grammar:

- **WHERE** — domain / location within a space of possibilities
- **WHAT** — the object or material being acted on
- **WHEN** — timing / triggering behavior
- **WHAT-changes-into** ... **HOW** — behavior / manner of the action

(Restate the current four-column grammar here as it's confirmed per
instrument generation — this file should track the grammar's current
definition, not any one instrument's use of it.)

Changing pages changes *what is being controlled*, not the underlying
physical language. A player who has internalized "the third column is
always WHEN" should be able to guess roughly what a new page's third column
does before touching it.

## Don't expose topology unless topology is musically meaningful

The player manipulates the sound's *behavior*, not a signal-routing graph.
Internal routing (how a parameter actually reaches the DSP) can be as
complicated as it needs to be; the external control model stays simple.

Ask before adding a control: does this expose a *decision the player is
making about the music*, or does it expose *how the code happens to be
wired*? Only the former belongs on the surface.

## Using this file

When starting a new instrument, restate the current WHERE/WHAT/WHEN/HOW
(or whatever the grammar has evolved to) at the top of this file first,
then check every proposed control against "does this preserve the physical
role of its position, or does it quietly redefine it?"
