# Generated project test guide

Use `make -C tests/unit run` for deterministic functional tests without Zephyr
or an SP-1 board. Compile production logic with `SP1_HOST_TEST` and replace
hardware reads through narrow seams.

Every new plugin should cover reset, silence, normal input, boundary values,
invalid inputs, buffer limits, and repeated processing. Keep DSP pure enough
for the root host suite and keep device assumptions documented in
`docs/knowledge-sources.yml`.

When a test reveals a reusable hardware or DSP fact, update this guide and
open a focused PR to the central knowledgebase and, when broadly useful, the
template repository.
