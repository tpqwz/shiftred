# shiftred

**Version 0.9.0-beta** (late beta)

Portable single-file C rewrite of the [redshifter](https://github.com/wizetek/bash/blob/master/redshifter) bash wrapper for [redshift](http://jonls.dk/redshift/).

**Inspired by and derived from** the original `redshifter` script by **Tom Wizetek** (wizetek), licensed under the GPL.  
Original: https://github.com/wizetek/bash/blob/master/redshifter  
(2020-03-20, 2018-03-13)

This project renames the tool to **shiftred** to clearly differentiate it from the upstream bash script while preserving the same interface and behavior.

## Features

- Single-file C source (`shiftred.c`)
- Versioned: `0.9.0-beta` (late beta)
- TCC-friendly and **cosmocc**-friendly (Actually Portable Executables)
- Aims for practical compliance with the SEI CERT C Secure Coding Standard
- Static portable binary possible; APE binary runs on Linux/macOS/Windows/FreeBSD/etc.
- Identical CLI and config behavior to the original redshifter
- Config stored in `~/.config/shiftred`
- Configurable redshift binary path
- `-v` / `--version` flag

## Configuration

The file `~/.config/shiftred` is a simple key=value file:

```
colortemp=6500
brightness=1.00
redshift_path=/usr/local/bin/redshift   # optional; defaults to /usr/bin/redshift
```

You can also override the path with the environment variable `REDSHIFT_PATH`.

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
shiftred --version    # show version
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

### With cosmocc (recommended for cross-platform APE)

```bash
# Download cosmocc once (see https://github.com/jart/cosmopolitan)
wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
unzip cosmocc.zip
./bin/cosmocc -O2 -o shiftred.com shiftred.c
./shiftred.com --version
```

The resulting `shiftred.com` is an Actually Portable Executable that runs natively on Linux, macOS, Windows, FreeBSD, OpenBSD, and NetBSD (x86_64 / aarch64 where supported).

### With Tiny C Compiler

```bash
tcc -std=c99 -O2 -o shiftred shiftred.c -lm
```

### Static Linux binary

```bash
gcc -std=c99 -O2 -static -o shiftred shiftred.c -lm
```

**Note:** The redshift binary path is configurable via `redshift_path=` in the config file or the `REDSHIFT_PATH` environment variable. Default is `/usr/bin/redshift`.

## CI / Autobuild

GitHub Actions (`.github/workflows/build.yml`) automatically builds the cosmocc APE binary and a static Linux binary on every push to `main`. Artifacts are uploaded.

## License

GPL-3.0-or-later (same family as the original redshifter script).

See the header of `shiftred.c` for the full attribution.
