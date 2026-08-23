# extconf.rb -- libmpng.c is Mere compiled to C, vendored at gem build time,
# so installing the gem needs a C compiler and nothing else.
require "mkmf"
$srcs = %w[mpng_ext.c libmpng.c]
$CFLAGS << " -O2 -w"
create_makefile("mpng_ext")
