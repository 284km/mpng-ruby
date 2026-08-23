/* mpng_ext.c -- the Ruby side of the boundary.
 *
 * libmpng.c is Mere compiled to C (`mere -c --lib`); this file only adapts
 * its three exports to Ruby: a Ruby String crosses as a borrowed (ptr, len)
 * pair, the result comes back as a malloc'd copy that is turned into a Ruby
 * String and freed, and a Mere `fail` becomes a Ruby exception instead of
 * anybody calling exit(1).
 */
#include <ruby.h>
#include "libmpng.h"

static VALUE take_result(mere_buf out) {
  VALUE s = rb_str_new((const char*)out.ptr, (long)out.len);
  mere_lib_free((void*)out.ptr);
  return s;
}

static void raise_fail(mere_buf err) {
  char msg[512];
  long n = err.len < (long long)sizeof(msg) - 1 ? (long)err.len : (long)sizeof(msg) - 1;
  memcpy(msg, err.ptr, (size_t)n);
  msg[n] = 0;
  mere_lib_free((void*)err.ptr);
  rb_raise(rb_eRuntimeError, "%s", msg);
}

static mere_buf borrow(VALUE str) {
  mere_buf b;
  Check_Type(str, T_STRING);
  b.ptr = (const unsigned char*)RSTRING_PTR(str);
  b.len = (long long)RSTRING_LEN(str);
  return b;
}

static VALUE m_info(VALUE self, VALUE data) {
  mere_buf out, err;
  (void)self;
  if (mere_libmpng_png_info(borrow(data), &out, &err) != MERE_OK) raise_fail(err);
  return take_result(out);
}

static VALUE m_to_ppm(VALUE self, VALUE data) {
  mere_buf out, err;
  (void)self;
  if (mere_libmpng_png_to_ppm(borrow(data), &out, &err) != MERE_OK) raise_fail(err);
  return take_result(out);
}

static VALUE m_from_ppm(VALUE self, VALUE data) {
  mere_buf out, err;
  (void)self;
  if (mere_libmpng_ppm_to_png(borrow(data), &out, &err) != MERE_OK) raise_fail(err);
  return take_result(out);
}

/* exposed so a caller can read MERE_REGION_STATS=1 output at a chosen
 * moment; never required for correctness */
static VALUE m_shutdown(VALUE self) {
  (void)self;
  mere_lib_shutdown();
  return Qnil;
}

void Init_mpng_ext(void) {
  VALUE mod = rb_define_module("Mpng");
  rb_define_module_function(mod, "info", m_info, 1);
  rb_define_module_function(mod, "to_ppm", m_to_ppm, 1);
  rb_define_module_function(mod, "from_ppm", m_from_ppm, 1);
  rb_define_module_function(mod, "shutdown", m_shutdown, 0);
}
