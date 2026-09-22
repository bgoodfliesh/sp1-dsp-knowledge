cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project({{ project_slug }})

target_sources(app PRIVATE
  src/main.c
  src/core/audio.c
  src/core/controls.c
  src/core/system.c
  src/core/leds.c
  src/core/midi.c
  src/core/power.c
)

# Example / selected modules
{% if include_example_gain %}
target_sources(app PRIVATE src/modules/example_gain.c)
{% endif %}
{% if "Reverb (Clouds-derived, from tape-looper)" in dsp_modules %}
target_sources(app PRIVATE src/modules/reverb.c)
{% endif %}

target_include_directories(app PRIVATE src lib)
