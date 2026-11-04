# shiftred

Portable single-file C rewrite of the [redshifter](https://github.com/wizetek/bash/blob/master/redshifter) bash wrapper for [redshift](http://jonls.dk/redshift/).

**Inspired by and derived from** the original `redshifter` script by **Tom Wizetek** (wizetek), licensed under the GPL.  
Original: https://github.com/wizetek/bash/blob/master/redshifter  
(2020-03-20, 2018-03-13)

This project renames the tool to **shiftred** to clearly differentiate it from the upstream bash script while preserving the same interface and behavior.

## Features

- Single-file C source (`shiftred.c`)
- TCC-friendly (Tiny C Compiler)
- Aims for practical compliance with the SEI CERT C Secure Coding Standard
- Static portable binary possible
- Identical CLI and config behavior to the original redshifter
- Config stored in `~/.config/shiftred`

## Usage

```bash
shiftred              # cycle to next lower preset
shiftred +            # increase temperature by 100 K
shiftred -            # decrease temperature by 100 K
shiftred 2700         # set exact temperature
shiftred =            # restore saved values
shiftred /            # reset temperature to daylight (6500 K)
shiftred 2700 0.8     # set temperature and brightness
shiftred + -          # brighter temp, dimmer brightness
```

Presets (cycled with no arguments):

- 6500K Daylight
- 5500K Sunlight
- 4200K Fluorescent
- 3400K Halogen
- 2700K Incandescent
- 2300K Dim Incandescent
- 1900K Candle
- 1200K Ember

## Build

With Tiny C Compiler:

```bash
tcc -std=c99 -O2 -o shiftred shiftred.c -lm
```

Static portable binary (requires a static-capable libc):

```bash
gcc -std=c99 -O2 -static -o shiftred shiftred.c -lm
```

Requires `redshift` at `/usr/bin/redshift` (or edit `REDSHIFT_BIN` in the source).

## License

GPL-3.0-or-later (same family as the original redshifter script).

See the header of `shiftred.c` for the full attribution.
