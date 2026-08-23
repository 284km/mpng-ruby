# frozen_string_literal: true

require_relative "lib/mpng/version"

Gem::Specification.new do |spec|
  spec.name = "mpng"
  spec.version = Mpng::VERSION
  spec.authors = ["Kazuma Furuhashi"]
  spec.email = ["k.furuhashi10@gmail.com"]

  spec.summary = "PNG decoding and encoding, powered by the mpng codec written in Mere"
  spec.description = "Decode PNGs to binary PPM and encode PPMs to PNG. " \
    "The codec is mpng, written in the Mere language and compiled to C; " \
    "this gem vendors that C and wraps its boundary, so installing needs " \
    "a C compiler and nothing else."
  spec.homepage = "https://github.com/284km/mpng-ruby"
  spec.license = "MIT"
  spec.required_ruby_version = ">= 3.0"

  spec.files = Dir["lib/**/*.rb", "ext/**/*.{c,h,rb}", "libmpng.mere",
                   "README.md", "LICENSE", "mere.toml", "mere.lock"]
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/mpng/extconf.rb"]
end
