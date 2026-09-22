# Effects

Composed, user-facing DSP components live here. Effects may depend on
reusable primitives in `algorithms/`, but should retain their own engineering
record when composition introduces latency, CPU, RAM, numerical, or artifact
behavior.

Do not represent an effect as production-ready because its primitives compile.
Record the effect-level validation and hardware benchmark status separately.

Category stubs:

- [`chorus/`](chorus/)
- [`delay/`](delay/)
- [`distortion/`](distortion/)
- [`dynamics/`](dynamics/)
- [`experimental/`](experimental/)
- [`flanger/`](flanger/)
- [`cloudverb/`](cloudverb/)
- [`tape/`](tape/)
