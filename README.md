# mpng (Ruby gem)

PNG decoding and encoding for Ruby, powered by [mpng](https://github.com/merelang/mpng) —
a PNG codec written in the [Mere](https://github.com/merelang/mere) language.

Mere compiles to C. `libmpng.mere` names the boundary (three functions over
binary strings), `mere -c --lib` turns it into a self-contained C translation
unit with a small C ABI, and this gem vendors that C and wraps it — so
installing the gem needs a C compiler and nothing else. No Mere toolchain,
no shared-library juggling.

## Usage

```ruby
require "mpng"

png = File.binread("photo.png")

Mpng.info(png)      #=> "9x6 8-bit rgb"
ppm  = Mpng.to_ppm(png)    # binary PPM (P6), pixels as another tool can check them
back = Mpng.from_ppm(ppm)  # PNG again (8-bit and 16-bit P6 accepted)
```

All three take and return binary `String`s. A malformed input raises
`RuntimeError` carrying the codec's own message — the codec's failures are
values at this boundary, never `exit`.

## What the codec handles

Everything mpng handles: 8/16-bit RGB, greyscale, palette, RGBA (alpha
dropped on decode), interlaced (Adam7) images, multi-IDAT streams, and
encoding with per-scanline filter choice. See mpng's README for the codec
itself; this gem adds nothing to it, by design.

## Regenerating the vendored C

```sh
mere -c --lib libmpng.mere > ext/mpng/libmpng.c
mere --header libmpng.mere > ext/mpng/libmpng.h
```

`verify.sh` does this first, then checks the gem's output byte-for-byte
against both the test images' computed expectations and the mpng CLI, and
holds the boundary to its memory contract (N calls cost what one costs).

## License

MIT
