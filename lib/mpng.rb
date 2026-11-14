# frozen_string_literal: true

require_relative "mpng/version"
require "mpng_ext"

# Mpng decodes and encodes PNG images. The implementation is mpng
# (https://github.com/merelang/mpng), a PNG codec written in Mere and compiled
# to C; this gem wraps its C boundary.
#
#   Mpng.info(png_bytes)     #=> "9x6 8-bit rgb"
#   Mpng.to_ppm(png_bytes)   #=> binary PPM (P6) string
#   Mpng.from_ppm(ppm_bytes) #=> PNG string
#
# All three take and return binary Strings. Errors raise RuntimeError with
# the codec's own message.
module Mpng
end
