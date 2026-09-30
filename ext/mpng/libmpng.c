#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <setjmp.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#pragma STDC FP_CONTRACT OFF
typedef double mere_f64x2 __attribute__((vector_size(16)));
typedef unsigned char mere_u8x16 __attribute__((vector_size(16)));
__attribute__((noreturn)) static void __lang_fail_idx(const char*, long long, long long);
static inline mere_f64x2 mere_f64x2_splat(double x) { mere_f64x2 v = { x, x }; return v; }
static inline double mere_f64x2_extract(mere_f64x2 v, long long i) {
  if (i < 0 || i >= 2) __lang_fail_idx("f64x2_extract: lane %lld out of range (lanes = %lld)", i, 2LL);
  return v[i];
}
static inline mere_u8x16 mere_u8x16_splat(long long x) {
  unsigned char b = (unsigned char)(x & 0xFF);
  mere_u8x16 v = { b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b }; return v;
}
static inline mere_f64x2 mere_f64x2_make(double a, double b) { mere_f64x2 v = { a, b }; return v; }
static inline double mere_f64x2_reduce_add(mere_f64x2 v) { return v[0] + v[1]; }
static inline mere_f64x2 mere_f64x2_fma(mere_f64x2 a, mere_f64x2 b, mere_f64x2 c) {
  mere_f64x2 r = { fma(a[0], b[0], c[0]), fma(a[1], b[1], c[1]) }; return r;
}
typedef float mere_f32x4 __attribute__((vector_size(16)));
static inline mere_f32x4 mere_f32x4_splat(double x) { float f = (float)x; mere_f32x4 v = { f, f, f, f }; return v; }
static inline mere_f32x4 mere_f32x4_make(double a, double b, double c, double d) {
  mere_f32x4 v = { (float)a, (float)b, (float)c, (float)d }; return v;
}
static inline double mere_f32x4_extract(mere_f32x4 v, long long i) {
  if (i < 0 || i >= 4) __lang_fail_idx("f32x4_extract: lane %lld out of range (lanes = %lld)", i, 4LL);
  return (double)v[i];
}
static inline double mere_f32x4_reduce_add(mere_f32x4 v) {
  float s = v[0] + v[1]; s = s + v[2]; s = s + v[3]; return (double)s;
}
#if defined(__aarch64__) || defined(__ARM_NEON)
#include <arm_neon.h>
#elif defined(__SSSE3__)
#include <immintrin.h>
#endif
typedef signed char mere_i8x16 __attribute__((vector_size(16)));
static inline mere_u8x16 mere_u8x16_sub_sat(mere_u8x16 a, mere_u8x16 b) {
#if __has_builtin(__builtin_elementwise_sub_sat)
  return __builtin_elementwise_sub_sat(a, b);
#else
  mere_u8x16 r; for (int i = 0; i < 16; i++) r[i] = a[i] > b[i] ? a[i] - b[i] : 0; return r;
#endif
}
static inline mere_u8x16 mere_u8x16_eq(mere_u8x16 a, mere_u8x16 b) { return (mere_u8x16)(a == b); }
static inline mere_u8x16 mere_u8x16_shr(mere_u8x16 v, long long k) {
  if (k < 0 || k > 7) __lang_fail_idx("u8x16_shr: shift %lld out of range 0..%lld", k, 7LL);
  return v >> (unsigned char)k;
}
static inline mere_u8x16 mere_u8x16_swizzle(mere_u8x16 t, mere_u8x16 idx) {
#if defined(__aarch64__) || defined(__ARM_NEON)
  return (mere_u8x16)vqtbl1q_u8((uint8x16_t)t, (uint8x16_t)idx);
#elif defined(__SSSE3__)
  /* pshufb zeroes a lane whose index has bit 7 set; an index of 16..127 would otherwise wrap */
  __m128i i = (__m128i)idx;
  __m128i big = _mm_cmpgt_epi8(i, _mm_set1_epi8(15));
  return (mere_u8x16)_mm_shuffle_epi8((__m128i)t, _mm_or_si128(i, big));
#else
  mere_u8x16 r; for (int i = 0; i < 16; i++) r[i] = idx[i] < 16 ? t[idx[i]] : 0; return r;
#endif
}
static inline mere_u8x16 mere_u8x16_shift_in(mere_u8x16 prev, mere_u8x16 cur, long long k) {
  switch (k) {
    case 0: return cur;
    case 1: return __builtin_shufflevector(prev, cur, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
    case 2: return __builtin_shufflevector(prev, cur, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29);
    case 3: return __builtin_shufflevector(prev, cur, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28);
    case 4: return __builtin_shufflevector(prev, cur, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27);
    case 5: return __builtin_shufflevector(prev, cur, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26);
    case 6: return __builtin_shufflevector(prev, cur, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25);
    case 7: return __builtin_shufflevector(prev, cur, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24);
    case 8: return __builtin_shufflevector(prev, cur, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23);
    case 9: return __builtin_shufflevector(prev, cur, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22);
    case 10: return __builtin_shufflevector(prev, cur, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21);
    case 11: return __builtin_shufflevector(prev, cur, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
    case 12: return __builtin_shufflevector(prev, cur, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19);
    case 13: return __builtin_shufflevector(prev, cur, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18);
    case 14: return __builtin_shufflevector(prev, cur, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
    case 15: return __builtin_shufflevector(prev, cur, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    case 16: return prev;
    default: __lang_fail_idx("u8x16_shift_in: shift %lld out of range 0..%lld", k, 16LL);
  }
}
static inline int mere_u8x16_any_true(mere_u8x16 v) {
  unsigned long long lo, hi; memcpy(&lo, &v, 8); memcpy(&hi, (const char*)&v + 8, 8); return (lo | hi) != 0;
}
static inline long long mere_u8x16_first_true(mere_u8x16 v) {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  unsigned long long lo, hi; memcpy(&lo, &v, 8); memcpy(&hi, (const char*)&v + 8, 8);
  if (lo) return (long long)(__builtin_ctzll(lo) >> 3);
  if (hi) return (long long)(8 + (__builtin_ctzll(hi) >> 3));
  return -1;
#else
  for (int i = 0; i < 16; i++) if (v[i]) return (long long)i;
  return -1;
#endif
}
static inline long long mere_u8x16_reduce_add(mere_u8x16 v) {
  long long s = 0; for (int i = 0; i < 16; i++) s += v[i]; return s;
}
static inline long long mere_u8x16_extract(mere_u8x16 v, long long i) {
  if (i < 0 || i >= 16) __lang_fail_idx("u8x16_extract: lane %lld out of range (lanes = %lld)", i, 16LL);
  return (long long)v[i];
}

static const struct { size_t l; char s[1]; } __sl_0 = { 0, "" };
static const struct { size_t l; char s[45]; } __sl_1 = { 44, "str_of_codepoint: not a Unicode scalar value" };
static const struct { size_t l; char s[7]; } __sl_2 = { 6, "fail: " };
static const struct { size_t l; char s[46]; } __sl_3 = { 45, "codepoint_of: expected a single-codepoint str" };
static const struct { size_t l; char s[2]; } __sl_4 = { 1, " " };
static const struct { size_t l; char s[24]; } __sl_5 = { 23, "pow: negative exponent " };
static const struct { size_t l; char s[25]; } __sl_6 = { 24, "divmod: division by zero" };
static const struct { size_t l; char s[19]; } __sl_7 = { 18, "assertion failed: " };
static const struct { size_t l; char s[8]; } __sl_8 = { 7, "unknown" };
static const struct { size_t l; char s[5]; } __sl_9 = { 4, "rgba" };
static const struct { size_t l; char s[11]; } __sl_10 = { 10, "grey+alpha" };
static const struct { size_t l; char s[8]; } __sl_11 = { 7, "palette" };
static const struct { size_t l; char s[4]; } __sl_12 = { 3, "rgb" };
static const struct { size_t l; char s[5]; } __sl_13 = { 4, "grey" };
static const struct { size_t l; char s[2]; } __sl_14 = { 1, "x" };
static const struct { size_t l; char s[6]; } __sl_15 = { 5, "-bit " };
static const struct { size_t l; char s[12]; } __sl_16 = { 11, " interlaced" };
static const struct { size_t l; char s[7]; } __sl_17 = { 6, " +tRNS" };
static const struct { size_t l; char s[8]; } __sl_18 = { 7, " gamma " };
static const struct { size_t l; char s[33]; } __sl_19 = { 32, "no IHDR: not a PNG this can read" };
static const struct { size_t l; char s[4]; } __sl_20 = { 3, "P6\n" };
static const struct { size_t l; char s[6]; } __sl_21 = { 5, "\n255\n" };
static const struct { size_t l; char s[9]; } __sl_22 = { 8, "-bit -> " };
static const struct { size_t l; char s[3]; } __sl_23 = { 2, " (" };
static const struct { size_t l; char s[8]; } __sl_24 = { 7, " bytes)" };
static const struct { size_t l; char s[34]; } __sl_25 = { 33, "not a PPM this can read (P6 only)" };
static const struct { size_t l; char s[11]; } __sl_26 = { 10, " bytes -> " };
static const struct { size_t l; char s[4]; } __sl_27 = { 3, "ppm" };
static const struct { size_t l; char s[5]; } __sl_28 = { 4, "info" };
static const struct { size_t l; char s[10]; } __sl_29 = { 9, "not a PNG" };
static const struct { size_t l; char s[3]; } __sl_30 = { 2, ": " };
static const struct { size_t l; char s[2]; } __sl_31 = { 1, "\n" };
static const struct { size_t l; char s[3]; } __sl_32 = { 2, "  " };
static const struct { size_t l; char s[97]; } __sl_33 = { 96, "usage: mpng info <file.png>  |  mpng ppm <file.png> <out.ppm>  |  mpng encode <in.ppm> <out.png>" };

/* A region is a chain of bump-allocated blocks. When the current
   block fills, a new (geometrically larger) block is chained on
   instead of aborting — v0.1.25, found by a long-running server
   (mkv) whose per-command allocations exhausted the old fixed-cap
   region after a few thousand requests. Blocks never move, so
   pointers into earlier blocks stay valid across growth. */
typedef struct __lang_region_block {
  struct __lang_region_block* prev;
  size_t pad;  /* the block's capacity (the header keeps the data 16-aligned) */
  unsigned long ep;   /* __lang_region_epoch when it was allocated */
  size_t dflt;   /* on the default region's list (see __lang_blk_link) */
  struct __lang_region_s* owner;   /* whose chain it is on: its bump block ends at owner->top */
  struct __lang_region_block* gprev;
  struct __lang_region_block* gnext;
} __lang_region_block;
/* bumped by every coroutine switch (see __lang_region_pinned): a block
   allocated after a coroutine last stopped cannot be pointed into by it */
static _Thread_local unsigned long __lang_region_epoch = 1;

typedef struct __lang_region_s {
  char* base;               /* current block: data start */
  char* top;
  size_t cap;               /* current block capacity */
  __lang_region_block* blocks;  /* chain of every block, for free */
  size_t alloc_total;       /* cumulative bytes handed out (v0.1.307) */
  const char* site;
} __lang_region;
static __lang_region __lang_default_region;

/* EVERY LIVE BLOCK, so a conservative scan can ask whether an address may be
   read (coro_scan_ints): a word that points into no live block is never
   dereferenced. Two lists: a region other than the default one belongs to
   its thread (Map, Vec, StrBuf, ListBuf and region borrows are not Send),
   and the default region is shared, so its blocks have a list of their own
   with its own lock. Every block is made, grown and freed through the three
   helpers below, which is what keeps the lists true. */
static _Thread_local __lang_region_block* __lang_blk_tl = NULL;
static __lang_region_block* __lang_blk_dl = NULL;
static pthread_mutex_t __lang_blk_dl_lock = PTHREAD_MUTEX_INITIALIZER;
/* bumped whenever either list changes, so a scan can keep its sorted copy */
static _Thread_local unsigned long __lang_blk_tl_gen = 1;
static unsigned long __lang_blk_dl_gen = 1;
static void __lang_blk_link(__lang_region* r, __lang_region_block* b) {
  b->dflt = (r == &__lang_default_region);
  b->owner = r;
  b->gprev = NULL;
  if (b->dflt) {
    pthread_mutex_lock(&__lang_blk_dl_lock);
    b->gnext = __lang_blk_dl; if (__lang_blk_dl) __lang_blk_dl->gprev = b; __lang_blk_dl = b;
    __lang_blk_dl_gen++;
    pthread_mutex_unlock(&__lang_blk_dl_lock);
  } else {
    b->gnext = __lang_blk_tl; if (__lang_blk_tl) __lang_blk_tl->gprev = b; __lang_blk_tl = b;
    __lang_blk_tl_gen++;
  }
}
static void __lang_blk_unlink(__lang_region_block* b) {
  if (b->dflt) pthread_mutex_lock(&__lang_blk_dl_lock);
  __lang_region_block** head = b->dflt ? &__lang_blk_dl : &__lang_blk_tl;
  if (b->gprev) b->gprev->gnext = b->gnext; else *head = b->gnext;
  if (b->gnext) b->gnext->gprev = b->gprev;
  if (b->dflt) { __lang_blk_dl_gen++; pthread_mutex_unlock(&__lang_blk_dl_lock); }
  else __lang_blk_tl_gen++;
}
static void __lang_blk_free(__lang_region_block* b) {
  __lang_blk_unlink(b);
  free(b);
}

typedef struct {
  void* head;
  void* tail;
  __lang_region* region;
  long long len;
  int frozen;
} __lang_listbuf;

__attribute__((noreturn)) static void __lang_fail_impl(const char* msg);
__attribute__((noreturn)) static void __lang_fail_idx(
    const char* fmt, long long a, long long b) {
  char __m[160];
  snprintf(__m, sizeof __m, fmt, a, b);
  __lang_fail_impl(__m);
}
__attribute__((noreturn)) static void __lang_fail_num(
    const char* fmt, long long a) {
  char __m[160];
  snprintf(__m, sizeof __m, fmt, a);
  __lang_fail_impl(__m);
}
__attribute__((noreturn)) static void __lang_fail_chr(
    const char* fmt, int c) {
  char __m[160];
  snprintf(__m, sizeof __m, fmt, c);
  __lang_fail_impl(__m);
}
__attribute__((noreturn)) static void __lang_fail_range(
    const char* fmt, long long a, long long b, long long c) {
  char __m[160];
  snprintf(__m, sizeof __m, fmt, a, b, c);
  __lang_fail_impl(__m);
}
/* the address range this thread's region blocks have ever spanned: a
   suspended coroutine's stack is scanned only for words inside it (see
   __lang_region_pinned) */
static _Thread_local uintptr_t __lang_block_lo = UINTPTR_MAX, __lang_block_hi = 0;
static void __lang_block_note(void* b, size_t cap) {
  uintptr_t lo = (uintptr_t)b, hi = lo + sizeof(__lang_region_block) + cap;
  if (lo < __lang_block_lo) __lang_block_lo = lo;
  if (hi > __lang_block_hi) __lang_block_hi = hi;
}
static void __lang_region_add_block(__lang_region* r, size_t cap) {
  __lang_region_block* b =
    (__lang_region_block*) malloc(sizeof(__lang_region_block) + cap);
  if (!b) __lang_fail_impl("out of memory");
  __lang_block_note(b, cap);
  __lang_blk_link(r, b);
  b->ep = __lang_region_epoch;
  b->pad = cap;
  b->prev = r->blocks;
  r->blocks = b;
  r->base = (char*)(b + 1);
  r->top = r->base;
  r->cap = cap;
}

static void __lang_region_init(__lang_region* r, size_t cap) {
  r->blocks = NULL;
  r->alloc_total = 0;
  r->site = 0;
  __lang_region_add_block(r, cap);
}

/* Program-lifetime arena for closure envs and other long-lived
   allocations that outlive any user `region R { ... }` block. It is
   never freed, so pointers into it stay valid for the whole run —
   including in spawned threads (Q-012). Only its bump pointer needs
   guarding: allocations from THIS region take a lock, while per-block
   regions (thread-local stack locals) stay lock-free. */
static __lang_region __lang_default_region;
static pthread_mutex_t __lang_default_region_lock = PTHREAD_MUTEX_INITIALIZER;

/* v0.1.31 (str-lifetime stage B): the thread-local CURRENT region.
   Value allocations (strings, cons cells, variant nodes) target this
   instead of hardcoding the default region, so a `region R { ... }`
   block can make itself current and reclaim everything its body
   allocated at exit. Closure envs and container structs stay in the
   default region (or their binding region) — they carry identity and
   must not die with a scratch block. Each thread starts at the
   default region. */
static _Thread_local __lang_region* __lang_current_region = &__lang_default_region;

static void* __lang_region_alloc(__lang_region* r, size_t n) {
  int shared = (r == &__lang_default_region);
  if (shared) pthread_mutex_lock(&__lang_default_region_lock);
  size_t aligned = (n + 7) & ~((size_t)7);
  r->alloc_total += aligned;
  if (r->top + aligned > r->base + r->cap) {
    if (aligned > r->cap / 4) {
      __lang_region_block* big =
        (__lang_region_block*) malloc(sizeof(__lang_region_block) + aligned);
      if (!big) __lang_fail_impl("out of memory");
      __lang_block_note(big, aligned);
      __lang_blk_link(r, big);
      big->ep = __lang_region_epoch;
      big->pad = aligned;
      big->prev = r->blocks->prev;
      r->blocks->prev = big;
      void* bp = (void*)(big + 1);
      if (shared) pthread_mutex_unlock(&__lang_default_region_lock);
      return bp;
    }
    size_t ncap = r->cap * 2;
    while (ncap < aligned) {
      if (ncap > (size_t)-1 / 2) { ncap = aligned; break; }
      ncap *= 2;
    }
    __lang_region_add_block(r, ncap);
  }
  void* p = r->top;
  r->top += aligned;
  if (shared) pthread_mutex_unlock(&__lang_default_region_lock);
  return p;
}

static void* __lang_region_grow(__lang_region* r, void* old, size_t old_n, size_t new_n) {
  int shared = (r == &__lang_default_region);
  if (old && old_n) {
    size_t old_al = (old_n + 7) & ~((size_t)7);
    size_t new_al = (new_n + 7) & ~((size_t)7);
    if (shared) pthread_mutex_lock(&__lang_default_region_lock);
    if ((char*)old + old_al == r->top && (char*)old + new_al <= r->base + r->cap) {
      r->top = (char*)old + new_al;
      r->alloc_total += new_al - old_al;
      if (shared) pthread_mutex_unlock(&__lang_default_region_lock);
      return old;
    }
    __lang_region_block* pred = r->blocks;
    while (pred && pred->prev) {
      __lang_region_block* blk = pred->prev;
      if ((char*)(blk + 1) == (char*)old && blk->pad == old_al) {
        __lang_blk_unlink(blk);
        __lang_region_block* nb =
          (__lang_region_block*) realloc(blk, sizeof(__lang_region_block) + new_al);
        if (!nb) __lang_fail_impl("out of memory");
        __lang_block_note(nb, new_al);
        __lang_blk_link(r, nb);
        nb->pad = new_al;
        pred->prev = nb;
        r->alloc_total += new_al - old_al;
        if (shared) pthread_mutex_unlock(&__lang_default_region_lock);
        return (void*)(nb + 1);
      }
      pred = blk;
    }
    if (shared) pthread_mutex_unlock(&__lang_default_region_lock);
  }
  void* p = __lang_region_alloc(r, new_n);
  if (old && old_n) memcpy(p, old, old_n < new_n ? old_n : new_n);
  return p;
}

/* v0.1.299: total capacity of a region's block chain. Capacity, not
   bump-used: within 2x of used (blocks double), free of hot-path cost,
   and monotone between compactions -- which is all a collection
   trigger needs. */
static size_t __lang_region_bytes(__lang_region* r) {
  size_t n = 0;
  for (__lang_region_block* b = r->blocks; b; b = b->prev) n += b->pad;
  return n;
}

#define __LANG_REGION_SITES 32
typedef struct { const char* name; size_t entries; size_t alloc_total;
                 size_t peak_cap; } __lang_region_site;
static __lang_region_site __lang_region_sites[__LANG_REGION_SITES];
static int __lang_region_site_n = 0;
static size_t __lang_region_sites_dropped = 0;
static pthread_mutex_t __lang_region_sites_lock = PTHREAD_MUTEX_INITIALIZER;

static void __lang_region_charge(__lang_region* r) {
  if (!r->site) return;
  size_t cap = 0;
  for (__lang_region_block* b = r->blocks; b; b = b->prev) cap += b->pad;
  pthread_mutex_lock(&__lang_region_sites_lock);
  for (int i = 0; i < __lang_region_site_n; i++) {
    if (strcmp(__lang_region_sites[i].name, r->site) == 0) {
      __lang_region_sites[i].entries++;
      __lang_region_sites[i].alloc_total += r->alloc_total;
      if (cap > __lang_region_sites[i].peak_cap)
        __lang_region_sites[i].peak_cap = cap;
      pthread_mutex_unlock(&__lang_region_sites_lock);
      return;
    }
  }
  if (__lang_region_site_n < __LANG_REGION_SITES) {
    __lang_region_site* s = &__lang_region_sites[__lang_region_site_n++];
    s->name = r->site; s->entries = 1;
    s->alloc_total = r->alloc_total; s->peak_cap = cap;
  } else { __lang_region_sites_dropped++; }
  pthread_mutex_unlock(&__lang_region_sites_lock);
}

#define __LANG_REGION_CACHE_CAP 8
#define __LANG_REGION_KEEP_MAX (16 * 1024 * 1024)
static _Thread_local __lang_region* __lang_region_cache[__LANG_REGION_CACHE_CAP];
static _Thread_local int __lang_region_cache_n = 0;
static void __lang_region_stats_report(void) {
  static int __done = 0;
  if (__done || !getenv("MERE_REGION_STATS")) return;
  __done = 1;
  __lang_region* r = &__lang_default_region;
  size_t nblocks = 0, cap = 0;
  for (__lang_region_block* b = r->blocks; b; b = b->prev) { nblocks++; cap += b->pad; }
  fprintf(stderr, "region-stats default: blocks=%zu cap=%zu alloc_total=%zu\n",
          nblocks, cap, r->alloc_total);
  size_t named_alloc = 0;
  for (int i = 0; i < __lang_region_site_n; i++) {
    __lang_region_site* s = &__lang_region_sites[i];
    named_alloc += s->alloc_total;
    fprintf(stderr, "region-stats named %s: arenas=%zu alloc_total=%zu peak_cap=%zu\n",
            s->name, s->entries, s->alloc_total, s->peak_cap);
  }
  if (__lang_region_site_n > 0)
    fprintf(stderr, "region-stats named-total: sites=%d alloc_total=%zu\n",
            __lang_region_site_n, named_alloc);
  if (__lang_region_sites_dropped)
    fprintf(stderr, "region-stats WARNING: %zu arena releases uncounted (more than %d names)\n",
            __lang_region_sites_dropped, __LANG_REGION_SITES);
  size_t cached_bytes = 0;
  for (int i = 0; i < __lang_region_cache_n; i++)
    cached_bytes += __lang_region_bytes(__lang_region_cache[i]);
  fprintf(stderr, "region-stats cache: regions=%d retained=%zu\n",
          __lang_region_cache_n, cached_bytes);
}

static void __lang_region_free(__lang_region* r) {
  __lang_region_charge(r);
  __lang_region_block* b = r->blocks;
  while (b) {
    __lang_region_block* prev = b->prev;
    __lang_blk_free(b);
    b = prev;
  }
  r->blocks = NULL;
}

/* An arena a SUSPENDED COROUTINE may still point into is not freed.
   Compaction and recycling free an arena whole, and a coroutine that stopped
   in the middle of an expression can hold a pointer into it in a C variable
   of its own stack. A program that makes coroutines installs this hook (the
   coroutine runtime does, on the first coro_new): it answers whether any
   suspended stack holds a word inside one of the arena's blocks. Such an
   arena is retired instead -- kept, and tried again at the next retirement --
   so compacting under a suspended coroutine is safe; what it cannot give
   back yet, it gives back later. Programs without coroutines free at once,
   as before. */
static _Thread_local int (*__lang_region_pinned)(__lang_region*, int) = 0;
static _Thread_local __lang_region** __lang_retired = NULL;
static _Thread_local int __lang_retired_n = 0, __lang_retired_cap = 0;
static void __lang_region_retire(__lang_region* r) {
  if (__lang_region_pinned) {
    int w = 0;
    for (int i = 0; i < __lang_retired_n; i++) {
      __lang_region* o = __lang_retired[i];
      if (__lang_region_pinned(o, 0)) __lang_retired[w++] = o;
      else { __lang_region_free(o); free(o); }
    }
    __lang_retired_n = w;
    if (__lang_region_pinned(r, 0)) {
      if (__lang_retired_n == __lang_retired_cap) {
        int nc = __lang_retired_cap ? __lang_retired_cap * 2 : 16;
        __lang_region** p = (__lang_region**)realloc(__lang_retired, sizeof(__lang_region*) * nc);
        if (!p) __lang_fail_impl("out of memory");
        __lang_retired = p; __lang_retired_cap = nc;
      }
      __lang_retired[__lang_retired_n++] = r;
      return;
    }
  }
  __lang_region_free(r);
  free(r);
}

/* Block regions are heap structs, cached on a small per-thread STACK so a
   per-iteration `region R { }` in a hot loop costs a pointer swap
   and a bump reset instead of malloc + free. (Also: a STACK region
   struct whose address escapes into the thread-local current region
   defeats clang's sibling-call optimization — deep tail loops with
   per-iteration regions overflowed the stack.) Release keeps the
   region only if it never grew past its first block (the common
   case); a grown chain is freed and re-seeded.

   v0.1.295: the cache was ONE deep, and one is not enough as soon as
   regions nest: a loop iteration's region inside a statement's region,
   with statement regions inside the body, releases out of cache order.
   The innermost release filled the single slot, so the next-outer
   release FREED its region -- struct, 1 MB seed and all -- and the next
   acquire at that depth malloc'd them back. One interpreter workload
   paid a 1 MB malloc+free round trip per loop iteration: ~200 GB of
   churn over a 200k-iteration run, visible in its block counters. Eight
   slots cover any realistic nesting depth; deeper than that falls back
   to malloc, which is correct and merely slower. */

/* v0.1.301: the ACTIVE region stack. Every live block region is on
   it, so a fail that longjmps out of nested `region R { }` blocks can
   release the chains it jumps over. Before this, try_or only restored
   the current-region pointer and the abandoned chains leaked -- noted
   as acceptable when regions were rare; measured at 1 MB per caught
   fail once an interpreter wrapped every call in one (2000 rescues =
   2.1 GB, flat at 4 MB with the unwind). */
static _Thread_local __lang_region** __lang_region_active = NULL;
static _Thread_local int __lang_region_active_n = 0;
static _Thread_local int __lang_region_active_cap = 0;

static void __lang_region_active_push(__lang_region* r) {
  if (__lang_region_active_n == __lang_region_active_cap) {
    int ncap = __lang_region_active_cap ? __lang_region_active_cap * 2 : 64;
    __lang_region** p = (__lang_region**)realloc(__lang_region_active,
                                                 sizeof(__lang_region*) * ncap);
    if (!p) __lang_fail_impl("out of memory");
    __lang_region_active = p; __lang_region_active_cap = ncap;
  }
  __lang_region_active[__lang_region_active_n++] = r;
}

/* normal exits release in LIFO order; the region-loop swap releases the
   entry UNDER the just-pushed next arena, so drop searches from the top */
static void __lang_region_active_drop(__lang_region* r) {
  for (int i = __lang_region_active_n - 1; i >= 0; i--)
    if (__lang_region_active[i] == r) {
      memmove(__lang_region_active + i, __lang_region_active + i + 1,
              sizeof(__lang_region*) * (size_t)(__lang_region_active_n - i - 1));
      __lang_region_active_n--;
      return;
    }
}

static void __lang_region_block_release(__lang_region* r);
static void __lang_region_unwind(int to_n) {
  while (__lang_region_active_n > to_n) {
    __lang_region* r = __lang_region_active[__lang_region_active_n - 1];
    /* release() drops it from the stack itself */
    __lang_region_block_release(r);
  }
}

static __lang_region* __lang_region_block_acquire(const char* __site) {
  if (__lang_region_cache_n > 0) {
    __lang_region* r = __lang_region_cache[--__lang_region_cache_n];
    r->top = r->base;
    r->alloc_total = 0;
    r->site = __site;
    __lang_region_active_push(r);
    return r;
  }
  __lang_region* r = (__lang_region*)malloc(sizeof(__lang_region));
  __lang_region_init(r, 1 << 20);
  r->site = __site;
  __lang_region_active_push(r);
  return r;
}

static void __lang_region_block_release(__lang_region* r) {
  __lang_region_active_drop(r);
  __lang_region_charge(r);
  r->site = 0;
  if (r->blocks->prev) {
    __lang_region_block* best = r->blocks; size_t bestcap = r->blocks->pad;
    for (__lang_region_block* b = r->blocks; b; b = b->prev)
      if (b->pad > bestcap) { best = b; bestcap = b->pad; }
    if (bestcap > __LANG_REGION_KEEP_MAX) {
      /* one-off and enormous: give it back rather than hold it */
      __lang_region_free(r);
      __lang_region_init(r, 1 << 20);
    } else {
      for (__lang_region_block* b = r->blocks; b; ) {
        __lang_region_block* prev = b->prev;
        if (b != best) __lang_blk_free(b);
        b = prev;
      }
      best->prev = NULL;
      r->blocks = best;
      r->base = (char*)(best + 1);
      r->cap = best->pad;
    }
  }
  r->top = r->base;
  if (__lang_region_cache_n < __LANG_REGION_CACHE_CAP)
    __lang_region_cache[__lang_region_cache_n++] = r;
  else { __lang_region_free(r); free(r); }
}

static char* __lang_str_alloc(__lang_region* r, size_t len);
static size_t __lang_str_size(const char* s);
static int __lang_str_cmp(const char* a, const char* b);
static const char* __lang_str_of_cstr(const char* s);
static const char* __lang_str_dup_n(const char* s, size_t n);


typedef struct { pthread_t tid; } ThreadHandle;
typedef struct { void* env; int (*fn)(void*, int); } __mere_unit_closure;
static void* __lang_thread_segv_enter(void);
static void __lang_thread_segv_leave(void*);
static void* __mere_spawn_trampoline(void* __p) {
  __mere_unit_closure* __c = (__mere_unit_closure*)__p;
  void* __alt = __lang_thread_segv_enter();
  __c->fn(__c->env, 0);
  free(__c);
  __lang_thread_segv_leave(__alt);
  /* v0.1.31: drop this thread's cached block regions — _Thread_local
     has no destructor, so a spawn-per-connection server would leak
     ~1 MB per finished thread without this. (v0.1.295: the cache is a
     small stack now; drain the lot.) */
  while (__lang_region_cache_n > 0) {
    __lang_region* __rc = __lang_region_cache[--__lang_region_cache_n];
    __lang_region_free(__rc);
    free(__rc);
  }
  /* v0.1.301: and the active-region stack's storage */
  free(__lang_region_active);
  __lang_region_active = NULL; __lang_region_active_n = 0; __lang_region_active_cap = 0;
  return NULL;
}

static char* __lang_str_alloc(__lang_region* r, size_t len) {
  size_t* p = (size_t*) __lang_region_alloc(r, sizeof(size_t) + len + 1);
  p[0] = len;
  char* d = (char*)(p + 1);
  d[len] = '\0';
  return d;
}
static size_t __lang_str_size(const char* s) { return ((const size_t*)s)[-1]; }
static void __lang_write_all(int fd, const char* p, size_t n) {
  fflush(stdout);
  while (n) {
    ssize_t w = write(fd, p, n);
    if (w > 0) { p += (size_t)w; n -= (size_t)w; continue; }
    if (w < 0 && errno == EINTR) continue;
    break;
  }
}
static int __lang_str_cmp(const char* a, const char* b) {
  size_t la = __lang_str_size(a), lb = __lang_str_size(b);
  size_t m = la < lb ? la : lb;
  int r = m ? memcmp(a, b, m) : 0;
  if (r != 0) return r;
  return la < lb ? -1 : (la > lb ? 1 : 0);
}
static const char* __lang_str_of_cstr(const char* s) {
  if (!s) return __lang_str_alloc(__lang_current_region, 0);
  size_t n = strlen(s);
  char* r = __lang_str_alloc(__lang_current_region, n);
  memcpy(r, s, n);
  return r;
}
static const char* __lang_str_take_cstr(char* s) {
  const char* r = __lang_str_of_cstr(s);
  free(s);
  return r;
}
static const char* __lang_str_dup_n(const char* s, size_t n) {
  char* r = __lang_str_alloc(__lang_current_region, n);
  memcpy(r, s, n);
  return r;
}

static const char* __lang_str_concat(const char* a, const char* b) {
  size_t la = __lang_str_size(a), lb = __lang_str_size(b);
  char* r = __lang_str_alloc(__lang_current_region, la + lb);
  memcpy(r, a, la);
  memcpy(r + la, b, lb);
  return r;
}

static int64_t __lang_str_index_of(const char* h, const char* n) {
  size_t hn = __lang_str_size(h), nn = __lang_str_size(n);
  if (nn == 0) return 0;
  if (nn > hn) return -1;
  const char* last = h + (hn - nn);
  const char* p = h;
  while (p <= last) {
    const char* q = (const char*)memchr(p, (unsigned char)n[0], (size_t)(last - p) + 1);
    if (!q) return -1;
    if (memcmp(q, n, nn) == 0) return (int64_t)(q - h);
    p = q + 1;
  }
  return -1;
}

static int64_t __lang_str_last_index_of(const char* h, const char* n) {
  size_t hn = __lang_str_size(h), nn = __lang_str_size(n);
  if (nn == 0) return (int64_t)hn;
  if (nn > hn) return -1;
  for (size_t i = hn - nn + 1; i-- > 0; )
    if (memcmp(h + i, n, nn) == 0) return (int64_t)i;
  return -1;
}

static struct { size_t __n; char __c[2]; } __lang_char_table[256];
static int __lang_char_table_init = 0;
static void __lang_char_table_setup(void) {
  if (__lang_char_table_init) return;
  for (int k = 0; k < 256; k++) {
    __lang_char_table[k].__n = 1;
    __lang_char_table[k].__c[0] = (char)k;
    __lang_char_table[k].__c[1] = '\0';
  }
  __lang_char_table_init = 1;
}
static const char* __lang_char_at(const char* s, int64_t i) {
  __lang_char_table_setup();
  long long __n = (long long)__lang_str_size(s);
  if (i < 0 || i >= __n)
    __lang_fail_idx("char_at: index %lld out of range (len=%lld)", (long long)i, __n);
  return __lang_char_table[(unsigned char)s[i]].__c;
}
static int __lang_is_digit(const char* s) {
  unsigned char c = (unsigned char)s[0];
  return c >= '0' && c <= '9';
}
static int __lang_is_alpha(const char* s) {
  unsigned char c = (unsigned char)s[0];
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static int __lang_is_space(const char* s) {
  unsigned char c = (unsigned char)s[0];
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static _Thread_local int __lang_fail_jmpbuf_set = 0;
static _Thread_local jmp_buf __lang_fail_jmpbuf;
static _Thread_local char __lang_fail_msg[256];
static char __lang_sigstack[65536];
static _Thread_local char* __lang_stack_lo = 0;
static _Thread_local char* __lang_stack_hi = 0;
static void __lang_segv(int sig, siginfo_t* info, void* uap) {
  (void)sig; (void)uap;
  const char* m;
  char* a = (char*)(info ? info->si_addr : 0);
  if (__lang_stack_lo && a >= __lang_stack_lo - (1 << 20) && a <= __lang_stack_hi)
    m = "stack overflow (recursion too deep)\n";
  else
    m = "segmentation fault\n";
  ssize_t __unused_w = write(2, m, strlen(m)); (void)__unused_w;
  _exit(1);
}
static void __lang_stack_bounds(void) {
#ifdef __APPLE__
  __lang_stack_hi = (char*) pthread_get_stackaddr_np(pthread_self());
  __lang_stack_lo = __lang_stack_hi - pthread_get_stacksize_np(pthread_self());
#else
  pthread_attr_t at;
  void* lo; size_t sz;
  if (pthread_getattr_np(pthread_self(), &at) == 0) {
    if (pthread_attr_getstack(&at, &lo, &sz) == 0) {
      __lang_stack_lo = (char*)lo; __lang_stack_hi = __lang_stack_lo + sz;
    }
    pthread_attr_destroy(&at);
  }
#endif
}
static void __lang_install_segv(void) {
  stack_t ss;
  ss.ss_sp = __lang_sigstack;
  ss.ss_size = sizeof(__lang_sigstack);
  ss.ss_flags = 0;
  if (sigaltstack(&ss, 0) != 0) return;
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = __lang_segv;
  sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGSEGV, &sa, 0);
  sigaction(SIGBUS, &sa, 0);
}
static void* __lang_thread_segv_enter(void) {
  __lang_stack_bounds();
  void* alt = malloc(65536);
  if (!alt) return 0;
  stack_t ss;
  ss.ss_sp = alt;
  ss.ss_size = 65536;
  ss.ss_flags = 0;
  if (sigaltstack(&ss, 0) != 0) { free(alt); return 0; }
  return alt;
}
static void __lang_thread_segv_leave(void* alt) {
  if (!alt) return;
  stack_t ss;
  memset(&ss, 0, sizeof(ss));
  ss.ss_flags = SS_DISABLE;
  sigaltstack(&ss, 0);
  free(alt);
}
__attribute__((noreturn)) static void __lang_fail_impl(const char* msg) {
  /* v0.1.67 (mere-ruby dogfood): print only when the failure is NOT
     caught by an active try_or — a caught fail is control flow, not an
     error, so it must be silent (matching the LLVM backend and the
     interpreter). Previously this printed unconditionally, leaking a
     'fail: ...' line on every try_or-caught failure. */
  if (__lang_fail_jmpbuf_set) {
    snprintf(__lang_fail_msg, sizeof __lang_fail_msg, "%s", msg);
    _longjmp(__lang_fail_jmpbuf, 1);
  }
  fprintf(stderr, "%s\n", msg);
  { const char* t = getenv("MERE_FAIL_TRAP");
    if (t && *t && strcmp(t, "0") != 0) raise(SIGTRAP); }
  exit(1);
}
static int __lang_fail_int(const char* msg) {
  __lang_fail_impl(msg); return 0;
}
static const char* __lang_fail_str(const char* msg) {
  __lang_fail_impl(msg); return __lang_str_of_cstr("");
}

static const char* __lang_substring(const char* s, int64_t start, int64_t end_) {
  long long __n = (long long)__lang_str_size(s);
  if (start < 0 || end_ > __n || start > end_)
    __lang_fail_range("substring: range [%lld, %lld) invalid for str of length %lld",
                      (long long)start, (long long)end_, __n);
  int64_t len = end_ - start;
  if (len < 0) len = 0;
  char* r = __lang_str_alloc(__lang_current_region, (size_t)len);
  memcpy(r, s + start, (size_t)len);
  r[len] = '\0';
  return r;
}

static const char* __lang_str_escape(const char* s) {
  size_t n = __lang_str_size(s);
  char* r = __lang_str_alloc(__lang_current_region, n * 2);
  size_t j = 0;
  for (size_t i = 0; i < n; i++) {
    char c = s[i];
    switch (c) {
      case '\n': r[j++] = '\\'; r[j++] = 'n'; break;
      case '\t': r[j++] = '\\'; r[j++] = 't'; break;
      case '\r': r[j++] = '\\'; r[j++] = 'r'; break;
      case '\\': r[j++] = '\\'; r[j++] = '\\'; break;
      case '\"': r[j++] = '\\'; r[j++] = '\"'; break;
      default:   r[j++] = c; break;
    }
  }
  r[j] = '\0';
  return r;
}

static long long __lang_str_count(const char* s, const char* n) {
  if (n[0] == '\0') return 0;
  size_t slen = __lang_str_size(s);
  size_t nlen = __lang_str_size(n);
  int acc = 0;
  for (size_t i = 0; i + nlen <= slen; ) {
    if (memcmp(s + i, n, nlen) == 0) { acc++; i += nlen; }
    else i++;
  }
  return acc;
}

static const char* __lang_str_trim(const char* s) {
  size_t n = __lang_str_size(s);
  size_t a = 0;
  while (a < n && (s[a] == ' ' || s[a] == '\t' || s[a] == '\n' || s[a] == '\r' || s[a] == '\x0c')) a++;
  size_t b = n;
  while (b > a) {
    char c = s[b - 1];
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\x0c') b--;
    else break;
  }
  size_t len = b - a;
  if (a == 0 && len == n) return s;
  char* buf = __lang_str_alloc(__lang_current_region, len);
  if (len > 0) memcpy(buf, s + a, len);
  return buf;
}

static int __lang_str_starts_with(const char* s, const char* p) {
  size_t pl = __lang_str_size(p);
  if (__lang_str_size(s) < pl) return 0;
  return memcmp(s, p, pl) == 0;
}

static int __lang_str_ends_with(const char* s, const char* p) {
  size_t sl = __lang_str_size(s);
  size_t pl = __lang_str_size(p);
  if (pl > sl) return 0;
  return memcmp(s + sl - pl, p, pl) == 0;
}

static const char* __lang_str_repeat(const char* s, int64_t n) {
  if (n < 0)
    __lang_fail_num("str_repeat: negative count %lld", (long long)n);
  if (n == 0) return __lang_str_of_cstr("");
  size_t sl = __lang_str_size(s);
  if (sl && (size_t)n > ((size_t)-1 - 1) / sl) __lang_fail_impl("str_repeat: result too large");
  size_t total = sl * (size_t)n;
  char* buf = __lang_str_alloc(__lang_current_region, total);
  for (int64_t i = 0; i < n; i++) memcpy(buf + (size_t)i * sl, s, sl);
  buf[total] = '\0';
  return buf;
}

static const char* __lang_str_rev(const char* s) {
  size_t sl = __lang_str_size(s);
  char* buf = __lang_str_alloc(__lang_current_region, sl);
  for (size_t i = 0; i < sl; i++) buf[i] = s[sl - 1 - i];
  buf[sl] = '\0';
  return buf;
}

static const char* __lang_char_at_chr(long long n) {
  __lang_char_table_setup();
  if (n < 0 || n > 255)
    __lang_fail_num("chr: %lld out of byte range [0, 255]", n);
  return __lang_char_table[(unsigned char)n].__c;
}

static const char* __lang_to_upper(const char* s) {
  size_t sl = __lang_str_size(s);
  char* buf = __lang_str_alloc(__lang_current_region, sl);
  for (size_t i = 0; i < sl; i++) {
    char c = s[i];
    buf[i] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
  }
  buf[sl] = '\0';
  return buf;
}
static const char* __lang_to_lower(const char* s) {
  size_t sl = __lang_str_size(s);
  char* buf = __lang_str_alloc(__lang_current_region, sl);
  for (size_t i = 0; i < sl; i++) {
    char c = s[i];
    buf[i] = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
  }
  buf[sl] = '\0';
  return buf;
}

static long long __lang_idiv(long long a, long long b) {
  if (b == 0) __lang_fail_impl("division by zero");
  if (b == -1) return (long long)(0ULL - (unsigned long long)a);
  return a / b;
}
static long long __lang_imod(long long a, long long b) {
  if (b == 0) __lang_fail_impl("modulo by zero");
  if (b == -1) return 0;
  return a % b;
}

static long long __lang_gcd(long long a, long long b) {
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  while (b != 0) { long long t = b; b = a % b; a = t; }
  return a;
}

static const char* __lang_str_replace(const char* s, const char* old, const char* new_str) {
  size_t slen = __lang_str_size(s);
  size_t olen = __lang_str_size(old);
  size_t nlen = __lang_str_size(new_str);
  if (olen == 0) return s;
  /* Worst-case size: every char becomes new_str-length. */
  size_t cap = slen;
  if (nlen > olen) cap += (slen / olen) * (nlen - olen) + nlen;
  char* buf = __lang_str_alloc(__lang_current_region, cap);
  size_t bi = 0;
  for (size_t i = 0; i < slen; ) {
    if (i + olen <= slen && memcmp(s + i, old, olen) == 0) {
      memcpy(buf + bi, new_str, nlen); bi += nlen; i += olen;
    } else {
      buf[bi++] = s[i++];
    }
  }
  ((size_t*)buf)[-1] = bi;
  buf[bi] = '\0';
  return buf;
}

static const char* __lang_read_file(const char* path) {
  FILE* f = fopen(path, "rb");
  if (!f) __lang_fail_impl(path);
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buf = __lang_str_alloc(__lang_current_region, (size_t)len);
  if (len > 0) { size_t r = fread(buf, 1, (size_t)len, f); (void)r; }
  buf[len] = '\0';
  fclose(f);
  return buf;
}
static const char* __lang_read_stdin(void) {
  size_t cap = 4096, n = 0;
  char* tmp = (char*)malloc(cap);
  size_t r;
  while ((r = fread(tmp + n, 1, cap - n, stdin)) > 0) {
    n += r;
    if (n == cap) { cap *= 2; tmp = (char*)realloc(tmp, cap); }
  }
  char* buf = __lang_str_alloc(__lang_current_region, n);
  memcpy(buf, tmp, n); buf[n] = '\0';
  free(tmp);
  return buf;
}
static const char* __lang_read_line(void) {
  /* One stdin line, without the trailing newline; "" on EOF (matching
     the interpreter's input_line + End_of_file catch). A final line
     with no trailing newline is returned as-is. The result lands in
     the default region like every other native string. */
  size_t cap = 256, n = 0;
  char* tmp = (char*)malloc(cap);
  int c;
  while ((c = fgetc(stdin)) != EOF && c != '\n') {
    if (n + 1 == cap) { cap *= 2; tmp = (char*)realloc(tmp, cap); }
    tmp[n++] = (char)c;
  }
  char* buf = __lang_str_alloc(__lang_current_region, n);
  memcpy(buf, tmp, n); buf[n] = '\0';
  free(tmp);
  return buf;
}
static int __lang_write_file(const char* path, const char* content) {
  FILE* f = fopen(path, "wb");
  if (!f) __lang_fail_impl(path);
  size_t len = __lang_str_size(content);
  if (len > 0) { size_t w = fwrite(content, 1, len, f); (void)w; }
  fclose(f);
  return 0;
}

#include <sys/wait.h>
#include <spawn.h>
extern char **environ;
static int __lang_run(const char* cmd) {
  pid_t pid;
  char* argv[] = { "sh", "-c", (char*)cmd, NULL };
  if (posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, environ) != 0)
    return -1;
  int st;
  if (waitpid(pid, &st, 0) < 0) return -1;
  if (WIFEXITED(st)) return WEXITSTATUS(st);
  if (WIFSIGNALED(st)) return 128 + WTERMSIG(st);
  return -1;
}

#include <termios.h>
static struct termios __lang_saved_tio;
static int __lang_tio_saved = 0;
static int __lang_tty_raw(void) {
  if (!isatty(0)) return 0;
  struct termios tio;
  if (tcgetattr(0, &tio) != 0) return 0;
  if (!__lang_tio_saved) { __lang_saved_tio = tio; __lang_tio_saved = 1; }
  tio.c_lflag &= ~(ICANON | ECHO | IEXTEN);
  tio.c_iflag &= ~(IXON | IXOFF);
  tio.c_cc[VMIN] = 1; tio.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &tio);
  return 0;
}
static int __lang_tty_no_signal_keys(void) {
  if (!isatty(0)) return 0;
  struct termios tio;
  if (tcgetattr(0, &tio) != 0) return 0;
  if (!__lang_tio_saved) { __lang_saved_tio = tio; __lang_tio_saved = 1; }
  tio.c_lflag &= ~ISIG;
  tcsetattr(0, TCSANOW, &tio);
  return 0;
}
static int __lang_tty_restore(void) {
  if (__lang_tio_saved && isatty(0)) tcsetattr(0, TCSANOW, &__lang_saved_tio);
  return 0;
}
#include <sys/select.h>
static int __lang_stdin_byte(void) {
  fd_set fds; struct timeval tv;
  FD_ZERO(&fds); FD_SET(0, &fds);
  tv.tv_sec = 0; tv.tv_usec = 0;
  if (select(1, &fds, NULL, NULL, &tv) <= 0) return -1;
  unsigned char c;
  ssize_t n = read(0, &c, 1);
  if (n <= 0) return -1;
  return (int)c;
}
static const char* __lang_read_key(void) {
  char c;
  ssize_t n = read(0, &c, 1);
  if (n <= 0) return __lang_str_alloc(__lang_current_region, 0);
  char* s = __lang_str_alloc(__lang_current_region, 1);
  s[0] = c;
  return s;
}

#include <time.h>
static long long __lang_random_int(long long n) {
  static int __seeded = 0;
  if (!__seeded) { srand((unsigned)(time(NULL) ^ getpid())); __seeded = 1; }
  if (n <= 0) __lang_fail_impl("random_int: bound must be positive");
  return ((long long)rand() * RAND_MAX + rand()) % n;
}

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
static double __lang_file_mtime(const char* path) {
  struct stat st;
  if (stat(path, &st) != 0) __lang_fail_impl(path);
  return (double)st.st_mtime;
}
static long long __lang_file_size(const char* path) {
  struct stat st;
  if (stat(path, &st) != 0) __lang_fail_impl(path);
  return (long long)st.st_size;
}
static int __lang_file_exists(const char* path) {
  struct stat st;
  return stat(path, &st) == 0;
}
static int __lang_sleep_ms(long long ms) {
  if (ms <= 0) return 0;
  usleep((useconds_t)ms * 1000);
  return 0;
}
static int __lang_mkdir_p(const char* path) {
  /* recursive mkdir -p: skip if exists; on failure call __lang_fail_impl */
  size_t n = __lang_str_size(path);
  if (n == 0) return 0;
  char* buf = (char*)malloc(n + 1);
  memcpy(buf, path, n + 1);
  for (size_t i = 1; i <= n; i++) {
    if (buf[i] == '/' || buf[i] == '\0') {
      char saved = buf[i];
      buf[i] = '\0';
      if (mkdir(buf, 0755) != 0 && errno != EEXIST) {
        free(buf);
        __lang_fail_impl(path);
      }
      buf[i] = saved;
    }
  }
  free(buf);
  return 0;
}

static double __lang_float_of_str_checked(const char* s) {
  size_t __n = __lang_str_size(s);
  char __b[512];
  char* __d = __b;
  size_t __cap = sizeof __b - 1;
  char* __heap = 0;
  if (__n > __cap) { __heap = (char*)malloc(__n + 1);
    if (!__heap) __lang_fail_impl("out of memory"); __d = __heap; }
  { size_t __i = 0, __j = 0;
    while (__i < __n && (s[__i] == ' ' || s[__i] == '\t' || s[__i] == '\r' || s[__i] == '\n')) __i++;
    size_t __e = __n;
    while (__e > __i && (s[__e-1] == ' ' || s[__e-1] == '\t' || s[__e-1] == '\r' || s[__e-1] == '\n')) __e--;
    for (; __i < __e; __i++) if (s[__i] != '_') __d[__j++] = s[__i];
    __d[__j] = '\0'; }
  char* end = 0;
  double v = strtod(__d, &end);
  const char* p = __d;
  const char* r = end;
  int __bad = (end == p || *r != '\0');
  if (__heap) free(__heap);
  if (__bad) {
    char __m[160];
    snprintf(__m, sizeof __m, "float_of_str: \"%s\" is not a valid float", s);
    __lang_fail_impl(__m);
  }
  return v;
}
static int __lang_bool_of_str(const char* s) {
  if (strcmp(s, "true") == 0) return 1;
  if (strcmp(s, "false") == 0) return 0;
  { char __m[160];
    snprintf(__m, sizeof __m, "bool_of_str: \"%s\" is not 'true' or 'false'", s);
    __lang_fail_impl(__m); }
  return 0;
}
static long long __lang_ord(const char* s) {
  long long __n = (long long)__lang_str_size(s);
  if (__n != 1)
    __lang_fail_num("ord: expected single-char str, got length %lld", __n);
  return (long long)(unsigned char)s[0];
}
static const char* __lang_str_unescape(const char* s) {
  size_t n = __lang_str_size(s);
  { size_t k = 0; while (k < n && s[k] != '\\') k++; if (k == n) return s; }
  char* r = __lang_str_alloc(__lang_current_region, n);
  size_t i = 0, j = 0;
  while (i < n) {
    if (s[i] == '\\' && i + 1 < n) {
      char c = s[i + 1];
      switch (c) {
        case 'n': r[j++] = '\n'; break;
        case 't': r[j++] = '\t'; break;
        case 'r': r[j++] = '\r'; break;
        case '\\': r[j++] = '\\'; break;
        case '"': r[j++] = '"'; break;
        case '/': r[j++] = '/'; break;
        default: __lang_fail_chr("str_unescape: unknown escape \'\\%c\'", c); break;
      }
      i += 2;
    } else {
      r[j++] = s[i++];
    }
  }
  r[j] = '\0';
  ((size_t*)r)[-1] = j;
  return r;
}

__attribute__((noinline)) static const char* __lang_str_of_float(double f) {
  char tmp[32];
  for (int p = 12; ; p++) {
    snprintf(tmp, sizeof tmp, "%.*g", p, f);
    if (p >= 17 || strtod(tmp, NULL) == f) break;
  }
  int has_dot = 0;
  for (char* p = tmp; *p; p++) {
    if (*p == '.' || *p == 'e' || *p == 'E' || *p == 'n' || *p == 'i') {
      has_dot = 1; break;
    }
  }
  char* buf;
  asprintf(&buf, has_dot ? "%s" : "%s.0", tmp);
  return __lang_str_take_cstr(buf);
}

static double __lang_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

typedef struct option_int option_int;
typedef struct list_str_node list_str_node;
typedef list_str_node* list_str;
#define list_str__tag(v) ((int)(((uintptr_t)(v)) & (uintptr_t)7))
#define list_str__node(v) ((list_str_node*)(((uintptr_t)(v)) & ~(uintptr_t)7))
#define list_str__mk(t, p) ((list_str)(((uintptr_t)(p)) | (uintptr_t)(t)))
typedef struct result_int_int result_int_int;
typedef struct list_list_int_node list_list_int_node;
typedef list_list_int_node* list_list_int;
#define list_list_int__tag(v) ((int)(((uintptr_t)(v)) & (uintptr_t)7))
#define list_list_int__node(v) ((list_list_int_node*)(((uintptr_t)(v)) & ~(uintptr_t)7))
#define list_list_int__mk(t, p) ((list_list_int)(((uintptr_t)(p)) | (uintptr_t)(t)))
typedef struct list_tuple_int_int_node list_tuple_int_int_node;
typedef list_tuple_int_int_node* list_tuple_int_int;
#define list_tuple_int_int__tag(v) ((int)(((uintptr_t)(v)) & (uintptr_t)7))
#define list_tuple_int_int__node(v) ((list_tuple_int_int_node*)(((uintptr_t)(v)) & ~(uintptr_t)7))
#define list_tuple_int_int__mk(t, p) ((list_tuple_int_int)(((uintptr_t)(p)) | (uintptr_t)(t)))
typedef struct list_int_node list_int_node;
typedef list_int_node* list_int;
#define list_int__tag(v) ((int)(((uintptr_t)(v)) & (uintptr_t)7))
#define list_int__node(v) ((list_int_node*)(((uintptr_t)(v)) & ~(uintptr_t)7))
#define list_int__mk(t, p) ((list_int)(((uintptr_t)(p)) | (uintptr_t)(t)))

typedef struct tuple_int_int tuple_int_int;
typedef struct tuple_int_int_int tuple_int_int_int;
typedef struct tuple_tuple_int_int_list_tuple_int_int tuple_tuple_int_int_list_tuple_int_int;
typedef struct tuple_int_list_int tuple_int_list_int;
typedef struct tuple_Vec_int_Vec_int tuple_Vec_int_Vec_int;
typedef struct tuple_str_list_str tuple_str_list_str;
typedef struct tuple_list_int_list_list_int tuple_list_int_list_list_int;

typedef struct mere_vec_int mere_vec_int;
typedef struct mere_vec_str mere_vec_str;

typedef struct mere_strbuf mere_strbuf;

typedef struct mere_bytes mere_bytes;
typedef struct __lang_coro __lang_coro;
typedef struct mere_bytebuf mere_bytebuf;

typedef struct {
  void* env;
  long long (*fn)(void*, long long);
} closure_int_int;
typedef struct {
  void* env;
  closure_int_int (*fn)(void*, long long);
  long long (*fn2)(void*, long long, long long);
} closure_int_closure_int_int;
typedef struct {
  void* env;
  long long (*fn)(void*, closure_int_closure_int_int);
} closure_closure_int_closure_int_int_int;
typedef struct {
  void* env;
  closure_closure_int_closure_int_int_int (*fn)(void*, long long);
  long long (*fn2)(void*, long long, closure_int_closure_int_int);
} closure_int_closure_closure_int_closure_int_int_int;
typedef struct {
  void* env;
  closure_int_closure_closure_int_closure_int_int_int (*fn)(void*, list_int);
  closure_closure_int_closure_int_int_int (*fn2)(void*, list_int, long long);
} closure_list_int_closure_int_closure_closure_int_closure_int_int_int;
typedef struct {
  void* env;
  list_int (*fn)(void*, list_int);
} closure_list_int_list_int;
typedef struct {
  void* env;
  closure_list_int_list_int (*fn)(void*, long long);
  list_int (*fn2)(void*, long long, list_int);
} closure_int_closure_list_int_list_int;
typedef struct {
  void* env;
  closure_int_closure_list_int_list_int (*fn)(void*, long long);
  closure_list_int_list_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_list_int_list_int;
typedef struct {
  void* env;
  list_int (*fn)(void*, long long);
} closure_int_list_int;
typedef struct {
  void* env;
  closure_int_list_int (*fn)(void*, long long);
  list_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_list_int;
typedef struct {
  void* env;
  long long (*fn)(void*, list_int);
} closure_list_int_int;
typedef struct {
  void* env;
  const char* (*fn)(void*, long long);
} closure_int_str;
typedef struct {
  void* env;
  closure_int_str (*fn)(void*, list_str);
  const char* (*fn2)(void*, list_str, long long);
} closure_list_str_closure_int_str;
typedef struct {
  void* env;
  closure_int_str (*fn)(void*, const char*);
  const char* (*fn2)(void*, const char*, long long);
} closure_str_closure_int_str;
typedef struct {
  void* env;
  list_str (*fn)(void*, const char*);
} closure_str_list_str;
typedef struct {
  void* env;
  int (*fn)(void*, long long);
} closure_int_bool;
typedef struct {
  void* env;
  const char* (*fn)(void*, const char*);
} closure_str_str;
typedef struct {
  void* env;
  closure_int_int (*fn)(void*, const char*);
  long long (*fn2)(void*, const char*, long long);
} closure_str_closure_int_int;
typedef struct {
  void* env;
  long long (*fn)(void*, const char*);
} closure_str_int;
typedef struct {
  void* env;
  int (*fn)(void*, int);
} closure_bool_bool;
typedef struct {
  void* env;
  closure_str_str (*fn)(void*, long long);
  const char* (*fn2)(void*, long long, const char*);
} closure_int_closure_str_str;
typedef struct {
  void* env;
  closure_int_closure_str_str (*fn)(void*, long long);
  closure_str_str (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_str_str;
typedef struct {
  void* env;
  closure_int_closure_int_closure_str_str (*fn)(void*, list_str);
  closure_int_closure_str_str (*fn2)(void*, list_str, long long);
} closure_list_str_closure_int_closure_int_closure_str_str;
typedef struct {
  void* env;
  closure_int_str (*fn)(void*, long long);
  const char* (*fn2)(void*, long long, long long);
} closure_int_closure_int_str;
typedef struct {
  void* env;
  closure_int_closure_int_str (*fn)(void*, const char*);
  closure_int_str (*fn2)(void*, const char*, long long);
} closure_str_closure_int_closure_int_str;
typedef struct {
  void* env;
  closure_str_str (*fn)(void*, list_str);
  const char* (*fn2)(void*, list_str, const char*);
} closure_list_str_closure_str_str;
typedef struct {
  void* env;
  closure_int_closure_int_int (*fn)(void*, long long);
  closure_int_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_int (*fn)(void*, const char*);
  closure_int_closure_int_int (*fn2)(void*, const char*, long long);
} closure_str_closure_int_closure_int_closure_int_int;
typedef struct {
  void* env;
  tuple_int_int (*fn)(void*, long long);
} closure_int_tuple_int_int;
typedef struct {
  void* env;
  closure_int_tuple_int_int (*fn)(void*, long long);
  tuple_int_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_tuple_int_int;
typedef struct {
  void* env;
  tuple_int_int (*fn)(void*, const char*);
} closure_str_tuple_int_int;
typedef struct {
  void* env;
  int (*fn)(void*, const char*);
} closure_str_unit;
typedef struct {
  void* env;
  closure_str_unit (*fn)(void*, int);
  int (*fn2)(void*, int, const char*);
} closure_bool_closure_str_unit;
typedef struct {
  void* env;
  long long (*fn)(void*, mere_vec_int*);
} closure_Vec_int_int;
typedef struct {
  void* env;
  closure_int_int (*fn)(void*, mere_vec_int*);
  long long (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_int;
typedef struct {
  void* env;
  mere_vec_int* (*fn)(void*, long long);
} closure_int_Vec_int;
typedef struct {
  void* env;
  mere_vec_int* (*fn)(void*, int);
} closure_unit_Vec_int;
typedef struct {
  void* env;
  int (*fn)(void*, long long);
} closure_int_unit;
typedef struct {
  void* env;
  closure_int_unit (*fn)(void*, mere_vec_int*);
  int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_int (*fn)(void*, mere_vec_int*);
  long long (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_int;
typedef struct {
  void* env;
  closure_int_unit (*fn)(void*, long long);
  int (*fn2)(void*, long long, long long);
} closure_int_closure_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_unit (*fn)(void*, mere_vec_int*);
  closure_int_unit (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_int_int (*fn)(void*, mere_vec_int*);
  closure_int_int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_int_int;
typedef struct {
  void* env;
  int (*fn)(void*, mere_vec_int*);
} closure_Vec_int_unit;
typedef struct {
  void* env;
  tuple_Vec_int_Vec_int (*fn)(void*, mere_vec_int*);
} closure_Vec_int_tuple_Vec_int_Vec_int;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_int (*fn)(void*, mere_vec_int*);
  closure_Vec_int_int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_int;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_closure_Vec_int_int (*fn)(void*, mere_vec_int*);
  closure_Vec_int_closure_Vec_int_int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_int (*fn)(void*, long long);
  closure_int_closure_int_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_int_int;
typedef struct {
  void* env;
  mere_vec_int* (*fn)(void*, list_int);
} closure_list_int_Vec_int;
typedef struct {
  void* env;
  closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  closure_Vec_int_unit (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  closure_Vec_int_closure_Vec_int_unit (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  int (*fn)(void*, int);
} closure_unit_unit;
typedef struct {
  void* env;
  closure_Vec_int_tuple_Vec_int_Vec_int (*fn)(void*, mere_vec_int*);
  tuple_Vec_int_Vec_int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_tuple_Vec_int_Vec_int;
typedef struct {
  void* env;
  closure_int_Vec_int (*fn)(void*, mere_vec_int*);
  mere_vec_int* (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_Vec_int;
typedef struct {
  void* env;
  int (*fn)(void*, mere_vec_int*);
} closure_Vec_int_bool;
typedef struct {
  void* env;
  mere_vec_int* (*fn)(void*, mere_vec_int*);
} closure_Vec_int_Vec_int;
typedef struct {
  void* env;
  closure_Vec_int_bool (*fn)(void*, mere_vec_int*);
  int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_bool;
typedef struct {
  void* env;
  closure_int_Vec_int (*fn)(void*, long long);
  mere_vec_int* (*fn2)(void*, long long, long long);
} closure_int_closure_int_Vec_int;
typedef struct {
  void* env;
  closure_int_bool (*fn)(void*, long long);
  int (*fn2)(void*, long long, long long);
} closure_int_closure_int_bool;
typedef struct {
  void* env;
  closure_int_closure_int_bool (*fn)(void*, long long);
  closure_int_bool (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_bool;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_bool (*fn)(void*, long long);
  closure_int_closure_int_bool (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_int_bool;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_closure_int_bool (*fn)(void*, long long);
  closure_int_closure_int_closure_int_bool (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_int_closure_int_bool;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_closure_int_closure_int_bool (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_closure_int_closure_int_bool (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_closure_int_closure_int_closure_int_bool;
typedef struct {
  void* env;
  closure_int_bool (*fn)(void*, mere_vec_int*);
  int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_bool;
typedef struct {
  void* env;
  mere_vec_str* (*fn)(void*, mere_vec_int*);
} closure_Vec_int_Vec_str;
typedef struct {
  void* env;
  mere_vec_str* (*fn)(void*, int);
} closure_unit_Vec_str;
typedef struct {
  void* env;
  mere_vec_str* (*fn)(void*, long long);
} closure_int_Vec_str;
typedef struct {
  void* env;
  mere_strbuf* (*fn)(void*, int);
} closure_unit_StrBuf;
typedef struct {
  void* env;
  int (*fn)(void*, int);
} closure_bool_unit;
typedef struct {
  void* env;
  closure_bool_unit (*fn)(void*, long long);
  int (*fn2)(void*, long long, int);
} closure_int_closure_bool_unit;
typedef struct {
  void* env;
  closure_str_unit (*fn)(void*, mere_strbuf*);
  int (*fn2)(void*, mere_strbuf*, const char*);
} closure_StrBuf_closure_str_unit;
typedef struct {
  void* env;
  closure_str_unit (*fn)(void*, mere_vec_str*);
  int (*fn2)(void*, mere_vec_str*, const char*);
} closure_Vec_str_closure_str_unit;
typedef struct {
  void* env;
  const char* (*fn)(void*, mere_strbuf*);
} closure_StrBuf_str;
typedef struct {
  void* env;
  mere_bytebuf* (*fn)(void*, long long);
} closure_int_ByteBuf;
typedef struct {
  void* env;
  closure_int_ByteBuf (*fn)(void*, long long);
  mere_bytebuf* (*fn2)(void*, long long, long long);
} closure_int_closure_int_ByteBuf;
typedef struct {
  void* env;
  closure_int_closure_int_ByteBuf (*fn)(void*, long long);
  closure_int_ByteBuf (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_ByteBuf;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_ByteBuf (*fn)(void*, long long);
  closure_int_closure_int_ByteBuf (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_int_ByteBuf;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_closure_int_ByteBuf (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_closure_int_ByteBuf (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_closure_int_closure_int_ByteBuf;
typedef struct {
  void* env;
  closure_int_int (*fn)(void*, mere_bytebuf*);
  long long (*fn2)(void*, mere_bytebuf*, long long);
} closure_ByteBuf_closure_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_unit (*fn)(void*, mere_bytebuf*);
  closure_int_unit (*fn2)(void*, mere_bytebuf*, long long);
} closure_ByteBuf_closure_int_closure_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_ByteBuf (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_ByteBuf (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_closure_int_ByteBuf;
typedef struct {
  void* env;
  mere_bytebuf* (*fn)(void*, mere_vec_int*);
} closure_Vec_int_ByteBuf;
typedef struct {
  void* env;
  closure_Vec_int_closure_int_closure_int_unit (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_unit (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_int_closure_int_unit;
typedef struct {
  void* env;
  tuple_int_int_int (*fn)(void*, long long);
} closure_int_tuple_int_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_tuple_int_int (*fn)(void*, mere_vec_int*);
  closure_int_tuple_int_int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_tuple_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_tuple_int_int (*fn)(void*, long long);
  closure_int_tuple_int_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_tuple_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_Vec_int (*fn)(void*, mere_vec_int*);
  closure_int_Vec_int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_Vec_int;
typedef struct {
  void* env;
  closure_int_closure_int_int (*fn)(void*, mere_vec_int*);
  closure_int_int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_int;
typedef struct {
  void* env;
  closure_Vec_int_unit (*fn)(void*, long long);
  int (*fn2)(void*, long long, mere_vec_int*);
} closure_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_int_closure_Vec_int_unit (*fn)(void*, long long);
  closure_Vec_int_unit (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_closure_Vec_int_unit (*fn)(void*, long long);
  closure_int_closure_Vec_int_unit (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_closure_Vec_int_unit (*fn)(void*, long long);
  closure_int_closure_int_closure_Vec_int_unit (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_closure_int_closure_Vec_int_unit (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit;
typedef struct {
  void* env;
  closure_Vec_int_closure_int_Vec_int (*fn)(void*, mere_vec_int*);
  closure_int_Vec_int (*fn2)(void*, mere_vec_int*, mere_vec_int*);
} closure_Vec_int_closure_Vec_int_closure_int_Vec_int;
typedef struct {
  void* env;
  closure_Vec_int_closure_Vec_int_closure_int_Vec_int (*fn)(void*, long long);
  closure_Vec_int_closure_int_Vec_int (*fn2)(void*, long long, mere_vec_int*);
} closure_int_closure_Vec_int_closure_Vec_int_closure_int_Vec_int;
typedef struct {
  void* env;
  closure_Vec_int_Vec_int (*fn)(void*, long long);
  mere_vec_int* (*fn2)(void*, long long, mere_vec_int*);
} closure_int_closure_Vec_int_Vec_int;
typedef struct {
  void* env;
  closure_int_closure_Vec_int_Vec_int (*fn)(void*, long long);
  closure_Vec_int_Vec_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_Vec_int_Vec_int;
typedef struct {
  void* env;
  closure_int_closure_int_closure_Vec_int_Vec_int (*fn)(void*, long long);
  closure_int_closure_Vec_int_Vec_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_Vec_int_Vec_int;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_int (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_closure_int_int;
typedef struct {
  void* env;
  closure_int_closure_int_Vec_int (*fn)(void*, long long);
  closure_int_Vec_int (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_Vec_int;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_Vec_int (*fn)(void*, mere_vec_int*);
  closure_int_closure_int_Vec_int (*fn2)(void*, mere_vec_int*, long long);
} closure_Vec_int_closure_int_closure_int_closure_int_Vec_int;
typedef struct {
  void* env;
  long long (*fn)(void*, mere_vec_str*);
} closure_Vec_str_int;
typedef struct {
  void* env;
  closure_int_str (*fn)(void*, mere_vec_str*);
  const char* (*fn2)(void*, mere_vec_str*, long long);
} closure_Vec_str_closure_int_str;
typedef struct {
  void* env;
  closure_int_unit (*fn)(void*, const char*);
  int (*fn2)(void*, const char*, long long);
} closure_str_closure_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_unit (*fn)(void*, long long);
  closure_int_unit (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_unit;
typedef struct {
  void* env;
  closure_int_closure_int_closure_int_unit (*fn)(void*, long long);
  closure_int_closure_int_unit (*fn2)(void*, long long, long long);
} closure_int_closure_int_closure_int_closure_int_unit;
typedef struct {
  void* env;
  mere_vec_int* (*fn)(void*, const char*);
} closure_str_Vec_int;
typedef struct {
  void* env;
  mere_vec_int* (*fn)(void*, mere_bytes*);
} closure_bytes_Vec_int;
typedef struct {
  void* env;
  mere_bytes* (*fn)(void*, const char*);
} closure_str_bytes;
typedef struct {
  void* env;
  closure_str_unit (*fn)(void*, const char*);
  int (*fn2)(void*, const char*, const char*);
} closure_str_closure_str_unit;
typedef struct {
  void* env;
  mere_bytes* (*fn)(void*, mere_vec_int*);
} closure_Vec_int_bytes;
typedef struct {
  void* env;
  int (*fn)(void*, mere_bytes*);
} closure_bytes_unit;
typedef struct {
  void* env;
  closure_bytes_unit (*fn)(void*, const char*);
  int (*fn2)(void*, const char*, mere_bytes*);
} closure_str_closure_bytes_unit;
typedef struct {
  void* env;
  long long (*fn)(void*, mere_bytes*);
} closure_bytes_int;
typedef struct {
  void* env;
  list_str (*fn)(void*, int);
} closure_unit_list_str;
typedef struct {
  void* env;
  const char* (*fn)(void*, mere_bytes*);
} closure_bytes_str;
typedef struct {
  void* env;
  mere_bytes* (*fn)(void*, mere_bytes*);
} closure_bytes_bytes;

struct option_int {
  int tag;
  union {
    long long Some;
  } payload;
};
struct tuple_str_list_str {
  const char* f0;
  list_str f1;
};
struct list_str_node {
  union {
    tuple_str_list_str Cons;
  } payload;
};
static struct list_str_node __mshared_list_str_t0;
struct result_int_int {
  int tag;
  union {
    long long Ok;
    long long Err;
  } payload;
};
struct tuple_list_int_list_list_int {
  list_int f0;
  list_list_int f1;
};
struct list_list_int_node {
  union {
    tuple_list_int_list_list_int Cons;
  } payload;
};
static struct list_list_int_node __mshared_list_list_int_t0;
struct tuple_int_int {
  long long f0;
  long long f1;
};
struct tuple_tuple_int_int_list_tuple_int_int {
  tuple_int_int f0;
  list_tuple_int_int f1;
};
struct list_tuple_int_int_node {
  union {
    tuple_tuple_int_int_list_tuple_int_int Cons;
  } payload;
};
static struct list_tuple_int_int_node __mshared_list_tuple_int_int_t0;
struct tuple_int_list_int {
  long long f0;
  list_int f1;
};
struct list_int_node {
  union {
    tuple_int_list_int Cons;
  } payload;
};
static struct list_int_node __mshared_list_int_t0;
struct tuple_Vec_int_Vec_int {
  mere_vec_int* f0;
  mere_vec_int* f1;
};
struct tuple_int_int_int {
  long long f0;
  long long f1;
  long long f2;
};

typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_int mu_xs;
} __anon_2_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_3_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_4_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_str mu_cs;
} __anon_7_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_8_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_9_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_10_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_11_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_str mu_cs;
} __anon_12_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_13_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_str mu_cs;
} __anon_14_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_15_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_16_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
} __anon_17_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_18_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_acc;
} __anon_19_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_b;
} __anon_20_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_21_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_22_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  int mu_cond;
} __anon_23_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_crc;
} __anon_24_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_25_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_26_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_27_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_28_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_29_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_30_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_31_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_32_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_n;
} __anon_33_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_34_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_35_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_36_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_37_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_38_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_39_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_40_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_41_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_42_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_colour;
} __anon_43_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_44_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_raw;
} __anon_45_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_raw;
} __anon_46_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_47_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_p_uq26;
} __anon_48_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_p_uq27;
} __anon_49_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_raw;
} __anon_50_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_crc;
} __anon_51_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_52_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
} __anon_53_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_v;
} __anon_54_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
} __anon_55_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_freq;
} __anon_56_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_lengths;
} __anon_57_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_v;
} __anon_58_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
} __anon_59_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
} __anon_60_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
} __anon_61_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_62_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_kind;
} __anon_63_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_row_uq63;
} __anon_64_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
} __anon_65_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
} __anon_66_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pal;
} __anon_67_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
} __anon_68_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
} __anon_69_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
} __anon_70_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_path;
} __anon_71_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
} __anon_72_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_bytes;
} __anon_73_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_bytes;
} __anon_74_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
} __anon_75_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  mere_vec_int* mu_syms;
} __anon_76_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  mere_vec_int* mu_syms;
} __anon_77_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_len;
  mere_vec_int* mu_data;
  long long mu_d;
  mere_strbuf* mu_buf;
} __anon_78_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_len;
  mere_vec_int* mu_data;
  long long mu_d;
  mere_strbuf* mu_buf;
} __anon_79_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_p_uq28;
  mere_bytebuf* mu_out;
  long long mu_width;
  long long mu_height;
  mere_vec_int* mu_raw;
  long long mu_bpp;
} __anon_80_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
} __anon_81_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_bytes;
} __anon_82_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_bytes;
} __anon_83_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  long long mu_v;
} __anon_84_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_d;
  long long mu_max_back;
  long long mu_pos;
  long long mu_n;
  mere_vec_int* mu_data;
} __anon_85_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_np;
  mere_vec_int* mu_alive;
  long long mu_skip;
  mere_vec_int* mu_nf;
} __anon_86_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_np;
  mere_vec_int* mu_alive;
  long long mu_skip;
  mere_vec_int* mu_nf;
} __anon_87_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_leafsym;
  long long mu_node;
  mere_vec_int* mu_lengths;
  mere_vec_int* mu_nl;
  mere_vec_int* mu_nr;
} __anon_88_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_bits;
  long long mu_maxbits;
  mere_vec_int* mu_bl_count;
  mere_vec_int* mu_next_code;
} __anon_89_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_bits;
  long long mu_maxbits;
  mere_vec_int* mu_bl_count;
  mere_vec_int* mu_next_code;
} __anon_90_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_v;
} __anon_91_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_v;
} __anon_92_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_v;
} __anon_93_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_data;
} __anon_94_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_data;
} __anon_95_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_row_uq60;
} __anon_96_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_row_uq60;
} __anon_97_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_kind;
  mere_vec_int* mu_row_uq63;
  mere_vec_int* mu_prev;
  long long mu_bpp;
} __anon_98_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_y;
  long long mu_height;
  long long mu_stride;
  int mu_grey;
  mere_vec_int* mu_pixels;
  long long mu_width;
  int mu_indexed;
  mere_vec_int* mu_pal;
  mere_vec_int* mu_raw;
  long long mu_bpp;
} __anon_99_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
  mere_vec_int* mu_out;
} __anon_100_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_data;
  mere_vec_int* mu_acc;
} __anon_101_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  mere_vec_str* mu_t;
} __anon_102_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  mere_vec_str* mu_t;
} __anon_103_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_104_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_105_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_int mu_xs;
  long long mu_acc;
} __anon_106_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_a;
} __anon_107_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_108_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
} __anon_109_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_str mu_cs;
  long long mu_start;
} __anon_110_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_str mu_cs;
  long long mu_start;
  long long mu_len;
} __anon_111_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  const char* mu_s;
  long long mu_start;
} __anon_112_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  const char* mu_s;
} __anon_113_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  const char* mu_s;
} __anon_114_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_acc;
  long long mu_sq;
} __anon_115_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
} __anon_116_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
} __anon_117_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
} __anon_118_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
} __anon_119_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_out;
} __anon_120_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
} __anon_121_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
} __anon_122_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
  long long mu_b;
} __anon_123_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_start;
  mere_vec_int* mu_raw;
} __anon_124_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_start;
  mere_vec_int* mu_raw;
} __anon_125_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_raw;
  long long mu_width;
} __anon_126_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_raw;
  long long mu_width;
  long long mu_height;
} __anon_127_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  mere_vec_int* mu_raw;
} __anon_128_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  mere_vec_int* mu_raw;
} __anon_129_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_out;
} __anon_130_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_v;
  mere_vec_int* mu_s;
  mere_vec_int* mu_out;
} __anon_131_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_pos;
  mere_vec_int* mu_data;
} __anon_132_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_nsym;
  mere_vec_int* mu_freq;
} __anon_133_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_n;
  mere_vec_int* mu_lengths;
} __anon_134_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
} __anon_135_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
} __anon_136_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_a;
  long long mu_b;
} __anon_137_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_row_uq58;
  long long mu_kind;
} __anon_138_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_row_uq58;
  mere_vec_int* mu_prev;
  long long mu_kind;
} __anon_139_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_row_uq63;
  mere_vec_int* mu_prev;
} __anon_140_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pal;
  long long mu_r;
} __anon_141_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pal;
  long long mu_r;
  long long mu_g;
} __anon_142_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  mere_vec_int* mu_pixels;
} __anon_143_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  mere_vec_int* mu_pixels;
} __anon_144_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
  long long mu_width;
} __anon_145_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pixels;
  long long mu_width;
} __anon_146_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  long long mu_code0;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  mere_vec_int* mu_syms;
} __anon_147_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  long long mu_code0;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  long long mu_first;
  mere_vec_int* mu_syms;
} __anon_148_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  long long mu_code0;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  mere_vec_int* mu_syms;
} __anon_149_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  long long mu_code0;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  long long mu_first;
  mere_vec_int* mu_syms;
} __anon_150_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_d;
  long long mu_max_back;
  long long mu_blen;
  long long mu_pos;
  long long mu_n;
  mere_vec_int* mu_data;
} __anon_151_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  mere_vec_int* mu_v;
} __anon_152_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  long long mu_a;
  mere_vec_int* mu_data;
} __anon_153_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  long long mu_a;
  mere_vec_int* mu_data;
} __anon_154_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_kind;
  long long mu_bk;
  mere_vec_int* mu_row_uq63;
  mere_vec_int* mu_prev;
  long long mu_bpp;
} __anon_155_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_kind;
  long long mu_bk;
  mere_vec_int* mu_row_uq63;
  mere_vec_int* mu_prev;
  long long mu_bpp;
  long long mu_bc;
} __anon_156_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_data;
  long long mu_got;
  mere_vec_int* mu_acc;
} __anon_157_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_data;
  long long mu_got;
  long long mu_cur;
  mere_vec_int* mu_acc;
} __anon_158_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  list_str mu_cs;
  long long mu_start;
  long long mu_len;
} __anon_159_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  const char* mu_s;
} __anon_160_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
} __anon_161_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_out;
} __anon_162_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_out;
} __anon_163_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_out;
} __anon_164_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
} __anon_165_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
} __anon_166_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
} __anon_167_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
} __anon_168_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
  long long mu_c;
} __anon_169_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_start;
  mere_vec_int* mu_raw;
} __anon_170_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  long long mu_start;
  mere_vec_int* mu_raw;
} __anon_171_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  long long mu_start;
  mere_vec_int* mu_raw;
} __anon_172_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_raw;
  long long mu_width;
  long long mu_height;
} __anon_173_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  mere_vec_int* mu_raw;
} __anon_174_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_v;
  mere_vec_int* mu_s;
  mere_vec_int* mu_out;
} __anon_175_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
} __anon_176_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
} __anon_177_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
} __anon_178_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
  long long mu_t3;
} __anon_179_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_row_uq58;
  mere_vec_int* mu_prev;
  long long mu_kind;
} __anon_180_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_pal;
  long long mu_r;
  long long mu_g;
} __anon_181_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  mere_vec_int* mu_pixels;
} __anon_182_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  long long mu_code0;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  long long mu_first;
  mere_vec_int* mu_syms;
} __anon_183_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_len;
  long long mu_code0;
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_count;
  long long mu_first;
  mere_vec_int* mu_syms;
} __anon_184_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_kind;
  long long mu_bk;
  mere_vec_int* mu_row_uq63;
  mere_vec_int* mu_prev;
  long long mu_bpp;
  long long mu_bc;
} __anon_185_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_i;
  long long mu_n;
  mere_vec_int* mu_data;
  long long mu_got;
  long long mu_cur;
  mere_vec_int* mu_acc;
} __anon_186_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_out;
} __anon_187_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
} __anon_188_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
} __anon_189_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_190_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
} __anon_191_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_192_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_193_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
} __anon_194_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
  long long mu_c;
} __anon_195_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
  long long mu_c;
} __anon_196_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
  long long mu_c;
} __anon_197_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  long long mu_width;
  long long mu_height;
  long long mu_start;
  mere_vec_int* mu_raw;
} __anon_198_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
} __anon_199_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
  long long mu_t3;
} __anon_200_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
  long long mu_t3;
} __anon_201_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
  long long mu_t3;
} __anon_202_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
} __anon_203_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_204_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_205_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_206_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_207_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_data;
  long long mu_i;
  long long mu_a;
  long long mu_b;
  long long mu_c;
} __anon_208_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_out;
  long long mu_t0;
  long long mu_t1;
  long long mu_t2;
  long long mu_t3;
} __anon_209_env;
typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
  mere_vec_int* mu_s;
  mere_vec_int* mu_data;
  mere_vec_int* mu_lcount;
  mere_vec_int* mu_lsyms;
  mere_vec_int* mu_out;
  mere_vec_int* mu_dcount;
} __anon_210_env;

typedef struct {
  __lang_region* __r;
  void* (*__copy)(__lang_region*, void*);
} __lang_env_hdr;

/* An FFI shim's env: a borrowed pointer with a header, so reading the
   header is always valid for any closure with a non-NULL env. */
typedef struct { __lang_region* __r; void* (*__copy)(__lang_region*, void*); const char* s; } __lang_shim_env;

static void* __mcopy_env___anon_210_env(__lang_region*, void*);
static void* __mcopy_env___anon_209_env(__lang_region*, void*);
static void* __mcopy_env___anon_208_env(__lang_region*, void*);
static void* __mcopy_env___anon_207_env(__lang_region*, void*);
static void* __mcopy_env___anon_206_env(__lang_region*, void*);
static void* __mcopy_env___anon_205_env(__lang_region*, void*);
static void* __mcopy_env___anon_204_env(__lang_region*, void*);
static void* __mcopy_env___anon_203_env(__lang_region*, void*);
static void* __mcopy_env___anon_202_env(__lang_region*, void*);
static void* __mcopy_env___anon_201_env(__lang_region*, void*);
static void* __mcopy_env___anon_200_env(__lang_region*, void*);
static void* __mcopy_env___anon_199_env(__lang_region*, void*);
static void* __mcopy_env___anon_198_env(__lang_region*, void*);
static void* __mcopy_env___anon_197_env(__lang_region*, void*);
static void* __mcopy_env___anon_196_env(__lang_region*, void*);
static void* __mcopy_env___anon_195_env(__lang_region*, void*);
static void* __mcopy_env___anon_194_env(__lang_region*, void*);
static void* __mcopy_env___anon_193_env(__lang_region*, void*);
static void* __mcopy_env___anon_192_env(__lang_region*, void*);
static void* __mcopy_env___anon_191_env(__lang_region*, void*);
static void* __mcopy_env___anon_190_env(__lang_region*, void*);
static void* __mcopy_env___anon_189_env(__lang_region*, void*);
static void* __mcopy_env___anon_188_env(__lang_region*, void*);
static void* __mcopy_env___anon_187_env(__lang_region*, void*);
static void* __mcopy_env___anon_186_env(__lang_region*, void*);
static void* __mcopy_env___anon_185_env(__lang_region*, void*);
static void* __mcopy_env___anon_184_env(__lang_region*, void*);
static void* __mcopy_env___anon_183_env(__lang_region*, void*);
static void* __mcopy_env___anon_182_env(__lang_region*, void*);
static void* __mcopy_env___anon_181_env(__lang_region*, void*);
static void* __mcopy_env___anon_180_env(__lang_region*, void*);
static void* __mcopy_env___anon_179_env(__lang_region*, void*);
static void* __mcopy_env___anon_178_env(__lang_region*, void*);
static void* __mcopy_env___anon_177_env(__lang_region*, void*);
static void* __mcopy_env___anon_176_env(__lang_region*, void*);
static void* __mcopy_env___anon_175_env(__lang_region*, void*);
static void* __mcopy_env___anon_174_env(__lang_region*, void*);
static void* __mcopy_env___anon_173_env(__lang_region*, void*);
static void* __mcopy_env___anon_172_env(__lang_region*, void*);
static void* __mcopy_env___anon_171_env(__lang_region*, void*);
static void* __mcopy_env___anon_170_env(__lang_region*, void*);
static void* __mcopy_env___anon_169_env(__lang_region*, void*);
static void* __mcopy_env___anon_168_env(__lang_region*, void*);
static void* __mcopy_env___anon_167_env(__lang_region*, void*);
static void* __mcopy_env___anon_166_env(__lang_region*, void*);
static void* __mcopy_env___anon_165_env(__lang_region*, void*);
static void* __mcopy_env___anon_164_env(__lang_region*, void*);
static void* __mcopy_env___anon_163_env(__lang_region*, void*);
static void* __mcopy_env___anon_162_env(__lang_region*, void*);
static void* __mcopy_env___anon_161_env(__lang_region*, void*);
static void* __mcopy_env___anon_160_env(__lang_region*, void*);
static void* __mcopy_env___anon_159_env(__lang_region*, void*);
static void* __mcopy_env___anon_158_env(__lang_region*, void*);
static void* __mcopy_env___anon_157_env(__lang_region*, void*);
static void* __mcopy_env___anon_156_env(__lang_region*, void*);
static void* __mcopy_env___anon_155_env(__lang_region*, void*);
static void* __mcopy_env___anon_154_env(__lang_region*, void*);
static void* __mcopy_env___anon_153_env(__lang_region*, void*);
static void* __mcopy_env___anon_152_env(__lang_region*, void*);
static void* __mcopy_env___anon_151_env(__lang_region*, void*);
static void* __mcopy_env___anon_150_env(__lang_region*, void*);
static void* __mcopy_env___anon_149_env(__lang_region*, void*);
static void* __mcopy_env___anon_148_env(__lang_region*, void*);
static void* __mcopy_env___anon_147_env(__lang_region*, void*);
static void* __mcopy_env___anon_146_env(__lang_region*, void*);
static void* __mcopy_env___anon_145_env(__lang_region*, void*);
static void* __mcopy_env___anon_144_env(__lang_region*, void*);
static void* __mcopy_env___anon_143_env(__lang_region*, void*);
static void* __mcopy_env___anon_142_env(__lang_region*, void*);
static void* __mcopy_env___anon_141_env(__lang_region*, void*);
static void* __mcopy_env___anon_140_env(__lang_region*, void*);
static void* __mcopy_env___anon_139_env(__lang_region*, void*);
static void* __mcopy_env___anon_138_env(__lang_region*, void*);
static void* __mcopy_env___anon_137_env(__lang_region*, void*);
static void* __mcopy_env___anon_136_env(__lang_region*, void*);
static void* __mcopy_env___anon_135_env(__lang_region*, void*);
static void* __mcopy_env___anon_134_env(__lang_region*, void*);
static void* __mcopy_env___anon_133_env(__lang_region*, void*);
static void* __mcopy_env___anon_132_env(__lang_region*, void*);
static void* __mcopy_env___anon_131_env(__lang_region*, void*);
static void* __mcopy_env___anon_130_env(__lang_region*, void*);
static void* __mcopy_env___anon_129_env(__lang_region*, void*);
static void* __mcopy_env___anon_128_env(__lang_region*, void*);
static void* __mcopy_env___anon_127_env(__lang_region*, void*);
static void* __mcopy_env___anon_126_env(__lang_region*, void*);
static void* __mcopy_env___anon_125_env(__lang_region*, void*);
static void* __mcopy_env___anon_124_env(__lang_region*, void*);
static void* __mcopy_env___anon_123_env(__lang_region*, void*);
static void* __mcopy_env___anon_122_env(__lang_region*, void*);
static void* __mcopy_env___anon_121_env(__lang_region*, void*);
static void* __mcopy_env___anon_120_env(__lang_region*, void*);
static void* __mcopy_env___anon_119_env(__lang_region*, void*);
static void* __mcopy_env___anon_118_env(__lang_region*, void*);
static void* __mcopy_env___anon_117_env(__lang_region*, void*);
static void* __mcopy_env___anon_116_env(__lang_region*, void*);
static void* __mcopy_env___anon_115_env(__lang_region*, void*);
static void* __mcopy_env___anon_114_env(__lang_region*, void*);
static void* __mcopy_env___anon_113_env(__lang_region*, void*);
static void* __mcopy_env___anon_112_env(__lang_region*, void*);
static void* __mcopy_env___anon_111_env(__lang_region*, void*);
static void* __mcopy_env___anon_110_env(__lang_region*, void*);
static void* __mcopy_env___anon_109_env(__lang_region*, void*);
static void* __mcopy_env___anon_108_env(__lang_region*, void*);
static void* __mcopy_env___anon_107_env(__lang_region*, void*);
static void* __mcopy_env___anon_106_env(__lang_region*, void*);
static void* __mcopy_env___anon_105_env(__lang_region*, void*);
static void* __mcopy_env___anon_104_env(__lang_region*, void*);
static void* __mcopy_env___anon_103_env(__lang_region*, void*);
static void* __mcopy_env___anon_102_env(__lang_region*, void*);
static void* __mcopy_env___anon_101_env(__lang_region*, void*);
static void* __mcopy_env___anon_100_env(__lang_region*, void*);
static void* __mcopy_env___anon_99_env(__lang_region*, void*);
static void* __mcopy_env___anon_98_env(__lang_region*, void*);
static void* __mcopy_env___anon_97_env(__lang_region*, void*);
static void* __mcopy_env___anon_96_env(__lang_region*, void*);
static void* __mcopy_env___anon_95_env(__lang_region*, void*);
static void* __mcopy_env___anon_94_env(__lang_region*, void*);
static void* __mcopy_env___anon_93_env(__lang_region*, void*);
static void* __mcopy_env___anon_92_env(__lang_region*, void*);
static void* __mcopy_env___anon_91_env(__lang_region*, void*);
static void* __mcopy_env___anon_90_env(__lang_region*, void*);
static void* __mcopy_env___anon_89_env(__lang_region*, void*);
static void* __mcopy_env___anon_88_env(__lang_region*, void*);
static void* __mcopy_env___anon_87_env(__lang_region*, void*);
static void* __mcopy_env___anon_86_env(__lang_region*, void*);
static void* __mcopy_env___anon_85_env(__lang_region*, void*);
static void* __mcopy_env___anon_84_env(__lang_region*, void*);
static void* __mcopy_env___anon_83_env(__lang_region*, void*);
static void* __mcopy_env___anon_82_env(__lang_region*, void*);
static void* __mcopy_env___anon_81_env(__lang_region*, void*);
static void* __mcopy_env___anon_80_env(__lang_region*, void*);
static void* __mcopy_env___anon_79_env(__lang_region*, void*);
static void* __mcopy_env___anon_78_env(__lang_region*, void*);
static void* __mcopy_env___anon_77_env(__lang_region*, void*);
static void* __mcopy_env___anon_76_env(__lang_region*, void*);
static void* __mcopy_env___anon_75_env(__lang_region*, void*);
static void* __mcopy_env___anon_74_env(__lang_region*, void*);
static void* __mcopy_env___anon_73_env(__lang_region*, void*);
static void* __mcopy_env___anon_72_env(__lang_region*, void*);
static void* __mcopy_env___anon_71_env(__lang_region*, void*);
static void* __mcopy_env___anon_70_env(__lang_region*, void*);
static void* __mcopy_env___anon_69_env(__lang_region*, void*);
static void* __mcopy_env___anon_68_env(__lang_region*, void*);
static void* __mcopy_env___anon_67_env(__lang_region*, void*);
static void* __mcopy_env___anon_66_env(__lang_region*, void*);
static void* __mcopy_env___anon_65_env(__lang_region*, void*);
static void* __mcopy_env___anon_64_env(__lang_region*, void*);
static void* __mcopy_env___anon_63_env(__lang_region*, void*);
static void* __mcopy_env___anon_62_env(__lang_region*, void*);
static void* __mcopy_env___anon_61_env(__lang_region*, void*);
static void* __mcopy_env___anon_60_env(__lang_region*, void*);
static void* __mcopy_env___anon_59_env(__lang_region*, void*);
static void* __mcopy_env___anon_58_env(__lang_region*, void*);
static void* __mcopy_env___anon_57_env(__lang_region*, void*);
static void* __mcopy_env___anon_56_env(__lang_region*, void*);
static void* __mcopy_env___anon_55_env(__lang_region*, void*);
static void* __mcopy_env___anon_54_env(__lang_region*, void*);
static void* __mcopy_env___anon_53_env(__lang_region*, void*);
static void* __mcopy_env___anon_52_env(__lang_region*, void*);
static void* __mcopy_env___anon_51_env(__lang_region*, void*);
static void* __mcopy_env___anon_50_env(__lang_region*, void*);
static void* __mcopy_env___anon_49_env(__lang_region*, void*);
static void* __mcopy_env___anon_48_env(__lang_region*, void*);
static void* __mcopy_env___anon_47_env(__lang_region*, void*);
static void* __mcopy_env___anon_46_env(__lang_region*, void*);
static void* __mcopy_env___anon_45_env(__lang_region*, void*);
static void* __mcopy_env___anon_44_env(__lang_region*, void*);
static void* __mcopy_env___anon_43_env(__lang_region*, void*);
static void* __mcopy_env___anon_42_env(__lang_region*, void*);
static void* __mcopy_env___anon_41_env(__lang_region*, void*);
static void* __mcopy_env___anon_40_env(__lang_region*, void*);
static void* __mcopy_env___anon_39_env(__lang_region*, void*);
static void* __mcopy_env___anon_38_env(__lang_region*, void*);
static void* __mcopy_env___anon_37_env(__lang_region*, void*);
static void* __mcopy_env___anon_36_env(__lang_region*, void*);
static void* __mcopy_env___anon_35_env(__lang_region*, void*);
static void* __mcopy_env___anon_34_env(__lang_region*, void*);
static void* __mcopy_env___anon_33_env(__lang_region*, void*);
static void* __mcopy_env___anon_32_env(__lang_region*, void*);
static void* __mcopy_env___anon_31_env(__lang_region*, void*);
static void* __mcopy_env___anon_30_env(__lang_region*, void*);
static void* __mcopy_env___anon_29_env(__lang_region*, void*);
static void* __mcopy_env___anon_28_env(__lang_region*, void*);
static void* __mcopy_env___anon_27_env(__lang_region*, void*);
static void* __mcopy_env___anon_26_env(__lang_region*, void*);
static void* __mcopy_env___anon_25_env(__lang_region*, void*);
static void* __mcopy_env___anon_24_env(__lang_region*, void*);
static void* __mcopy_env___anon_23_env(__lang_region*, void*);
static void* __mcopy_env___anon_22_env(__lang_region*, void*);
static void* __mcopy_env___anon_21_env(__lang_region*, void*);
static void* __mcopy_env___anon_20_env(__lang_region*, void*);
static void* __mcopy_env___anon_19_env(__lang_region*, void*);
static void* __mcopy_env___anon_18_env(__lang_region*, void*);
static void* __mcopy_env___anon_17_env(__lang_region*, void*);
static void* __mcopy_env___anon_16_env(__lang_region*, void*);
static void* __mcopy_env___anon_15_env(__lang_region*, void*);
static void* __mcopy_env___anon_14_env(__lang_region*, void*);
static void* __mcopy_env___anon_13_env(__lang_region*, void*);
static void* __mcopy_env___anon_12_env(__lang_region*, void*);
static void* __mcopy_env___anon_11_env(__lang_region*, void*);
static void* __mcopy_env___anon_10_env(__lang_region*, void*);
static void* __mcopy_env___anon_9_env(__lang_region*, void*);
static void* __mcopy_env___anon_8_env(__lang_region*, void*);
static void* __mcopy_env___anon_7_env(__lang_region*, void*);
static void* __mcopy_env___anon_4_env(__lang_region*, void*);
static void* __mcopy_env___anon_3_env(__lang_region*, void*);
static void* __mcopy_env___anon_2_env(__lang_region*, void*);

static mere_bytebuf* __mcopy_ByteBuf(__lang_region*, mere_bytebuf*);
static mere_strbuf* __mcopy_StrBuf(__lang_region*, mere_strbuf*);
static mere_vec_int* __mcopy_Vec_int(__lang_region*, mere_vec_int*);
static mere_vec_str* __mcopy_Vec_str(__lang_region*, mere_vec_str*);
static __lang_region* __mcopy___heap(__lang_region*, __lang_region*);
static __lang_region* __mcopy___rp4881(__lang_region*, __lang_region*);
static int __mcopy_bool(__lang_region*, int);
static long long __mcopy_int(__lang_region*, long long);
static list_int __mcopy_list_int(__lang_region*, list_int);
static list_str __mcopy_list_str(__lang_region*, list_str);
static const char* __mcopy_str(__lang_region*, const char*);
static tuple_int_list_int __mcopy_tuple_int_list_int(__lang_region*, tuple_int_list_int);
static tuple_str_list_str __mcopy_tuple_str_list_str(__lang_region*, tuple_str_list_str);

typedef struct mere_vec_int {
  long long* data;
  int len;
  int cap;
  __lang_region* region;
  int owns_region;  /* 1 after the first compact (see the map twin) */
} mere_vec_int;

static mere_vec_int* mere_vec_int_new(__lang_region* r) {
  mere_vec_int* v = (mere_vec_int*)__lang_region_alloc(r, sizeof(mere_vec_int));
  v->cap = 4;
  v->len = 0;
  v->data = (long long*)__lang_region_alloc(r, sizeof(long long) * 4);
  v->region = r;
  v->owns_region = 0;
  return v;
}

static int mere_vec_int_push(mere_vec_int* v, long long x) {
  x = __mcopy_int(v->region, x);
  if (v->len == v->cap) {
    int new_cap = v->cap * 2;
    v->data = (long long*)__lang_region_grow(v->region, v->data, sizeof(long long) * (size_t)v->cap, sizeof(long long) * (size_t)new_cap);
    v->cap = new_cap;
  }
  v->data[v->len++] = x;
  return 0; /* unit */
}

static int mere_vec_int_compact(mere_vec_int* v) {
  __lang_region* fresh = (__lang_region*)malloc(sizeof(__lang_region));
  if (!fresh) __lang_fail_impl("out of memory");
  __lang_region_init(fresh, 4096);
  __lang_region* old = v->region;
  int owned = v->owns_region;
  v->region = fresh;
  int ncap = v->len < 4 ? 4 : v->len;
  long long* nd = (long long*)__lang_region_alloc(fresh, sizeof(long long) * ncap);
  for (int i = 0; i < v->len; i++) nd[i] = __mcopy_int(fresh, v->data[i]);
  v->data = nd;
  v->cap = ncap;
  v->owns_region = 1;
  if (owned) __lang_region_retire(old);
  return 0;
}

static long long mere_vec_int_bytes(mere_vec_int* v) {
  return v->owns_region ? (long long)__lang_region_bytes(v->region) : 0LL;
}

static long long mere_vec_int_get(mere_vec_int* v, long long i) {
  if (i < 0 || i >= (long long)v->len) {
    __lang_fail_idx("vec_get: index %lld out of bounds (len = %lld)", i, (long long)v->len);
  }
  return v->data[i];
}

static long long mere_vec_int_len(mere_vec_int* v) { return v->len; }

static int mere_vec_int_set(mere_vec_int* v, long long i, long long x) {
  if (i < 0 || i >= (long long)v->len) {
    __lang_fail_idx("vec_set: index %lld out of bounds (len = %lld)", i, (long long)v->len);
  }
  x = __mcopy_int(v->region, x);
  v->data[i] = x;
  return 0; /* unit */
}

static long long mere_vec_int_get_unchecked(mere_vec_int* v, long long i) { return v->data[i]; }
static int mere_vec_int_set_unchecked(mere_vec_int* v, long long i, long long x) {
  v->data[i] = __mcopy_int(v->region, x);
  return 0; /* unit */
}
typedef struct mere_vec_str {
  const char** data;
  int len;
  int cap;
  __lang_region* region;
  int owns_region;  /* 1 after the first compact (see the map twin) */
} mere_vec_str;

static mere_vec_str* mere_vec_str_new(__lang_region* r) {
  mere_vec_str* v = (mere_vec_str*)__lang_region_alloc(r, sizeof(mere_vec_str));
  v->cap = 4;
  v->len = 0;
  v->data = (const char**)__lang_region_alloc(r, sizeof(const char*) * 4);
  v->region = r;
  v->owns_region = 0;
  return v;
}

static int mere_vec_str_push(mere_vec_str* v, const char* x) {
  x = __mcopy_str(v->region, x);
  if (v->len == v->cap) {
    int new_cap = v->cap * 2;
    v->data = (const char**)__lang_region_grow(v->region, v->data, sizeof(const char*) * (size_t)v->cap, sizeof(const char*) * (size_t)new_cap);
    v->cap = new_cap;
  }
  v->data[v->len++] = x;
  return 0; /* unit */
}

static int mere_vec_str_compact(mere_vec_str* v) {
  __lang_region* fresh = (__lang_region*)malloc(sizeof(__lang_region));
  if (!fresh) __lang_fail_impl("out of memory");
  __lang_region_init(fresh, 4096);
  __lang_region* old = v->region;
  int owned = v->owns_region;
  v->region = fresh;
  int ncap = v->len < 4 ? 4 : v->len;
  const char** nd = (const char**)__lang_region_alloc(fresh, sizeof(const char*) * ncap);
  for (int i = 0; i < v->len; i++) nd[i] = __mcopy_str(fresh, v->data[i]);
  v->data = nd;
  v->cap = ncap;
  v->owns_region = 1;
  if (owned) __lang_region_retire(old);
  return 0;
}

static long long mere_vec_str_bytes(mere_vec_str* v) {
  return v->owns_region ? (long long)__lang_region_bytes(v->region) : 0LL;
}

static const char* mere_vec_str_get(mere_vec_str* v, long long i) {
  if (i < 0 || i >= (long long)v->len) {
    __lang_fail_idx("vec_get: index %lld out of bounds (len = %lld)", i, (long long)v->len);
  }
  return v->data[i];
}

static long long mere_vec_str_len(mere_vec_str* v) { return v->len; }

static int mere_vec_str_set(mere_vec_str* v, long long i, const char* x) {
  if (i < 0 || i >= (long long)v->len) {
    __lang_fail_idx("vec_set: index %lld out of bounds (len = %lld)", i, (long long)v->len);
  }
  x = __mcopy_str(v->region, x);
  v->data[i] = x;
  return 0; /* unit */
}

static const char* mere_vec_str_get_unchecked(mere_vec_str* v, long long i) { return v->data[i]; }
static int mere_vec_str_set_unchecked(mere_vec_str* v, long long i, const char* x) {
  v->data[i] = __mcopy_str(v->region, x);
  return 0; /* unit */
}

typedef struct mere_strbuf {
  char* data;
  int len;
  int cap;
  __lang_region* region;
} mere_strbuf;

static mere_strbuf* mere_strbuf_new(__lang_region* r) {
  mere_strbuf* sb = (mere_strbuf*)__lang_region_alloc(r, sizeof(mere_strbuf));
  sb->cap = 16;
  sb->len = 0;
  sb->data = (char*)__lang_region_alloc(r, sizeof(char) * 16);
  sb->region = r;
  return sb;
}

static int mere_strbuf_push(mere_strbuf* sb, const char* s) {
  int slen = (int)__lang_str_size(s);
  if (sb->len + slen > sb->cap) {
    int new_cap = sb->cap * 2;
    while (sb->len + slen > new_cap) new_cap *= 2;
    sb->data = (char*)__lang_region_grow(sb->region, sb->data, (size_t)sb->cap, (size_t)new_cap);
    sb->cap = new_cap;
  }
  for (int i = 0; i < slen; i++) sb->data[sb->len + i] = s[i];
  sb->len += slen;
  return 0; /* unit */
}

static const char* mere_strbuf_to_str(mere_strbuf* sb) {
  /* Phase 36 (DEFERRED §1.16 fix): allocate result in the process-
     wide default region so the returned str outlives the StrBuf's
     scoped region. Avoids dangling pointers when
     `region R { ...; strbuf_to_str b }` returns a value out of R. */
  char* r = __lang_str_alloc(__lang_current_region, sb->len);
  for (int i = 0; i < sb->len; i++) r[i] = sb->data[i];
  r[sb->len] = '\0';
  return r;
}

static long long mere_strbuf_len(mere_strbuf* sb) { return sb->len; }

struct mere_bytes { long long len; unsigned char data[]; };
static mere_bytes* __lang_read_bytes(const char* path) {
  FILE* f = fopen(path, "rb");
  if (!f) { fprintf(stderr, "read_bytes: %s\n", path); exit(1); }
  fseek(f, 0, SEEK_END); long long n = ftell(f); fseek(f, 0, SEEK_SET);
  mere_bytes* b = (mere_bytes*)__lang_region_alloc(__lang_current_region, sizeof(mere_bytes) + (size_t)n);
  b->len = n;
  if (n > 0 && fread(b->data, 1, (size_t)n, f) != (size_t)n) { fclose(f); fprintf(stderr, "read_bytes: short read\n"); exit(1); }
  fclose(f);
  return b;
}
static long long __lang_write_bytes(const char* path, mere_bytes* b) {
  FILE* f = fopen(path, "wb");
  if (!f) { fprintf(stderr, "write_bytes: %s\n", path); exit(1); }
  fwrite(b->data, 1, (size_t)b->len, f);
  fclose(f);
  return 0;
}
static mere_bytes* __lang_bytes_alloc(long long len) {
  mere_bytes* b = (mere_bytes*)__lang_region_alloc(__lang_current_region, sizeof(mere_bytes) + (size_t)len);
  b->len = len;
  return b;
}
static mere_bytes* __lang_bytes_of_str(const char* s) {
  long long n = (long long)__lang_str_size(s);
  mere_bytes* b = __lang_bytes_alloc(n);
  memcpy(b->data, s, (size_t)n);
  return b;
}
static const char* __lang_str_of_bytes(mere_bytes* b) {
  char* s = __lang_str_alloc(__lang_current_region, (size_t)b->len);
  memcpy(s, b->data, (size_t)b->len);
  s[b->len] = '\0';
  return s;
}
static const char __lang_hexdig[] = "0123456789abcdef";
static const char* __lang_hex_of_bytes(mere_bytes* b) {
  char* s = __lang_str_alloc(__lang_current_region, (size_t)b->len * 2);
  for (long long i = 0; i < b->len; i++) {
    s[i*2]   = __lang_hexdig[b->data[i] >> 4];
    s[i*2+1] = __lang_hexdig[b->data[i] & 15];
  }
  s[b->len*2] = '\0';
  return s;
}
static int __lang_hexval(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  fprintf(stderr, "bytes_of_hex: non-hex character\n"); abort();
}
static mere_bytes* __lang_bytes_of_hex(const char* h) {
  long long n = (long long)strlen(h);
  if (n % 2) { fprintf(stderr, "bytes_of_hex: odd-length hex string\n"); abort(); }
  mere_bytes* b = __lang_bytes_alloc(n / 2);
  for (long long i = 0; i < n / 2; i++)
    b->data[i] = (unsigned char)(__lang_hexval(h[i*2]) * 16 + __lang_hexval(h[i*2+1]));
  return b;
}
static mere_bytes* __lang_bytes_slice(mere_bytes* b, long long start, long long len) {
  if (start < 0 || len < 0 || start + len > b->len) {
    __lang_fail_impl("bytes_slice: range out of bounds");
  }
  mere_bytes* o = __lang_bytes_alloc(len);
  memcpy(o->data, b->data + start, (size_t)len);
  return o;
}
static mere_bytes* __lang_bytes_concat(mere_bytes* a, mere_bytes* b) {
  mere_bytes* o = __lang_bytes_alloc(a->len + b->len);
  memcpy(o->data, a->data, (size_t)a->len);
  memcpy(o->data + a->len, b->data, (size_t)b->len);
  return o;
}
static long long __lang_bytes_get(mere_bytes* b, long long i) {
  if (i < 0 || i >= b->len) __lang_fail_impl("bytes_get: index out of range");
  return (long long)b->data[i];
}
static long long __lang_bytes_get_unchecked(mere_bytes* b, long long i) { return (long long)b->data[i]; }
static mere_u8x16 __lang_u8x16_load_unchecked(mere_bytes* b, long long i) { mere_u8x16 r; memcpy(&r, b->data + i, 16); return r; }
static mere_u8x16 __lang_u8x16_load(mere_bytes* b, long long i) {
  if (i < 0 || i + 16 > b->len) __lang_fail_idx("u8x16_load: bytes [%lld, +16) out of bounds (len = %lld)", i, b->len);
  return __lang_u8x16_load_unchecked(b, i);
}
static mere_u8x16 __lang_u8x16_from_bytes(mere_bytes* b) {
  if (b->len < 16) __lang_fail_idx("u8x16_from_bytes: bytes [%lld, +16) out of bounds (len = %lld)", 0LL, b->len);
  return __lang_u8x16_load_unchecked(b, 0);
}

typedef struct mere_bytebuf {
  unsigned char* data;
  long long len;
  long long cap;
  __lang_region* region;
} mere_bytebuf;

static mere_bytebuf* mere_bytebuf_new(__lang_region* r, long long n) {
  mere_bytebuf* bb = (mere_bytebuf*)__lang_region_alloc(r, sizeof(mere_bytebuf));
  long long cap = n > 0 ? n : 1;
  bb->data = (unsigned char*)__lang_region_alloc(r, (size_t)cap);
  for (long long i = 0; i < n; i++) bb->data[i] = 0;
  bb->len = n;
  bb->cap = cap;
  bb->region = r;
  return bb;
}

static long long mere_bytebuf_len(mere_bytebuf* bb) { return bb->len; }

static long long mere_bytebuf_get(mere_bytebuf* bb, long long i) {
  if (i < 0 || i >= bb->len)
    __lang_fail_idx("bytebuf_get: index %lld out of bounds (len = %lld)", i, bb->len);
  return (long long)bb->data[i];
}

static long long mere_bytebuf_set(mere_bytebuf* bb, long long i, long long v) {
  if (i < 0 || i >= bb->len)
    __lang_fail_idx("bytebuf_set: index %lld out of bounds (len = %lld)", i, bb->len);
  bb->data[i] = (unsigned char)(v & 255);
  return 0; /* unit */
}

static long long mere_bytebuf_push(mere_bytebuf* bb, long long v) {
  if (bb->len + 1 > bb->cap) {
    long long new_cap = bb->cap * 2 + 16;
    bb->data = (unsigned char*)__lang_region_grow(bb->region, bb->data, (size_t)bb->cap, (size_t)new_cap);
    bb->cap = new_cap;
  }
  bb->data[bb->len] = (unsigned char)(v & 255);
  bb->len += 1;
  return 0; /* unit */
}

static mere_bytes* mere_bytes_of_bytebuf(mere_bytebuf* bb) {
  /* Into the current region, so a `bytes` frozen inside `region R { }` can
     be returned out of it — the same reason strbuf_to_str does. */
  mere_bytes* b = __lang_bytes_alloc(bb->len);
  for (long long i = 0; i < bb->len; i++) b->data[i] = bb->data[i];
  return b;
}

static mere_bytebuf* mere_bytebuf_of_bytes(__lang_region* r, mere_bytes* b) {
  mere_bytebuf* bb = mere_bytebuf_new(r, b->len);
  for (long long i = 0; i < b->len; i++) bb->data[i] = b->data[i];
  return bb;
}

static mere_bytes* __lang_bytes_of_vec(mere_vec_int* v) {
  mere_bytes* b = __lang_bytes_alloc((long long)v->len);
  for (int i = 0; i < v->len; i++) b->data[i] = (unsigned char)(v->data[i] & 0xFF);
  return b;
}
static mere_vec_int* __lang_vec_of_bytes(mere_bytes* b, __lang_region* r) {
  mere_vec_int* v = mere_vec_int_new(r);
  for (long long i = 0; i < b->len; i++) mere_vec_int_push(v, (long long)b->data[i]);
  return v;
}

static mere_bytebuf* __mcopy_ByteBuf(__lang_region* r, mere_bytebuf* v) {

  return v;
}
static mere_strbuf* __mcopy_StrBuf(__lang_region* r, mere_strbuf* v) { (void)r; return v; }
static mere_vec_int* __mcopy_Vec_int(__lang_region* r, mere_vec_int* v) { (void)r; return v; }
static mere_vec_str* __mcopy_Vec_str(__lang_region* r, mere_vec_str* v) { (void)r; return v; }
static __lang_region* __mcopy___heap(__lang_region* r, __lang_region* v) { (void)r; return v; }
static __lang_region* __mcopy___rp4881(__lang_region* r, __lang_region* v) { (void)r; return v; }
static int __mcopy_bool(__lang_region* r, int v) { (void)r; return v; }
static long long __mcopy_int(__lang_region* r, long long v) { (void)r; return v; }
static list_int __mcopy_list_int(__lang_region* r, list_int v) {
  int __t = list_int__tag(v);
  if (__t == 0) return v;
  list_int_node* p = (list_int_node*)__lang_region_alloc(r, sizeof(list_int_node));
  *p = *list_int__node(v);
  if (__t == 1) p->payload.Cons = __mcopy_tuple_int_list_int(r, p->payload.Cons);
  return list_int__mk(__t, p);
}
static list_str __mcopy_list_str(__lang_region* r, list_str v) {
  int __t = list_str__tag(v);
  if (__t == 0) return v;
  list_str_node* p = (list_str_node*)__lang_region_alloc(r, sizeof(list_str_node));
  *p = *list_str__node(v);
  if (__t == 1) p->payload.Cons = __mcopy_tuple_str_list_str(r, p->payload.Cons);
  return list_str__mk(__t, p);
}
static const char* __mcopy_str(__lang_region* r, const char* v) {
  size_t n = __lang_str_size(v);
  char* s = __lang_str_alloc(r, n);
  memcpy(s, v, n);
  return s;
}
static tuple_int_list_int __mcopy_tuple_int_list_int(__lang_region* r, tuple_int_list_int v) {
  v.f0 = __mcopy_int(r, v.f0);
  v.f1 = __mcopy_list_int(r, v.f1);
  return v;
}
static tuple_str_list_str __mcopy_tuple_str_list_str(__lang_region* r, tuple_str_list_str v) {
  v.f0 = __mcopy_str(r, v.f0);
  v.f1 = __mcopy_list_str(r, v.f1);
  return v;
}

static void* __mcopy_env___anon_210_env(__lang_region* r, void* __p) {
  __anon_210_env* __s = (__anon_210_env*)__p;
  if (__s->__r == r) return __p;
  __anon_210_env* __d = (__anon_210_env*)__lang_region_alloc(r, sizeof(__anon_210_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_209_env(__lang_region* r, void* __p) {
  __anon_209_env* __s = (__anon_209_env*)__p;
  if (__s->__r == r) return __p;
  __anon_209_env* __d = (__anon_209_env*)__lang_region_alloc(r, sizeof(__anon_209_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  __d->mu_t3 = __mcopy_int(r, __s->mu_t3);
  return (void*)__d;
}
static void* __mcopy_env___anon_208_env(__lang_region* r, void* __p) {
  __anon_208_env* __s = (__anon_208_env*)__p;
  if (__s->__r == r) return __p;
  __anon_208_env* __d = (__anon_208_env*)__lang_region_alloc(r, sizeof(__anon_208_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  __d->mu_c = __mcopy_int(r, __s->mu_c);
  return (void*)__d;
}
static void* __mcopy_env___anon_207_env(__lang_region* r, void* __p) {
  __anon_207_env* __s = (__anon_207_env*)__p;
  if (__s->__r == r) return __p;
  __anon_207_env* __d = (__anon_207_env*)__lang_region_alloc(r, sizeof(__anon_207_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_206_env(__lang_region* r, void* __p) {
  __anon_206_env* __s = (__anon_206_env*)__p;
  if (__s->__r == r) return __p;
  __anon_206_env* __d = (__anon_206_env*)__lang_region_alloc(r, sizeof(__anon_206_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_205_env(__lang_region* r, void* __p) {
  __anon_205_env* __s = (__anon_205_env*)__p;
  if (__s->__r == r) return __p;
  __anon_205_env* __d = (__anon_205_env*)__lang_region_alloc(r, sizeof(__anon_205_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_204_env(__lang_region* r, void* __p) {
  __anon_204_env* __s = (__anon_204_env*)__p;
  if (__s->__r == r) return __p;
  __anon_204_env* __d = (__anon_204_env*)__lang_region_alloc(r, sizeof(__anon_204_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_203_env(__lang_region* r, void* __p) {
  __anon_203_env* __s = (__anon_203_env*)__p;
  if (__s->__r == r) return __p;
  __anon_203_env* __d = (__anon_203_env*)__lang_region_alloc(r, sizeof(__anon_203_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_202_env(__lang_region* r, void* __p) {
  __anon_202_env* __s = (__anon_202_env*)__p;
  if (__s->__r == r) return __p;
  __anon_202_env* __d = (__anon_202_env*)__lang_region_alloc(r, sizeof(__anon_202_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  __d->mu_t3 = __mcopy_int(r, __s->mu_t3);
  return (void*)__d;
}
static void* __mcopy_env___anon_201_env(__lang_region* r, void* __p) {
  __anon_201_env* __s = (__anon_201_env*)__p;
  if (__s->__r == r) return __p;
  __anon_201_env* __d = (__anon_201_env*)__lang_region_alloc(r, sizeof(__anon_201_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  __d->mu_t3 = __mcopy_int(r, __s->mu_t3);
  return (void*)__d;
}
static void* __mcopy_env___anon_200_env(__lang_region* r, void* __p) {
  __anon_200_env* __s = (__anon_200_env*)__p;
  if (__s->__r == r) return __p;
  __anon_200_env* __d = (__anon_200_env*)__lang_region_alloc(r, sizeof(__anon_200_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  __d->mu_t3 = __mcopy_int(r, __s->mu_t3);
  return (void*)__d;
}
static void* __mcopy_env___anon_199_env(__lang_region* r, void* __p) {
  __anon_199_env* __s = (__anon_199_env*)__p;
  if (__s->__r == r) return __p;
  __anon_199_env* __d = (__anon_199_env*)__lang_region_alloc(r, sizeof(__anon_199_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  return (void*)__d;
}
static void* __mcopy_env___anon_198_env(__lang_region* r, void* __p) {
  __anon_198_env* __s = (__anon_198_env*)__p;
  if (__s->__r == r) return __p;
  __anon_198_env* __d = (__anon_198_env*)__lang_region_alloc(r, sizeof(__anon_198_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_197_env(__lang_region* r, void* __p) {
  __anon_197_env* __s = (__anon_197_env*)__p;
  if (__s->__r == r) return __p;
  __anon_197_env* __d = (__anon_197_env*)__lang_region_alloc(r, sizeof(__anon_197_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  __d->mu_c = __mcopy_int(r, __s->mu_c);
  return (void*)__d;
}
static void* __mcopy_env___anon_196_env(__lang_region* r, void* __p) {
  __anon_196_env* __s = (__anon_196_env*)__p;
  if (__s->__r == r) return __p;
  __anon_196_env* __d = (__anon_196_env*)__lang_region_alloc(r, sizeof(__anon_196_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  __d->mu_c = __mcopy_int(r, __s->mu_c);
  return (void*)__d;
}
static void* __mcopy_env___anon_195_env(__lang_region* r, void* __p) {
  __anon_195_env* __s = (__anon_195_env*)__p;
  if (__s->__r == r) return __p;
  __anon_195_env* __d = (__anon_195_env*)__lang_region_alloc(r, sizeof(__anon_195_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  __d->mu_c = __mcopy_int(r, __s->mu_c);
  return (void*)__d;
}
static void* __mcopy_env___anon_194_env(__lang_region* r, void* __p) {
  __anon_194_env* __s = (__anon_194_env*)__p;
  if (__s->__r == r) return __p;
  __anon_194_env* __d = (__anon_194_env*)__lang_region_alloc(r, sizeof(__anon_194_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  return (void*)__d;
}
static void* __mcopy_env___anon_193_env(__lang_region* r, void* __p) {
  __anon_193_env* __s = (__anon_193_env*)__p;
  if (__s->__r == r) return __p;
  __anon_193_env* __d = (__anon_193_env*)__lang_region_alloc(r, sizeof(__anon_193_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_192_env(__lang_region* r, void* __p) {
  __anon_192_env* __s = (__anon_192_env*)__p;
  if (__s->__r == r) return __p;
  __anon_192_env* __d = (__anon_192_env*)__lang_region_alloc(r, sizeof(__anon_192_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_191_env(__lang_region* r, void* __p) {
  __anon_191_env* __s = (__anon_191_env*)__p;
  if (__s->__r == r) return __p;
  __anon_191_env* __d = (__anon_191_env*)__lang_region_alloc(r, sizeof(__anon_191_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_190_env(__lang_region* r, void* __p) {
  __anon_190_env* __s = (__anon_190_env*)__p;
  if (__s->__r == r) return __p;
  __anon_190_env* __d = (__anon_190_env*)__lang_region_alloc(r, sizeof(__anon_190_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_dcount = __mcopy_Vec_int(r, __s->mu_dcount);
  return (void*)__d;
}
static void* __mcopy_env___anon_189_env(__lang_region* r, void* __p) {
  __anon_189_env* __s = (__anon_189_env*)__p;
  if (__s->__r == r) return __p;
  __anon_189_env* __d = (__anon_189_env*)__lang_region_alloc(r, sizeof(__anon_189_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_188_env(__lang_region* r, void* __p) {
  __anon_188_env* __s = (__anon_188_env*)__p;
  if (__s->__r == r) return __p;
  __anon_188_env* __d = (__anon_188_env*)__lang_region_alloc(r, sizeof(__anon_188_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_187_env(__lang_region* r, void* __p) {
  __anon_187_env* __s = (__anon_187_env*)__p;
  if (__s->__r == r) return __p;
  __anon_187_env* __d = (__anon_187_env*)__lang_region_alloc(r, sizeof(__anon_187_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_186_env(__lang_region* r, void* __p) {
  __anon_186_env* __s = (__anon_186_env*)__p;
  if (__s->__r == r) return __p;
  __anon_186_env* __d = (__anon_186_env*)__lang_region_alloc(r, sizeof(__anon_186_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_got = __mcopy_int(r, __s->mu_got);
  __d->mu_cur = __mcopy_int(r, __s->mu_cur);
  __d->mu_acc = __mcopy_Vec_int(r, __s->mu_acc);
  return (void*)__d;
}
static void* __mcopy_env___anon_185_env(__lang_region* r, void* __p) {
  __anon_185_env* __s = (__anon_185_env*)__p;
  if (__s->__r == r) return __p;
  __anon_185_env* __d = (__anon_185_env*)__lang_region_alloc(r, sizeof(__anon_185_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  __d->mu_bk = __mcopy_int(r, __s->mu_bk);
  __d->mu_row_uq63 = __mcopy_Vec_int(r, __s->mu_row_uq63);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  __d->mu_bpp = __mcopy_int(r, __s->mu_bpp);
  __d->mu_bc = __mcopy_int(r, __s->mu_bc);
  return (void*)__d;
}
static void* __mcopy_env___anon_184_env(__lang_region* r, void* __p) {
  __anon_184_env* __s = (__anon_184_env*)__p;
  if (__s->__r == r) return __p;
  __anon_184_env* __d = (__anon_184_env*)__lang_region_alloc(r, sizeof(__anon_184_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_code0 = __mcopy_int(r, __s->mu_code0);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_first = __mcopy_int(r, __s->mu_first);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_183_env(__lang_region* r, void* __p) {
  __anon_183_env* __s = (__anon_183_env*)__p;
  if (__s->__r == r) return __p;
  __anon_183_env* __d = (__anon_183_env*)__lang_region_alloc(r, sizeof(__anon_183_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_code0 = __mcopy_int(r, __s->mu_code0);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_first = __mcopy_int(r, __s->mu_first);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_182_env(__lang_region* r, void* __p) {
  __anon_182_env* __s = (__anon_182_env*)__p;
  if (__s->__r == r) return __p;
  __anon_182_env* __d = (__anon_182_env*)__lang_region_alloc(r, sizeof(__anon_182_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_181_env(__lang_region* r, void* __p) {
  __anon_181_env* __s = (__anon_181_env*)__p;
  if (__s->__r == r) return __p;
  __anon_181_env* __d = (__anon_181_env*)__lang_region_alloc(r, sizeof(__anon_181_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pal = __mcopy_Vec_int(r, __s->mu_pal);
  __d->mu_r = __mcopy_int(r, __s->mu_r);
  __d->mu_g = __mcopy_int(r, __s->mu_g);
  return (void*)__d;
}
static void* __mcopy_env___anon_180_env(__lang_region* r, void* __p) {
  __anon_180_env* __s = (__anon_180_env*)__p;
  if (__s->__r == r) return __p;
  __anon_180_env* __d = (__anon_180_env*)__lang_region_alloc(r, sizeof(__anon_180_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_row_uq58 = __mcopy_Vec_int(r, __s->mu_row_uq58);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  return (void*)__d;
}
static void* __mcopy_env___anon_179_env(__lang_region* r, void* __p) {
  __anon_179_env* __s = (__anon_179_env*)__p;
  if (__s->__r == r) return __p;
  __anon_179_env* __d = (__anon_179_env*)__lang_region_alloc(r, sizeof(__anon_179_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  __d->mu_t3 = __mcopy_int(r, __s->mu_t3);
  return (void*)__d;
}
static void* __mcopy_env___anon_178_env(__lang_region* r, void* __p) {
  __anon_178_env* __s = (__anon_178_env*)__p;
  if (__s->__r == r) return __p;
  __anon_178_env* __d = (__anon_178_env*)__lang_region_alloc(r, sizeof(__anon_178_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  return (void*)__d;
}
static void* __mcopy_env___anon_177_env(__lang_region* r, void* __p) {
  __anon_177_env* __s = (__anon_177_env*)__p;
  if (__s->__r == r) return __p;
  __anon_177_env* __d = (__anon_177_env*)__lang_region_alloc(r, sizeof(__anon_177_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  __d->mu_t2 = __mcopy_int(r, __s->mu_t2);
  return (void*)__d;
}
static void* __mcopy_env___anon_176_env(__lang_region* r, void* __p) {
  __anon_176_env* __s = (__anon_176_env*)__p;
  if (__s->__r == r) return __p;
  __anon_176_env* __d = (__anon_176_env*)__lang_region_alloc(r, sizeof(__anon_176_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  return (void*)__d;
}
static void* __mcopy_env___anon_175_env(__lang_region* r, void* __p) {
  __anon_175_env* __s = (__anon_175_env*)__p;
  if (__s->__r == r) return __p;
  __anon_175_env* __d = (__anon_175_env*)__lang_region_alloc(r, sizeof(__anon_175_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_v = __mcopy_int(r, __s->mu_v);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_174_env(__lang_region* r, void* __p) {
  __anon_174_env* __s = (__anon_174_env*)__p;
  if (__s->__r == r) return __p;
  __anon_174_env* __d = (__anon_174_env*)__lang_region_alloc(r, sizeof(__anon_174_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_173_env(__lang_region* r, void* __p) {
  __anon_173_env* __s = (__anon_173_env*)__p;
  if (__s->__r == r) return __p;
  __anon_173_env* __d = (__anon_173_env*)__lang_region_alloc(r, sizeof(__anon_173_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  return (void*)__d;
}
static void* __mcopy_env___anon_172_env(__lang_region* r, void* __p) {
  __anon_172_env* __s = (__anon_172_env*)__p;
  if (__s->__r == r) return __p;
  __anon_172_env* __d = (__anon_172_env*)__lang_region_alloc(r, sizeof(__anon_172_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_171_env(__lang_region* r, void* __p) {
  __anon_171_env* __s = (__anon_171_env*)__p;
  if (__s->__r == r) return __p;
  __anon_171_env* __d = (__anon_171_env*)__lang_region_alloc(r, sizeof(__anon_171_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_170_env(__lang_region* r, void* __p) {
  __anon_170_env* __s = (__anon_170_env*)__p;
  if (__s->__r == r) return __p;
  __anon_170_env* __d = (__anon_170_env*)__lang_region_alloc(r, sizeof(__anon_170_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_169_env(__lang_region* r, void* __p) {
  __anon_169_env* __s = (__anon_169_env*)__p;
  if (__s->__r == r) return __p;
  __anon_169_env* __d = (__anon_169_env*)__lang_region_alloc(r, sizeof(__anon_169_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  __d->mu_c = __mcopy_int(r, __s->mu_c);
  return (void*)__d;
}
static void* __mcopy_env___anon_168_env(__lang_region* r, void* __p) {
  __anon_168_env* __s = (__anon_168_env*)__p;
  if (__s->__r == r) return __p;
  __anon_168_env* __d = (__anon_168_env*)__lang_region_alloc(r, sizeof(__anon_168_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  return (void*)__d;
}
static void* __mcopy_env___anon_167_env(__lang_region* r, void* __p) {
  __anon_167_env* __s = (__anon_167_env*)__p;
  if (__s->__r == r) return __p;
  __anon_167_env* __d = (__anon_167_env*)__lang_region_alloc(r, sizeof(__anon_167_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  return (void*)__d;
}
static void* __mcopy_env___anon_166_env(__lang_region* r, void* __p) {
  __anon_166_env* __s = (__anon_166_env*)__p;
  if (__s->__r == r) return __p;
  __anon_166_env* __d = (__anon_166_env*)__lang_region_alloc(r, sizeof(__anon_166_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_165_env(__lang_region* r, void* __p) {
  __anon_165_env* __s = (__anon_165_env*)__p;
  if (__s->__r == r) return __p;
  __anon_165_env* __d = (__anon_165_env*)__lang_region_alloc(r, sizeof(__anon_165_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_lsyms = __mcopy_Vec_int(r, __s->mu_lsyms);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_164_env(__lang_region* r, void* __p) {
  __anon_164_env* __s = (__anon_164_env*)__p;
  if (__s->__r == r) return __p;
  __anon_164_env* __d = (__anon_164_env*)__lang_region_alloc(r, sizeof(__anon_164_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_163_env(__lang_region* r, void* __p) {
  __anon_163_env* __s = (__anon_163_env*)__p;
  if (__s->__r == r) return __p;
  __anon_163_env* __d = (__anon_163_env*)__lang_region_alloc(r, sizeof(__anon_163_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_lcount = __mcopy_Vec_int(r, __s->mu_lcount);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_162_env(__lang_region* r, void* __p) {
  __anon_162_env* __s = (__anon_162_env*)__p;
  if (__s->__r == r) return __p;
  __anon_162_env* __d = (__anon_162_env*)__lang_region_alloc(r, sizeof(__anon_162_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_161_env(__lang_region* r, void* __p) {
  __anon_161_env* __s = (__anon_161_env*)__p;
  if (__s->__r == r) return __p;
  __anon_161_env* __d = (__anon_161_env*)__lang_region_alloc(r, sizeof(__anon_161_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  return (void*)__d;
}
static void* __mcopy_env___anon_160_env(__lang_region* r, void* __p) {
  __anon_160_env* __s = (__anon_160_env*)__p;
  if (__s->__r == r) return __p;
  __anon_160_env* __d = (__anon_160_env*)__lang_region_alloc(r, sizeof(__anon_160_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_159_env(__lang_region* r, void* __p) {
  __anon_159_env* __s = (__anon_159_env*)__p;
  if (__s->__r == r) return __p;
  __anon_159_env* __d = (__anon_159_env*)__lang_region_alloc(r, sizeof(__anon_159_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cs = __mcopy_list_str(r, __s->mu_cs);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  return (void*)__d;
}
static void* __mcopy_env___anon_158_env(__lang_region* r, void* __p) {
  __anon_158_env* __s = (__anon_158_env*)__p;
  if (__s->__r == r) return __p;
  __anon_158_env* __d = (__anon_158_env*)__lang_region_alloc(r, sizeof(__anon_158_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_got = __mcopy_int(r, __s->mu_got);
  __d->mu_cur = __mcopy_int(r, __s->mu_cur);
  __d->mu_acc = __mcopy_Vec_int(r, __s->mu_acc);
  return (void*)__d;
}
static void* __mcopy_env___anon_157_env(__lang_region* r, void* __p) {
  __anon_157_env* __s = (__anon_157_env*)__p;
  if (__s->__r == r) return __p;
  __anon_157_env* __d = (__anon_157_env*)__lang_region_alloc(r, sizeof(__anon_157_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_got = __mcopy_int(r, __s->mu_got);
  __d->mu_acc = __mcopy_Vec_int(r, __s->mu_acc);
  return (void*)__d;
}
static void* __mcopy_env___anon_156_env(__lang_region* r, void* __p) {
  __anon_156_env* __s = (__anon_156_env*)__p;
  if (__s->__r == r) return __p;
  __anon_156_env* __d = (__anon_156_env*)__lang_region_alloc(r, sizeof(__anon_156_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  __d->mu_bk = __mcopy_int(r, __s->mu_bk);
  __d->mu_row_uq63 = __mcopy_Vec_int(r, __s->mu_row_uq63);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  __d->mu_bpp = __mcopy_int(r, __s->mu_bpp);
  __d->mu_bc = __mcopy_int(r, __s->mu_bc);
  return (void*)__d;
}
static void* __mcopy_env___anon_155_env(__lang_region* r, void* __p) {
  __anon_155_env* __s = (__anon_155_env*)__p;
  if (__s->__r == r) return __p;
  __anon_155_env* __d = (__anon_155_env*)__lang_region_alloc(r, sizeof(__anon_155_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  __d->mu_bk = __mcopy_int(r, __s->mu_bk);
  __d->mu_row_uq63 = __mcopy_Vec_int(r, __s->mu_row_uq63);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  __d->mu_bpp = __mcopy_int(r, __s->mu_bpp);
  return (void*)__d;
}
static void* __mcopy_env___anon_154_env(__lang_region* r, void* __p) {
  __anon_154_env* __s = (__anon_154_env*)__p;
  if (__s->__r == r) return __p;
  __anon_154_env* __d = (__anon_154_env*)__lang_region_alloc(r, sizeof(__anon_154_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_153_env(__lang_region* r, void* __p) {
  __anon_153_env* __s = (__anon_153_env*)__p;
  if (__s->__r == r) return __p;
  __anon_153_env* __d = (__anon_153_env*)__lang_region_alloc(r, sizeof(__anon_153_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_152_env(__lang_region* r, void* __p) {
  __anon_152_env* __s = (__anon_152_env*)__p;
  if (__s->__r == r) return __p;
  __anon_152_env* __d = (__anon_152_env*)__lang_region_alloc(r, sizeof(__anon_152_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_v = __mcopy_Vec_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_151_env(__lang_region* r, void* __p) {
  __anon_151_env* __s = (__anon_151_env*)__p;
  if (__s->__r == r) return __p;
  __anon_151_env* __d = (__anon_151_env*)__lang_region_alloc(r, sizeof(__anon_151_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_d = __mcopy_int(r, __s->mu_d);
  __d->mu_max_back = __mcopy_int(r, __s->mu_max_back);
  __d->mu_blen = __mcopy_int(r, __s->mu_blen);
  __d->mu_pos = __mcopy_int(r, __s->mu_pos);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_150_env(__lang_region* r, void* __p) {
  __anon_150_env* __s = (__anon_150_env*)__p;
  if (__s->__r == r) return __p;
  __anon_150_env* __d = (__anon_150_env*)__lang_region_alloc(r, sizeof(__anon_150_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_code0 = __mcopy_int(r, __s->mu_code0);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_first = __mcopy_int(r, __s->mu_first);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_149_env(__lang_region* r, void* __p) {
  __anon_149_env* __s = (__anon_149_env*)__p;
  if (__s->__r == r) return __p;
  __anon_149_env* __d = (__anon_149_env*)__lang_region_alloc(r, sizeof(__anon_149_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_code0 = __mcopy_int(r, __s->mu_code0);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_148_env(__lang_region* r, void* __p) {
  __anon_148_env* __s = (__anon_148_env*)__p;
  if (__s->__r == r) return __p;
  __anon_148_env* __d = (__anon_148_env*)__lang_region_alloc(r, sizeof(__anon_148_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_code0 = __mcopy_int(r, __s->mu_code0);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_first = __mcopy_int(r, __s->mu_first);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_147_env(__lang_region* r, void* __p) {
  __anon_147_env* __s = (__anon_147_env*)__p;
  if (__s->__r == r) return __p;
  __anon_147_env* __d = (__anon_147_env*)__lang_region_alloc(r, sizeof(__anon_147_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_code0 = __mcopy_int(r, __s->mu_code0);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_146_env(__lang_region* r, void* __p) {
  __anon_146_env* __s = (__anon_146_env*)__p;
  if (__s->__r == r) return __p;
  __anon_146_env* __d = (__anon_146_env*)__lang_region_alloc(r, sizeof(__anon_146_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  return (void*)__d;
}
static void* __mcopy_env___anon_145_env(__lang_region* r, void* __p) {
  __anon_145_env* __s = (__anon_145_env*)__p;
  if (__s->__r == r) return __p;
  __anon_145_env* __d = (__anon_145_env*)__lang_region_alloc(r, sizeof(__anon_145_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  return (void*)__d;
}
static void* __mcopy_env___anon_144_env(__lang_region* r, void* __p) {
  __anon_144_env* __s = (__anon_144_env*)__p;
  if (__s->__r == r) return __p;
  __anon_144_env* __d = (__anon_144_env*)__lang_region_alloc(r, sizeof(__anon_144_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_143_env(__lang_region* r, void* __p) {
  __anon_143_env* __s = (__anon_143_env*)__p;
  if (__s->__r == r) return __p;
  __anon_143_env* __d = (__anon_143_env*)__lang_region_alloc(r, sizeof(__anon_143_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_142_env(__lang_region* r, void* __p) {
  __anon_142_env* __s = (__anon_142_env*)__p;
  if (__s->__r == r) return __p;
  __anon_142_env* __d = (__anon_142_env*)__lang_region_alloc(r, sizeof(__anon_142_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pal = __mcopy_Vec_int(r, __s->mu_pal);
  __d->mu_r = __mcopy_int(r, __s->mu_r);
  __d->mu_g = __mcopy_int(r, __s->mu_g);
  return (void*)__d;
}
static void* __mcopy_env___anon_141_env(__lang_region* r, void* __p) {
  __anon_141_env* __s = (__anon_141_env*)__p;
  if (__s->__r == r) return __p;
  __anon_141_env* __d = (__anon_141_env*)__lang_region_alloc(r, sizeof(__anon_141_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pal = __mcopy_Vec_int(r, __s->mu_pal);
  __d->mu_r = __mcopy_int(r, __s->mu_r);
  return (void*)__d;
}
static void* __mcopy_env___anon_140_env(__lang_region* r, void* __p) {
  __anon_140_env* __s = (__anon_140_env*)__p;
  if (__s->__r == r) return __p;
  __anon_140_env* __d = (__anon_140_env*)__lang_region_alloc(r, sizeof(__anon_140_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_row_uq63 = __mcopy_Vec_int(r, __s->mu_row_uq63);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  return (void*)__d;
}
static void* __mcopy_env___anon_139_env(__lang_region* r, void* __p) {
  __anon_139_env* __s = (__anon_139_env*)__p;
  if (__s->__r == r) return __p;
  __anon_139_env* __d = (__anon_139_env*)__lang_region_alloc(r, sizeof(__anon_139_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_row_uq58 = __mcopy_Vec_int(r, __s->mu_row_uq58);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  return (void*)__d;
}
static void* __mcopy_env___anon_138_env(__lang_region* r, void* __p) {
  __anon_138_env* __s = (__anon_138_env*)__p;
  if (__s->__r == r) return __p;
  __anon_138_env* __d = (__anon_138_env*)__lang_region_alloc(r, sizeof(__anon_138_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_row_uq58 = __mcopy_Vec_int(r, __s->mu_row_uq58);
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  return (void*)__d;
}
static void* __mcopy_env___anon_137_env(__lang_region* r, void* __p) {
  __anon_137_env* __s = (__anon_137_env*)__p;
  if (__s->__r == r) return __p;
  __anon_137_env* __d = (__anon_137_env*)__lang_region_alloc(r, sizeof(__anon_137_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  return (void*)__d;
}
static void* __mcopy_env___anon_136_env(__lang_region* r, void* __p) {
  __anon_136_env* __s = (__anon_136_env*)__p;
  if (__s->__r == r) return __p;
  __anon_136_env* __d = (__anon_136_env*)__lang_region_alloc(r, sizeof(__anon_136_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  __d->mu_t1 = __mcopy_int(r, __s->mu_t1);
  return (void*)__d;
}
static void* __mcopy_env___anon_135_env(__lang_region* r, void* __p) {
  __anon_135_env* __s = (__anon_135_env*)__p;
  if (__s->__r == r) return __p;
  __anon_135_env* __d = (__anon_135_env*)__lang_region_alloc(r, sizeof(__anon_135_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  __d->mu_t0 = __mcopy_int(r, __s->mu_t0);
  return (void*)__d;
}
static void* __mcopy_env___anon_134_env(__lang_region* r, void* __p) {
  __anon_134_env* __s = (__anon_134_env*)__p;
  if (__s->__r == r) return __p;
  __anon_134_env* __d = (__anon_134_env*)__lang_region_alloc(r, sizeof(__anon_134_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_lengths = __mcopy_Vec_int(r, __s->mu_lengths);
  return (void*)__d;
}
static void* __mcopy_env___anon_133_env(__lang_region* r, void* __p) {
  __anon_133_env* __s = (__anon_133_env*)__p;
  if (__s->__r == r) return __p;
  __anon_133_env* __d = (__anon_133_env*)__lang_region_alloc(r, sizeof(__anon_133_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_nsym = __mcopy_int(r, __s->mu_nsym);
  __d->mu_freq = __mcopy_Vec_int(r, __s->mu_freq);
  return (void*)__d;
}
static void* __mcopy_env___anon_132_env(__lang_region* r, void* __p) {
  __anon_132_env* __s = (__anon_132_env*)__p;
  if (__s->__r == r) return __p;
  __anon_132_env* __d = (__anon_132_env*)__lang_region_alloc(r, sizeof(__anon_132_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pos = __mcopy_int(r, __s->mu_pos);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_131_env(__lang_region* r, void* __p) {
  __anon_131_env* __s = (__anon_131_env*)__p;
  if (__s->__r == r) return __p;
  __anon_131_env* __d = (__anon_131_env*)__lang_region_alloc(r, sizeof(__anon_131_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_v = __mcopy_int(r, __s->mu_v);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_130_env(__lang_region* r, void* __p) {
  __anon_130_env* __s = (__anon_130_env*)__p;
  if (__s->__r == r) return __p;
  __anon_130_env* __d = (__anon_130_env*)__lang_region_alloc(r, sizeof(__anon_130_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_129_env(__lang_region* r, void* __p) {
  __anon_129_env* __s = (__anon_129_env*)__p;
  if (__s->__r == r) return __p;
  __anon_129_env* __d = (__anon_129_env*)__lang_region_alloc(r, sizeof(__anon_129_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_128_env(__lang_region* r, void* __p) {
  __anon_128_env* __s = (__anon_128_env*)__p;
  if (__s->__r == r) return __p;
  __anon_128_env* __d = (__anon_128_env*)__lang_region_alloc(r, sizeof(__anon_128_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_127_env(__lang_region* r, void* __p) {
  __anon_127_env* __s = (__anon_127_env*)__p;
  if (__s->__r == r) return __p;
  __anon_127_env* __d = (__anon_127_env*)__lang_region_alloc(r, sizeof(__anon_127_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  return (void*)__d;
}
static void* __mcopy_env___anon_126_env(__lang_region* r, void* __p) {
  __anon_126_env* __s = (__anon_126_env*)__p;
  if (__s->__r == r) return __p;
  __anon_126_env* __d = (__anon_126_env*)__lang_region_alloc(r, sizeof(__anon_126_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  return (void*)__d;
}
static void* __mcopy_env___anon_125_env(__lang_region* r, void* __p) {
  __anon_125_env* __s = (__anon_125_env*)__p;
  if (__s->__r == r) return __p;
  __anon_125_env* __d = (__anon_125_env*)__lang_region_alloc(r, sizeof(__anon_125_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_124_env(__lang_region* r, void* __p) {
  __anon_124_env* __s = (__anon_124_env*)__p;
  if (__s->__r == r) return __p;
  __anon_124_env* __d = (__anon_124_env*)__lang_region_alloc(r, sizeof(__anon_124_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_123_env(__lang_region* r, void* __p) {
  __anon_123_env* __s = (__anon_123_env*)__p;
  if (__s->__r == r) return __p;
  __anon_123_env* __d = (__anon_123_env*)__lang_region_alloc(r, sizeof(__anon_123_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  return (void*)__d;
}
static void* __mcopy_env___anon_122_env(__lang_region* r, void* __p) {
  __anon_122_env* __s = (__anon_122_env*)__p;
  if (__s->__r == r) return __p;
  __anon_122_env* __d = (__anon_122_env*)__lang_region_alloc(r, sizeof(__anon_122_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_121_env(__lang_region* r, void* __p) {
  __anon_121_env* __s = (__anon_121_env*)__p;
  if (__s->__r == r) return __p;
  __anon_121_env* __d = (__anon_121_env*)__lang_region_alloc(r, sizeof(__anon_121_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  return (void*)__d;
}
static void* __mcopy_env___anon_120_env(__lang_region* r, void* __p) {
  __anon_120_env* __s = (__anon_120_env*)__p;
  if (__s->__r == r) return __p;
  __anon_120_env* __d = (__anon_120_env*)__lang_region_alloc(r, sizeof(__anon_120_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_119_env(__lang_region* r, void* __p) {
  __anon_119_env* __s = (__anon_119_env*)__p;
  if (__s->__r == r) return __p;
  __anon_119_env* __d = (__anon_119_env*)__lang_region_alloc(r, sizeof(__anon_119_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_118_env(__lang_region* r, void* __p) {
  __anon_118_env* __s = (__anon_118_env*)__p;
  if (__s->__r == r) return __p;
  __anon_118_env* __d = (__anon_118_env*)__lang_region_alloc(r, sizeof(__anon_118_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  return (void*)__d;
}
static void* __mcopy_env___anon_117_env(__lang_region* r, void* __p) {
  __anon_117_env* __s = (__anon_117_env*)__p;
  if (__s->__r == r) return __p;
  __anon_117_env* __d = (__anon_117_env*)__lang_region_alloc(r, sizeof(__anon_117_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_116_env(__lang_region* r, void* __p) {
  __anon_116_env* __s = (__anon_116_env*)__p;
  if (__s->__r == r) return __p;
  __anon_116_env* __d = (__anon_116_env*)__lang_region_alloc(r, sizeof(__anon_116_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_115_env(__lang_region* r, void* __p) {
  __anon_115_env* __s = (__anon_115_env*)__p;
  if (__s->__r == r) return __p;
  __anon_115_env* __d = (__anon_115_env*)__lang_region_alloc(r, sizeof(__anon_115_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_acc = __mcopy_int(r, __s->mu_acc);
  __d->mu_sq = __mcopy_int(r, __s->mu_sq);
  return (void*)__d;
}
static void* __mcopy_env___anon_114_env(__lang_region* r, void* __p) {
  __anon_114_env* __s = (__anon_114_env*)__p;
  if (__s->__r == r) return __p;
  __anon_114_env* __d = (__anon_114_env*)__lang_region_alloc(r, sizeof(__anon_114_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_113_env(__lang_region* r, void* __p) {
  __anon_113_env* __s = (__anon_113_env*)__p;
  if (__s->__r == r) return __p;
  __anon_113_env* __d = (__anon_113_env*)__lang_region_alloc(r, sizeof(__anon_113_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_112_env(__lang_region* r, void* __p) {
  __anon_112_env* __s = (__anon_112_env*)__p;
  if (__s->__r == r) return __p;
  __anon_112_env* __d = (__anon_112_env*)__lang_region_alloc(r, sizeof(__anon_112_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  return (void*)__d;
}
static void* __mcopy_env___anon_111_env(__lang_region* r, void* __p) {
  __anon_111_env* __s = (__anon_111_env*)__p;
  if (__s->__r == r) return __p;
  __anon_111_env* __d = (__anon_111_env*)__lang_region_alloc(r, sizeof(__anon_111_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cs = __mcopy_list_str(r, __s->mu_cs);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  return (void*)__d;
}
static void* __mcopy_env___anon_110_env(__lang_region* r, void* __p) {
  __anon_110_env* __s = (__anon_110_env*)__p;
  if (__s->__r == r) return __p;
  __anon_110_env* __d = (__anon_110_env*)__lang_region_alloc(r, sizeof(__anon_110_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cs = __mcopy_list_str(r, __s->mu_cs);
  __d->mu_start = __mcopy_int(r, __s->mu_start);
  return (void*)__d;
}
static void* __mcopy_env___anon_109_env(__lang_region* r, void* __p) {
  __anon_109_env* __s = (__anon_109_env*)__p;
  if (__s->__r == r) return __p;
  __anon_109_env* __d = (__anon_109_env*)__lang_region_alloc(r, sizeof(__anon_109_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_108_env(__lang_region* r, void* __p) {
  __anon_108_env* __s = (__anon_108_env*)__p;
  if (__s->__r == r) return __p;
  __anon_108_env* __d = (__anon_108_env*)__lang_region_alloc(r, sizeof(__anon_108_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_107_env(__lang_region* r, void* __p) {
  __anon_107_env* __s = (__anon_107_env*)__p;
  if (__s->__r == r) return __p;
  __anon_107_env* __d = (__anon_107_env*)__lang_region_alloc(r, sizeof(__anon_107_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_106_env(__lang_region* r, void* __p) {
  __anon_106_env* __s = (__anon_106_env*)__p;
  if (__s->__r == r) return __p;
  __anon_106_env* __d = (__anon_106_env*)__lang_region_alloc(r, sizeof(__anon_106_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_xs = __mcopy_list_int(r, __s->mu_xs);
  __d->mu_acc = __mcopy_int(r, __s->mu_acc);
  return (void*)__d;
}
static void* __mcopy_env___anon_105_env(__lang_region* r, void* __p) {
  __anon_105_env* __s = (__anon_105_env*)__p;
  if (__s->__r == r) return __p;
  __anon_105_env* __d = (__anon_105_env*)__lang_region_alloc(r, sizeof(__anon_105_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_104_env(__lang_region* r, void* __p) {
  __anon_104_env* __s = (__anon_104_env*)__p;
  if (__s->__r == r) return __p;
  __anon_104_env* __d = (__anon_104_env*)__lang_region_alloc(r, sizeof(__anon_104_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_103_env(__lang_region* r, void* __p) {
  __anon_103_env* __s = (__anon_103_env*)__p;
  if (__s->__r == r) return __p;
  __anon_103_env* __d = (__anon_103_env*)__lang_region_alloc(r, sizeof(__anon_103_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_t = __mcopy_Vec_str(r, __s->mu_t);
  return (void*)__d;
}
static void* __mcopy_env___anon_102_env(__lang_region* r, void* __p) {
  __anon_102_env* __s = (__anon_102_env*)__p;
  if (__s->__r == r) return __p;
  __anon_102_env* __d = (__anon_102_env*)__lang_region_alloc(r, sizeof(__anon_102_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_t = __mcopy_Vec_str(r, __s->mu_t);
  return (void*)__d;
}
static void* __mcopy_env___anon_101_env(__lang_region* r, void* __p) {
  __anon_101_env* __s = (__anon_101_env*)__p;
  if (__s->__r == r) return __p;
  __anon_101_env* __d = (__anon_101_env*)__lang_region_alloc(r, sizeof(__anon_101_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_acc = __mcopy_Vec_int(r, __s->mu_acc);
  return (void*)__d;
}
static void* __mcopy_env___anon_100_env(__lang_region* r, void* __p) {
  __anon_100_env* __s = (__anon_100_env*)__p;
  if (__s->__r == r) return __p;
  __anon_100_env* __d = (__anon_100_env*)__lang_region_alloc(r, sizeof(__anon_100_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_99_env(__lang_region* r, void* __p) {
  __anon_99_env* __s = (__anon_99_env*)__p;
  if (__s->__r == r) return __p;
  __anon_99_env* __d = (__anon_99_env*)__lang_region_alloc(r, sizeof(__anon_99_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_y = __mcopy_int(r, __s->mu_y);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_stride = __mcopy_int(r, __s->mu_stride);
  __d->mu_grey = __mcopy_bool(r, __s->mu_grey);
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_indexed = __mcopy_bool(r, __s->mu_indexed);
  __d->mu_pal = __mcopy_Vec_int(r, __s->mu_pal);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  __d->mu_bpp = __mcopy_int(r, __s->mu_bpp);
  return (void*)__d;
}
static void* __mcopy_env___anon_98_env(__lang_region* r, void* __p) {
  __anon_98_env* __s = (__anon_98_env*)__p;
  if (__s->__r == r) return __p;
  __anon_98_env* __d = (__anon_98_env*)__lang_region_alloc(r, sizeof(__anon_98_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  __d->mu_row_uq63 = __mcopy_Vec_int(r, __s->mu_row_uq63);
  __d->mu_prev = __mcopy_Vec_int(r, __s->mu_prev);
  __d->mu_bpp = __mcopy_int(r, __s->mu_bpp);
  return (void*)__d;
}
static void* __mcopy_env___anon_97_env(__lang_region* r, void* __p) {
  __anon_97_env* __s = (__anon_97_env*)__p;
  if (__s->__r == r) return __p;
  __anon_97_env* __d = (__anon_97_env*)__lang_region_alloc(r, sizeof(__anon_97_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_row_uq60 = __mcopy_Vec_int(r, __s->mu_row_uq60);
  return (void*)__d;
}
static void* __mcopy_env___anon_96_env(__lang_region* r, void* __p) {
  __anon_96_env* __s = (__anon_96_env*)__p;
  if (__s->__r == r) return __p;
  __anon_96_env* __d = (__anon_96_env*)__lang_region_alloc(r, sizeof(__anon_96_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_row_uq60 = __mcopy_Vec_int(r, __s->mu_row_uq60);
  return (void*)__d;
}
static void* __mcopy_env___anon_95_env(__lang_region* r, void* __p) {
  __anon_95_env* __s = (__anon_95_env*)__p;
  if (__s->__r == r) return __p;
  __anon_95_env* __d = (__anon_95_env*)__lang_region_alloc(r, sizeof(__anon_95_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_94_env(__lang_region* r, void* __p) {
  __anon_94_env* __s = (__anon_94_env*)__p;
  if (__s->__r == r) return __p;
  __anon_94_env* __d = (__anon_94_env*)__lang_region_alloc(r, sizeof(__anon_94_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_93_env(__lang_region* r, void* __p) {
  __anon_93_env* __s = (__anon_93_env*)__p;
  if (__s->__r == r) return __p;
  __anon_93_env* __d = (__anon_93_env*)__lang_region_alloc(r, sizeof(__anon_93_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_v = __mcopy_Vec_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_92_env(__lang_region* r, void* __p) {
  __anon_92_env* __s = (__anon_92_env*)__p;
  if (__s->__r == r) return __p;
  __anon_92_env* __d = (__anon_92_env*)__lang_region_alloc(r, sizeof(__anon_92_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_v = __mcopy_Vec_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_91_env(__lang_region* r, void* __p) {
  __anon_91_env* __s = (__anon_91_env*)__p;
  if (__s->__r == r) return __p;
  __anon_91_env* __d = (__anon_91_env*)__lang_region_alloc(r, sizeof(__anon_91_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_v = __mcopy_Vec_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_90_env(__lang_region* r, void* __p) {
  __anon_90_env* __s = (__anon_90_env*)__p;
  if (__s->__r == r) return __p;
  __anon_90_env* __d = (__anon_90_env*)__lang_region_alloc(r, sizeof(__anon_90_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_bits = __mcopy_int(r, __s->mu_bits);
  __d->mu_maxbits = __mcopy_int(r, __s->mu_maxbits);
  __d->mu_bl_count = __mcopy_Vec_int(r, __s->mu_bl_count);
  __d->mu_next_code = __mcopy_Vec_int(r, __s->mu_next_code);
  return (void*)__d;
}
static void* __mcopy_env___anon_89_env(__lang_region* r, void* __p) {
  __anon_89_env* __s = (__anon_89_env*)__p;
  if (__s->__r == r) return __p;
  __anon_89_env* __d = (__anon_89_env*)__lang_region_alloc(r, sizeof(__anon_89_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_bits = __mcopy_int(r, __s->mu_bits);
  __d->mu_maxbits = __mcopy_int(r, __s->mu_maxbits);
  __d->mu_bl_count = __mcopy_Vec_int(r, __s->mu_bl_count);
  __d->mu_next_code = __mcopy_Vec_int(r, __s->mu_next_code);
  return (void*)__d;
}
static void* __mcopy_env___anon_88_env(__lang_region* r, void* __p) {
  __anon_88_env* __s = (__anon_88_env*)__p;
  if (__s->__r == r) return __p;
  __anon_88_env* __d = (__anon_88_env*)__lang_region_alloc(r, sizeof(__anon_88_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_leafsym = __mcopy_Vec_int(r, __s->mu_leafsym);
  __d->mu_node = __mcopy_int(r, __s->mu_node);
  __d->mu_lengths = __mcopy_Vec_int(r, __s->mu_lengths);
  __d->mu_nl = __mcopy_Vec_int(r, __s->mu_nl);
  __d->mu_nr = __mcopy_Vec_int(r, __s->mu_nr);
  return (void*)__d;
}
static void* __mcopy_env___anon_87_env(__lang_region* r, void* __p) {
  __anon_87_env* __s = (__anon_87_env*)__p;
  if (__s->__r == r) return __p;
  __anon_87_env* __d = (__anon_87_env*)__lang_region_alloc(r, sizeof(__anon_87_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_np = __mcopy_int(r, __s->mu_np);
  __d->mu_alive = __mcopy_Vec_int(r, __s->mu_alive);
  __d->mu_skip = __mcopy_int(r, __s->mu_skip);
  __d->mu_nf = __mcopy_Vec_int(r, __s->mu_nf);
  return (void*)__d;
}
static void* __mcopy_env___anon_86_env(__lang_region* r, void* __p) {
  __anon_86_env* __s = (__anon_86_env*)__p;
  if (__s->__r == r) return __p;
  __anon_86_env* __d = (__anon_86_env*)__lang_region_alloc(r, sizeof(__anon_86_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_np = __mcopy_int(r, __s->mu_np);
  __d->mu_alive = __mcopy_Vec_int(r, __s->mu_alive);
  __d->mu_skip = __mcopy_int(r, __s->mu_skip);
  __d->mu_nf = __mcopy_Vec_int(r, __s->mu_nf);
  return (void*)__d;
}
static void* __mcopy_env___anon_85_env(__lang_region* r, void* __p) {
  __anon_85_env* __s = (__anon_85_env*)__p;
  if (__s->__r == r) return __p;
  __anon_85_env* __d = (__anon_85_env*)__lang_region_alloc(r, sizeof(__anon_85_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_d = __mcopy_int(r, __s->mu_d);
  __d->mu_max_back = __mcopy_int(r, __s->mu_max_back);
  __d->mu_pos = __mcopy_int(r, __s->mu_pos);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_84_env(__lang_region* r, void* __p) {
  __anon_84_env* __s = (__anon_84_env*)__p;
  if (__s->__r == r) return __p;
  __anon_84_env* __d = (__anon_84_env*)__lang_region_alloc(r, sizeof(__anon_84_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_v = __mcopy_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_83_env(__lang_region* r, void* __p) {
  __anon_83_env* __s = (__anon_83_env*)__p;
  if (__s->__r == r) return __p;
  __anon_83_env* __d = (__anon_83_env*)__lang_region_alloc(r, sizeof(__anon_83_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_bytes = __mcopy_Vec_int(r, __s->mu_bytes);
  return (void*)__d;
}
static void* __mcopy_env___anon_82_env(__lang_region* r, void* __p) {
  __anon_82_env* __s = (__anon_82_env*)__p;
  if (__s->__r == r) return __p;
  __anon_82_env* __d = (__anon_82_env*)__lang_region_alloc(r, sizeof(__anon_82_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_bytes = __mcopy_Vec_int(r, __s->mu_bytes);
  return (void*)__d;
}
static void* __mcopy_env___anon_81_env(__lang_region* r, void* __p) {
  __anon_81_env* __s = (__anon_81_env*)__p;
  if (__s->__r == r) return __p;
  __anon_81_env* __d = (__anon_81_env*)__lang_region_alloc(r, sizeof(__anon_81_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  return (void*)__d;
}
static void* __mcopy_env___anon_80_env(__lang_region* r, void* __p) {
  __anon_80_env* __s = (__anon_80_env*)__p;
  if (__s->__r == r) return __p;
  __anon_80_env* __d = (__anon_80_env*)__lang_region_alloc(r, sizeof(__anon_80_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_p_uq28 = __mcopy_int(r, __s->mu_p_uq28);
  __d->mu_out = __mcopy_ByteBuf(r, __s->mu_out);
  __d->mu_width = __mcopy_int(r, __s->mu_width);
  __d->mu_height = __mcopy_int(r, __s->mu_height);
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  __d->mu_bpp = __mcopy_int(r, __s->mu_bpp);
  return (void*)__d;
}
static void* __mcopy_env___anon_79_env(__lang_region* r, void* __p) {
  __anon_79_env* __s = (__anon_79_env*)__p;
  if (__s->__r == r) return __p;
  __anon_79_env* __d = (__anon_79_env*)__lang_region_alloc(r, sizeof(__anon_79_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_d = __mcopy_int(r, __s->mu_d);
  __d->mu_buf = __mcopy_StrBuf(r, __s->mu_buf);
  return (void*)__d;
}
static void* __mcopy_env___anon_78_env(__lang_region* r, void* __p) {
  __anon_78_env* __s = (__anon_78_env*)__p;
  if (__s->__r == r) return __p;
  __anon_78_env* __d = (__anon_78_env*)__lang_region_alloc(r, sizeof(__anon_78_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_d = __mcopy_int(r, __s->mu_d);
  __d->mu_buf = __mcopy_StrBuf(r, __s->mu_buf);
  return (void*)__d;
}
static void* __mcopy_env___anon_77_env(__lang_region* r, void* __p) {
  __anon_77_env* __s = (__anon_77_env*)__p;
  if (__s->__r == r) return __p;
  __anon_77_env* __d = (__anon_77_env*)__lang_region_alloc(r, sizeof(__anon_77_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_76_env(__lang_region* r, void* __p) {
  __anon_76_env* __s = (__anon_76_env*)__p;
  if (__s->__r == r) return __p;
  __anon_76_env* __d = (__anon_76_env*)__lang_region_alloc(r, sizeof(__anon_76_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_len = __mcopy_int(r, __s->mu_len);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  __d->mu_count = __mcopy_Vec_int(r, __s->mu_count);
  __d->mu_syms = __mcopy_Vec_int(r, __s->mu_syms);
  return (void*)__d;
}
static void* __mcopy_env___anon_75_env(__lang_region* r, void* __p) {
  __anon_75_env* __s = (__anon_75_env*)__p;
  if (__s->__r == r) return __p;
  __anon_75_env* __d = (__anon_75_env*)__lang_region_alloc(r, sizeof(__anon_75_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_74_env(__lang_region* r, void* __p) {
  __anon_74_env* __s = (__anon_74_env*)__p;
  if (__s->__r == r) return __p;
  __anon_74_env* __d = (__anon_74_env*)__lang_region_alloc(r, sizeof(__anon_74_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_bytes = __mcopy_Vec_int(r, __s->mu_bytes);
  return (void*)__d;
}
static void* __mcopy_env___anon_73_env(__lang_region* r, void* __p) {
  __anon_73_env* __s = (__anon_73_env*)__p;
  if (__s->__r == r) return __p;
  __anon_73_env* __d = (__anon_73_env*)__lang_region_alloc(r, sizeof(__anon_73_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  __d->mu_bytes = __mcopy_Vec_int(r, __s->mu_bytes);
  return (void*)__d;
}
static void* __mcopy_env___anon_72_env(__lang_region* r, void* __p) {
  __anon_72_env* __s = (__anon_72_env*)__p;
  if (__s->__r == r) return __p;
  __anon_72_env* __d = (__anon_72_env*)__lang_region_alloc(r, sizeof(__anon_72_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_i = __mcopy_int(r, __s->mu_i);
  return (void*)__d;
}
static void* __mcopy_env___anon_71_env(__lang_region* r, void* __p) {
  __anon_71_env* __s = (__anon_71_env*)__p;
  if (__s->__r == r) return __p;
  __anon_71_env* __d = (__anon_71_env*)__lang_region_alloc(r, sizeof(__anon_71_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_path = __mcopy_str(r, __s->mu_path);
  return (void*)__d;
}
static void* __mcopy_env___anon_70_env(__lang_region* r, void* __p) {
  __anon_70_env* __s = (__anon_70_env*)__p;
  if (__s->__r == r) return __p;
  __anon_70_env* __d = (__anon_70_env*)__lang_region_alloc(r, sizeof(__anon_70_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_69_env(__lang_region* r, void* __p) {
  __anon_69_env* __s = (__anon_69_env*)__p;
  if (__s->__r == r) return __p;
  __anon_69_env* __d = (__anon_69_env*)__lang_region_alloc(r, sizeof(__anon_69_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_68_env(__lang_region* r, void* __p) {
  __anon_68_env* __s = (__anon_68_env*)__p;
  if (__s->__r == r) return __p;
  __anon_68_env* __d = (__anon_68_env*)__lang_region_alloc(r, sizeof(__anon_68_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_67_env(__lang_region* r, void* __p) {
  __anon_67_env* __s = (__anon_67_env*)__p;
  if (__s->__r == r) return __p;
  __anon_67_env* __d = (__anon_67_env*)__lang_region_alloc(r, sizeof(__anon_67_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pal = __mcopy_Vec_int(r, __s->mu_pal);
  return (void*)__d;
}
static void* __mcopy_env___anon_66_env(__lang_region* r, void* __p) {
  __anon_66_env* __s = (__anon_66_env*)__p;
  if (__s->__r == r) return __p;
  __anon_66_env* __d = (__anon_66_env*)__lang_region_alloc(r, sizeof(__anon_66_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_65_env(__lang_region* r, void* __p) {
  __anon_65_env* __s = (__anon_65_env*)__p;
  if (__s->__r == r) return __p;
  __anon_65_env* __d = (__anon_65_env*)__lang_region_alloc(r, sizeof(__anon_65_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_pixels = __mcopy_Vec_int(r, __s->mu_pixels);
  return (void*)__d;
}
static void* __mcopy_env___anon_64_env(__lang_region* r, void* __p) {
  __anon_64_env* __s = (__anon_64_env*)__p;
  if (__s->__r == r) return __p;
  __anon_64_env* __d = (__anon_64_env*)__lang_region_alloc(r, sizeof(__anon_64_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_row_uq63 = __mcopy_Vec_int(r, __s->mu_row_uq63);
  return (void*)__d;
}
static void* __mcopy_env___anon_63_env(__lang_region* r, void* __p) {
  __anon_63_env* __s = (__anon_63_env*)__p;
  if (__s->__r == r) return __p;
  __anon_63_env* __d = (__anon_63_env*)__lang_region_alloc(r, sizeof(__anon_63_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_kind = __mcopy_int(r, __s->mu_kind);
  return (void*)__d;
}
static void* __mcopy_env___anon_62_env(__lang_region* r, void* __p) {
  __anon_62_env* __s = (__anon_62_env*)__p;
  if (__s->__r == r) return __p;
  __anon_62_env* __d = (__anon_62_env*)__lang_region_alloc(r, sizeof(__anon_62_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_61_env(__lang_region* r, void* __p) {
  __anon_61_env* __s = (__anon_61_env*)__p;
  if (__s->__r == r) return __p;
  __anon_61_env* __d = (__anon_61_env*)__lang_region_alloc(r, sizeof(__anon_61_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_60_env(__lang_region* r, void* __p) {
  __anon_60_env* __s = (__anon_60_env*)__p;
  if (__s->__r == r) return __p;
  __anon_60_env* __d = (__anon_60_env*)__lang_region_alloc(r, sizeof(__anon_60_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_59_env(__lang_region* r, void* __p) {
  __anon_59_env* __s = (__anon_59_env*)__p;
  if (__s->__r == r) return __p;
  __anon_59_env* __d = (__anon_59_env*)__lang_region_alloc(r, sizeof(__anon_59_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_out = __mcopy_Vec_int(r, __s->mu_out);
  return (void*)__d;
}
static void* __mcopy_env___anon_58_env(__lang_region* r, void* __p) {
  __anon_58_env* __s = (__anon_58_env*)__p;
  if (__s->__r == r) return __p;
  __anon_58_env* __d = (__anon_58_env*)__lang_region_alloc(r, sizeof(__anon_58_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_v = __mcopy_Vec_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_57_env(__lang_region* r, void* __p) {
  __anon_57_env* __s = (__anon_57_env*)__p;
  if (__s->__r == r) return __p;
  __anon_57_env* __d = (__anon_57_env*)__lang_region_alloc(r, sizeof(__anon_57_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_lengths = __mcopy_Vec_int(r, __s->mu_lengths);
  return (void*)__d;
}
static void* __mcopy_env___anon_56_env(__lang_region* r, void* __p) {
  __anon_56_env* __s = (__anon_56_env*)__p;
  if (__s->__r == r) return __p;
  __anon_56_env* __d = (__anon_56_env*)__lang_region_alloc(r, sizeof(__anon_56_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_freq = __mcopy_Vec_int(r, __s->mu_freq);
  return (void*)__d;
}
static void* __mcopy_env___anon_55_env(__lang_region* r, void* __p) {
  __anon_55_env* __s = (__anon_55_env*)__p;
  if (__s->__r == r) return __p;
  __anon_55_env* __d = (__anon_55_env*)__lang_region_alloc(r, sizeof(__anon_55_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_54_env(__lang_region* r, void* __p) {
  __anon_54_env* __s = (__anon_54_env*)__p;
  if (__s->__r == r) return __p;
  __anon_54_env* __d = (__anon_54_env*)__lang_region_alloc(r, sizeof(__anon_54_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_v = __mcopy_int(r, __s->mu_v);
  return (void*)__d;
}
static void* __mcopy_env___anon_53_env(__lang_region* r, void* __p) {
  __anon_53_env* __s = (__anon_53_env*)__p;
  if (__s->__r == r) return __p;
  __anon_53_env* __d = (__anon_53_env*)__lang_region_alloc(r, sizeof(__anon_53_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_52_env(__lang_region* r, void* __p) {
  __anon_52_env* __s = (__anon_52_env*)__p;
  if (__s->__r == r) return __p;
  __anon_52_env* __d = (__anon_52_env*)__lang_region_alloc(r, sizeof(__anon_52_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_51_env(__lang_region* r, void* __p) {
  __anon_51_env* __s = (__anon_51_env*)__p;
  if (__s->__r == r) return __p;
  __anon_51_env* __d = (__anon_51_env*)__lang_region_alloc(r, sizeof(__anon_51_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_crc = __mcopy_int(r, __s->mu_crc);
  return (void*)__d;
}
static void* __mcopy_env___anon_50_env(__lang_region* r, void* __p) {
  __anon_50_env* __s = (__anon_50_env*)__p;
  if (__s->__r == r) return __p;
  __anon_50_env* __d = (__anon_50_env*)__lang_region_alloc(r, sizeof(__anon_50_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_49_env(__lang_region* r, void* __p) {
  __anon_49_env* __s = (__anon_49_env*)__p;
  if (__s->__r == r) return __p;
  __anon_49_env* __d = (__anon_49_env*)__lang_region_alloc(r, sizeof(__anon_49_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_p_uq27 = __mcopy_int(r, __s->mu_p_uq27);
  return (void*)__d;
}
static void* __mcopy_env___anon_48_env(__lang_region* r, void* __p) {
  __anon_48_env* __s = (__anon_48_env*)__p;
  if (__s->__r == r) return __p;
  __anon_48_env* __d = (__anon_48_env*)__lang_region_alloc(r, sizeof(__anon_48_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_p_uq26 = __mcopy_int(r, __s->mu_p_uq26);
  return (void*)__d;
}
static void* __mcopy_env___anon_47_env(__lang_region* r, void* __p) {
  __anon_47_env* __s = (__anon_47_env*)__p;
  if (__s->__r == r) return __p;
  __anon_47_env* __d = (__anon_47_env*)__lang_region_alloc(r, sizeof(__anon_47_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_46_env(__lang_region* r, void* __p) {
  __anon_46_env* __s = (__anon_46_env*)__p;
  if (__s->__r == r) return __p;
  __anon_46_env* __d = (__anon_46_env*)__lang_region_alloc(r, sizeof(__anon_46_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_45_env(__lang_region* r, void* __p) {
  __anon_45_env* __s = (__anon_45_env*)__p;
  if (__s->__r == r) return __p;
  __anon_45_env* __d = (__anon_45_env*)__lang_region_alloc(r, sizeof(__anon_45_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_raw = __mcopy_Vec_int(r, __s->mu_raw);
  return (void*)__d;
}
static void* __mcopy_env___anon_44_env(__lang_region* r, void* __p) {
  __anon_44_env* __s = (__anon_44_env*)__p;
  if (__s->__r == r) return __p;
  __anon_44_env* __d = (__anon_44_env*)__lang_region_alloc(r, sizeof(__anon_44_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_43_env(__lang_region* r, void* __p) {
  __anon_43_env* __s = (__anon_43_env*)__p;
  if (__s->__r == r) return __p;
  __anon_43_env* __d = (__anon_43_env*)__lang_region_alloc(r, sizeof(__anon_43_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_colour = __mcopy_int(r, __s->mu_colour);
  return (void*)__d;
}
static void* __mcopy_env___anon_42_env(__lang_region* r, void* __p) {
  __anon_42_env* __s = (__anon_42_env*)__p;
  if (__s->__r == r) return __p;
  __anon_42_env* __d = (__anon_42_env*)__lang_region_alloc(r, sizeof(__anon_42_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_41_env(__lang_region* r, void* __p) {
  __anon_41_env* __s = (__anon_41_env*)__p;
  if (__s->__r == r) return __p;
  __anon_41_env* __d = (__anon_41_env*)__lang_region_alloc(r, sizeof(__anon_41_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_40_env(__lang_region* r, void* __p) {
  __anon_40_env* __s = (__anon_40_env*)__p;
  if (__s->__r == r) return __p;
  __anon_40_env* __d = (__anon_40_env*)__lang_region_alloc(r, sizeof(__anon_40_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_39_env(__lang_region* r, void* __p) {
  __anon_39_env* __s = (__anon_39_env*)__p;
  if (__s->__r == r) return __p;
  __anon_39_env* __d = (__anon_39_env*)__lang_region_alloc(r, sizeof(__anon_39_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_38_env(__lang_region* r, void* __p) {
  __anon_38_env* __s = (__anon_38_env*)__p;
  if (__s->__r == r) return __p;
  __anon_38_env* __d = (__anon_38_env*)__lang_region_alloc(r, sizeof(__anon_38_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_37_env(__lang_region* r, void* __p) {
  __anon_37_env* __s = (__anon_37_env*)__p;
  if (__s->__r == r) return __p;
  __anon_37_env* __d = (__anon_37_env*)__lang_region_alloc(r, sizeof(__anon_37_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_36_env(__lang_region* r, void* __p) {
  __anon_36_env* __s = (__anon_36_env*)__p;
  if (__s->__r == r) return __p;
  __anon_36_env* __d = (__anon_36_env*)__lang_region_alloc(r, sizeof(__anon_36_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_35_env(__lang_region* r, void* __p) {
  __anon_35_env* __s = (__anon_35_env*)__p;
  if (__s->__r == r) return __p;
  __anon_35_env* __d = (__anon_35_env*)__lang_region_alloc(r, sizeof(__anon_35_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_34_env(__lang_region* r, void* __p) {
  __anon_34_env* __s = (__anon_34_env*)__p;
  if (__s->__r == r) return __p;
  __anon_34_env* __d = (__anon_34_env*)__lang_region_alloc(r, sizeof(__anon_34_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_33_env(__lang_region* r, void* __p) {
  __anon_33_env* __s = (__anon_33_env*)__p;
  if (__s->__r == r) return __p;
  __anon_33_env* __d = (__anon_33_env*)__lang_region_alloc(r, sizeof(__anon_33_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_n = __mcopy_int(r, __s->mu_n);
  return (void*)__d;
}
static void* __mcopy_env___anon_32_env(__lang_region* r, void* __p) {
  __anon_32_env* __s = (__anon_32_env*)__p;
  if (__s->__r == r) return __p;
  __anon_32_env* __d = (__anon_32_env*)__lang_region_alloc(r, sizeof(__anon_32_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_31_env(__lang_region* r, void* __p) {
  __anon_31_env* __s = (__anon_31_env*)__p;
  if (__s->__r == r) return __p;
  __anon_31_env* __d = (__anon_31_env*)__lang_region_alloc(r, sizeof(__anon_31_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_30_env(__lang_region* r, void* __p) {
  __anon_30_env* __s = (__anon_30_env*)__p;
  if (__s->__r == r) return __p;
  __anon_30_env* __d = (__anon_30_env*)__lang_region_alloc(r, sizeof(__anon_30_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_data = __mcopy_Vec_int(r, __s->mu_data);
  return (void*)__d;
}
static void* __mcopy_env___anon_29_env(__lang_region* r, void* __p) {
  __anon_29_env* __s = (__anon_29_env*)__p;
  if (__s->__r == r) return __p;
  __anon_29_env* __d = (__anon_29_env*)__lang_region_alloc(r, sizeof(__anon_29_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_28_env(__lang_region* r, void* __p) {
  __anon_28_env* __s = (__anon_28_env*)__p;
  if (__s->__r == r) return __p;
  __anon_28_env* __d = (__anon_28_env*)__lang_region_alloc(r, sizeof(__anon_28_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_27_env(__lang_region* r, void* __p) {
  __anon_27_env* __s = (__anon_27_env*)__p;
  if (__s->__r == r) return __p;
  __anon_27_env* __d = (__anon_27_env*)__lang_region_alloc(r, sizeof(__anon_27_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_26_env(__lang_region* r, void* __p) {
  __anon_26_env* __s = (__anon_26_env*)__p;
  if (__s->__r == r) return __p;
  __anon_26_env* __d = (__anon_26_env*)__lang_region_alloc(r, sizeof(__anon_26_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_25_env(__lang_region* r, void* __p) {
  __anon_25_env* __s = (__anon_25_env*)__p;
  if (__s->__r == r) return __p;
  __anon_25_env* __d = (__anon_25_env*)__lang_region_alloc(r, sizeof(__anon_25_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_Vec_int(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_24_env(__lang_region* r, void* __p) {
  __anon_24_env* __s = (__anon_24_env*)__p;
  if (__s->__r == r) return __p;
  __anon_24_env* __d = (__anon_24_env*)__lang_region_alloc(r, sizeof(__anon_24_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_crc = __mcopy_int(r, __s->mu_crc);
  return (void*)__d;
}
static void* __mcopy_env___anon_23_env(__lang_region* r, void* __p) {
  __anon_23_env* __s = (__anon_23_env*)__p;
  if (__s->__r == r) return __p;
  __anon_23_env* __d = (__anon_23_env*)__lang_region_alloc(r, sizeof(__anon_23_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cond = __mcopy_bool(r, __s->mu_cond);
  return (void*)__d;
}
static void* __mcopy_env___anon_22_env(__lang_region* r, void* __p) {
  __anon_22_env* __s = (__anon_22_env*)__p;
  if (__s->__r == r) return __p;
  __anon_22_env* __d = (__anon_22_env*)__lang_region_alloc(r, sizeof(__anon_22_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_21_env(__lang_region* r, void* __p) {
  __anon_21_env* __s = (__anon_21_env*)__p;
  if (__s->__r == r) return __p;
  __anon_21_env* __d = (__anon_21_env*)__lang_region_alloc(r, sizeof(__anon_21_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_20_env(__lang_region* r, void* __p) {
  __anon_20_env* __s = (__anon_20_env*)__p;
  if (__s->__r == r) return __p;
  __anon_20_env* __d = (__anon_20_env*)__lang_region_alloc(r, sizeof(__anon_20_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_b = __mcopy_int(r, __s->mu_b);
  return (void*)__d;
}
static void* __mcopy_env___anon_19_env(__lang_region* r, void* __p) {
  __anon_19_env* __s = (__anon_19_env*)__p;
  if (__s->__r == r) return __p;
  __anon_19_env* __d = (__anon_19_env*)__lang_region_alloc(r, sizeof(__anon_19_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_acc = __mcopy_int(r, __s->mu_acc);
  return (void*)__d;
}
static void* __mcopy_env___anon_18_env(__lang_region* r, void* __p) {
  __anon_18_env* __s = (__anon_18_env*)__p;
  if (__s->__r == r) return __p;
  __anon_18_env* __d = (__anon_18_env*)__lang_region_alloc(r, sizeof(__anon_18_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_17_env(__lang_region* r, void* __p) {
  __anon_17_env* __s = (__anon_17_env*)__p;
  if (__s->__r == r) return __p;
  __anon_17_env* __d = (__anon_17_env*)__lang_region_alloc(r, sizeof(__anon_17_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_16_env(__lang_region* r, void* __p) {
  __anon_16_env* __s = (__anon_16_env*)__p;
  if (__s->__r == r) return __p;
  __anon_16_env* __d = (__anon_16_env*)__lang_region_alloc(r, sizeof(__anon_16_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_15_env(__lang_region* r, void* __p) {
  __anon_15_env* __s = (__anon_15_env*)__p;
  if (__s->__r == r) return __p;
  __anon_15_env* __d = (__anon_15_env*)__lang_region_alloc(r, sizeof(__anon_15_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_14_env(__lang_region* r, void* __p) {
  __anon_14_env* __s = (__anon_14_env*)__p;
  if (__s->__r == r) return __p;
  __anon_14_env* __d = (__anon_14_env*)__lang_region_alloc(r, sizeof(__anon_14_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cs = __mcopy_list_str(r, __s->mu_cs);
  return (void*)__d;
}
static void* __mcopy_env___anon_13_env(__lang_region* r, void* __p) {
  __anon_13_env* __s = (__anon_13_env*)__p;
  if (__s->__r == r) return __p;
  __anon_13_env* __d = (__anon_13_env*)__lang_region_alloc(r, sizeof(__anon_13_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_12_env(__lang_region* r, void* __p) {
  __anon_12_env* __s = (__anon_12_env*)__p;
  if (__s->__r == r) return __p;
  __anon_12_env* __d = (__anon_12_env*)__lang_region_alloc(r, sizeof(__anon_12_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cs = __mcopy_list_str(r, __s->mu_cs);
  return (void*)__d;
}
static void* __mcopy_env___anon_11_env(__lang_region* r, void* __p) {
  __anon_11_env* __s = (__anon_11_env*)__p;
  if (__s->__r == r) return __p;
  __anon_11_env* __d = (__anon_11_env*)__lang_region_alloc(r, sizeof(__anon_11_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_10_env(__lang_region* r, void* __p) {
  __anon_10_env* __s = (__anon_10_env*)__p;
  if (__s->__r == r) return __p;
  __anon_10_env* __d = (__anon_10_env*)__lang_region_alloc(r, sizeof(__anon_10_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_9_env(__lang_region* r, void* __p) {
  __anon_9_env* __s = (__anon_9_env*)__p;
  if (__s->__r == r) return __p;
  __anon_9_env* __d = (__anon_9_env*)__lang_region_alloc(r, sizeof(__anon_9_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_8_env(__lang_region* r, void* __p) {
  __anon_8_env* __s = (__anon_8_env*)__p;
  if (__s->__r == r) return __p;
  __anon_8_env* __d = (__anon_8_env*)__lang_region_alloc(r, sizeof(__anon_8_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_s = __mcopy_str(r, __s->mu_s);
  return (void*)__d;
}
static void* __mcopy_env___anon_7_env(__lang_region* r, void* __p) {
  __anon_7_env* __s = (__anon_7_env*)__p;
  if (__s->__r == r) return __p;
  __anon_7_env* __d = (__anon_7_env*)__lang_region_alloc(r, sizeof(__anon_7_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_cs = __mcopy_list_str(r, __s->mu_cs);
  return (void*)__d;
}
static void* __mcopy_env___anon_4_env(__lang_region* r, void* __p) {
  __anon_4_env* __s = (__anon_4_env*)__p;
  if (__s->__r == r) return __p;
  __anon_4_env* __d = (__anon_4_env*)__lang_region_alloc(r, sizeof(__anon_4_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_3_env(__lang_region* r, void* __p) {
  __anon_3_env* __s = (__anon_3_env*)__p;
  if (__s->__r == r) return __p;
  __anon_3_env* __d = (__anon_3_env*)__lang_region_alloc(r, sizeof(__anon_3_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_a = __mcopy_int(r, __s->mu_a);
  return (void*)__d;
}
static void* __mcopy_env___anon_2_env(__lang_region* r, void* __p) {
  __anon_2_env* __s = (__anon_2_env*)__p;
  if (__s->__r == r) return __p;
  __anon_2_env* __d = (__anon_2_env*)__lang_region_alloc(r, sizeof(__anon_2_env));
  *__d = *__s;
  __d->__r = r;
  __d->__copy = __s->__copy;
  __d->mu_xs = __mcopy_list_int(r, __s->mu_xs);
  return (void*)__d;
}

/* Phase 24.3: str_split builds a list_str by tokenizing s by delim. */
/* v0.1.38 (Unicode): the codepoint view. A str stays bytes; these
   walk UTF-8 sequences (invalid bytes count as single units). */
static int __lang_utf8_span(unsigned char b) {
  if (b < 0x80) return 1;
  if (b >= 0xC0 && b <= 0xDF) return 2;
  if (b >= 0xE0 && b <= 0xEF) return 3;
  if (b >= 0xF0 && b <= 0xF7) return 4;
  return 1;
}
static int64_t __lang_utf8_len(const char* s) {
  int64_t n = (int64_t)__lang_str_size(s), i = 0, c = 0;
  while (i < n) {
    int64_t l = __lang_utf8_span((unsigned char)s[i]);
    i += (l > n - i) ? (n - i) : l;
    c++;
  }
  return c;
}
static list_str __lang_utf8_chars(const char* s) {
  int n = (int)__lang_str_size(s);
  list_str acc = list_str__mk(0, &__mshared_list_str_t0);
  if (n == 0) return acc;
  int* st = (int*)malloc(sizeof(int) * ((size_t)n + 1));
  int k = 0, i = 0;
  while (i < n) {
    int l = __lang_utf8_span((unsigned char)s[i]);
    if (l > n - i) l = n - i;
    st[k++] = i; i += l;
  }
  st[k] = n;
  while (k > 0) {
    k--;
    int a = st[k], l = st[k + 1] - st[k];
    char* tok = __lang_str_alloc(__lang_current_region, (size_t)l);
    memcpy(tok, s + a, (size_t)l);
    tok[l] = '\0';
    struct list_str_node* cons = (struct list_str_node*)__lang_region_alloc(__lang_current_region, sizeof(struct list_str_node));
    cons->payload.Cons.f0 = tok;
    cons->payload.Cons.f1 = acc;
    acc = list_str__mk(1, cons);
  }
  free(st);
  return acc;
}
static list_str __lang_str_split(const char* s, const char* delim) {
  size_t slen = __lang_str_size(s);
  size_t dlen = __lang_str_size(delim);
  list_str nil_node = list_str__mk(0, &__mshared_list_str_t0);
  if (dlen == 0) {
    /* empty delim: return the whole string as a single element (matches interp) */
    struct list_str_node* cons = (struct list_str_node*)__lang_region_alloc(__lang_current_region, sizeof(struct list_str_node));
    cons->payload.Cons.f0 = s;
    cons->payload.Cons.f1 = nil_node;
    return list_str__mk(1, cons);
  }
  /* count tokens */
  size_t n = 1;
  for (size_t i = 0; i + dlen <= slen; ) {
    if (memcmp(s + i, delim, dlen) == 0) { n++; i += dlen; }
    else i++;
  }
  /* allocate cons cells */
  struct list_str_node** cells = (struct list_str_node**)__lang_region_alloc(__lang_current_region, n * sizeof(struct list_str_node*));
  for (size_t k = 0; k < n; k++) {
    cells[k] = (struct list_str_node*)__lang_region_alloc(__lang_current_region, sizeof(struct list_str_node));
  }
  /* fill tokens + link */
  size_t start = 0, idx = 0;
  for (size_t i = 0; i + dlen <= slen; ) {
    if (memcmp(s + i, delim, dlen) == 0) {
      size_t tlen = i - start;
      char* tok = __lang_str_alloc(__lang_current_region, tlen);
      memcpy(tok, s + start, tlen);
      tok[tlen] = '\0';
      cells[idx]->payload.Cons.f0 = tok;
      cells[idx]->payload.Cons.f1 = list_str__mk(1, cells[idx + 1]);
      idx++;
      start = i + dlen;
      i = start;
    } else i++;
  }
  /* last token */
  size_t tlen = slen - start;
  char* tok = __lang_str_alloc(__lang_current_region, tlen);
  memcpy(tok, s + start, tlen);
  tok[tlen] = '\0';
  cells[idx]->payload.Cons.f0 = tok;
  cells[idx]->payload.Cons.f1 = nil_node;
  return list_str__mk(1, cells[0]);
}

/* Phase 24.3: str_join sep xs — concat list_str elements with sep. */
static const char* __lang_str_join(const char* sep, list_str xs) {
  if (list_str__tag(xs) == 0) return __lang_str_of_cstr("");
  size_t seplen = __lang_str_size(sep);
  size_t total = 0;
  int first = 1;
  list_str cur = xs;
  while (list_str__tag(cur) == 1) {
    if (!first) total += seplen;
    total += __lang_str_size(list_str__node(cur)->payload.Cons.f0);
    first = 0;
    cur = list_str__node(cur)->payload.Cons.f1;
  }
  char* r = __lang_str_alloc(__lang_current_region, total);
  size_t pos = 0;
  first = 1;
  cur = xs;
  while (list_str__tag(cur) == 1) {
    if (!first) {
      memcpy(r + pos, sep, seplen);
      pos += seplen;
    }
    size_t l = __lang_str_size(list_str__node(cur)->payload.Cons.f0);
    memcpy(r + pos, list_str__node(cur)->payload.Cons.f0, l);
    pos += l;
    first = 0;
    cur = list_str__node(cur)->payload.Cons.f1;
  }
  r[pos] = '\0';
  return r;
}

#include <dirent.h>
static int __lang_list_dir_qsort(const void* a, const void* b) {
  return strcmp(*(const char* const*)a, *(const char* const*)b);
}
static list_str __lang_list_dir(const char* path) {
  DIR* d = opendir(path);
  if (!d) __lang_fail_impl(path);
  size_t cap = 16, n = 0;
  const char** arr = (const char**)malloc(cap * sizeof(char*));
  struct dirent* ent;
  while ((ent = readdir(d)) != NULL) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
    if (n == cap) { cap *= 2; arr = (const char**)realloc(arr, cap * sizeof(char*)); }
    size_t nlen = strlen(ent->d_name);
    char* copy = __lang_str_alloc(__lang_current_region, nlen);
    memcpy(copy, ent->d_name, nlen + 1);
    arr[n++] = copy;
  }
  closedir(d);
  qsort(arr, n, sizeof(char*), __lang_list_dir_qsort);
  list_str nil_node = list_str__mk(0, &__mshared_list_str_t0);
  list_str head = nil_node;
  for (size_t k = 0; k < n; k++) {
    size_t i = n - 1 - k;
    struct list_str_node* cons = (struct list_str_node*)__lang_region_alloc(__lang_current_region, sizeof(struct list_str_node));
    cons->payload.Cons.f0 = arr[i];
    cons->payload.Cons.f1 = head;
    head = list_str__mk(1, cons);
  }
  free(arr);
  return head;
}

static int __lang_argc = 0;
static char** __lang_argv = 0;
static list_str __lang_args(void) {
  list_str head = list_str__mk(0, &__mshared_list_str_t0);
  for (int k = 1; k < __lang_argc; k++) {
    int i = __lang_argc - k;  /* reverse so argv[1] ends up first */
    const char* a = __lang_argv[i];
    size_t nlen = strlen(a);
    char* copy = __lang_str_alloc(__lang_current_region, nlen);
    memcpy(copy, a, nlen + 1);
    struct list_str_node* cons = (struct list_str_node*)__lang_region_alloc(__lang_current_region, sizeof(struct list_str_node));
    cons->payload.Cons.f0 = copy;
    cons->payload.Cons.f1 = head;
    head = list_str__mk(1, cons);
  }
  return head;
}

static closure_int_closure_closure_int_closure_int_int_int mu_list_fold(list_int);
static closure_int_closure_list_int_list_int mu__range_down(long long);
static closure_int_list_int mu_range(long long);
static long long mu_list_sum(list_int);
static long long mu_list_product(list_int);
static closure_int_str mu__u8_nth(list_str);
static closure_int_str mu_utf8_at(const char*);
static int mu__cp_bad(long long);
static const char* mu_str_of_codepoint(long long);
static long long mu__cp_span(long long);
static closure_int_int mu__cp_cont(const char*);
static long long mu_codepoint_of(const char*);
static closure_int_int mu_codepoint_or(const char*);
static closure_int_int mu_codepoint_at(const char*);
static closure_int_closure_int_closure_str_str mu__u8_slice(list_str);
static closure_int_closure_int_str mu_utf8_sub(const char*);
static closure_str_str mu__u8_rev_join(list_str);
static const char* mu_utf8_rev(const char*);
static long long mu__eaw_width(long long);
static closure_int_closure_int_closure_int_int mu__u8w_go(const char*);
static long long mu_utf8_width(const char*);
static closure_int_str mu_pad_right(const char*);
static closure_int_str mu_pad_left(const char*);
static long long mu_sign(long long);
static long long mu_incr(long long);
static long long mu_decr(long long);
static long long mu_square(long long);
static long long mu_cube(long long);
static closure_int_int mu_sum_range(long long);
static closure_int_closure_int_int mu__pow_go(long long);
static closure_int_int mu_pow(long long);
static closure_int_int mu_lcm(long long);
static closure_int_tuple_int_int mu_divmod(long long);
static closure_str_unit mu_assert(int);
static closure_int_int mu_crc_byte(long long);
static long long mu_crc32_vec(mere_vec_int*);
static mere_vec_int* mu_br_new__int__Vec_int(long long);
static closure_Vec_int_int mu_br_bit(mere_vec_int*);
static closure_Vec_int_closure_int_int mu_br_bits(mere_vec_int*);
static int mu_br_align(mere_vec_int*);
static tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int(mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_int mu_huff_decode(mere_vec_int*);
static mere_vec_int* mu_vec_of(list_int);
static mere_vec_int* mu_fixed_lit_lengths(int);
static mere_vec_int* mu_fixed_dist_lengths(int);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit mu_inflate_codes(mere_vec_int*);
static closure_Vec_int_tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int(mere_vec_int*);
static closure_int_Vec_int mu_inflate(mere_vec_int*);
static long long mu_gzip_body_start(mere_vec_int*);
static int mu_is_gzip(mere_vec_int*);
static mere_vec_int* mu_gunzip(mere_vec_int*);
static closure_int_int mu_gzip_trailer_u32(mere_vec_int*);
static closure_Vec_int_bool mu_gunzip_ok(mere_vec_int*);
static closure_int_Vec_int mu_new_vec(long long);
static int mu_is_png(mere_vec_int*);
static closure_int_int mu_u32be(mere_vec_int*);
static closure_int_closure_int_closure_int_closure_int_closure_int_bool mu_type_is(mere_vec_int*);
static closure_int_bool mu_is_ihdr(mere_vec_int*);
static closure_int_bool mu_is_idat(mere_vec_int*);
static closure_int_bool mu_is_iend(mere_vec_int*);
static closure_int_bool mu_is_plte(mere_vec_int*);
static closure_int_bool mu_is_text(mere_vec_int*);
static closure_int_bool mu_is_gama(mere_vec_int*);
static closure_int_bool mu_is_trns(mere_vec_int*);
static mere_vec_int* mu_read_header__Vec_int__Vec_int(mere_vec_int*);
static long long mu_channels(long long);
static closure_int_int mu_pixel_bytes(long long);
static mere_vec_int* mu_palette(mere_vec_int*);
static mere_vec_int* mu_idat_bytes(mere_vec_int*);
static mere_vec_str* mu_text_chunks(mere_vec_int*);
static long long mu_gamma(mere_vec_int*);
static int mu_has_transparency(mere_vec_int*);
static closure_int_closure_int_int mu_paeth(long long);
static closure_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf(mere_vec_int*);
static closure_int_closure_int_closure_int_ByteBuf mu_unfilter__Vec_int__int__int__int__ByteBuf(mere_vec_int*);
static int mu_is_interlaced(mere_vec_int*);
static long long mu_adam7_xstart(long long);
static long long mu_adam7_ystart(long long);
static long long mu_adam7_xstep(long long);
static long long mu_adam7_ystep(long long);
static closure_int_int mu_ceil_div(long long);
static closure_int_int mu_pass_width(long long);
static closure_int_int mu_pass_height(long long);
static closure_int_closure_int_closure_int_ByteBuf mu_deinterlace__Vec_int__int__int__int__ByteBuf(mere_vec_int*);
static mere_bytebuf* mu_decode__Vec_int__ByteBuf(mere_vec_int*);
static mere_vec_int* mu_rgb8(mere_vec_int*);
static mere_vec_int* mu_vec_of__v2(list_int);
static closure_int_int mu_crc_byte__v2(long long);
static long long mu_crc32_vec__v2(mere_vec_int*);
static mere_vec_int* mu_bw_new__unit__Vec_int(int);
static closure_Vec_int_closure_int_closure_int_unit mu_bw_bits(mere_vec_int*);
static closure_Vec_int_unit mu_bw_flush(mere_vec_int*);
static closure_int_int mu_rev_bits(long long);
static tuple_int_int_int mu_len_encode(long long);
static tuple_int_int_int mu_dist_encode(long long);
static closure_int_closure_int_tuple_int_int mu_find_match(mere_vec_int*);
static closure_int_closure_int_Vec_int mu_build_lengths__Vec_int__int__int__Vec_int(mere_vec_int*);
static closure_int_closure_int_Vec_int mu_canon_codes__Vec_int__int__int__Vec_int(mere_vec_int*);
static closure_int_int mu_vmax(mere_vec_int*);
static mere_vec_int* mu_tokenize__Vec_int__Vec_int(mere_vec_int*);
static mere_vec_int* mu_deflate_dynamic__Vec_int__Vec_int(mere_vec_int*);
static mere_vec_int* mu_deflate_stored__Vec_int__Vec_int(mere_vec_int*);
static mere_vec_int* mu_deflate_body__Vec_int__Vec_int(mere_vec_int*);
static mere_vec_int* mu_deflate_gzip(mere_vec_int*);
static closure_int_unit mu_bytes_push4be(mere_vec_int*);
static closure_Vec_int_unit mu_push_all(mere_vec_int*);
static long long mu_adler32(mere_vec_int*);
static closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit mu_chunk(mere_vec_int*);
static closure_int_closure_int_int mu_paeth_e(long long);
static closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_filter_row(long long);
static long long mu_row_cost(mere_vec_int*);
static closure_Vec_int_closure_int_Vec_int mu_best_filter__Vec_int__Vec_int__int__Vec_int(mere_vec_int*);
static closure_int_bool mu_all_grey(mere_vec_int*);
static closure_int_Vec_int mu_collect_palette(mere_vec_int*);
static closure_int_closure_int_closure_int_int mu_palette_index(mere_vec_int*);
static closure_int_closure_int_closure_int_Vec_int mu_encode_at_depth__Vec_int__int__int__int__Vec_int(mere_vec_int*);
static closure_int_closure_int_Vec_int mu_encode_rgb8__Vec_int__int__int__Vec_int(mere_vec_int*);
static closure_int_closure_int_Vec_int mu_encode16__Vec_int__int__int__Vec_int(mere_vec_int*);
static const char* mu_colour_name(long long);
static int mu_info(mere_vec_int*);
static mere_vec_int* mu_ppm_bytes(mere_vec_int*);
static mere_vec_int* mu_read_ppm__Vec_int__Vec_int(mere_vec_int*);
static mere_vec_int* mu_load(const char*);
static closure_str_unit mu_encode_cmd(const char*);
static int mu___mere_user_main(int);
static const char* mu_png_info(mere_bytes*);
static mere_bytes* mu_png_to_ppm(mere_bytes*);
static mere_bytes* mu_ppm_to_png(mere_bytes*);
static long long mu_list_fold__direct(list_int, long long, closure_int_closure_int_int);
static list_int mu__range_down__direct(long long, long long, list_int);
static list_int mu_range__direct(long long, long long);
static long long mu_list_sum__direct(list_int);
static long long mu_list_product__direct(list_int);
static const char* mu__u8_nth__direct(list_str, long long);
static const char* mu_utf8_at__direct(const char*, long long);
static int mu__cp_bad__direct(long long);
static const char* mu_str_of_codepoint__direct(long long);
static long long mu__cp_span__direct(long long);
static long long mu__cp_cont__direct(const char*, long long);
static long long mu_codepoint_of__direct(const char*);
static long long mu_codepoint_or__direct(const char*, long long);
static long long mu_codepoint_at__direct(const char*, long long);
static const char* mu__u8_slice__direct(list_str, long long, long long, const char*);
static const char* mu_utf8_sub__direct(const char*, long long, long long);
static const char* mu__u8_rev_join__direct(list_str, const char*);
static const char* mu_utf8_rev__direct(const char*);
static long long mu__eaw_width__direct(long long);
static long long mu__u8w_go__direct(const char*, long long, long long, long long);
static long long mu_utf8_width__direct(const char*);
static const char* mu_pad_right__direct(const char*, long long);
static const char* mu_pad_left__direct(const char*, long long);
static long long mu_sign__direct(long long);
static long long mu_incr__direct(long long);
static long long mu_decr__direct(long long);
static long long mu_square__direct(long long);
static long long mu_cube__direct(long long);
static long long mu_sum_range__direct(long long, long long);
static long long mu__pow_go__direct(long long, long long, long long);
static long long mu_pow__direct(long long, long long);
static long long mu_lcm__direct(long long, long long);
static tuple_int_int mu_divmod__direct(long long, long long);
static int mu_assert__direct(int, const char*);
static long long mu_crc_byte__direct(long long, long long);
static long long mu_crc32_vec__direct(mere_vec_int*);
static mere_vec_int* mu_br_new__int__Vec_int__direct(__lang_region* /* __rp4010 */, long long);
static long long mu_br_bit__direct(mere_vec_int*, mere_vec_int*);
static long long mu_br_bits__direct(mere_vec_int*, mere_vec_int*, long long);
static int mu_br_align__direct(mere_vec_int*);
static tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_region* /* __rp4060 */, __lang_region* /* __rp4091 */, mere_vec_int*);
static long long mu_huff_decode__direct(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*);
static mere_vec_int* mu_vec_of__direct(list_int);
static mere_vec_int* mu_fixed_lit_lengths__direct(int);
static mere_vec_int* mu_fixed_dist_lengths__direct(int);
static int mu_inflate_codes__direct(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*);
static tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_region* /* __rp4446 */, __lang_region* /* __rp4450 */, mere_vec_int*, mere_vec_int*);
static mere_vec_int* mu_inflate__direct(mere_vec_int*, long long);
static long long mu_gzip_body_start__direct(mere_vec_int*);
static int mu_is_gzip__direct(mere_vec_int*);
static mere_vec_int* mu_gunzip__direct(mere_vec_int*);
static long long mu_gzip_trailer_u32__direct(mere_vec_int*, long long);
static int mu_gunzip_ok__direct(mere_vec_int*, mere_vec_int*);
static mere_vec_int* mu_new_vec__direct(__lang_region* /* __rp4583 */, long long, long long);
static int mu_is_png__direct(mere_vec_int*);
static long long mu_u32be__direct(mere_vec_int*, long long);
static int mu_type_is__direct(mere_vec_int*, long long, long long, long long, long long, long long);
static int mu_is_ihdr__direct(mere_vec_int*, long long);
static int mu_is_idat__direct(mere_vec_int*, long long);
static int mu_is_iend__direct(mere_vec_int*, long long);
static int mu_is_plte__direct(mere_vec_int*, long long);
static int mu_is_text__direct(mere_vec_int*, long long);
static int mu_is_gama__direct(mere_vec_int*, long long);
static int mu_is_trns__direct(mere_vec_int*, long long);
static mere_vec_int* mu_read_header__Vec_int__Vec_int__direct(__lang_region* /* __rp4699 */, mere_vec_int*);
static long long mu_channels__direct(long long);
static long long mu_pixel_bytes__direct(long long, long long);
static mere_vec_int* mu_palette__direct(mere_vec_int*);
static mere_vec_int* mu_idat_bytes__direct(mere_vec_int*);
static mere_vec_str* mu_text_chunks__direct(mere_vec_int*);
static long long mu_gamma__direct(mere_vec_int*);
static int mu_has_transparency__direct(mere_vec_int*);
static long long mu_paeth__direct(long long, long long, long long);
static mere_bytebuf* mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_region* /* __rp4848 */, mere_vec_int*, long long, long long, long long, long long);
static mere_bytebuf* mu_unfilter__Vec_int__int__int__int__ByteBuf__direct(__lang_region* /* __rp4853 */, mere_vec_int*, long long, long long, long long);
static int mu_is_interlaced__direct(mere_vec_int*);
static long long mu_adam7_xstart__direct(long long);
static long long mu_adam7_ystart__direct(long long);
static long long mu_adam7_xstep__direct(long long);
static long long mu_adam7_ystep__direct(long long);
static long long mu_ceil_div__direct(long long, long long);
static long long mu_pass_width__direct(long long, long long);
static long long mu_pass_height__direct(long long, long long);
static mere_bytebuf* mu_deinterlace__Vec_int__int__int__int__ByteBuf__direct(__lang_region* /* __rp4899 */, mere_vec_int*, long long, long long, long long);
static mere_bytebuf* mu_decode__Vec_int__ByteBuf__direct(__lang_region* /* __rp4923 */, mere_vec_int*);
static mere_vec_int* mu_rgb8__direct(mere_vec_int*);
static mere_vec_int* mu_vec_of__v2__direct(list_int);
static long long mu_crc_byte__v2__direct(long long, long long);
static long long mu_crc32_vec__v2__direct(mere_vec_int*);
static mere_vec_int* mu_bw_new__unit__Vec_int__direct(__lang_region* /* __rp5155 */, int);
static int mu_bw_bits__direct(mere_vec_int*, mere_vec_int*, long long, long long);
static int mu_bw_flush__direct(mere_vec_int*, mere_vec_int*);
static long long mu_rev_bits__direct(long long, long long);
static tuple_int_int_int mu_len_encode__direct(long long);
static tuple_int_int_int mu_dist_encode__direct(long long);
static tuple_int_int mu_find_match__direct(mere_vec_int*, long long, long long);
static mere_vec_int* mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_region* /* __rp5340 */, mere_vec_int*, long long, long long);
static mere_vec_int* mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_region* /* __rp5499 */, mere_vec_int*, long long, long long);
static long long mu_vmax__direct(mere_vec_int*, long long);
static mere_vec_int* mu_tokenize__Vec_int__Vec_int__direct(__lang_region* /* __rp5580 */, mere_vec_int*);
static mere_vec_int* mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_region* /* __rp5839 */, mere_vec_int*);
static mere_vec_int* mu_deflate_stored__Vec_int__Vec_int__direct(__lang_region* /* __rp5938 */, mere_vec_int*);
static mere_vec_int* mu_deflate_body__Vec_int__Vec_int__direct(__lang_region* /* __rp6075 */, mere_vec_int*);
static mere_vec_int* mu_deflate_gzip__direct(__lang_region* /* __rp6080 */, mere_vec_int*);
static int mu_bytes_push4be__direct(mere_vec_int*, long long);
static int mu_push_all__direct(mere_vec_int*, mere_vec_int*);
static long long mu_adler32__direct(mere_vec_int*);
static int mu_chunk__direct(mere_vec_int*, long long, long long, long long, long long, mere_vec_int*);
static long long mu_paeth_e__direct(long long, long long, long long);
static mere_vec_int* mu_filter_row__direct(long long, mere_vec_int*, mere_vec_int*, long long);
static long long mu_row_cost__direct(mere_vec_int*);
static mere_vec_int* mu_best_filter__Vec_int__Vec_int__int__Vec_int__direct(__lang_region* /* __rp6275 */, mere_vec_int*, mere_vec_int*, long long);
static int mu_all_grey__direct(mere_vec_int*, long long);
static mere_vec_int* mu_collect_palette__direct(mere_vec_int*, long long);
static long long mu_palette_index__direct(mere_vec_int*, long long, long long, long long);
static mere_vec_int* mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__lang_region* /* __rp6484 */, mere_vec_int*, long long, long long, long long);
static mere_vec_int* mu_encode_rgb8__Vec_int__int__int__Vec_int__direct(__lang_region* /* __rp6529 */, mere_vec_int*, long long, long long);
static mere_vec_int* mu_encode16__Vec_int__int__int__Vec_int__direct(__lang_region* /* __rp6534 */, mere_vec_int*, long long, long long);
static const char* mu_colour_name__direct(long long);
static int mu_info__direct(mere_vec_int*);
static mere_vec_int* mu_ppm_bytes__direct(mere_vec_int*);
static mere_vec_int* mu_read_ppm__Vec_int__Vec_int__direct(__lang_region* /* __rp6610 */, mere_vec_int*);
static mere_vec_int* mu_load__direct(const char*);
static int mu_encode_cmd__direct(const char*, const char*);
static int mu___mere_user_main__direct(int);
static const char* mu_png_info__direct(mere_bytes*);
static mere_bytes* mu_png_to_ppm__direct(mere_bytes*);
static mere_bytes* mu_ppm_to_png__direct(mere_bytes*);
static closure_int_int __lifted_go_0(long long);
static closure_int_int __lifted_go__rvfast_1(long long, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq1_2(long long, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq2_3(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_zc_4(mere_vec_int*, long long);
static int __lifted_tally__rvfast_5(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_tally_6(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_bylen_7(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_scan_8(long long, mere_vec_int*, long long, mere_vec_int*, long long);
static closure_int_closure_int_closure_int_int __lifted_go__rvfast_uq3_9(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static closure_int_closure_int_closure_int_int __lifted_go_uq4_10(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static mere_vec_int* __lifted_go_uq5_11(mere_vec_int*, list_int);
static mere_vec_int* __lifted_go_uq6_12(mere_vec_int*, long long);
static mere_vec_int* __lifted_go_uq7_13(mere_vec_int*, long long);
static int __lifted_loop_14(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, int);
static int __lifted_copy_15(long long, mere_vec_int*, long long, long long);
static int __lifted_zero19_16(mere_vec_int*, long long);
static int __lifted_rd__rvfast_17(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_rd_18(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_fill_19(mere_vec_int*, long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, int);
static int __lifted_p_uq9_20(long long, mere_vec_int*, long long, long long);
static int __lifted_p_uq8_21(long long, mere_vec_int*, long long);
static int __lifted_p_22(long long, mere_vec_int*, long long);
static int __lifted_split_23(long long, long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static mere_vec_int* __lifted_block_24(mere_vec_int*, mere_vec_int*, mere_vec_int*, int);
static int __lifted_copy_uq10_25(long long, mere_vec_int*, mere_vec_int*, long long, long long);
static long long __lifted_skip_z_26(mere_vec_int*, long long);
static int __lifted_go_uq11_27(long long, mere_vec_int*, long long, long long);
static int __lifted_go__rvfast_uq13_28(mere_vec_int*, mere_vec_int*, long long);
static int __lifted_go_uq14_29(mere_vec_int*, mere_vec_int*, long long);
static mere_vec_int* __lifted_walk_30(mere_vec_int*, long long);
static mere_vec_int* __lifted_walk_uq15_31(long long, mere_vec_int*, mere_vec_int*, long long);
static mere_vec_int* __lifted_copy_uq16_32(long long, mere_vec_int*, mere_vec_int*, long long, long long);
static mere_vec_int* __lifted_walk_uq17_33(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_copy_uq18_34(long long, mere_vec_int*, mere_vec_int*, long long, long long);
static mere_vec_str* __lifted_walk_uq19_35(long long, mere_vec_str*, mere_vec_int*, long long);
static closure_bool_unit __lifted_chars__rvfast_36(long long, mere_vec_int*, long long, mere_strbuf*, long long);
static closure_bool_unit __lifted_chars_37(long long, mere_vec_int*, long long, mere_strbuf*, long long);
static long long __lifted_walk_uq20_38(long long, mere_vec_int*, long long);
static int __lifted_walk_uq21_39(long long, mere_vec_int*, long long);
static mere_bytebuf* __lifted_row_40(long long, mere_bytebuf*, long long, long long, mere_vec_int*, long long, long long);
static int __lifted_col__rvfast_41(long long, mere_vec_int*, long long, long long, mere_bytebuf*, long long, long long, long long, long long);
static int __lifted_col_42(long long, mere_vec_int*, long long, long long, mere_bytebuf*, long long, long long, long long, long long);
static closure_int_ByteBuf __lifted_pass_43(mere_bytebuf*, long long, long long, mere_vec_int*, long long, long long);
static int __lifted_place_44(long long, long long, long long, long long, long long, mere_bytebuf*, mere_bytebuf*, long long);
static int __lifted_px_45(long long, long long, long long, long long, long long, mere_bytebuf*, mere_bytebuf*, long long);
static int __lifted_byte_46(long long, mere_bytebuf*, long long, mere_bytebuf*, long long, long long);
static mere_vec_int* __lifted_go_uq29_47(long long, long long, mere_vec_int*, long long, mere_bytebuf*, long long, mere_vec_int*, long long, long long);
static mere_vec_int* __lifted_go_uq30_48(mere_vec_int*, list_int);
static closure_int_int __lifted_go_uq31_49(long long);
static closure_int_int __lifted_go__rvfast_uq32_50(long long, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq33_51(long long, mere_vec_int*, long long);
static int __lifted_go_uq34_52(long long, long long, mere_vec_int*, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq35_53(long long, long long, long long);
static tuple_int_int_int __lifted_go__rvfast_uq36_54(mere_vec_int*, mere_vec_int*, long long, long long);
static tuple_int_int_int __lifted_go_uq37_55(mere_vec_int*, mere_vec_int*, long long, long long);
static tuple_int_int_int __lifted_go__rvfast_uq38_56(mere_vec_int*, long long, mere_vec_int*, long long);
static tuple_int_int_int __lifted_go_uq39_57(mere_vec_int*, long long, mere_vec_int*, long long);
static closure_int_closure_int_tuple_int_int __lifted_best_58(long long, long long, long long, mere_vec_int*, long long);
static long long __lifted_mlen__rvfast_59(long long, long long, mere_vec_int*, long long, long long);
static long long __lifted_mlen_60(long long, long long, mere_vec_int*, long long, long long);
static int __lifted_seed_61(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_seed2_62(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_zl_63(long long, mere_vec_int*, long long);
static int __lifted_al_64(long long, mere_vec_int*, long long);
static int __lifted_ls_65(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_merge_66(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static long long __lifted_find_min_67(long long, mere_vec_int*, mere_vec_int*, long long);
static closure_int_int __lifted_go__rvfast_uq40_68(long long, mere_vec_int*, long long, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq42_69(long long, mere_vec_int*, long long, mere_vec_int*, long long);
static closure_int_unit __lifted_assign_70(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_z_71(long long, mere_vec_int*, long long);
static int __lifted_cnt__rvfast_72(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_cnt_73(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_z2_74(long long, mere_vec_int*, long long);
static closure_int_unit __lifted_fill__rvfast_75(long long, mere_vec_int*, mere_vec_int*, long long);
static closure_int_unit __lifted_fill_uq44_76(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_z3_77(long long, mere_vec_int*, long long);
static int __lifted_asg__rvfast_78(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_asg_79(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static closure_int_int __lifted_go__rvfast_uq45_80(long long, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq46_81(long long, mere_vec_int*, long long);
static int __lifted_go_uq47_82(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_zl_uq48_83(mere_vec_int*, long long);
static int __lifted_zd_84(mere_vec_int*, long long);
static int __lifted_count__rvfast_85(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_count_86(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static closure_int_closure_int_int __lifted_last_nz_87(mere_vec_int*);
static int __lifted_cc1_88(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_cc2_89(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_rle_90(long long, mere_vec_int*, mere_vec_int*, long long);
static long long __lifted_runlen_91(long long, mere_vec_int*, long long, long long, long long);
static int __lifted_reps_92(mere_vec_int*, long long, long long);
static int __lifted_ones_93(mere_vec_int*, long long, long long);
static int __lifted_zc_uq49_94(mere_vec_int*, long long);
static int __lifted_ccount__rvfast_95(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_ccount_96(long long, mere_vec_int*, mere_vec_int*, long long);
static long long __lifted_last_cl_97(mere_vec_int*, mere_vec_int*, long long);
static int __lifted_emit_cl_98(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_emit_rle_99(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_emit_data_100(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_blocks_101(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_cp_102(long long, mere_vec_int*, mere_vec_int*, long long, long long);
static int __lifted_zl_uq50_103(mere_vec_int*, long long);
static int __lifted_zd_uq51_104(mere_vec_int*, long long);
static int __lifted_count__rvfast_uq52_105(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_count_uq53_106(long long, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_cp_uq54_107(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_push_u32_108(mere_vec_int*, long long);
static int __lifted_go_uq55_109(long long, mere_vec_int*, mere_vec_int*, long long);
static closure_int_closure_int_int __lifted_go__rvfast_uq56_110(long long, mere_vec_int*, long long);
static closure_int_closure_int_int __lifted_go_uq57_111(long long, mere_vec_int*, long long);
static mere_vec_int* __lifted_go_uq59_112(long long, mere_vec_int*, mere_vec_int*, long long, mere_vec_int*, long long, long long);
static closure_int_int __lifted_go__rvfast_uq61_113(long long, mere_vec_int*, long long);
static closure_int_int __lifted_go_uq62_114(long long, mere_vec_int*, long long);
static closure_int_closure_int_closure_Vec_int_Vec_int __lifted_go_uq64_115(mere_vec_int*, mere_vec_int*, long long, long long);
static int __lifted_go_uq66_116(long long, mere_vec_int*, long long);
static mere_vec_int* __lifted_go_uq67_117(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_seen__rvfast_118(mere_vec_int*, long long, long long, long long, long long);
static int __lifted_seen_119(mere_vec_int*, long long, long long, long long, long long);
static long long __lifted_go__rvfast_uq68_120(mere_vec_int*, long long, long long, long long, long long);
static long long __lifted_go_uq69_121(mere_vec_int*, long long, long long, long long, long long);
static closure_Vec_int_unit __lifted_rows_122(long long, long long, int, mere_vec_int*, long long, int, mere_vec_int*, mere_vec_int*, long long, long long);
static int __lifted_col_uq71_123(long long, int, mere_vec_int*, mere_vec_int*, long long, long long, int, mere_vec_int*, long long);
static mere_vec_int* __lifted_go_uq70_124(mere_vec_int*, long long);
static int __lifted_show__rvfast_125(mere_vec_str*, long long);
static int __lifted_show_126(mere_vec_str*, long long);
static closure_int_unit __lifted_header_127(mere_vec_int*, const char*);
static mere_vec_int* __lifted_go_uq72_128(long long, mere_vec_int*, mere_vec_int*, long long);
static closure_int_closure_int_closure_int_unit __lifted_fields_129(long long, mere_vec_int*, mere_vec_int*, long long);
static int __lifted_copy_uq74_130(long long, long long, long long, mere_vec_int*, mere_vec_int*, long long, long long);
static closure_str_str __lifted_join__rvfast_131(mere_vec_str*, long long);
static closure_str_str __lifted_join_132(mere_vec_str*, long long);
static int __lifted_copy_uq75_133(long long, long long, long long, mere_vec_int*, mere_vec_int*, long long, long long);
static int __lifted_go_uq12_134(mere_vec_int*, long long);
long long __lifted_go_0__direct(long long, long long);
long long __lifted_go__rvfast_1__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq1_2__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq2_3__direct(long long, mere_vec_int*, mere_vec_int*, long long, long long);
long long __lifted_go__rvfast_uq3_9__direct(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long, long long, long long, long long);
long long __lifted_go_uq4_10__direct(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long, long long, long long, long long);
int __lifted_chars__rvfast_36__direct(long long, mere_vec_int*, long long, mere_strbuf*, long long, int);
int __lifted_chars_37__direct(long long, mere_vec_int*, long long, mere_strbuf*, long long, int);
mere_bytebuf* __lifted_pass_43__direct(mere_bytebuf*, long long, long long, mere_vec_int*, long long, long long, long long);
long long __lifted_go_uq31_49__direct(long long, long long);
long long __lifted_go__rvfast_uq32_50__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq33_51__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq35_53__direct(long long, long long, long long, long long);
tuple_int_int __lifted_best_58__direct(long long, long long, long long, mere_vec_int*, long long, long long, long long);
long long __lifted_go__rvfast_uq40_68__direct(long long, mere_vec_int*, long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq42_69__direct(long long, mere_vec_int*, long long, mere_vec_int*, long long, long long);
int __lifted_assign_70__direct(mere_vec_int*, mere_vec_int*, mere_vec_int*, mere_vec_int*, long long, long long);
int __lifted_fill__rvfast_75__direct(long long, mere_vec_int*, mere_vec_int*, long long, long long);
int __lifted_fill_uq44_76__direct(long long, mere_vec_int*, mere_vec_int*, long long, long long);
long long __lifted_go__rvfast_uq45_80__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq46_81__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_last_nz_87__direct(mere_vec_int*, long long, long long);
long long __lifted_go__rvfast_uq56_110__direct(long long, mere_vec_int*, long long, long long, long long);
long long __lifted_go_uq57_111__direct(long long, mere_vec_int*, long long, long long, long long);
long long __lifted_go__rvfast_uq61_113__direct(long long, mere_vec_int*, long long, long long);
long long __lifted_go_uq62_114__direct(long long, mere_vec_int*, long long, long long);
mere_vec_int* __lifted_go_uq64_115__direct(mere_vec_int*, mere_vec_int*, long long, long long, long long, long long, mere_vec_int*);
int __lifted_rows_122__direct(long long, long long, int, mere_vec_int*, long long, int, mere_vec_int*, mere_vec_int*, long long, long long, mere_vec_int*);
int __lifted_header_127__direct(mere_vec_int*, const char*, long long);
int __lifted_fields_129__direct(long long, mere_vec_int*, mere_vec_int*, long long, long long, long long, long long);
const char* __lifted_join__rvfast_131__direct(mere_vec_str*, long long, const char*);
const char* __lifted_join_132__direct(mere_vec_str*, long long, const char*);
static closure_int_int __anon_0_fn(void*, long long);
static long long __anon_0_fn2(void*, long long, long long);
static closure_int_int __anon_1_fn(void*, long long);
static long long __anon_1_fn2(void*, long long, long long);
static closure_closure_int_closure_int_int_int __anon_2_fn(void*, long long);
static long long __anon_2_fn2(void*, long long, closure_int_closure_int_int);
static closure_list_int_list_int __anon_3_fn(void*, long long);
static list_int __anon_3_fn2(void*, long long, list_int);
static list_int __anon_4_fn(void*, long long);
static closure_int_int __anon_5_fn(void*, long long);
static long long __anon_5_fn2(void*, long long, long long);
static closure_int_int __anon_6_fn(void*, long long);
static long long __anon_6_fn2(void*, long long, long long);
static const char* __anon_7_fn(void*, long long);
static const char* __anon_8_fn(void*, long long);
static long long __anon_9_fn(void*, long long);
static long long __anon_10_fn(void*, long long);
static long long __anon_11_fn(void*, long long);
static closure_int_closure_str_str __anon_12_fn(void*, long long);
static closure_str_str __anon_12_fn2(void*, long long, long long);
static closure_int_str __anon_13_fn(void*, long long);
static const char* __anon_13_fn2(void*, long long, long long);
static const char* __anon_14_fn(void*, const char*);
static closure_int_closure_int_int __anon_15_fn(void*, long long);
static closure_int_int __anon_15_fn2(void*, long long, long long);
static const char* __anon_16_fn(void*, long long);
static const char* __anon_17_fn(void*, long long);
static long long __anon_18_fn(void*, long long);
static closure_int_int __anon_19_fn(void*, long long);
static long long __anon_19_fn2(void*, long long, long long);
static long long __anon_20_fn(void*, long long);
static long long __anon_21_fn(void*, long long);
static tuple_int_int __anon_22_fn(void*, long long);
static int __anon_23_fn(void*, const char*);
static long long __anon_24_fn(void*, long long);
static long long __anon_25_fn(void*, mere_vec_int*);
static closure_int_int __anon_26_fn(void*, mere_vec_int*);
static long long __anon_26_fn2(void*, mere_vec_int*, long long);
static closure_Vec_int_closure_Vec_int_int __anon_27_fn(void*, mere_vec_int*);
static closure_Vec_int_int __anon_27_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_28_fn(void*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_28_fn2(void*, mere_vec_int*, mere_vec_int*);
static tuple_Vec_int_Vec_int __anon_29_fn(void*, mere_vec_int*);
static mere_vec_int* __anon_30_fn(void*, long long);
static long long __anon_31_fn(void*, long long);
static int __anon_32_fn(void*, mere_vec_int*);
static mere_vec_int* __anon_33_fn(void*, long long);
static long long __anon_34_fn(void*, long long);
static closure_int_closure_int_closure_int_closure_int_bool __anon_35_fn(void*, long long);
static closure_int_closure_int_closure_int_bool __anon_35_fn2(void*, long long, long long);
static int __anon_36_fn(void*, long long);
static int __anon_37_fn(void*, long long);
static int __anon_38_fn(void*, long long);
static int __anon_39_fn(void*, long long);
static int __anon_40_fn(void*, long long);
static int __anon_41_fn(void*, long long);
static int __anon_42_fn(void*, long long);
static long long __anon_43_fn(void*, long long);
static closure_int_int __anon_44_fn(void*, long long);
static long long __anon_44_fn2(void*, long long, long long);
static closure_int_closure_int_closure_int_ByteBuf __anon_45_fn(void*, long long);
static closure_int_closure_int_ByteBuf __anon_45_fn2(void*, long long, long long);
static closure_int_closure_int_ByteBuf __anon_46_fn(void*, long long);
static closure_int_ByteBuf __anon_46_fn2(void*, long long, long long);
static long long __anon_47_fn(void*, long long);
static long long __anon_48_fn(void*, long long);
static long long __anon_49_fn(void*, long long);
static closure_int_closure_int_ByteBuf __anon_50_fn(void*, long long);
static closure_int_ByteBuf __anon_50_fn2(void*, long long, long long);
static long long __anon_51_fn(void*, long long);
static closure_int_closure_int_unit __anon_52_fn(void*, mere_vec_int*);
static closure_int_unit __anon_52_fn2(void*, mere_vec_int*, long long);
static int __anon_53_fn(void*, mere_vec_int*);
static long long __anon_54_fn(void*, long long);
static closure_int_tuple_int_int __anon_55_fn(void*, long long);
static tuple_int_int __anon_55_fn2(void*, long long, long long);
static closure_int_Vec_int __anon_56_fn(void*, long long);
static mere_vec_int* __anon_56_fn2(void*, long long, long long);
static closure_int_Vec_int __anon_57_fn(void*, long long);
static mere_vec_int* __anon_57_fn2(void*, long long, long long);
static long long __anon_58_fn(void*, long long);
static int __anon_59_fn(void*, long long);
static int __anon_60_fn(void*, mere_vec_int*);
static closure_int_closure_int_closure_int_closure_Vec_int_unit __anon_61_fn(void*, long long);
static closure_int_closure_int_closure_Vec_int_unit __anon_61_fn2(void*, long long, long long);
static closure_int_int __anon_62_fn(void*, long long);
static long long __anon_62_fn2(void*, long long, long long);
static closure_Vec_int_closure_int_Vec_int __anon_63_fn(void*, mere_vec_int*);
static closure_int_Vec_int __anon_63_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_int_Vec_int __anon_64_fn(void*, mere_vec_int*);
static mere_vec_int* __anon_64_fn2(void*, mere_vec_int*, long long);
static int __anon_65_fn(void*, long long);
static mere_vec_int* __anon_66_fn(void*, long long);
static closure_int_closure_int_int __anon_67_fn(void*, long long);
static closure_int_int __anon_67_fn2(void*, long long, long long);
static closure_int_closure_int_Vec_int __anon_68_fn(void*, long long);
static closure_int_Vec_int __anon_68_fn2(void*, long long, long long);
static closure_int_Vec_int __anon_69_fn(void*, long long);
static mere_vec_int* __anon_69_fn2(void*, long long, long long);
static closure_int_Vec_int __anon_70_fn(void*, long long);
static mere_vec_int* __anon_70_fn2(void*, long long, long long);
static int __anon_71_fn(void*, const char*);
static long long __anon_72_fn(void*, long long);
static long long __anon_73_fn(void*, long long);
static long long __anon_74_fn(void*, long long);
static long long __anon_75_fn(void*, long long);
static closure_int_closure_int_int __anon_76_fn(void*, long long);
static closure_int_int __anon_76_fn2(void*, long long, long long);
static closure_int_closure_int_int __anon_77_fn(void*, long long);
static closure_int_int __anon_77_fn2(void*, long long, long long);
static int __anon_78_fn(void*, int);
static int __anon_79_fn(void*, int);
static mere_bytebuf* __anon_80_fn(void*, long long);
static long long __anon_81_fn(void*, long long);
static long long __anon_82_fn(void*, long long);
static long long __anon_83_fn(void*, long long);
static long long __anon_84_fn(void*, long long);
static closure_int_tuple_int_int __anon_85_fn(void*, long long);
static tuple_int_int __anon_85_fn2(void*, long long, long long);
static long long __anon_86_fn(void*, long long);
static long long __anon_87_fn(void*, long long);
static int __anon_88_fn(void*, long long);
static int __anon_89_fn(void*, long long);
static int __anon_90_fn(void*, long long);
static long long __anon_91_fn(void*, long long);
static long long __anon_92_fn(void*, long long);
static closure_int_int __anon_93_fn(void*, long long);
static long long __anon_93_fn2(void*, long long, long long);
static closure_int_int __anon_94_fn(void*, long long);
static long long __anon_94_fn2(void*, long long, long long);
static closure_int_int __anon_95_fn(void*, long long);
static long long __anon_95_fn2(void*, long long, long long);
static long long __anon_96_fn(void*, long long);
static long long __anon_97_fn(void*, long long);
static closure_int_closure_Vec_int_Vec_int __anon_98_fn(void*, long long);
static closure_Vec_int_Vec_int __anon_98_fn2(void*, long long, long long);
static int __anon_99_fn(void*, mere_vec_int*);
static int __anon_100_fn(void*, long long);
static closure_int_closure_int_unit __anon_101_fn(void*, long long);
static closure_int_unit __anon_101_fn2(void*, long long, long long);
static const char* __anon_102_fn(void*, const char*);
static const char* __anon_103_fn(void*, const char*);
static long long __anon_104_fn(void*, long long);
static long long __anon_105_fn(void*, long long);
static long long __anon_106_fn(void*, closure_int_closure_int_int);
static list_int __anon_107_fn(void*, list_int);
static long long __anon_108_fn(void*, long long);
static long long __anon_109_fn(void*, long long);
static closure_str_str __anon_110_fn(void*, long long);
static const char* __anon_110_fn2(void*, long long, const char*);
static const char* __anon_111_fn(void*, const char*);
static const char* __anon_112_fn(void*, long long);
static closure_int_int __anon_113_fn(void*, long long);
static long long __anon_113_fn2(void*, long long, long long);
static long long __anon_114_fn(void*, long long);
static long long __anon_115_fn(void*, long long);
static long long __anon_116_fn(void*, long long);
static closure_Vec_int_int __anon_117_fn(void*, mere_vec_int*);
static long long __anon_117_fn2(void*, mere_vec_int*, mere_vec_int*);
static long long __anon_118_fn(void*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_119_fn(void*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_119_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_120_fn(void*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_unit __anon_120_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_int_closure_int_closure_int_bool __anon_121_fn(void*, long long);
static closure_int_closure_int_bool __anon_121_fn2(void*, long long, long long);
static closure_int_closure_int_bool __anon_122_fn(void*, long long);
static closure_int_bool __anon_122_fn2(void*, long long, long long);
static long long __anon_123_fn(void*, long long);
static closure_int_closure_int_ByteBuf __anon_124_fn(void*, long long);
static closure_int_ByteBuf __anon_124_fn2(void*, long long, long long);
static closure_int_ByteBuf __anon_125_fn(void*, long long);
static mere_bytebuf* __anon_125_fn2(void*, long long, long long);
static closure_int_ByteBuf __anon_126_fn(void*, long long);
static mere_bytebuf* __anon_126_fn2(void*, long long, long long);
static mere_bytebuf* __anon_127_fn(void*, long long);
static closure_int_ByteBuf __anon_128_fn(void*, long long);
static mere_bytebuf* __anon_128_fn2(void*, long long, long long);
static mere_bytebuf* __anon_129_fn(void*, long long);
static closure_int_unit __anon_130_fn(void*, long long);
static int __anon_130_fn2(void*, long long, long long);
static int __anon_131_fn(void*, long long);
static tuple_int_int __anon_132_fn(void*, long long);
static mere_vec_int* __anon_133_fn(void*, long long);
static mere_vec_int* __anon_134_fn(void*, long long);
static closure_int_closure_int_closure_Vec_int_unit __anon_135_fn(void*, long long);
static closure_int_closure_Vec_int_unit __anon_135_fn2(void*, long long, long long);
static closure_int_closure_Vec_int_unit __anon_136_fn(void*, long long);
static closure_Vec_int_unit __anon_136_fn2(void*, long long, long long);
static long long __anon_137_fn(void*, long long);
static closure_int_Vec_int __anon_138_fn(void*, mere_vec_int*);
static mere_vec_int* __anon_138_fn2(void*, mere_vec_int*, long long);
static mere_vec_int* __anon_139_fn(void*, long long);
static mere_vec_int* __anon_140_fn(void*, long long);
static closure_int_int __anon_141_fn(void*, long long);
static long long __anon_141_fn2(void*, long long, long long);
static long long __anon_142_fn(void*, long long);
static closure_int_Vec_int __anon_143_fn(void*, long long);
static mere_vec_int* __anon_143_fn2(void*, long long, long long);
static mere_vec_int* __anon_144_fn(void*, long long);
static mere_vec_int* __anon_145_fn(void*, long long);
static mere_vec_int* __anon_146_fn(void*, long long);
static closure_int_int __anon_147_fn(void*, long long);
static long long __anon_147_fn2(void*, long long, long long);
static long long __anon_148_fn(void*, long long);
static closure_int_int __anon_149_fn(void*, long long);
static long long __anon_149_fn2(void*, long long, long long);
static long long __anon_150_fn(void*, long long);
static tuple_int_int __anon_151_fn(void*, long long);
static long long __anon_152_fn(void*, long long);
static long long __anon_153_fn(void*, long long);
static long long __anon_154_fn(void*, long long);
static closure_Vec_int_Vec_int __anon_155_fn(void*, long long);
static mere_vec_int* __anon_155_fn2(void*, long long, mere_vec_int*);
static mere_vec_int* __anon_156_fn(void*, mere_vec_int*);
static closure_int_unit __anon_157_fn(void*, long long);
static int __anon_157_fn2(void*, long long, long long);
static int __anon_158_fn(void*, long long);
static const char* __anon_159_fn(void*, const char*);
static long long __anon_160_fn(void*, long long);
static long long __anon_161_fn(void*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_162_fn(void*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_unit __anon_162_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_unit __anon_163_fn(void*, mere_vec_int*);
static closure_Vec_int_unit __anon_163_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_closure_Vec_int_unit __anon_164_fn(void*, mere_vec_int*);
static closure_Vec_int_unit __anon_164_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_unit __anon_165_fn(void*, mere_vec_int*);
static int __anon_165_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_int_closure_int_bool __anon_166_fn(void*, long long);
static closure_int_bool __anon_166_fn2(void*, long long, long long);
static closure_int_bool __anon_167_fn(void*, long long);
static int __anon_167_fn2(void*, long long, long long);
static closure_int_bool __anon_168_fn(void*, long long);
static int __anon_168_fn2(void*, long long, long long);
static int __anon_169_fn(void*, long long);
static closure_int_ByteBuf __anon_170_fn(void*, long long);
static mere_bytebuf* __anon_170_fn2(void*, long long, long long);
static mere_bytebuf* __anon_171_fn(void*, long long);
static mere_bytebuf* __anon_172_fn(void*, long long);
static mere_bytebuf* __anon_173_fn(void*, long long);
static mere_bytebuf* __anon_174_fn(void*, long long);
static int __anon_175_fn(void*, long long);
static closure_int_closure_Vec_int_unit __anon_176_fn(void*, long long);
static closure_Vec_int_unit __anon_176_fn2(void*, long long, long long);
static closure_Vec_int_unit __anon_177_fn(void*, long long);
static int __anon_177_fn2(void*, long long, mere_vec_int*);
static closure_Vec_int_unit __anon_178_fn(void*, long long);
static int __anon_178_fn2(void*, long long, mere_vec_int*);
static int __anon_179_fn(void*, mere_vec_int*);
static mere_vec_int* __anon_180_fn(void*, long long);
static long long __anon_181_fn(void*, long long);
static mere_vec_int* __anon_182_fn(void*, long long);
static long long __anon_183_fn(void*, long long);
static long long __anon_184_fn(void*, long long);
static mere_vec_int* __anon_185_fn(void*, mere_vec_int*);
static int __anon_186_fn(void*, long long);
static closure_Vec_int_closure_Vec_int_unit __anon_187_fn(void*, mere_vec_int*);
static closure_Vec_int_unit __anon_187_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_unit __anon_188_fn(void*, mere_vec_int*);
static int __anon_188_fn2(void*, mere_vec_int*, mere_vec_int*);
static closure_Vec_int_unit __anon_189_fn(void*, mere_vec_int*);
static int __anon_189_fn2(void*, mere_vec_int*, mere_vec_int*);
static int __anon_190_fn(void*, mere_vec_int*);
static closure_Vec_int_unit __anon_191_fn(void*, mere_vec_int*);
static int __anon_191_fn2(void*, mere_vec_int*, mere_vec_int*);
static int __anon_192_fn(void*, mere_vec_int*);
static int __anon_193_fn(void*, mere_vec_int*);
static closure_int_bool __anon_194_fn(void*, long long);
static int __anon_194_fn2(void*, long long, long long);
static int __anon_195_fn(void*, long long);
static int __anon_196_fn(void*, long long);
static int __anon_197_fn(void*, long long);
static mere_bytebuf* __anon_198_fn(void*, long long);
static closure_Vec_int_unit __anon_199_fn(void*, long long);
static int __anon_199_fn2(void*, long long, mere_vec_int*);
static int __anon_200_fn(void*, mere_vec_int*);
static int __anon_201_fn(void*, mere_vec_int*);
static int __anon_202_fn(void*, mere_vec_int*);
static closure_Vec_int_unit __anon_203_fn(void*, mere_vec_int*);
static int __anon_203_fn2(void*, mere_vec_int*, mere_vec_int*);
static int __anon_204_fn(void*, mere_vec_int*);
static int __anon_205_fn(void*, mere_vec_int*);
static int __anon_206_fn(void*, mere_vec_int*);
static int __anon_207_fn(void*, mere_vec_int*);
static int __anon_208_fn(void*, long long);
static int __anon_209_fn(void*, mere_vec_int*);
static int __anon_210_fn(void*, mere_vec_int*);
static const closure_list_int_closure_int_closure_closure_int_closure_int_int_int mu_list_fold_as_value;
static const closure_int_closure_int_closure_list_int_list_int mu__range_down_as_value;
static const closure_int_closure_int_list_int mu_range_as_value;
static const closure_list_int_int mu_list_sum_as_value;
static const closure_list_int_int mu_list_product_as_value;
static const closure_list_str_closure_int_str mu__u8_nth_as_value;
static const closure_str_closure_int_str mu_utf8_at_as_value;
static const closure_int_bool mu__cp_bad_as_value;
static const closure_int_str mu_str_of_codepoint_as_value;
static const closure_int_int mu__cp_span_as_value;
static const closure_str_closure_int_int mu__cp_cont_as_value;
static const closure_str_int mu_codepoint_of_as_value;
static const closure_str_closure_int_int mu_codepoint_or_as_value;
static const closure_str_closure_int_int mu_codepoint_at_as_value;
static const closure_list_str_closure_int_closure_int_closure_str_str mu__u8_slice_as_value;
static const closure_str_closure_int_closure_int_str mu_utf8_sub_as_value;
static const closure_list_str_closure_str_str mu__u8_rev_join_as_value;
static const closure_str_str mu_utf8_rev_as_value;
static const closure_int_int mu__eaw_width_as_value;
static const closure_str_closure_int_closure_int_closure_int_int mu__u8w_go_as_value;
static const closure_str_int mu_utf8_width_as_value;
static const closure_str_closure_int_str mu_pad_right_as_value;
static const closure_str_closure_int_str mu_pad_left_as_value;
static const closure_int_int mu_sign_as_value;
static const closure_int_int mu_incr_as_value;
static const closure_int_int mu_decr_as_value;
static const closure_int_int mu_square_as_value;
static const closure_int_int mu_cube_as_value;
static const closure_int_closure_int_int mu_sum_range_as_value;
static const closure_int_closure_int_closure_int_int mu__pow_go_as_value;
static const closure_int_closure_int_int mu_pow_as_value;
static const closure_int_closure_int_int mu_lcm_as_value;
static const closure_int_closure_int_tuple_int_int mu_divmod_as_value;
static const closure_bool_closure_str_unit mu_assert_as_value;
static const closure_int_closure_int_int mu_crc_byte_as_value;
static const closure_Vec_int_int mu_crc32_vec_as_value;
static const closure_int_Vec_int mu_br_new__int__Vec_int_as_value;
static const closure_Vec_int_closure_Vec_int_int mu_br_bit_as_value;
static const closure_Vec_int_closure_Vec_int_closure_int_int mu_br_bits_as_value;
static const closure_Vec_int_unit mu_br_align_as_value;
static const closure_Vec_int_tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int_as_value;
static const closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_int mu_huff_decode_as_value;
static const closure_list_int_Vec_int mu_vec_of_as_value;
static const closure_unit_Vec_int mu_fixed_lit_lengths_as_value;
static const closure_unit_Vec_int mu_fixed_dist_lengths_as_value;
static const closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit mu_inflate_codes_as_value;
static const closure_Vec_int_closure_Vec_int_tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int_as_value;
static const closure_Vec_int_closure_int_Vec_int mu_inflate_as_value;
static const closure_Vec_int_int mu_gzip_body_start_as_value;
static const closure_Vec_int_bool mu_is_gzip_as_value;
static const closure_Vec_int_Vec_int mu_gunzip_as_value;
static const closure_Vec_int_closure_int_int mu_gzip_trailer_u32_as_value;
static const closure_Vec_int_closure_Vec_int_bool mu_gunzip_ok_as_value;
static const closure_int_closure_int_Vec_int mu_new_vec_as_value;
static const closure_Vec_int_bool mu_is_png_as_value;
static const closure_Vec_int_closure_int_int mu_u32be_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_closure_int_closure_int_bool mu_type_is_as_value;
static const closure_Vec_int_closure_int_bool mu_is_ihdr_as_value;
static const closure_Vec_int_closure_int_bool mu_is_idat_as_value;
static const closure_Vec_int_closure_int_bool mu_is_iend_as_value;
static const closure_Vec_int_closure_int_bool mu_is_plte_as_value;
static const closure_Vec_int_closure_int_bool mu_is_text_as_value;
static const closure_Vec_int_closure_int_bool mu_is_gama_as_value;
static const closure_Vec_int_closure_int_bool mu_is_trns_as_value;
static const closure_Vec_int_Vec_int mu_read_header__Vec_int__Vec_int_as_value;
static const closure_int_int mu_channels_as_value;
static const closure_int_closure_int_int mu_pixel_bytes_as_value;
static const closure_Vec_int_Vec_int mu_palette_as_value;
static const closure_Vec_int_Vec_int mu_idat_bytes_as_value;
static const closure_Vec_int_Vec_str mu_text_chunks_as_value;
static const closure_Vec_int_int mu_gamma_as_value;
static const closure_Vec_int_bool mu_has_transparency_as_value;
static const closure_int_closure_int_closure_int_int mu_paeth_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter__Vec_int__int__int__int__ByteBuf_as_value;
static const closure_Vec_int_bool mu_is_interlaced_as_value;
static const closure_int_int mu_adam7_xstart_as_value;
static const closure_int_int mu_adam7_ystart_as_value;
static const closure_int_int mu_adam7_xstep_as_value;
static const closure_int_int mu_adam7_ystep_as_value;
static const closure_int_closure_int_int mu_ceil_div_as_value;
static const closure_int_closure_int_int mu_pass_width_as_value;
static const closure_int_closure_int_int mu_pass_height_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_ByteBuf mu_deinterlace__Vec_int__int__int__int__ByteBuf_as_value;
static const closure_Vec_int_ByteBuf mu_decode__Vec_int__ByteBuf_as_value;
static const closure_Vec_int_Vec_int mu_rgb8_as_value;
static const closure_list_int_Vec_int mu_vec_of__v2_as_value;
static const closure_int_closure_int_int mu_crc_byte__v2_as_value;
static const closure_Vec_int_int mu_crc32_vec__v2_as_value;
static const closure_unit_Vec_int mu_bw_new__unit__Vec_int_as_value;
static const closure_Vec_int_closure_Vec_int_closure_int_closure_int_unit mu_bw_bits_as_value;
static const closure_Vec_int_closure_Vec_int_unit mu_bw_flush_as_value;
static const closure_int_closure_int_int mu_rev_bits_as_value;
static const closure_int_tuple_int_int_int mu_len_encode_as_value;
static const closure_int_tuple_int_int_int mu_dist_encode_as_value;
static const closure_Vec_int_closure_int_closure_int_tuple_int_int mu_find_match_as_value;
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_build_lengths__Vec_int__int__int__Vec_int_as_value;
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_canon_codes__Vec_int__int__int__Vec_int_as_value;
static const closure_Vec_int_closure_int_int mu_vmax_as_value;
static const closure_Vec_int_Vec_int mu_tokenize__Vec_int__Vec_int_as_value;
static const closure_Vec_int_Vec_int mu_deflate_dynamic__Vec_int__Vec_int_as_value;
static const closure_Vec_int_Vec_int mu_deflate_stored__Vec_int__Vec_int_as_value;
static const closure_Vec_int_Vec_int mu_deflate_body__Vec_int__Vec_int_as_value;
static const closure_Vec_int_Vec_int mu_deflate_gzip_as_value;
static const closure_Vec_int_closure_int_unit mu_bytes_push4be_as_value;
static const closure_Vec_int_closure_Vec_int_unit mu_push_all_as_value;
static const closure_Vec_int_int mu_adler32_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit mu_chunk_as_value;
static const closure_int_closure_int_closure_int_int mu_paeth_e_as_value;
static const closure_int_closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_filter_row_as_value;
static const closure_Vec_int_int mu_row_cost_as_value;
static const closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_best_filter__Vec_int__Vec_int__int__Vec_int_as_value;
static const closure_Vec_int_closure_int_bool mu_all_grey_as_value;
static const closure_Vec_int_closure_int_Vec_int mu_collect_palette_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_int mu_palette_index_as_value;
static const closure_Vec_int_closure_int_closure_int_closure_int_Vec_int mu_encode_at_depth__Vec_int__int__int__int__Vec_int_as_value;
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_encode_rgb8__Vec_int__int__int__Vec_int_as_value;
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_encode16__Vec_int__int__int__Vec_int_as_value;
static const closure_int_str mu_colour_name_as_value;
static const closure_Vec_int_unit mu_info_as_value;
static const closure_Vec_int_Vec_int mu_ppm_bytes_as_value;
static const closure_Vec_int_Vec_int mu_read_ppm__Vec_int__Vec_int_as_value;
static const closure_str_Vec_int mu_load_as_value;
static const closure_str_closure_str_unit mu_encode_cmd_as_value;
static const closure_unit_unit mu___mere_user_main_as_value;
static const closure_bytes_str mu_png_info_as_value;
static const closure_bytes_bytes mu_png_to_ppm_as_value;
static const closure_bytes_bytes mu_ppm_to_png_as_value;
static const char* show_int(long long);

/* Phase 30.2: top-level non-fn let values as file-scope globals */
static long long mu_mask32;
static mere_vec_int* mu_len_base;
static mere_vec_int* mu_len_extra;
static mere_vec_int* mu_dist_base;
static mere_vec_int* mu_dist_extra;
static mere_vec_int* mu_clen_order;
static mere_vec_int* mu_png_sig;
static long long mu_mask32__v2;
static mere_vec_int* mu_g_len_base;
static mere_vec_int* mu_g_len_extra;
static mere_vec_int* mu_g_dist_base;
static mere_vec_int* mu_g_dist_extra;
static mere_vec_int* mu_cl_order;
static const char* mu_usage;

static closure_int_int __lifted_go_0(long long mu_i) {
  return ({ __anon_72_env* __env = (__anon_72_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_72_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_72_env; __env->mu_i = mu_i; (closure_int_int){.env = __env, .fn = __anon_72_fn}; });
}

static closure_int_int __lifted_go__rvfast_1(long long mu_n, mere_vec_int* mu_bytes, long long mu_i) {
  return ({ __anon_73_env* __env = (__anon_73_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_73_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_73_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_bytes = mu_bytes; (closure_int_int){.env = __env, .fn = __anon_73_fn}; });
}

static closure_int_int __lifted_go_uq1_2(long long mu_n, mere_vec_int* mu_bytes, long long mu_i) {
  return ({ __anon_74_env* __env = (__anon_74_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_74_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_74_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_bytes = mu_bytes; (closure_int_int){.env = __env, .fn = __anon_74_fn}; });
}

static closure_int_int __lifted_go_uq2_3(long long mu_n, mere_vec_int* mu_s, mere_vec_int* mu_data, long long mu_i) {
  return ({ __anon_75_env* __env = (__anon_75_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_75_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_75_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_s = mu_s; __env->mu_data = mu_data; (closure_int_int){.env = __env, .fn = __anon_75_fn}; });
}

static int __lifted_zc_4(mere_vec_int* mu_count, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_count;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 16LL) ? 0 : ({ (void)(mere_vec_int_push(mu_count, 0LL)); ({ __auto_type __mt0 = mu_count; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_tally__rvfast_5(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_count, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_count;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_l = mere_vec_int_get_unchecked(mu_lengths, mu_i); __auto_type mu_l = __let_tmp_l; (void)(mere_vec_int_set(mu_count, mu_l, (mere_vec_int_get(mu_count, mu_l) + 1LL))); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_count; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_tally_6(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_count, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_count;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_l = mere_vec_int_get(mu_lengths, mu_i); __auto_type mu_l = __let_tmp_l; (void)(mere_vec_int_set(mu_count, mu_l, (mere_vec_int_get(mu_count, mu_l) + 1LL))); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_count; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_bylen_7(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_syms, long long mu_l) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_syms;
  __auto_type __mere_a3 = &mu_l;
  __mere_tail: ;
  return ((mu_l == 16LL) ? 0 : ({ (void)(__lifted_scan_8(mu_n, mu_lengths, mu_l, mu_syms, 0LL)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_syms; __auto_type __mt3 = (mu_l + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_scan_8(long long mu_n, mere_vec_int* mu_lengths, long long mu_l, mere_vec_int* mu_syms, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_l;
  __auto_type __mere_a3 = &mu_syms;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ (void)(((mere_vec_int_get(mu_lengths, mu_i) == mu_l) ? mere_vec_int_push(mu_syms, mu_i) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_l; __auto_type __mt3 = mu_syms; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_closure_int_closure_int_int __lifted_go__rvfast_uq3_9(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_count, mere_vec_int* mu_syms, long long mu_len) {
  return ({ __anon_76_env* __env = (__anon_76_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_76_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_76_env; __env->mu_len = mu_len; __env->mu_s = mu_s; __env->mu_data = mu_data; __env->mu_count = mu_count; __env->mu_syms = mu_syms; (closure_int_closure_int_closure_int_int){.env = __env, .fn = __anon_76_fn, .fn2 = __anon_76_fn2}; });
}

static closure_int_closure_int_closure_int_int __lifted_go_uq4_10(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_count, mere_vec_int* mu_syms, long long mu_len) {
  return ({ __anon_77_env* __env = (__anon_77_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_77_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_77_env; __env->mu_len = mu_len; __env->mu_s = mu_s; __env->mu_data = mu_data; __env->mu_count = mu_count; __env->mu_syms = mu_syms; (closure_int_closure_int_closure_int_int){.env = __env, .fn = __anon_77_fn, .fn2 = __anon_77_fn2}; });
}

static mere_vec_int* __lifted_go_uq5_11(mere_vec_int* mu_v, list_int mu_l) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_v;
  __auto_type __mere_a1 = &mu_l;
  __mere_tail: ;
  return ({ __auto_type __scrut = mu_l; (list_int__tag(__scrut) == 0) ? ({ mu_v; }) : ((list_int__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_int__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_int__node(__scrut)->payload.Cons).f1; ({ (void)(mere_vec_int_push(mu_v, mu_h)); ({ __auto_type __mt0 = mu_v; __auto_type __mt1 = mu_t; *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }); }) : (({ __lang_fail_impl("no matching arm in match"); (mere_vec_int*)0; }))); });
}

static mere_vec_int* __lifted_go_uq6_12(mere_vec_int* mu_v, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_v;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 288LL) ? mu_v : ({ __auto_type __let_tmp_l = ((mu_i < 144LL) ? 8LL : ((mu_i < 256LL) ? 9LL : ((mu_i < 280LL) ? 7LL : 8LL))); __auto_type mu_l = __let_tmp_l; (void)(mere_vec_int_push(mu_v, mu_l)); ({ __auto_type __mt0 = mu_v; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_int* __lifted_go_uq7_13(mere_vec_int* mu_v, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_v;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 30LL) ? mu_v : ({ (void)(mere_vec_int_push(mu_v, 5LL)); ({ __auto_type __mt0 = mu_v; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_loop_14(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_lcount, mere_vec_int* mu_lsyms, mere_vec_int* mu_out, mere_vec_int* mu_len_base, mere_vec_int* mu_len_extra, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms, mere_vec_int* mu_dist_base, mere_vec_int* mu_dist_extra, int mu__u) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_s;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_lcount;
  __auto_type __mere_a3 = &mu_lsyms;
  __auto_type __mere_a4 = &mu_out;
  __auto_type __mere_a5 = &mu_len_base;
  __auto_type __mere_a6 = &mu_len_extra;
  __auto_type __mere_a7 = &mu_dcount;
  __auto_type __mere_a8 = &mu_dsyms;
  __auto_type __mere_a9 = &mu_dist_base;
  __auto_type __mere_a10 = &mu_dist_extra;
  __auto_type __mere_a11 = &mu__u;
  __mere_tail: ;
  return ({ __auto_type __let_tmp_sym = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mu_lcount; __auto_type __da3 = mu_lsyms; mu_huff_decode__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_sym = __let_tmp_sym; ((mu_sym < 256LL) ? ({ (void)(mere_vec_int_push(mu_out, mu_sym)); ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_lcount; __auto_type __mt3 = mu_lsyms; __auto_type __mt4 = mu_out; __auto_type __mt5 = mu_len_base; __auto_type __mt6 = mu_len_extra; __auto_type __mt7 = mu_dcount; __auto_type __mt8 = mu_dsyms; __auto_type __mt9 = mu_dist_base; __auto_type __mt10 = mu_dist_extra; __auto_type __mt11 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; *__mere_a9 = __mt9; *__mere_a10 = __mt10; *__mere_a11 = __mt11; goto __mere_tail; __mere_ret; }); }) : ((mu_sym == 256LL) ? 0 : ({ __auto_type __let_tmp_li = (mu_sym - 257LL); __auto_type mu_li = __let_tmp_li; __auto_type __let_tmp_len = ({ __auto_type __bc215 = mere_vec_int_get(mu_len_base, mu_li); __auto_type __bc216 = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mere_vec_int_get(mu_len_extra, mu_li); mu_br_bits__direct(__da0, __da1, __da2); }); __auto_type __bc217 = (__bc215 + __bc216); __bc217; }); __auto_type mu_len = __let_tmp_len; __auto_type __let_tmp_dsym = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mu_dcount; __auto_type __da3 = mu_dsyms; mu_huff_decode__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_dsym = __let_tmp_dsym; __auto_type __let_tmp_dist = ({ __auto_type __bc218 = mere_vec_int_get(mu_dist_base, mu_dsym); __auto_type __bc219 = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mere_vec_int_get(mu_dist_extra, mu_dsym); mu_br_bits__direct(__da0, __da1, __da2); }); __auto_type __bc220 = (__bc218 + __bc219); __bc220; }); __auto_type mu_dist = __let_tmp_dist; __auto_type __let_tmp_from = (mere_vec_int_len(mu_out) - mu_dist); __auto_type mu_from = __let_tmp_from; ({ (void)(__lifted_copy_15(mu_len, mu_out, mu_from, 0LL)); ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_lcount; __auto_type __mt3 = mu_lsyms; __auto_type __mt4 = mu_out; __auto_type __mt5 = mu_len_base; __auto_type __mt6 = mu_len_extra; __auto_type __mt7 = mu_dcount; __auto_type __mt8 = mu_dsyms; __auto_type __mt9 = mu_dist_base; __auto_type __mt10 = mu_dist_extra; __auto_type __mt11 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; *__mere_a9 = __mt9; *__mere_a10 = __mt10; *__mere_a11 = __mt11; goto __mere_tail; __mere_ret; }); }); }))); });
}

static int __lifted_copy_15(long long mu_len, mere_vec_int* mu_out, long long mu_from, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_from;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_len) ? 0 : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_out, (mu_from + mu_i)))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_from; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_zero19_16(mere_vec_int* mu_cl_lengths, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cl_lengths;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 19LL) ? 0 : ({ (void)(mere_vec_int_push(mu_cl_lengths, 0LL)); ({ __auto_type __mt0 = mu_cl_lengths; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_rd__rvfast_17(long long mu_hclen, mere_vec_int* mu_cl_lengths, mere_vec_int* mu_clen_order, mere_vec_int* mu_s, mere_vec_int* mu_data, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_hclen;
  __auto_type __mere_a1 = &mu_cl_lengths;
  __auto_type __mere_a2 = &mu_clen_order;
  __auto_type __mere_a3 = &mu_s;
  __auto_type __mere_a4 = &mu_data;
  __auto_type __mere_a5 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_hclen) ? 0 : ({ (void)(mere_vec_int_set(mu_cl_lengths, mere_vec_int_get_unchecked(mu_clen_order, mu_i), ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 3LL; mu_br_bits__direct(__da0, __da1, __da2); }))); ({ __auto_type __mt0 = mu_hclen; __auto_type __mt1 = mu_cl_lengths; __auto_type __mt2 = mu_clen_order; __auto_type __mt3 = mu_s; __auto_type __mt4 = mu_data; __auto_type __mt5 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_rd_18(long long mu_hclen, mere_vec_int* mu_cl_lengths, mere_vec_int* mu_clen_order, mere_vec_int* mu_s, mere_vec_int* mu_data, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_hclen;
  __auto_type __mere_a1 = &mu_cl_lengths;
  __auto_type __mere_a2 = &mu_clen_order;
  __auto_type __mere_a3 = &mu_s;
  __auto_type __mere_a4 = &mu_data;
  __auto_type __mere_a5 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_hclen) ? 0 : ({ (void)(mere_vec_int_set(mu_cl_lengths, mere_vec_int_get(mu_clen_order, mu_i), ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 3LL; mu_br_bits__direct(__da0, __da1, __da2); }))); ({ __auto_type __mt0 = mu_hclen; __auto_type __mt1 = mu_cl_lengths; __auto_type __mt2 = mu_clen_order; __auto_type __mt3 = mu_s; __auto_type __mt4 = mu_data; __auto_type __mt5 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_fill_19(mere_vec_int* mu_lengths, long long mu_total, mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_clc, mere_vec_int* mu_cls, int mu__u) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_lengths;
  __auto_type __mere_a1 = &mu_total;
  __auto_type __mere_a2 = &mu_s;
  __auto_type __mere_a3 = &mu_data;
  __auto_type __mere_a4 = &mu_clc;
  __auto_type __mere_a5 = &mu_cls;
  __auto_type __mere_a6 = &mu__u;
  __mere_tail: ;
  return ((mere_vec_int_len(mu_lengths) >= mu_total) ? 0 : ({ __auto_type __let_tmp_sym = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mu_clc; __auto_type __da3 = mu_cls; mu_huff_decode__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_sym = __let_tmp_sym; ((mu_sym < 16LL) ? ({ (void)(mere_vec_int_push(mu_lengths, mu_sym)); ({ __auto_type __mt0 = mu_lengths; __auto_type __mt1 = mu_total; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_data; __auto_type __mt4 = mu_clc; __auto_type __mt5 = mu_cls; __auto_type __mt6 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }) : ((mu_sym == 16LL) ? ({ __auto_type __let_tmp_prev = mere_vec_int_get(mu_lengths, (mere_vec_int_len(mu_lengths) - 1LL)); __auto_type mu_prev = __let_tmp_prev; __auto_type __let_tmp_rep = (3LL + ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 2LL; mu_br_bits__direct(__da0, __da1, __da2); })); __auto_type mu_rep = __let_tmp_rep; ({ (void)(__lifted_p_uq9_20(mu_rep, mu_lengths, mu_prev, 0LL)); ({ __auto_type __mt0 = mu_lengths; __auto_type __mt1 = mu_total; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_data; __auto_type __mt4 = mu_clc; __auto_type __mt5 = mu_cls; __auto_type __mt6 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }); }) : ((mu_sym == 17LL) ? ({ __auto_type __let_tmp_rep = (3LL + ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 3LL; mu_br_bits__direct(__da0, __da1, __da2); })); __auto_type mu_rep = __let_tmp_rep; ({ (void)(__lifted_p_uq8_21(mu_rep, mu_lengths, 0LL)); ({ __auto_type __mt0 = mu_lengths; __auto_type __mt1 = mu_total; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_data; __auto_type __mt4 = mu_clc; __auto_type __mt5 = mu_cls; __auto_type __mt6 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }); }) : ({ __auto_type __let_tmp_rep = (11LL + ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 7LL; mu_br_bits__direct(__da0, __da1, __da2); })); __auto_type mu_rep = __let_tmp_rep; ({ (void)(__lifted_p_22(mu_rep, mu_lengths, 0LL)); ({ __auto_type __mt0 = mu_lengths; __auto_type __mt1 = mu_total; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_data; __auto_type __mt4 = mu_clc; __auto_type __mt5 = mu_cls; __auto_type __mt6 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }); })))); }));
}

static int __lifted_p_uq9_20(long long mu_rep, mere_vec_int* mu_lengths, long long mu_prev, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_rep;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_prev;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_rep) ? 0 : ({ (void)(mere_vec_int_push(mu_lengths, mu_prev)); ({ __auto_type __mt0 = mu_rep; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_prev; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_p_uq8_21(long long mu_rep, mere_vec_int* mu_lengths, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_rep;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_rep) ? 0 : ({ (void)(mere_vec_int_push(mu_lengths, 0LL)); ({ __auto_type __mt0 = mu_rep; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_p_22(long long mu_rep, mere_vec_int* mu_lengths, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_rep;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_rep) ? 0 : ({ (void)(mere_vec_int_push(mu_lengths, 0LL)); ({ __auto_type __mt0 = mu_rep; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_split_23(long long mu_total, long long mu_hlit, mere_vec_int* mu_llen, mere_vec_int* mu_lengths, mere_vec_int* mu_dlen, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_total;
  __auto_type __mere_a1 = &mu_hlit;
  __auto_type __mere_a2 = &mu_llen;
  __auto_type __mere_a3 = &mu_lengths;
  __auto_type __mere_a4 = &mu_dlen;
  __auto_type __mere_a5 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_total) ? 0 : ({ (void)(((mu_i < mu_hlit) ? mere_vec_int_push(mu_llen, mere_vec_int_get(mu_lengths, mu_i)) : mere_vec_int_push(mu_dlen, mere_vec_int_get(mu_lengths, mu_i)))); ({ __auto_type __mt0 = mu_total; __auto_type __mt1 = mu_hlit; __auto_type __mt2 = mu_llen; __auto_type __mt3 = mu_lengths; __auto_type __mt4 = mu_dlen; __auto_type __mt5 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_int* __lifted_block_24(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_out, int mu__u) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_s;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_out;
  __auto_type __mere_a3 = &mu__u;
  __mere_tail: ;
  return ({ __auto_type __let_tmp_final = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; mu_br_bit__direct(__da0, __da1); }); __auto_type mu_final = __let_tmp_final; __auto_type __let_tmp_btype = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 2LL; mu_br_bits__direct(__da0, __da1, __da2); }); __auto_type mu_btype = __let_tmp_btype; (void)(((mu_btype == 0LL) ? ({ (void)(({ __auto_type __da0 = mu_s; mu_br_align__direct(__da0); })); __auto_type __let_tmp_p = mere_vec_int_get(mu_s, 0LL); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_len = ({ __auto_type __bc221 = mere_vec_int_get(mu_data, mu_p); __auto_type __bc222 = ({ long long __sa = (mere_vec_int_get(mu_data, (mu_p + 1LL))); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc223 = (__bc221 + __bc222); __bc223; }); __auto_type mu_len = __let_tmp_len; ({ (void)(__lifted_copy_uq10_25(mu_len, mu_out, mu_data, mu_p, 0LL)); mere_vec_int_set(mu_s, 0LL, ((mu_p + 4LL) + mu_len)); }); }) : ((mu_btype == 1LL) ? ({ __auto_type __let_tup = ({ __auto_type __da0 = ({ __auto_type __da0 = 0; mu_fixed_lit_lengths__direct(__da0); }); mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0); }); __auto_type mu_lc = (__let_tup).f0; __auto_type mu_ls = (__let_tup).f1; ({ __auto_type __let_tup = ({ __auto_type __da0 = ({ __auto_type __da0 = 0; mu_fixed_dist_lengths__direct(__da0); }); mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0); }); __auto_type mu_dc = (__let_tup).f0; __auto_type mu_ds = (__let_tup).f1; ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mu_out; __auto_type __da3 = mu_lc; __auto_type __da4 = mu_ls; __auto_type __da5 = mu_dc; __auto_type __da6 = mu_ds; mu_inflate_codes__direct(__da0, __da1, __da2, __da3, __da4, __da5, __da6); }); }); }) : ({ __auto_type __let_tup = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0, __da1); }); __auto_type mu_llen = (__let_tup).f0; __auto_type mu_dlen = (__let_tup).f1; ({ __auto_type __let_tup = ({ __auto_type __da0 = mu_llen; mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0); }); __auto_type mu_lc = (__let_tup).f0; __auto_type mu_ls = (__let_tup).f1; ({ __auto_type __let_tup = ({ __auto_type __da0 = mu_dlen; mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0); }); __auto_type mu_dc = (__let_tup).f0; __auto_type mu_ds = (__let_tup).f1; ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = mu_out; __auto_type __da3 = mu_lc; __auto_type __da4 = mu_ls; __auto_type __da5 = mu_dc; __auto_type __da6 = mu_ds; mu_inflate_codes__direct(__da0, __da1, __da2, __da3, __da4, __da5, __da6); }); }); }); })))); ((mu_final == 1LL) ? mu_out : ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_out; __auto_type __mt3 = 0; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; })); });
}

static int __lifted_copy_uq10_25(long long mu_len, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_p, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_p;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_len) ? 0 : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_data, ((mu_p + 4LL) + mu_i)))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = mu_p; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static long long __lifted_skip_z_26(mere_vec_int* mu_data, long long mu_o) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_data;
  __auto_type __mere_a1 = &mu_o;
  __mere_tail: ;
  return ((mere_vec_int_get(mu_data, mu_o) == 0LL) ? (mu_o + 1LL) : ({ __auto_type __mt0 = mu_data; __auto_type __mt1 = (mu_o + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }));
}

static int __lifted_go_uq11_27(long long mu_n, mere_vec_int* mu_v, long long mu_x, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_x;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_n) ? 0 : ({ (void)(mere_vec_int_push(mu_v, mu_x)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_v; __auto_type __mt2 = mu_x; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_go__rvfast_uq13_28(mere_vec_int* mu_data, mere_vec_int* mu_png_sig, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_data;
  __auto_type __mere_a1 = &mu_png_sig;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 8LL) ? 1 : ((mere_vec_int_get_unchecked(mu_data, mu_i) != mere_vec_int_get_unchecked(mu_png_sig, mu_i)) ? 0 : ({ __auto_type __mt0 = mu_data; __auto_type __mt1 = mu_png_sig; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; })));
}

static int __lifted_go_uq14_29(mere_vec_int* mu_data, mere_vec_int* mu_png_sig, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_data;
  __auto_type __mere_a1 = &mu_png_sig;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 8LL) ? 1 : ((mere_vec_int_get(mu_data, mu_i) != mere_vec_int_get(mu_png_sig, mu_i)) ? 0 : ({ __auto_type __mt0 = mu_data; __auto_type __mt1 = mu_png_sig; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; })));
}

static mere_vec_int* __lifted_walk_30(mere_vec_int* mu_data, long long mu_pos) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_data;
  __auto_type __mere_a1 = &mu_pos;
  __mere_tail: ;
  return (((mu_pos + 8LL) > mere_vec_int_len(mu_data)) ? mere_vec_int_new(__lang_current_region) : ({ __auto_type __let_tmp_len = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_pos; mu_u32be__direct(__da0, __da1); }); __auto_type mu_len = __let_tmp_len; (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_ihdr__direct(__da0, __da1); }) ? ({ __auto_type __let_tmp_d = (mu_pos + 8LL); __auto_type mu_d = __let_tmp_d; __auto_type __let_tmp_h = mere_vec_int_new(__lang_current_region); __auto_type mu_h = __let_tmp_h; (void)(mere_vec_int_push(mu_h, ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_d; mu_u32be__direct(__da0, __da1); }))); (void)(mere_vec_int_push(mu_h, ({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_d + 4LL); mu_u32be__direct(__da0, __da1); }))); (void)(mere_vec_int_push(mu_h, mere_vec_int_get(mu_data, (mu_d + 8LL)))); (void)(mere_vec_int_push(mu_h, mere_vec_int_get(mu_data, (mu_d + 9LL)))); (void)(mere_vec_int_push(mu_h, mere_vec_int_get(mu_data, (mu_d + 12LL)))); mu_h; }) : ({ __auto_type __mt0 = mu_data; __auto_type __mt1 = ((mu_pos + 12LL) + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; })); }));
}

static mere_vec_int* __lifted_walk_uq15_31(long long mu_n, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_pos) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_pos;
  __mere_tail: ;
  return (((mu_pos + 8LL) > mu_n) ? mu_out : ({ __auto_type __let_tmp_len = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_pos; mu_u32be__direct(__da0, __da1); }); __auto_type mu_len = __let_tmp_len; (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_iend__direct(__da0, __da1); }) ? mu_out : (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_plte__direct(__da0, __da1); }) ? __lifted_copy_uq16_32(mu_len, mu_out, mu_data, mu_pos, 0LL) : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = ((mu_pos + 12LL) + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }))); }));
}

static mere_vec_int* __lifted_copy_uq16_32(long long mu_len, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_pos, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_pos;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_len) ? mu_out : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_data, ((mu_pos + 8LL) + mu_i)))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = mu_pos; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_int* __lifted_walk_uq17_33(long long mu_n, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_pos) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_pos;
  __mere_tail: ;
  return (((mu_pos + 8LL) > mu_n) ? mu_out : ({ __auto_type __let_tmp_len = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_pos; mu_u32be__direct(__da0, __da1); }); __auto_type mu_len = __let_tmp_len; (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_iend__direct(__da0, __da1); }) ? mu_out : ({ (void)((({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_idat__direct(__da0, __da1); }) ? __lifted_copy_uq18_34(mu_len, mu_out, mu_data, mu_pos, 0LL) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = ((mu_pos + 12LL) + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); })); }));
}

static int __lifted_copy_uq18_34(long long mu_len, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_pos, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_pos;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_len) ? 0 : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_data, ((mu_pos + 8LL) + mu_i)))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = mu_pos; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_str* __lifted_walk_uq19_35(long long mu_n, mere_vec_str* mu_out, mere_vec_int* mu_data, long long mu_pos) {
  mere_vec_str* __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_pos;
  __mere_tail: ;
  return (((mu_pos + 8LL) > mu_n) ? mu_out : ({ __auto_type __let_tmp_len = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_pos; mu_u32be__direct(__da0, __da1); }); __auto_type mu_len = __let_tmp_len; (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_iend__direct(__da0, __da1); }) ? mu_out : ({ (void)((({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_text__direct(__da0, __da1); }) ? ({ __auto_type __let_tmp_d = (mu_pos + 8LL); __auto_type mu_d = __let_tmp_d; __auto_type __let_tmp_buf = mere_strbuf_new(__lang_current_region); __auto_type mu_buf = __let_tmp_buf; ({ (void)(((((0LL >= 0LL) && (0LL <= mu_len)) && ((((mu_d + 0LL) >= 0LL) && (((mu_d + 0LL) + 1LL) <= mere_vec_int_len(mu_data))) && (((mu_d + (mu_len - 1LL)) >= 0LL) && (((mu_d + (mu_len - 1LL)) + 1LL) <= mere_vec_int_len(mu_data))))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0; __lifted_chars__rvfast_36__direct(mu_len, mu_data, mu_d, mu_buf, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0; __lifted_chars_37__direct(mu_len, mu_data, mu_d, mu_buf, __ia0, __ia1); }))); mere_vec_str_push(mu_out, mere_strbuf_to_str(mu_buf)); }); }) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = ((mu_pos + 12LL) + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); })); }));
}

static closure_bool_unit __lifted_chars__rvfast_36(long long mu_len, mere_vec_int* mu_data, long long mu_d, mere_strbuf* mu_buf, long long mu_i) {
  return ({ __anon_78_env* __env = (__anon_78_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_78_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_78_env; __env->mu_i = mu_i; __env->mu_len = mu_len; __env->mu_data = mu_data; __env->mu_d = mu_d; __env->mu_buf = mu_buf; (closure_bool_unit){.env = __env, .fn = __anon_78_fn}; });
}

static closure_bool_unit __lifted_chars_37(long long mu_len, mere_vec_int* mu_data, long long mu_d, mere_strbuf* mu_buf, long long mu_i) {
  return ({ __anon_79_env* __env = (__anon_79_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_79_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_79_env; __env->mu_i = mu_i; __env->mu_len = mu_len; __env->mu_data = mu_data; __env->mu_d = mu_d; __env->mu_buf = mu_buf; (closure_bool_unit){.env = __env, .fn = __anon_79_fn}; });
}

static long long __lifted_walk_uq20_38(long long mu_n, mere_vec_int* mu_data, long long mu_pos) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_pos;
  __mere_tail: ;
  return (((mu_pos + 8LL) > mu_n) ? 0LL : ({ __auto_type __let_tmp_len = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_pos; mu_u32be__direct(__da0, __da1); }); __auto_type mu_len = __let_tmp_len; (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_iend__direct(__da0, __da1); }) ? 0LL : (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_gama__direct(__da0, __da1); }) ? ({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 8LL); mu_u32be__direct(__da0, __da1); }) : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = ((mu_pos + 12LL) + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }))); }));
}

static int __lifted_walk_uq21_39(long long mu_n, mere_vec_int* mu_data, long long mu_pos) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_pos;
  __mere_tail: ;
  return (((mu_pos + 8LL) > mu_n) ? 0 : ({ __auto_type __let_tmp_len = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_pos; mu_u32be__direct(__da0, __da1); }); __auto_type mu_len = __let_tmp_len; (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_iend__direct(__da0, __da1); }) ? 0 : (({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_pos + 4LL); mu_is_trns__direct(__da0, __da1); }) ? 1 : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = ((mu_pos + 12LL) + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }))); }));
}

static mere_bytebuf* __lifted_row_40(long long mu_height, mere_bytebuf* mu_out, long long mu_start, long long mu_stride, mere_vec_int* mu_raw, long long mu_bpp, long long mu_y) {
  mere_bytebuf* __mere_ret;
  __auto_type __mere_a0 = &mu_height;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_start;
  __auto_type __mere_a3 = &mu_stride;
  __auto_type __mere_a4 = &mu_raw;
  __auto_type __mere_a5 = &mu_bpp;
  __auto_type __mere_a6 = &mu_y;
  __mere_tail: ;
  return ((mu_y == mu_height) ? mu_out : ({ __auto_type __let_tmp_src = (mu_start + (mu_y * (mu_stride + 1LL))); __auto_type mu_src = __let_tmp_src; __auto_type __let_tmp_ft = mere_vec_int_get(mu_raw, mu_src); __auto_type mu_ft = __let_tmp_ft; __auto_type __let_tmp_dst = (mu_y * mu_stride); __auto_type mu_dst = __let_tmp_dst; ({ (void)(((((0LL >= 0LL) && (0LL <= mu_stride)) && (((((mu_src + 1LL) + 0LL) >= 0LL) && ((((mu_src + 1LL) + 0LL) + 1LL) <= mere_vec_int_len(mu_raw))) && ((((mu_src + 1LL) + (mu_stride - 1LL)) >= 0LL) && ((((mu_src + 1LL) + (mu_stride - 1LL)) + 1LL) <= mere_vec_int_len(mu_raw))))) ? __lifted_col__rvfast_41(mu_stride, mu_raw, mu_src, mu_bpp, mu_out, mu_dst, mu_y, mu_ft, 0LL) : __lifted_col_42(mu_stride, mu_raw, mu_src, mu_bpp, mu_out, mu_dst, mu_y, mu_ft, 0LL))); ({ __auto_type __mt0 = mu_height; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_start; __auto_type __mt3 = mu_stride; __auto_type __mt4 = mu_raw; __auto_type __mt5 = mu_bpp; __auto_type __mt6 = (mu_y + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }); }));
}

static int __lifted_col__rvfast_41(long long mu_stride, mere_vec_int* mu_raw, long long mu_src, long long mu_bpp, mere_bytebuf* mu_out, long long mu_dst, long long mu_y, long long mu_ft, long long mu_x) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_stride;
  __auto_type __mere_a1 = &mu_raw;
  __auto_type __mere_a2 = &mu_src;
  __auto_type __mere_a3 = &mu_bpp;
  __auto_type __mere_a4 = &mu_out;
  __auto_type __mere_a5 = &mu_dst;
  __auto_type __mere_a6 = &mu_y;
  __auto_type __mere_a7 = &mu_ft;
  __auto_type __mere_a8 = &mu_x;
  __mere_tail: ;
  return ((mu_x == mu_stride) ? 0 : ({ __auto_type __let_tmp_cur = mere_vec_int_get_unchecked(mu_raw, ((mu_src + 1LL) + mu_x)); __auto_type mu_cur = __let_tmp_cur; __auto_type __let_tmp_left = ((mu_x >= mu_bpp) ? mere_bytebuf_get(mu_out, ((mu_dst + mu_x) - mu_bpp)) : 0LL); __auto_type mu_left = __let_tmp_left; __auto_type __let_tmp_up = ((mu_y > 0LL) ? mere_bytebuf_get(mu_out, ((mu_dst - mu_stride) + mu_x)) : 0LL); __auto_type mu_up = __let_tmp_up; __auto_type __let_tmp_upleft = (((mu_y > 0LL) && (mu_x >= mu_bpp)) ? mere_bytebuf_get(mu_out, (((mu_dst - mu_stride) + mu_x) - mu_bpp)) : 0LL); __auto_type mu_upleft = __let_tmp_upleft; __auto_type __let_tmp_v = ((mu_ft == 0LL) ? mu_cur : ((mu_ft == 1LL) ? (mu_cur + mu_left) : ((mu_ft == 2LL) ? (mu_cur + mu_up) : ((mu_ft == 3LL) ? (mu_cur + __lang_idiv((mu_left + mu_up), 2LL)) : ((mu_ft == 4LL) ? (mu_cur + ({ __auto_type __da0 = mu_left; __auto_type __da1 = mu_up; __auto_type __da2 = mu_upleft; mu_paeth__direct(__da0, __da1, __da2); })) : mu_cur))))); __auto_type mu_v = __let_tmp_v; (void)(mere_bytebuf_set(mu_out, (mu_dst + mu_x), mu_v)); ({ __auto_type __mt0 = mu_stride; __auto_type __mt1 = mu_raw; __auto_type __mt2 = mu_src; __auto_type __mt3 = mu_bpp; __auto_type __mt4 = mu_out; __auto_type __mt5 = mu_dst; __auto_type __mt6 = mu_y; __auto_type __mt7 = mu_ft; __auto_type __mt8 = (mu_x + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_col_42(long long mu_stride, mere_vec_int* mu_raw, long long mu_src, long long mu_bpp, mere_bytebuf* mu_out, long long mu_dst, long long mu_y, long long mu_ft, long long mu_x) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_stride;
  __auto_type __mere_a1 = &mu_raw;
  __auto_type __mere_a2 = &mu_src;
  __auto_type __mere_a3 = &mu_bpp;
  __auto_type __mere_a4 = &mu_out;
  __auto_type __mere_a5 = &mu_dst;
  __auto_type __mere_a6 = &mu_y;
  __auto_type __mere_a7 = &mu_ft;
  __auto_type __mere_a8 = &mu_x;
  __mere_tail: ;
  return ((mu_x == mu_stride) ? 0 : ({ __auto_type __let_tmp_cur = mere_vec_int_get(mu_raw, ((mu_src + 1LL) + mu_x)); __auto_type mu_cur = __let_tmp_cur; __auto_type __let_tmp_left = ((mu_x >= mu_bpp) ? mere_bytebuf_get(mu_out, ((mu_dst + mu_x) - mu_bpp)) : 0LL); __auto_type mu_left = __let_tmp_left; __auto_type __let_tmp_up = ((mu_y > 0LL) ? mere_bytebuf_get(mu_out, ((mu_dst - mu_stride) + mu_x)) : 0LL); __auto_type mu_up = __let_tmp_up; __auto_type __let_tmp_upleft = (((mu_y > 0LL) && (mu_x >= mu_bpp)) ? mere_bytebuf_get(mu_out, (((mu_dst - mu_stride) + mu_x) - mu_bpp)) : 0LL); __auto_type mu_upleft = __let_tmp_upleft; __auto_type __let_tmp_v = ((mu_ft == 0LL) ? mu_cur : ((mu_ft == 1LL) ? (mu_cur + mu_left) : ((mu_ft == 2LL) ? (mu_cur + mu_up) : ((mu_ft == 3LL) ? (mu_cur + __lang_idiv((mu_left + mu_up), 2LL)) : ((mu_ft == 4LL) ? (mu_cur + ({ __auto_type __da0 = mu_left; __auto_type __da1 = mu_up; __auto_type __da2 = mu_upleft; mu_paeth__direct(__da0, __da1, __da2); })) : mu_cur))))); __auto_type mu_v = __let_tmp_v; (void)(mere_bytebuf_set(mu_out, (mu_dst + mu_x), mu_v)); ({ __auto_type __mt0 = mu_stride; __auto_type __mt1 = mu_raw; __auto_type __mt2 = mu_src; __auto_type __mt3 = mu_bpp; __auto_type __mt4 = mu_out; __auto_type __mt5 = mu_dst; __auto_type __mt6 = mu_y; __auto_type __mt7 = mu_ft; __auto_type __mt8 = (mu_x + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_ByteBuf __lifted_pass_43(mere_bytebuf* mu_out, long long mu_width, long long mu_height, mere_vec_int* mu_raw, long long mu_bpp, long long mu_p_uq28) {
  return ({ __anon_80_env* __env = (__anon_80_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_80_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_80_env; __env->mu_p_uq28 = mu_p_uq28; __env->mu_out = mu_out; __env->mu_width = mu_width; __env->mu_height = mu_height; __env->mu_raw = mu_raw; __env->mu_bpp = mu_bpp; (closure_int_ByteBuf){.env = __env, .fn = __anon_80_fn}; });
}

static int __lifted_place_44(long long mu_ph, long long mu_pw, long long mu_p_uq28, long long mu_bpp, long long mu_width, mere_bytebuf* mu_out, mere_bytebuf* mu_sub, long long mu_y) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_ph;
  __auto_type __mere_a1 = &mu_pw;
  __auto_type __mere_a2 = &mu_p_uq28;
  __auto_type __mere_a3 = &mu_bpp;
  __auto_type __mere_a4 = &mu_width;
  __auto_type __mere_a5 = &mu_out;
  __auto_type __mere_a6 = &mu_sub;
  __auto_type __mere_a7 = &mu_y;
  __mere_tail: ;
  return ((mu_y == mu_ph) ? 0 : ({ (void)(__lifted_px_45(mu_pw, mu_p_uq28, mu_y, mu_bpp, mu_width, mu_out, mu_sub, 0LL)); ({ __auto_type __mt0 = mu_ph; __auto_type __mt1 = mu_pw; __auto_type __mt2 = mu_p_uq28; __auto_type __mt3 = mu_bpp; __auto_type __mt4 = mu_width; __auto_type __mt5 = mu_out; __auto_type __mt6 = mu_sub; __auto_type __mt7 = (mu_y + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_px_45(long long mu_pw, long long mu_p_uq28, long long mu_y, long long mu_bpp, long long mu_width, mere_bytebuf* mu_out, mere_bytebuf* mu_sub, long long mu_x) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_pw;
  __auto_type __mere_a1 = &mu_p_uq28;
  __auto_type __mere_a2 = &mu_y;
  __auto_type __mere_a3 = &mu_bpp;
  __auto_type __mere_a4 = &mu_width;
  __auto_type __mere_a5 = &mu_out;
  __auto_type __mere_a6 = &mu_sub;
  __auto_type __mere_a7 = &mu_x;
  __mere_tail: ;
  return ((mu_x == mu_pw) ? 0 : ({ __auto_type __let_tmp_sx = ({ __auto_type __bc224 = ({ __auto_type __da0 = mu_p_uq28; mu_adam7_xstart__direct(__da0); }); __auto_type __bc225 = (mu_x * ({ __auto_type __da0 = mu_p_uq28; mu_adam7_xstep__direct(__da0); })); __auto_type __bc226 = (__bc224 + __bc225); __bc226; }); __auto_type mu_sx = __let_tmp_sx; __auto_type __let_tmp_sy = ({ __auto_type __bc227 = ({ __auto_type __da0 = mu_p_uq28; mu_adam7_ystart__direct(__da0); }); __auto_type __bc228 = (mu_y * ({ __auto_type __da0 = mu_p_uq28; mu_adam7_ystep__direct(__da0); })); __auto_type __bc229 = (__bc227 + __bc228); __bc229; }); __auto_type mu_sy = __let_tmp_sy; __auto_type __let_tmp_src = (((mu_y * mu_pw) + mu_x) * mu_bpp); __auto_type mu_src = __let_tmp_src; __auto_type __let_tmp_dst = (((mu_sy * mu_width) + mu_sx) * mu_bpp); __auto_type mu_dst = __let_tmp_dst; ({ (void)(__lifted_byte_46(mu_bpp, mu_out, mu_dst, mu_sub, mu_src, 0LL)); ({ __auto_type __mt0 = mu_pw; __auto_type __mt1 = mu_p_uq28; __auto_type __mt2 = mu_y; __auto_type __mt3 = mu_bpp; __auto_type __mt4 = mu_width; __auto_type __mt5 = mu_out; __auto_type __mt6 = mu_sub; __auto_type __mt7 = (mu_x + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; goto __mere_tail; __mere_ret; }); }); }));
}

static int __lifted_byte_46(long long mu_bpp, mere_bytebuf* mu_out, long long mu_dst, mere_bytebuf* mu_sub, long long mu_src, long long mu_k) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_bpp;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_dst;
  __auto_type __mere_a3 = &mu_sub;
  __auto_type __mere_a4 = &mu_src;
  __auto_type __mere_a5 = &mu_k;
  __mere_tail: ;
  return ((mu_k == mu_bpp) ? 0 : ({ (void)(mere_bytebuf_set(mu_out, (mu_dst + mu_k), mere_bytebuf_get(mu_sub, (mu_src + mu_k)))); ({ __auto_type __mt0 = mu_bpp; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_dst; __auto_type __mt3 = mu_sub; __auto_type __mt4 = mu_src; __auto_type __mt5 = (mu_k + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_int* __lifted_go_uq29_47(long long mu_width, long long mu_height, mere_vec_int* mu_out, long long mu_bpp, mere_bytebuf* mu_pixels, long long mu_colour, mere_vec_int* mu_pal, long long mu_step, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_width;
  __auto_type __mere_a1 = &mu_height;
  __auto_type __mere_a2 = &mu_out;
  __auto_type __mere_a3 = &mu_bpp;
  __auto_type __mere_a4 = &mu_pixels;
  __auto_type __mere_a5 = &mu_colour;
  __auto_type __mere_a6 = &mu_pal;
  __auto_type __mere_a7 = &mu_step;
  __auto_type __mere_a8 = &mu_i;
  __mere_tail: ;
  return ((mu_i == (mu_width * mu_height)) ? mu_out : ({ __auto_type __let_tmp_p = (mu_i * mu_bpp); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_s0 = mere_bytebuf_get(mu_pixels, mu_p); __auto_type mu_s0 = __let_tmp_s0; (void)(((mu_colour == 3LL) ? ({ __auto_type __let_tmp_e = (mu_s0 * 3LL); __auto_type mu_e = __let_tmp_e; (((mu_e + 2LL) < mere_vec_int_len(mu_pal)) ? ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_pal, mu_e))); (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_pal, (mu_e + 1LL)))); mere_vec_int_push(mu_out, mere_vec_int_get(mu_pal, (mu_e + 2LL))); }) : ({ (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); mere_vec_int_push(mu_out, 0LL); })); }) : (((mu_colour == 0LL) || (mu_colour == 4LL)) ? ({ (void)(mere_vec_int_push(mu_out, mu_s0)); (void)(mere_vec_int_push(mu_out, mu_s0)); mere_vec_int_push(mu_out, mu_s0); }) : ({ (void)(mere_vec_int_push(mu_out, mu_s0)); (void)(mere_vec_int_push(mu_out, mere_bytebuf_get(mu_pixels, (mu_p + mu_step)))); mere_vec_int_push(mu_out, mere_bytebuf_get(mu_pixels, (mu_p + (mu_step * 2LL)))); })))); ({ __auto_type __mt0 = mu_width; __auto_type __mt1 = mu_height; __auto_type __mt2 = mu_out; __auto_type __mt3 = mu_bpp; __auto_type __mt4 = mu_pixels; __auto_type __mt5 = mu_colour; __auto_type __mt6 = mu_pal; __auto_type __mt7 = mu_step; __auto_type __mt8 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_int* __lifted_go_uq30_48(mere_vec_int* mu_v, list_int mu_l) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_v;
  __auto_type __mere_a1 = &mu_l;
  __mere_tail: ;
  return ({ __auto_type __scrut = mu_l; (list_int__tag(__scrut) == 0) ? ({ mu_v; }) : ((list_int__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_int__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_int__node(__scrut)->payload.Cons).f1; ({ (void)(mere_vec_int_push(mu_v, mu_h)); ({ __auto_type __mt0 = mu_v; __auto_type __mt1 = mu_t; *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }); }) : (({ __lang_fail_impl("no matching arm in match"); (mere_vec_int*)0; }))); });
}

static closure_int_int __lifted_go_uq31_49(long long mu_i) {
  return ({ __anon_81_env* __env = (__anon_81_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_81_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_81_env; __env->mu_i = mu_i; (closure_int_int){.env = __env, .fn = __anon_81_fn}; });
}

static closure_int_int __lifted_go__rvfast_uq32_50(long long mu_n, mere_vec_int* mu_bytes, long long mu_i) {
  return ({ __anon_82_env* __env = (__anon_82_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_82_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_82_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_bytes = mu_bytes; (closure_int_int){.env = __env, .fn = __anon_82_fn}; });
}

static closure_int_int __lifted_go_uq33_51(long long mu_n, mere_vec_int* mu_bytes, long long mu_i) {
  return ({ __anon_83_env* __env = (__anon_83_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_83_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_83_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_bytes = mu_bytes; (closure_int_int){.env = __env, .fn = __anon_83_fn}; });
}

static int __lifted_go_uq34_52(long long mu_n, long long mu_v, mere_vec_int* mu_s, mere_vec_int* mu_out, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_s;
  __auto_type __mere_a3 = &mu_out;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_bit = ((({ long long __sa = (mu_v); long long __sn = (mu_i); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (1LL)); __auto_type mu_bit = __let_tmp_bit; __auto_type __let_tmp_buf = ((mere_vec_int_get(mu_s, 0LL)) | (({ long long __sa = (mu_bit); long long __sn = (mere_vec_int_get(mu_s, 1LL)); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }))); __auto_type mu_buf = __let_tmp_buf; __auto_type __let_tmp_cnt = (mere_vec_int_get(mu_s, 1LL) + 1LL); __auto_type mu_cnt = __let_tmp_cnt; (void)(mere_vec_int_set(mu_s, 0LL, mu_buf)); (void)(mere_vec_int_set(mu_s, 1LL, mu_cnt)); (void)(((mu_cnt == 8LL) ? ({ (void)(mere_vec_int_push(mu_out, ((mu_buf) & (255LL)))); (void)(mere_vec_int_set(mu_s, 0LL, 0LL)); mere_vec_int_set(mu_s, 1LL, 0LL); }) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_v; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_out; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_int __lifted_go_uq35_53(long long mu_n, long long mu_v, long long mu_i) {
  return ({ __anon_84_env* __env = (__anon_84_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_84_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_84_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_v = mu_v; (closure_int_int){.env = __env, .fn = __anon_84_fn}; });
}

static tuple_int_int_int __lifted_go__rvfast_uq36_54(mere_vec_int* mu_g_len_base, mere_vec_int* mu_g_len_extra, long long mu_len, long long mu_i) {
  tuple_int_int_int __mere_ret;
  __auto_type __mere_a0 = &mu_g_len_base;
  __auto_type __mere_a1 = &mu_g_len_extra;
  __auto_type __mere_a2 = &mu_len;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 28LL) ? ((tuple_int_int_int){.f0 = 285LL, .f1 = 0LL, .f2 = 0LL}) : ({ __auto_type __let_tmp_base = mere_vec_int_get_unchecked(mu_g_len_base, mu_i); __auto_type mu_base = __let_tmp_base; __auto_type __let_tmp_nb = mere_vec_int_get_unchecked(mu_g_len_extra, mu_i); __auto_type mu_nb = __let_tmp_nb; __auto_type __let_tmp_next = mere_vec_int_get_unchecked(mu_g_len_base, (mu_i + 1LL)); __auto_type mu_next = __let_tmp_next; ((mu_len < mu_next) ? ((tuple_int_int_int){.f0 = (257LL + mu_i), .f1 = mu_nb, .f2 = (mu_len - mu_base)}) : ({ __auto_type __mt0 = mu_g_len_base; __auto_type __mt1 = mu_g_len_extra; __auto_type __mt2 = mu_len; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; })); }));
}

static tuple_int_int_int __lifted_go_uq37_55(mere_vec_int* mu_g_len_base, mere_vec_int* mu_g_len_extra, long long mu_len, long long mu_i) {
  tuple_int_int_int __mere_ret;
  __auto_type __mere_a0 = &mu_g_len_base;
  __auto_type __mere_a1 = &mu_g_len_extra;
  __auto_type __mere_a2 = &mu_len;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 28LL) ? ((tuple_int_int_int){.f0 = 285LL, .f1 = 0LL, .f2 = 0LL}) : ({ __auto_type __let_tmp_base = mere_vec_int_get(mu_g_len_base, mu_i); __auto_type mu_base = __let_tmp_base; __auto_type __let_tmp_nb = mere_vec_int_get(mu_g_len_extra, mu_i); __auto_type mu_nb = __let_tmp_nb; __auto_type __let_tmp_next = mere_vec_int_get(mu_g_len_base, (mu_i + 1LL)); __auto_type mu_next = __let_tmp_next; ((mu_len < mu_next) ? ((tuple_int_int_int){.f0 = (257LL + mu_i), .f1 = mu_nb, .f2 = (mu_len - mu_base)}) : ({ __auto_type __mt0 = mu_g_len_base; __auto_type __mt1 = mu_g_len_extra; __auto_type __mt2 = mu_len; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; })); }));
}

static tuple_int_int_int __lifted_go__rvfast_uq38_56(mere_vec_int* mu_g_dist_extra, long long mu_dist, mere_vec_int* mu_g_dist_base, long long mu_i) {
  tuple_int_int_int __mere_ret;
  __auto_type __mere_a0 = &mu_g_dist_extra;
  __auto_type __mere_a1 = &mu_dist;
  __auto_type __mere_a2 = &mu_g_dist_base;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 29LL) ? ((tuple_int_int_int){.f0 = 29LL, .f1 = mere_vec_int_get(mu_g_dist_extra, 29LL), .f2 = (mu_dist - mere_vec_int_get(mu_g_dist_base, 29LL))}) : ({ __auto_type __let_tmp_base = mere_vec_int_get_unchecked(mu_g_dist_base, mu_i); __auto_type mu_base = __let_tmp_base; __auto_type __let_tmp_nb = mere_vec_int_get_unchecked(mu_g_dist_extra, mu_i); __auto_type mu_nb = __let_tmp_nb; __auto_type __let_tmp_next = mere_vec_int_get_unchecked(mu_g_dist_base, (mu_i + 1LL)); __auto_type mu_next = __let_tmp_next; ((mu_dist < mu_next) ? ((tuple_int_int_int){.f0 = mu_i, .f1 = mu_nb, .f2 = (mu_dist - mu_base)}) : ({ __auto_type __mt0 = mu_g_dist_extra; __auto_type __mt1 = mu_dist; __auto_type __mt2 = mu_g_dist_base; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; })); }));
}

static tuple_int_int_int __lifted_go_uq39_57(mere_vec_int* mu_g_dist_extra, long long mu_dist, mere_vec_int* mu_g_dist_base, long long mu_i) {
  tuple_int_int_int __mere_ret;
  __auto_type __mere_a0 = &mu_g_dist_extra;
  __auto_type __mere_a1 = &mu_dist;
  __auto_type __mere_a2 = &mu_g_dist_base;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 29LL) ? ((tuple_int_int_int){.f0 = 29LL, .f1 = mere_vec_int_get(mu_g_dist_extra, 29LL), .f2 = (mu_dist - mere_vec_int_get(mu_g_dist_base, 29LL))}) : ({ __auto_type __let_tmp_base = mere_vec_int_get(mu_g_dist_base, mu_i); __auto_type mu_base = __let_tmp_base; __auto_type __let_tmp_nb = mere_vec_int_get(mu_g_dist_extra, mu_i); __auto_type mu_nb = __let_tmp_nb; __auto_type __let_tmp_next = mere_vec_int_get(mu_g_dist_base, (mu_i + 1LL)); __auto_type mu_next = __let_tmp_next; ((mu_dist < mu_next) ? ((tuple_int_int_int){.f0 = mu_i, .f1 = mu_nb, .f2 = (mu_dist - mu_base)}) : ({ __auto_type __mt0 = mu_g_dist_extra; __auto_type __mt1 = mu_dist; __auto_type __mt2 = mu_g_dist_base; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; })); }));
}

static closure_int_closure_int_tuple_int_int __lifted_best_58(long long mu_max_back, long long mu_pos, long long mu_n, mere_vec_int* mu_data, long long mu_d) {
  return ({ __anon_85_env* __env = (__anon_85_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_85_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_85_env; __env->mu_d = mu_d; __env->mu_max_back = mu_max_back; __env->mu_pos = mu_pos; __env->mu_n = mu_n; __env->mu_data = mu_data; (closure_int_closure_int_tuple_int_int){.env = __env, .fn = __anon_85_fn, .fn2 = __anon_85_fn2}; });
}

static long long __lifted_mlen__rvfast_59(long long mu_pos, long long mu_n, mere_vec_int* mu_data, long long mu_start, long long mu_k) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_pos;
  __auto_type __mere_a1 = &mu_n;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_start;
  __auto_type __mere_a4 = &mu_k;
  __mere_tail: ;
  return ((mu_k == 258LL) ? 258LL : (((mu_pos + mu_k) >= mu_n) ? mu_k : ((mere_vec_int_get_unchecked(mu_data, (mu_start + mu_k)) == mere_vec_int_get_unchecked(mu_data, (mu_pos + mu_k))) ? ({ __auto_type __mt0 = mu_pos; __auto_type __mt1 = mu_n; __auto_type __mt2 = mu_data; __auto_type __mt3 = mu_start; __auto_type __mt4 = (mu_k + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }) : mu_k)));
}

static long long __lifted_mlen_60(long long mu_pos, long long mu_n, mere_vec_int* mu_data, long long mu_start, long long mu_k) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_pos;
  __auto_type __mere_a1 = &mu_n;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_start;
  __auto_type __mere_a4 = &mu_k;
  __mere_tail: ;
  return ((mu_k == 258LL) ? 258LL : (((mu_pos + mu_k) >= mu_n) ? mu_k : ((mere_vec_int_get(mu_data, (mu_start + mu_k)) == mere_vec_int_get(mu_data, (mu_pos + mu_k))) ? ({ __auto_type __mt0 = mu_pos; __auto_type __mt1 = mu_n; __auto_type __mt2 = mu_data; __auto_type __mt3 = mu_start; __auto_type __mt4 = (mu_k + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }) : mu_k)));
}

static int __lifted_seed_61(long long mu_nsym, mere_vec_int* mu_freq, mere_vec_int* mu_nf, mere_vec_int* mu_nl, mere_vec_int* mu_nr, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nsym;
  __auto_type __mere_a1 = &mu_freq;
  __auto_type __mere_a2 = &mu_nf;
  __auto_type __mere_a3 = &mu_nl;
  __auto_type __mere_a4 = &mu_nr;
  __auto_type __mere_a5 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_nsym) ? 0 : ({ (void)(((mere_vec_int_get(mu_freq, mu_i) > 0LL) ? ({ (void)(mere_vec_int_push(mu_nf, mere_vec_int_get(mu_freq, mu_i))); (void)(mere_vec_int_push(mu_nl, (0LL - 1LL))); (void)(mere_vec_int_push(mu_nr, (0LL - 1LL))); 0; }) : 0)); ({ __auto_type __mt0 = mu_nsym; __auto_type __mt1 = mu_freq; __auto_type __mt2 = mu_nf; __auto_type __mt3 = mu_nl; __auto_type __mt4 = mu_nr; __auto_type __mt5 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_seed2_62(long long mu_nsym, mere_vec_int* mu_freq, mere_vec_int* mu_syms, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nsym;
  __auto_type __mere_a1 = &mu_freq;
  __auto_type __mere_a2 = &mu_syms;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_nsym) ? 0 : ({ (void)(((mere_vec_int_get(mu_freq, mu_i) > 0LL) ? mere_vec_int_push(mu_syms, mu_i) : 0)); ({ __auto_type __mt0 = mu_nsym; __auto_type __mt1 = mu_freq; __auto_type __mt2 = mu_syms; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_zl_63(long long mu_nsym, mere_vec_int* mu_lengths, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nsym;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_nsym) ? 0 : ({ (void)(mere_vec_int_push(mu_lengths, 0LL)); ({ __auto_type __mt0 = mu_nsym; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_al_64(long long mu_nleaves, mere_vec_int* mu_alive, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nleaves;
  __auto_type __mere_a1 = &mu_alive;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_nleaves) ? 0 : ({ (void)(mere_vec_int_push(mu_alive, 1LL)); ({ __auto_type __mt0 = mu_nleaves; __auto_type __mt1 = mu_alive; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_ls_65(long long mu_nleaves, mere_vec_int* mu_leafsym, mere_vec_int* mu_syms, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nleaves;
  __auto_type __mere_a1 = &mu_leafsym;
  __auto_type __mere_a2 = &mu_syms;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_nleaves) ? 0 : ({ (void)(mere_vec_int_push(mu_leafsym, mere_vec_int_get(mu_syms, mu_i))); ({ __auto_type __mt0 = mu_nleaves; __auto_type __mt1 = mu_leafsym; __auto_type __mt2 = mu_syms; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_merge_66(mere_vec_int* mu_nf, mere_vec_int* mu_alive, mere_vec_int* mu_nl, mere_vec_int* mu_nr, mere_vec_int* mu_leafsym, long long mu_remaining) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nf;
  __auto_type __mere_a1 = &mu_alive;
  __auto_type __mere_a2 = &mu_nl;
  __auto_type __mere_a3 = &mu_nr;
  __auto_type __mere_a4 = &mu_leafsym;
  __auto_type __mere_a5 = &mu_remaining;
  __mere_tail: ;
  return ((mu_remaining == 1LL) ? 0 : ({ __auto_type __let_tmp_np = mere_vec_int_len(mu_nf); __auto_type mu_np = __let_tmp_np; __auto_type __let_tmp_a = __lifted_find_min_67(mu_np, mu_alive, mu_nf, (0LL - 1LL)); __auto_type mu_a = __let_tmp_a; __auto_type __let_tmp_b = __lifted_find_min_67(mu_np, mu_alive, mu_nf, mu_a); __auto_type mu_b = __let_tmp_b; (void)(mere_vec_int_set(mu_alive, mu_a, 0LL)); (void)(mere_vec_int_set(mu_alive, mu_b, 0LL)); (void)(mere_vec_int_push(mu_nf, ({ __auto_type __bc230 = mere_vec_int_get(mu_nf, mu_a); __auto_type __bc231 = mere_vec_int_get(mu_nf, mu_b); __auto_type __bc232 = (__bc230 + __bc231); __bc232; }))); (void)(mere_vec_int_push(mu_nl, mu_a)); (void)(mere_vec_int_push(mu_nr, mu_b)); (void)(mere_vec_int_push(mu_leafsym, (0LL - 1LL))); (void)(mere_vec_int_push(mu_alive, 1LL)); ({ __auto_type __mt0 = mu_nf; __auto_type __mt1 = mu_alive; __auto_type __mt2 = mu_nl; __auto_type __mt3 = mu_nr; __auto_type __mt4 = mu_leafsym; __auto_type __mt5 = (mu_remaining - 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static long long __lifted_find_min_67(long long mu_np, mere_vec_int* mu_alive, mere_vec_int* mu_nf, long long mu_skip) {
  return ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = (0LL - 1LL); __lifted_go_uq42_69__direct(mu_np, mu_alive, mu_skip, mu_nf, __ia0, __ia1); });
}

static closure_int_int __lifted_go__rvfast_uq40_68(long long mu_np, mere_vec_int* mu_alive, long long mu_skip, mere_vec_int* mu_nf, long long mu_i) {
  return ({ __anon_86_env* __env = (__anon_86_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_86_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_86_env; __env->mu_i = mu_i; __env->mu_np = mu_np; __env->mu_alive = mu_alive; __env->mu_skip = mu_skip; __env->mu_nf = mu_nf; (closure_int_int){.env = __env, .fn = __anon_86_fn}; });
}

static closure_int_int __lifted_go_uq42_69(long long mu_np, mere_vec_int* mu_alive, long long mu_skip, mere_vec_int* mu_nf, long long mu_i) {
  return ({ __anon_87_env* __env = (__anon_87_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_87_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_87_env; __env->mu_i = mu_i; __env->mu_np = mu_np; __env->mu_alive = mu_alive; __env->mu_skip = mu_skip; __env->mu_nf = mu_nf; (closure_int_int){.env = __env, .fn = __anon_87_fn}; });
}

static closure_int_unit __lifted_assign_70(mere_vec_int* mu_leafsym, mere_vec_int* mu_lengths, mere_vec_int* mu_nl, mere_vec_int* mu_nr, long long mu_node) {
  return ({ __anon_88_env* __env = (__anon_88_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_88_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_88_env; __env->mu_leafsym = mu_leafsym; __env->mu_node = mu_node; __env->mu_lengths = mu_lengths; __env->mu_nl = mu_nl; __env->mu_nr = mu_nr; (closure_int_unit){.env = __env, .fn = __anon_88_fn}; });
}

static int __lifted_z_71(long long mu_maxbits, mere_vec_int* mu_bl_count, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_maxbits;
  __auto_type __mere_a1 = &mu_bl_count;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i > mu_maxbits) ? 0 : ({ (void)(mere_vec_int_push(mu_bl_count, 0LL)); ({ __auto_type __mt0 = mu_maxbits; __auto_type __mt1 = mu_bl_count; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_cnt__rvfast_72(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_bl_count, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_bl_count;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_l = mere_vec_int_get_unchecked(mu_lengths, mu_i); __auto_type mu_l = __let_tmp_l; (void)(((mu_l > 0LL) ? mere_vec_int_set(mu_bl_count, mu_l, (mere_vec_int_get(mu_bl_count, mu_l) + 1LL)) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_bl_count; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_cnt_73(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_bl_count, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_bl_count;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_l = mere_vec_int_get(mu_lengths, mu_i); __auto_type mu_l = __let_tmp_l; (void)(((mu_l > 0LL) ? mere_vec_int_set(mu_bl_count, mu_l, (mere_vec_int_get(mu_bl_count, mu_l) + 1LL)) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_bl_count; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_z2_74(long long mu_maxbits, mere_vec_int* mu_next_code, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_maxbits;
  __auto_type __mere_a1 = &mu_next_code;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i > mu_maxbits) ? 0 : ({ (void)(mere_vec_int_push(mu_next_code, 0LL)); ({ __auto_type __mt0 = mu_maxbits; __auto_type __mt1 = mu_next_code; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_unit __lifted_fill__rvfast_75(long long mu_maxbits, mere_vec_int* mu_bl_count, mere_vec_int* mu_next_code, long long mu_bits) {
  return ({ __anon_89_env* __env = (__anon_89_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_89_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_89_env; __env->mu_bits = mu_bits; __env->mu_maxbits = mu_maxbits; __env->mu_bl_count = mu_bl_count; __env->mu_next_code = mu_next_code; (closure_int_unit){.env = __env, .fn = __anon_89_fn}; });
}

static closure_int_unit __lifted_fill_uq44_76(long long mu_maxbits, mere_vec_int* mu_bl_count, mere_vec_int* mu_next_code, long long mu_bits) {
  return ({ __anon_90_env* __env = (__anon_90_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_90_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_90_env; __env->mu_bits = mu_bits; __env->mu_maxbits = mu_maxbits; __env->mu_bl_count = mu_bl_count; __env->mu_next_code = mu_next_code; (closure_int_unit){.env = __env, .fn = __anon_90_fn}; });
}

static int __lifted_z3_77(long long mu_n, mere_vec_int* mu_codes, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_codes;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ (void)(mere_vec_int_push(mu_codes, 0LL)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_codes; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_asg__rvfast_78(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_next_code, mere_vec_int* mu_codes, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_next_code;
  __auto_type __mere_a3 = &mu_codes;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_l = mere_vec_int_get_unchecked(mu_lengths, mu_i); __auto_type mu_l = __let_tmp_l; (void)(((mu_l > 0LL) ? ({ __auto_type __let_tmp_c = mere_vec_int_get(mu_next_code, mu_l); __auto_type mu_c = __let_tmp_c; (void)(mere_vec_int_set_unchecked(mu_codes, mu_i, mu_c)); mere_vec_int_set(mu_next_code, mu_l, (mu_c + 1LL)); }) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_next_code; __auto_type __mt3 = mu_codes; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_asg_79(long long mu_n, mere_vec_int* mu_lengths, mere_vec_int* mu_next_code, mere_vec_int* mu_codes, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_next_code;
  __auto_type __mere_a3 = &mu_codes;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tmp_l = mere_vec_int_get(mu_lengths, mu_i); __auto_type mu_l = __let_tmp_l; (void)(((mu_l > 0LL) ? ({ __auto_type __let_tmp_c = mere_vec_int_get(mu_next_code, mu_l); __auto_type mu_c = __let_tmp_c; (void)(mere_vec_int_set(mu_codes, mu_i, mu_c)); mere_vec_int_set(mu_next_code, mu_l, (mu_c + 1LL)); }) : 0)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_next_code; __auto_type __mt3 = mu_codes; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_int __lifted_go__rvfast_uq45_80(long long mu_n, mere_vec_int* mu_v, long long mu_i) {
  return ({ __anon_91_env* __env = (__anon_91_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_91_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_91_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_v = mu_v; (closure_int_int){.env = __env, .fn = __anon_91_fn}; });
}

static closure_int_int __lifted_go_uq46_81(long long mu_n, mere_vec_int* mu_v, long long mu_i) {
  return ({ __anon_92_env* __env = (__anon_92_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_92_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_92_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_v = mu_v; (closure_int_int){.env = __env, .fn = __anon_92_fn}; });
}

static int __lifted_go_uq47_82(long long mu_n, mere_vec_int* mu_data, mere_vec_int* mu_toks, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_toks;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ __auto_type __let_tup = ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = mu_n; mu_find_match__direct(__da0, __da1, __da2); }); __auto_type mu_mlen = (__let_tup).f0; __auto_type mu_mdist = (__let_tup).f1; ((mu_mlen >= 3LL) ? ({ (void)(mere_vec_int_push(mu_toks, 1LL)); (void)(mere_vec_int_push(mu_toks, mu_mlen)); (void)(mere_vec_int_push(mu_toks, mu_mdist)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_toks; __auto_type __mt3 = (mu_i + mu_mlen); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }) : ({ (void)(mere_vec_int_push(mu_toks, 0LL)); (void)(mere_vec_int_push(mu_toks, mere_vec_int_get(mu_data, mu_i))); (void)(mere_vec_int_push(mu_toks, 0LL)); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_toks; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); })); }));
}

static int __lifted_zl_uq48_83(mere_vec_int* mu_lfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_lfreq;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 286LL) ? 0 : ({ (void)(mere_vec_int_push(mu_lfreq, 0LL)); ({ __auto_type __mt0 = mu_lfreq; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_zd_84(mere_vec_int* mu_dfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_dfreq;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 30LL) ? 0 : ({ (void)(mere_vec_int_push(mu_dfreq, 0LL)); ({ __auto_type __mt0 = mu_dfreq; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_count__rvfast_85(long long mu_nt, mere_vec_int* mu_toks, mere_vec_int* mu_lfreq, mere_vec_int* mu_dfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nt;
  __auto_type __mere_a1 = &mu_toks;
  __auto_type __mere_a2 = &mu_lfreq;
  __auto_type __mere_a3 = &mu_dfreq;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_nt) ? 0 : ({ __auto_type __let_tmp_kind = mere_vec_int_get_unchecked(mu_toks, mu_i); __auto_type mu_kind = __let_tmp_kind; (void)(((mu_kind == 0LL) ? ({ __auto_type __let_tmp_sym = mere_vec_int_get_unchecked(mu_toks, (mu_i + 1LL)); __auto_type mu_sym = __let_tmp_sym; mere_vec_int_set(mu_lfreq, mu_sym, (mere_vec_int_get(mu_lfreq, mu_sym) + 1LL)); }) : ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get_unchecked(mu_toks, (mu_i + 1LL)); mu_len_encode__direct(__da0); }); __auto_type mu_lsym = (__let_tup).f0; __auto_type mu_a = (__let_tup).f1; __auto_type mu_b = (__let_tup).f2; ({ (void)(mu_a); (void)(mu_b); (void)(mere_vec_int_set(mu_lfreq, mu_lsym, (mere_vec_int_get(mu_lfreq, mu_lsym) + 1LL))); ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get_unchecked(mu_toks, (mu_i + 2LL)); mu_dist_encode__direct(__da0); }); __auto_type mu_dsym = (__let_tup).f0; __auto_type mu_c = (__let_tup).f1; __auto_type mu_e = (__let_tup).f2; ({ (void)(mu_c); (void)(mu_e); mere_vec_int_set(mu_dfreq, mu_dsym, (mere_vec_int_get(mu_dfreq, mu_dsym) + 1LL)); }); }); }); }))); ({ __auto_type __mt0 = mu_nt; __auto_type __mt1 = mu_toks; __auto_type __mt2 = mu_lfreq; __auto_type __mt3 = mu_dfreq; __auto_type __mt4 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_count_86(long long mu_nt, mere_vec_int* mu_toks, mere_vec_int* mu_lfreq, mere_vec_int* mu_dfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nt;
  __auto_type __mere_a1 = &mu_toks;
  __auto_type __mere_a2 = &mu_lfreq;
  __auto_type __mere_a3 = &mu_dfreq;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_nt) ? 0 : ({ __auto_type __let_tmp_kind = mere_vec_int_get(mu_toks, mu_i); __auto_type mu_kind = __let_tmp_kind; (void)(((mu_kind == 0LL) ? ({ __auto_type __let_tmp_sym = mere_vec_int_get(mu_toks, (mu_i + 1LL)); __auto_type mu_sym = __let_tmp_sym; mere_vec_int_set(mu_lfreq, mu_sym, (mere_vec_int_get(mu_lfreq, mu_sym) + 1LL)); }) : ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get(mu_toks, (mu_i + 1LL)); mu_len_encode__direct(__da0); }); __auto_type mu_lsym = (__let_tup).f0; __auto_type mu_a = (__let_tup).f1; __auto_type mu_b = (__let_tup).f2; ({ (void)(mu_a); (void)(mu_b); (void)(mere_vec_int_set(mu_lfreq, mu_lsym, (mere_vec_int_get(mu_lfreq, mu_lsym) + 1LL))); ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get(mu_toks, (mu_i + 2LL)); mu_dist_encode__direct(__da0); }); __auto_type mu_dsym = (__let_tup).f0; __auto_type mu_c = (__let_tup).f1; __auto_type mu_e = (__let_tup).f2; ({ (void)(mu_c); (void)(mu_e); mere_vec_int_set(mu_dfreq, mu_dsym, (mere_vec_int_get(mu_dfreq, mu_dsym) + 1LL)); }); }); }); }))); ({ __auto_type __mt0 = mu_nt; __auto_type __mt1 = mu_toks; __auto_type __mt2 = mu_lfreq; __auto_type __mt3 = mu_dfreq; __auto_type __mt4 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_closure_int_int __lifted_last_nz_87(mere_vec_int* mu_v) {
  return ({ __anon_93_env* __env = (__anon_93_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_93_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_93_env; __env->mu_v = mu_v; (closure_int_closure_int_int){.env = __env, .fn = __anon_93_fn, .fn2 = __anon_93_fn2}; });
}

static int __lifted_cc1_88(long long mu_hlit, mere_vec_int* mu_combined, mere_vec_int* mu_llen, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_hlit;
  __auto_type __mere_a1 = &mu_combined;
  __auto_type __mere_a2 = &mu_llen;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_hlit) ? 0 : ({ (void)(mere_vec_int_push(mu_combined, mere_vec_int_get(mu_llen, mu_i))); ({ __auto_type __mt0 = mu_hlit; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_llen; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_cc2_89(long long mu_hdist, mere_vec_int* mu_combined, mere_vec_int* mu_dlen, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_hdist;
  __auto_type __mere_a1 = &mu_combined;
  __auto_type __mere_a2 = &mu_dlen;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_hdist) ? 0 : ({ (void)(mere_vec_int_push(mu_combined, mere_vec_int_get(mu_dlen, mu_i))); ({ __auto_type __mt0 = mu_hdist; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_dlen; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_rle_90(long long mu_cn, mere_vec_int* mu_combined, mere_vec_int* mu_cl_syms, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cn;
  __auto_type __mere_a1 = &mu_combined;
  __auto_type __mere_a2 = &mu_cl_syms;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_cn) ? 0 : ({ __auto_type __let_tmp_v = mere_vec_int_get(mu_combined, mu_i); __auto_type mu_v = __let_tmp_v; ({ __auto_type __let_tmp_run = __lifted_runlen_91(mu_cn, mu_combined, mu_v, mu_i, mu_i); __auto_type mu_run = __let_tmp_run; ((mu_v == 0LL) ? ((mu_run >= 11LL) ? ({ __auto_type __let_tmp_r = ((mu_run > 138LL) ? 138LL : mu_run); __auto_type mu_r = __let_tmp_r; (void)(mere_vec_int_push(mu_cl_syms, 18LL)); (void)(mere_vec_int_push(mu_cl_syms, 7LL)); (void)(mere_vec_int_push(mu_cl_syms, (mu_r - 11LL))); ({ __auto_type __mt0 = mu_cn; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_cl_syms; __auto_type __mt3 = (mu_i + mu_r); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }) : ((mu_run >= 3LL) ? ({ __auto_type __let_tmp_r = ((mu_run > 10LL) ? 10LL : mu_run); __auto_type mu_r = __let_tmp_r; (void)(mere_vec_int_push(mu_cl_syms, 17LL)); (void)(mere_vec_int_push(mu_cl_syms, 3LL)); (void)(mere_vec_int_push(mu_cl_syms, (mu_r - 3LL))); ({ __auto_type __mt0 = mu_cn; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_cl_syms; __auto_type __mt3 = (mu_i + mu_r); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }) : ({ (void)(mere_vec_int_push(mu_cl_syms, 0LL)); (void)(mere_vec_int_push(mu_cl_syms, 0LL)); (void)(mere_vec_int_push(mu_cl_syms, 0LL)); ({ __auto_type __mt0 = mu_cn; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_cl_syms; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }))) : ({ (void)(mere_vec_int_push(mu_cl_syms, mu_v)); (void)(mere_vec_int_push(mu_cl_syms, 0LL)); (void)(mere_vec_int_push(mu_cl_syms, 0LL)); __auto_type __let_tmp_rem = (mu_run - 1LL); __auto_type mu_rem = __let_tmp_rem; ({ (void)(__lifted_reps_92(mu_cl_syms, mu_v, mu_rem)); ({ __auto_type __mt0 = mu_cn; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_cl_syms; __auto_type __mt3 = (mu_i + mu_run); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }); })); }); }));
}

static long long __lifted_runlen_91(long long mu_cn, mere_vec_int* mu_combined, long long mu_v, long long mu_i, long long mu_j) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_cn;
  __auto_type __mere_a1 = &mu_combined;
  __auto_type __mere_a2 = &mu_v;
  __auto_type __mere_a3 = &mu_i;
  __auto_type __mere_a4 = &mu_j;
  __mere_tail: ;
  return (((mu_j < mu_cn) && (mere_vec_int_get(mu_combined, mu_j) == mu_v)) ? ({ __auto_type __mt0 = mu_cn; __auto_type __mt1 = mu_combined; __auto_type __mt2 = mu_v; __auto_type __mt3 = mu_i; __auto_type __mt4 = (mu_j + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }) : (mu_j - mu_i));
}

static int __lifted_reps_92(mere_vec_int* mu_cl_syms, long long mu_v, long long mu_k) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cl_syms;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_k;
  __mere_tail: ;
  return ((mu_k >= 3LL) ? ({ __auto_type __let_tmp_r = ((mu_k > 6LL) ? 6LL : mu_k); __auto_type mu_r = __let_tmp_r; (void)(mere_vec_int_push(mu_cl_syms, 16LL)); (void)(mere_vec_int_push(mu_cl_syms, 2LL)); (void)(mere_vec_int_push(mu_cl_syms, (mu_r - 3LL))); ({ __auto_type __mt0 = mu_cl_syms; __auto_type __mt1 = mu_v; __auto_type __mt2 = (mu_k - mu_r); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }) : ((mu_k > 0LL) ? __lifted_ones_93(mu_cl_syms, mu_v, mu_k) : 0));
}

static int __lifted_ones_93(mere_vec_int* mu_cl_syms, long long mu_v, long long mu_m) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cl_syms;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_m;
  __mere_tail: ;
  return ((mu_m == 0LL) ? 0 : ({ (void)(mere_vec_int_push(mu_cl_syms, mu_v)); (void)(mere_vec_int_push(mu_cl_syms, 0LL)); (void)(mere_vec_int_push(mu_cl_syms, 0LL)); ({ __auto_type __mt0 = mu_cl_syms; __auto_type __mt1 = mu_v; __auto_type __mt2 = (mu_m - 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_zc_uq49_94(mere_vec_int* mu_clfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_clfreq;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 19LL) ? 0 : ({ (void)(mere_vec_int_push(mu_clfreq, 0LL)); ({ __auto_type __mt0 = mu_clfreq; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_ccount__rvfast_95(long long mu_cs_n, mere_vec_int* mu_cl_syms, mere_vec_int* mu_clfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cs_n;
  __auto_type __mere_a1 = &mu_cl_syms;
  __auto_type __mere_a2 = &mu_clfreq;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_cs_n) ? 0 : ({ __auto_type __let_tmp_sym = mere_vec_int_get_unchecked(mu_cl_syms, mu_i); __auto_type mu_sym = __let_tmp_sym; (void)(mere_vec_int_set(mu_clfreq, mu_sym, (mere_vec_int_get(mu_clfreq, mu_sym) + 1LL))); ({ __auto_type __mt0 = mu_cs_n; __auto_type __mt1 = mu_cl_syms; __auto_type __mt2 = mu_clfreq; __auto_type __mt3 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_ccount_96(long long mu_cs_n, mere_vec_int* mu_cl_syms, mere_vec_int* mu_clfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cs_n;
  __auto_type __mere_a1 = &mu_cl_syms;
  __auto_type __mere_a2 = &mu_clfreq;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_cs_n) ? 0 : ({ __auto_type __let_tmp_sym = mere_vec_int_get(mu_cl_syms, mu_i); __auto_type mu_sym = __let_tmp_sym; (void)(mere_vec_int_set(mu_clfreq, mu_sym, (mere_vec_int_get(mu_clfreq, mu_sym) + 1LL))); ({ __auto_type __mt0 = mu_cs_n; __auto_type __mt1 = mu_cl_syms; __auto_type __mt2 = mu_clfreq; __auto_type __mt3 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static long long __lifted_last_cl_97(mere_vec_int* mu_cllen, mere_vec_int* mu_cl_order, long long mu_i) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_cllen;
  __auto_type __mere_a1 = &mu_cl_order;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i < 4LL) ? 4LL : ((mere_vec_int_get(mu_cllen, mere_vec_int_get(mu_cl_order, mu_i)) > 0LL) ? (mu_i + 1LL) : ({ __auto_type __mt0 = mu_cllen; __auto_type __mt1 = mu_cl_order; __auto_type __mt2 = (mu_i - 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; })));
}

static int __lifted_emit_cl_98(long long mu_hclen, mere_vec_int* mu_s, mere_vec_int* mu_out, mere_vec_int* mu_cllen, mere_vec_int* mu_cl_order, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_hclen;
  __auto_type __mere_a1 = &mu_s;
  __auto_type __mere_a2 = &mu_out;
  __auto_type __mere_a3 = &mu_cllen;
  __auto_type __mere_a4 = &mu_cl_order;
  __auto_type __mere_a5 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_hclen) ? 0 : ({ (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = mere_vec_int_get(mu_cllen, mere_vec_int_get(mu_cl_order, mu_i)); __auto_type __da3 = 3LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); ({ __auto_type __mt0 = mu_hclen; __auto_type __mt1 = mu_s; __auto_type __mt2 = mu_out; __auto_type __mt3 = mu_cllen; __auto_type __mt4 = mu_cl_order; __auto_type __mt5 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_emit_rle_99(long long mu_cs_n, mere_vec_int* mu_cl_syms, mere_vec_int* mu_s, mere_vec_int* mu_out, mere_vec_int* mu_clcode, mere_vec_int* mu_cllen, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_cs_n;
  __auto_type __mere_a1 = &mu_cl_syms;
  __auto_type __mere_a2 = &mu_s;
  __auto_type __mere_a3 = &mu_out;
  __auto_type __mere_a4 = &mu_clcode;
  __auto_type __mere_a5 = &mu_cllen;
  __auto_type __mere_a6 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_cs_n) ? 0 : ({ __auto_type __let_tmp_sym = mere_vec_int_get(mu_cl_syms, mu_i); __auto_type mu_sym = __let_tmp_sym; __auto_type __let_tmp_enb = mere_vec_int_get(mu_cl_syms, (mu_i + 1LL)); __auto_type mu_enb = __let_tmp_enb; __auto_type __let_tmp_eval = mere_vec_int_get(mu_cl_syms, (mu_i + 2LL)); __auto_type mu_eval = __let_tmp_eval; (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = ({ __auto_type __da0 = mere_vec_int_get(mu_clcode, mu_sym); __auto_type __da1 = mere_vec_int_get(mu_cllen, mu_sym); mu_rev_bits__direct(__da0, __da1); }); __auto_type __da3 = mere_vec_int_get(mu_cllen, mu_sym); mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(((mu_enb > 0LL) ? ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = mu_eval; __auto_type __da3 = mu_enb; mu_bw_bits__direct(__da0, __da1, __da2, __da3); }) : 0)); ({ __auto_type __mt0 = mu_cs_n; __auto_type __mt1 = mu_cl_syms; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_out; __auto_type __mt4 = mu_clcode; __auto_type __mt5 = mu_cllen; __auto_type __mt6 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_emit_data_100(long long mu_nt, mere_vec_int* mu_toks, mere_vec_int* mu_s, mere_vec_int* mu_out, mere_vec_int* mu_lcode, mere_vec_int* mu_llen, mere_vec_int* mu_dcode, mere_vec_int* mu_dlen, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nt;
  __auto_type __mere_a1 = &mu_toks;
  __auto_type __mere_a2 = &mu_s;
  __auto_type __mere_a3 = &mu_out;
  __auto_type __mere_a4 = &mu_lcode;
  __auto_type __mere_a5 = &mu_llen;
  __auto_type __mere_a6 = &mu_dcode;
  __auto_type __mere_a7 = &mu_dlen;
  __auto_type __mere_a8 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_nt) ? 0 : ({ __auto_type __let_tmp_kind = mere_vec_int_get(mu_toks, mu_i); __auto_type mu_kind = __let_tmp_kind; (void)(((mu_kind == 0LL) ? ({ __auto_type __let_tmp_sym = mere_vec_int_get(mu_toks, (mu_i + 1LL)); __auto_type mu_sym = __let_tmp_sym; ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = ({ __auto_type __da0 = mere_vec_int_get(mu_lcode, mu_sym); __auto_type __da1 = mere_vec_int_get(mu_llen, mu_sym); mu_rev_bits__direct(__da0, __da1); }); __auto_type __da3 = mere_vec_int_get(mu_llen, mu_sym); mu_bw_bits__direct(__da0, __da1, __da2, __da3); }); }) : ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get(mu_toks, (mu_i + 1LL)); mu_len_encode__direct(__da0); }); __auto_type mu_lsym = (__let_tup).f0; __auto_type mu_lnb = (__let_tup).f1; __auto_type mu_lval = (__let_tup).f2; ({ (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = ({ __auto_type __da0 = mere_vec_int_get(mu_lcode, mu_lsym); __auto_type __da1 = mere_vec_int_get(mu_llen, mu_lsym); mu_rev_bits__direct(__da0, __da1); }); __auto_type __da3 = mere_vec_int_get(mu_llen, mu_lsym); mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(((mu_lnb > 0LL) ? ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = mu_lval; __auto_type __da3 = mu_lnb; mu_bw_bits__direct(__da0, __da1, __da2, __da3); }) : 0)); ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get(mu_toks, (mu_i + 2LL)); mu_dist_encode__direct(__da0); }); __auto_type mu_dsym = (__let_tup).f0; __auto_type mu_dnb = (__let_tup).f1; __auto_type mu_dval = (__let_tup).f2; ({ (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = ({ __auto_type __da0 = mere_vec_int_get(mu_dcode, mu_dsym); __auto_type __da1 = mere_vec_int_get(mu_dlen, mu_dsym); mu_rev_bits__direct(__da0, __da1); }); __auto_type __da3 = mere_vec_int_get(mu_dlen, mu_dsym); mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(((mu_dnb > 0LL) ? ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = mu_dval; __auto_type __da3 = mu_dnb; mu_bw_bits__direct(__da0, __da1, __da2, __da3); }) : 0)); 0; }); }); }); }))); ({ __auto_type __mt0 = mu_nt; __auto_type __mt1 = mu_toks; __auto_type __mt2 = mu_s; __auto_type __mt3 = mu_out; __auto_type __mt4 = mu_lcode; __auto_type __mt5 = mu_llen; __auto_type __mt6 = mu_dcode; __auto_type __mt7 = mu_dlen; __auto_type __mt8 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_blocks_101(long long mu_n, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_off) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_off;
  __mere_tail: ;
  return ({ __auto_type __let_tmp_remain = (mu_n - mu_off); __auto_type mu_remain = __let_tmp_remain; __auto_type __let_tmp_len = ((mu_remain > 65535LL) ? 65535LL : mu_remain); __auto_type mu_len = __let_tmp_len; __auto_type __let_tmp_final = ((mu_off + mu_len) >= mu_n); __auto_type mu_final = __let_tmp_final; (void)(mere_vec_int_push(mu_out, (mu_final ? 1LL : 0LL))); (void)(mere_vec_int_push(mu_out, ((mu_len) & (255LL)))); (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_len); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); __auto_type __let_tmp_nlen = (((~(mu_len))) & (65535LL)); __auto_type mu_nlen = __let_tmp_nlen; (void)(mere_vec_int_push(mu_out, ((mu_nlen) & (255LL)))); (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_nlen); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); ({ (void)(__lifted_cp_102(mu_len, mu_out, mu_data, mu_off, 0LL)); (mu_final ? 0 : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = (mu_off + mu_len); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; })); }); });
}

static int __lifted_cp_102(long long mu_len, mere_vec_int* mu_out, mere_vec_int* mu_data, long long mu_off, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_off;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_len) ? 0 : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_data, (mu_off + mu_i)))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_data; __auto_type __mt3 = mu_off; __auto_type __mt4 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_zl_uq50_103(mere_vec_int* mu_lfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_lfreq;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 286LL) ? 0 : ({ (void)(mere_vec_int_push(mu_lfreq, 0LL)); ({ __auto_type __mt0 = mu_lfreq; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_zd_uq51_104(mere_vec_int* mu_dfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_dfreq;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i == 30LL) ? 0 : ({ (void)(mere_vec_int_push(mu_dfreq, 0LL)); ({ __auto_type __mt0 = mu_dfreq; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_count__rvfast_uq52_105(long long mu_nt, mere_vec_int* mu_toks, mere_vec_int* mu_lfreq, mere_vec_int* mu_dfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nt;
  __auto_type __mere_a1 = &mu_toks;
  __auto_type __mere_a2 = &mu_lfreq;
  __auto_type __mere_a3 = &mu_dfreq;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_nt) ? 0 : ({ __auto_type __let_tmp_kind = mere_vec_int_get_unchecked(mu_toks, mu_i); __auto_type mu_kind = __let_tmp_kind; (void)(((mu_kind == 0LL) ? ({ __auto_type __let_tmp_sym = mere_vec_int_get_unchecked(mu_toks, (mu_i + 1LL)); __auto_type mu_sym = __let_tmp_sym; mere_vec_int_set(mu_lfreq, mu_sym, (mere_vec_int_get(mu_lfreq, mu_sym) + 1LL)); }) : ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get_unchecked(mu_toks, (mu_i + 1LL)); mu_len_encode__direct(__da0); }); __auto_type mu_lsym = (__let_tup).f0; __auto_type mu_a = (__let_tup).f1; __auto_type mu_b = (__let_tup).f2; ({ (void)(mu_a); (void)(mu_b); (void)(mere_vec_int_set(mu_lfreq, mu_lsym, (mere_vec_int_get(mu_lfreq, mu_lsym) + 1LL))); ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get_unchecked(mu_toks, (mu_i + 2LL)); mu_dist_encode__direct(__da0); }); __auto_type mu_dsym = (__let_tup).f0; __auto_type mu_c = (__let_tup).f1; __auto_type mu_e = (__let_tup).f2; ({ (void)(mu_c); (void)(mu_e); mere_vec_int_set(mu_dfreq, mu_dsym, (mere_vec_int_get(mu_dfreq, mu_dsym) + 1LL)); }); }); }); }))); ({ __auto_type __mt0 = mu_nt; __auto_type __mt1 = mu_toks; __auto_type __mt2 = mu_lfreq; __auto_type __mt3 = mu_dfreq; __auto_type __mt4 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_count_uq53_106(long long mu_nt, mere_vec_int* mu_toks, mere_vec_int* mu_lfreq, mere_vec_int* mu_dfreq, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_nt;
  __auto_type __mere_a1 = &mu_toks;
  __auto_type __mere_a2 = &mu_lfreq;
  __auto_type __mere_a3 = &mu_dfreq;
  __auto_type __mere_a4 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_nt) ? 0 : ({ __auto_type __let_tmp_kind = mere_vec_int_get(mu_toks, mu_i); __auto_type mu_kind = __let_tmp_kind; (void)(((mu_kind == 0LL) ? ({ __auto_type __let_tmp_sym = mere_vec_int_get(mu_toks, (mu_i + 1LL)); __auto_type mu_sym = __let_tmp_sym; mere_vec_int_set(mu_lfreq, mu_sym, (mere_vec_int_get(mu_lfreq, mu_sym) + 1LL)); }) : ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get(mu_toks, (mu_i + 1LL)); mu_len_encode__direct(__da0); }); __auto_type mu_lsym = (__let_tup).f0; __auto_type mu_a = (__let_tup).f1; __auto_type mu_b = (__let_tup).f2; ({ (void)(mu_a); (void)(mu_b); (void)(mere_vec_int_set(mu_lfreq, mu_lsym, (mere_vec_int_get(mu_lfreq, mu_lsym) + 1LL))); ({ __auto_type __let_tup = ({ __auto_type __da0 = mere_vec_int_get(mu_toks, (mu_i + 2LL)); mu_dist_encode__direct(__da0); }); __auto_type mu_dsym = (__let_tup).f0; __auto_type mu_c = (__let_tup).f1; __auto_type mu_e = (__let_tup).f2; ({ (void)(mu_c); (void)(mu_e); mere_vec_int_set(mu_dfreq, mu_dsym, (mere_vec_int_get(mu_dfreq, mu_dsym) + 1LL)); }); }); }); }))); ({ __auto_type __mt0 = mu_nt; __auto_type __mt1 = mu_toks; __auto_type __mt2 = mu_lfreq; __auto_type __mt3 = mu_dfreq; __auto_type __mt4 = (mu_i + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_cp_uq54_107(long long mu_bn, mere_vec_int* mu_out, mere_vec_int* mu_body, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_bn;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_body;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_bn) ? 0 : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_body, mu_i))); ({ __auto_type __mt0 = mu_bn; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_body; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_push_u32_108(mere_vec_int* mu_out, long long mu_v) {
  return ({ (void)(mere_vec_int_push(mu_out, ((mu_v) & (255LL)))); (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_v); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_v); long long __sn = (16LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); mere_vec_int_push(mu_out, ((({ long long __sa = (mu_v); long long __sn = (24LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL))); });
}

static int __lifted_go_uq55_109(long long mu_n, mere_vec_int* mu_out, mere_vec_int* mu_src, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_src;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? 0 : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_src, mu_i))); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_src; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_closure_int_int __lifted_go__rvfast_uq56_110(long long mu_n, mere_vec_int* mu_data, long long mu_i) {
  return ({ __anon_94_env* __env = (__anon_94_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_94_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_94_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_data = mu_data; (closure_int_closure_int_int){.env = __env, .fn = __anon_94_fn, .fn2 = __anon_94_fn2}; });
}

static closure_int_closure_int_int __lifted_go_uq57_111(long long mu_n, mere_vec_int* mu_data, long long mu_i) {
  return ({ __anon_95_env* __env = (__anon_95_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_95_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_95_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_data = mu_data; (closure_int_closure_int_int){.env = __env, .fn = __anon_95_fn, .fn2 = __anon_95_fn2}; });
}

static mere_vec_int* __lifted_go_uq59_112(long long mu_n, mere_vec_int* mu_out, mere_vec_int* mu_row_uq58, long long mu_bpp, mere_vec_int* mu_prev, long long mu_kind, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_row_uq58;
  __auto_type __mere_a3 = &mu_bpp;
  __auto_type __mere_a4 = &mu_prev;
  __auto_type __mere_a5 = &mu_kind;
  __auto_type __mere_a6 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_out : ({ __auto_type __let_tmp_cur = mere_vec_int_get(mu_row_uq58, mu_i); __auto_type mu_cur = __let_tmp_cur; __auto_type __let_tmp_left = ((mu_i >= mu_bpp) ? mere_vec_int_get(mu_row_uq58, (mu_i - mu_bpp)) : 0LL); __auto_type mu_left = __let_tmp_left; __auto_type __let_tmp_up = ((mere_vec_int_len(mu_prev) > 0LL) ? mere_vec_int_get(mu_prev, mu_i) : 0LL); __auto_type mu_up = __let_tmp_up; __auto_type __let_tmp_upleft = (((mere_vec_int_len(mu_prev) > 0LL) && (mu_i >= mu_bpp)) ? mere_vec_int_get(mu_prev, (mu_i - mu_bpp)) : 0LL); __auto_type mu_upleft = __let_tmp_upleft; __auto_type __let_tmp_v = ((mu_kind == 0LL) ? mu_cur : ((mu_kind == 1LL) ? (mu_cur - mu_left) : ((mu_kind == 2LL) ? (mu_cur - mu_up) : ((mu_kind == 3LL) ? (mu_cur - __lang_idiv((mu_left + mu_up), 2LL)) : (mu_cur - ({ __auto_type __da0 = mu_left; __auto_type __da1 = mu_up; __auto_type __da2 = mu_upleft; mu_paeth_e__direct(__da0, __da1, __da2); })))))); __auto_type mu_v = __let_tmp_v; (void)(mere_vec_int_push(mu_out, ((mu_v) & (255LL)))); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_row_uq58; __auto_type __mt3 = mu_bpp; __auto_type __mt4 = mu_prev; __auto_type __mt5 = mu_kind; __auto_type __mt6 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_int __lifted_go__rvfast_uq61_113(long long mu_n, mere_vec_int* mu_row_uq60, long long mu_i) {
  return ({ __anon_96_env* __env = (__anon_96_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_96_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_96_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_row_uq60 = mu_row_uq60; (closure_int_int){.env = __env, .fn = __anon_96_fn}; });
}

static closure_int_int __lifted_go_uq62_114(long long mu_n, mere_vec_int* mu_row_uq60, long long mu_i) {
  return ({ __anon_97_env* __env = (__anon_97_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_97_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_97_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_row_uq60 = mu_row_uq60; (closure_int_int){.env = __env, .fn = __anon_97_fn}; });
}

static closure_int_closure_int_closure_Vec_int_Vec_int __lifted_go_uq64_115(mere_vec_int* mu_row_uq63, mere_vec_int* mu_prev, long long mu_bpp, long long mu_kind) {
  return ({ __anon_98_env* __env = (__anon_98_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_98_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_98_env; __env->mu_kind = mu_kind; __env->mu_row_uq63 = mu_row_uq63; __env->mu_prev = mu_prev; __env->mu_bpp = mu_bpp; (closure_int_closure_int_closure_Vec_int_Vec_int){.env = __env, .fn = __anon_98_fn, .fn2 = __anon_98_fn2}; });
}

static int __lifted_go_uq66_116(long long mu_n, mere_vec_int* mu_pixels, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_pixels;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mu_n) ? 1 : ({ __auto_type __let_tmp_p = (mu_i * 3LL); __auto_type mu_p = __let_tmp_p; (((mere_vec_int_get(mu_pixels, mu_p) == mere_vec_int_get(mu_pixels, (mu_p + 1LL))) && (mere_vec_int_get(mu_pixels, mu_p) == mere_vec_int_get(mu_pixels, (mu_p + 2LL)))) ? ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_pixels; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }) : 0); }));
}

static mere_vec_int* __lifted_go_uq67_117(long long mu_n, mere_vec_int* mu_pal, mere_vec_int* mu_pixels, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_pal;
  __auto_type __mere_a2 = &mu_pixels;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return (((mu_i >= mu_n) || (mere_vec_int_len(mu_pal) > 768LL)) ? mu_pal : ({ __auto_type __let_tmp_p = (mu_i * 3LL); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_r = mere_vec_int_get(mu_pixels, mu_p); __auto_type mu_r = __let_tmp_r; __auto_type __let_tmp_g = mere_vec_int_get(mu_pixels, (mu_p + 1LL)); __auto_type mu_g = __let_tmp_g; __auto_type __let_tmp_b = mere_vec_int_get(mu_pixels, (mu_p + 2LL)); __auto_type mu_b = __let_tmp_b; ({ (void)((((((((0LL >= 0LL) && (0LL <= mere_vec_int_len(mu_pal))) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len(mu_pal))) && ((((mere_vec_int_len(mu_pal) - 1LL) + 2LL) >= 0LL) && ((((mere_vec_int_len(mu_pal) - 1LL) + 2LL) + 1LL) <= mere_vec_int_len(mu_pal))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_pal))) && ((((mere_vec_int_len(mu_pal) - 1LL) + 1LL) >= 0LL) && ((((mere_vec_int_len(mu_pal) - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_pal))))) && (mere_vec_int_len(mu_pal) <= mere_vec_int_len(mu_pal))) ? __lifted_seen__rvfast_118(mu_pal, mu_r, mu_g, mu_b, 0LL) : __lifted_seen_119(mu_pal, mu_r, mu_g, mu_b, 0LL)) ? 0 : ({ (void)(mere_vec_int_push(mu_pal, mu_r)); (void)(mere_vec_int_push(mu_pal, mu_g)); mere_vec_int_push(mu_pal, mu_b); }))); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_pal; __auto_type __mt2 = mu_pixels; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }); }));
}

static int __lifted_seen__rvfast_118(mere_vec_int* mu_pal, long long mu_r, long long mu_g, long long mu_b, long long mu_k) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_pal;
  __auto_type __mere_a1 = &mu_r;
  __auto_type __mere_a2 = &mu_g;
  __auto_type __mere_a3 = &mu_b;
  __auto_type __mere_a4 = &mu_k;
  __mere_tail: ;
  return ((mu_k >= mere_vec_int_len(mu_pal)) ? 0 : ((((mere_vec_int_get_unchecked(mu_pal, mu_k) == mu_r) && (mere_vec_int_get_unchecked(mu_pal, (mu_k + 1LL)) == mu_g)) && (mere_vec_int_get_unchecked(mu_pal, (mu_k + 2LL)) == mu_b)) ? 1 : ({ __auto_type __mt0 = mu_pal; __auto_type __mt1 = mu_r; __auto_type __mt2 = mu_g; __auto_type __mt3 = mu_b; __auto_type __mt4 = (mu_k + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; })));
}

static int __lifted_seen_119(mere_vec_int* mu_pal, long long mu_r, long long mu_g, long long mu_b, long long mu_k) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_pal;
  __auto_type __mere_a1 = &mu_r;
  __auto_type __mere_a2 = &mu_g;
  __auto_type __mere_a3 = &mu_b;
  __auto_type __mere_a4 = &mu_k;
  __mere_tail: ;
  return ((mu_k >= mere_vec_int_len(mu_pal)) ? 0 : ((((mere_vec_int_get(mu_pal, mu_k) == mu_r) && (mere_vec_int_get(mu_pal, (mu_k + 1LL)) == mu_g)) && (mere_vec_int_get(mu_pal, (mu_k + 2LL)) == mu_b)) ? 1 : ({ __auto_type __mt0 = mu_pal; __auto_type __mt1 = mu_r; __auto_type __mt2 = mu_g; __auto_type __mt3 = mu_b; __auto_type __mt4 = (mu_k + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; })));
}

static long long __lifted_go__rvfast_uq68_120(mere_vec_int* mu_pal, long long mu_r, long long mu_g, long long mu_b, long long mu_k) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_pal;
  __auto_type __mere_a1 = &mu_r;
  __auto_type __mere_a2 = &mu_g;
  __auto_type __mere_a3 = &mu_b;
  __auto_type __mere_a4 = &mu_k;
  __mere_tail: ;
  return ((mu_k >= mere_vec_int_len(mu_pal)) ? 0LL : ((((mere_vec_int_get_unchecked(mu_pal, mu_k) == mu_r) && (mere_vec_int_get_unchecked(mu_pal, (mu_k + 1LL)) == mu_g)) && (mere_vec_int_get_unchecked(mu_pal, (mu_k + 2LL)) == mu_b)) ? __lang_idiv(mu_k, 3LL) : ({ __auto_type __mt0 = mu_pal; __auto_type __mt1 = mu_r; __auto_type __mt2 = mu_g; __auto_type __mt3 = mu_b; __auto_type __mt4 = (mu_k + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; })));
}

static long long __lifted_go_uq69_121(mere_vec_int* mu_pal, long long mu_r, long long mu_g, long long mu_b, long long mu_k) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_pal;
  __auto_type __mere_a1 = &mu_r;
  __auto_type __mere_a2 = &mu_g;
  __auto_type __mere_a3 = &mu_b;
  __auto_type __mere_a4 = &mu_k;
  __mere_tail: ;
  return ((mu_k >= mere_vec_int_len(mu_pal)) ? 0LL : ((((mere_vec_int_get(mu_pal, mu_k) == mu_r) && (mere_vec_int_get(mu_pal, (mu_k + 1LL)) == mu_g)) && (mere_vec_int_get(mu_pal, (mu_k + 2LL)) == mu_b)) ? __lang_idiv(mu_k, 3LL) : ({ __auto_type __mt0 = mu_pal; __auto_type __mt1 = mu_r; __auto_type __mt2 = mu_g; __auto_type __mt3 = mu_b; __auto_type __mt4 = (mu_k + 3LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; })));
}

static closure_Vec_int_unit __lifted_rows_122(long long mu_height, long long mu_stride, int mu_grey, mere_vec_int* mu_pixels, long long mu_width, int mu_indexed, mere_vec_int* mu_pal, mere_vec_int* mu_raw, long long mu_bpp, long long mu_y) {
  return ({ __anon_99_env* __env = (__anon_99_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_99_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_99_env; __env->mu_y = mu_y; __env->mu_height = mu_height; __env->mu_stride = mu_stride; __env->mu_grey = mu_grey; __env->mu_pixels = mu_pixels; __env->mu_width = mu_width; __env->mu_indexed = mu_indexed; __env->mu_pal = mu_pal; __env->mu_raw = mu_raw; __env->mu_bpp = mu_bpp; (closure_Vec_int_unit){.env = __env, .fn = __anon_99_fn}; });
}

static int __lifted_col_uq71_123(long long mu_stride, int mu_grey, mere_vec_int* mu_row, mere_vec_int* mu_pixels, long long mu_y, long long mu_width, int mu_indexed, mere_vec_int* mu_pal, long long mu_x) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_stride;
  __auto_type __mere_a1 = &mu_grey;
  __auto_type __mere_a2 = &mu_row;
  __auto_type __mere_a3 = &mu_pixels;
  __auto_type __mere_a4 = &mu_y;
  __auto_type __mere_a5 = &mu_width;
  __auto_type __mere_a6 = &mu_indexed;
  __auto_type __mere_a7 = &mu_pal;
  __auto_type __mere_a8 = &mu_x;
  __mere_tail: ;
  return ((mu_x == mu_stride) ? 0 : ({ (void)((mu_grey ? mere_vec_int_push(mu_row, mere_vec_int_get(mu_pixels, (((mu_y * mu_width) + mu_x) * 3LL))) : (mu_indexed ? ({ __auto_type __let_tmp_p = (((mu_y * mu_width) + mu_x) * 3LL); __auto_type mu_p = __let_tmp_p; mere_vec_int_push(mu_row, ({ __auto_type __da0 = mu_pal; __auto_type __da1 = mere_vec_int_get(mu_pixels, mu_p); __auto_type __da2 = mere_vec_int_get(mu_pixels, (mu_p + 1LL)); __auto_type __da3 = mere_vec_int_get(mu_pixels, (mu_p + 2LL)); mu_palette_index__direct(__da0, __da1, __da2, __da3); })); }) : mere_vec_int_push(mu_row, mere_vec_int_get(mu_pixels, ((mu_y * mu_stride) + mu_x)))))); ({ __auto_type __mt0 = mu_stride; __auto_type __mt1 = mu_grey; __auto_type __mt2 = mu_row; __auto_type __mt3 = mu_pixels; __auto_type __mt4 = mu_y; __auto_type __mt5 = mu_width; __auto_type __mt6 = mu_indexed; __auto_type __mt7 = mu_pal; __auto_type __mt8 = (mu_x + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; goto __mere_tail; __mere_ret; }); }));
}

static mere_vec_int* __lifted_go_uq70_124(mere_vec_int* mu_sig, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_sig;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= 8LL) ? mu_sig : ({ __auto_type __let_tmp_b = ((mu_i == 0LL) ? 137LL : ((mu_i == 1LL) ? 80LL : ((mu_i == 2LL) ? 78LL : ((mu_i == 3LL) ? 71LL : ((mu_i == 4LL) ? 13LL : ((mu_i == 5LL) ? 10LL : ((mu_i == 6LL) ? 26LL : 10LL))))))); __auto_type mu_b = __let_tmp_b; (void)(mere_vec_int_push(mu_sig, mu_b)); ({ __auto_type __mt0 = mu_sig; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_show__rvfast_125(mere_vec_str* mu_t, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_t;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mere_vec_str_len(mu_t)) ? 0 : ({ (void)(({ const char* __ps = __lang_str_concat((__sl_32.s), mere_vec_str_get_unchecked(mu_t, mu_i));            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; })); ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_show_126(mere_vec_str* mu_t, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_t;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= mere_vec_str_len(mu_t)) ? 0 : ({ (void)(({ const char* __ps = __lang_str_concat((__sl_32.s), mere_vec_str_get(mu_t, mu_i));            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; })); ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_unit __lifted_header_127(mere_vec_int* mu_out, const char* mu_s) {
  return ({ __anon_100_env* __env = (__anon_100_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_100_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_100_env; __env->mu_s = mu_s; __env->mu_out = mu_out; (closure_int_unit){.env = __env, .fn = __anon_100_fn}; });
}

static mere_vec_int* __lifted_go_uq72_128(long long mu_n, mere_vec_int* mu_out, mere_vec_int* mu_rgb, long long mu_i) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_out;
  __auto_type __mere_a2 = &mu_rgb;
  __auto_type __mere_a3 = &mu_i;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_out : ({ (void)(mere_vec_int_push(mu_out, mere_vec_int_get(mu_rgb, mu_i))); ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_out; __auto_type __mt2 = mu_rgb; __auto_type __mt3 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}

static closure_int_closure_int_closure_int_unit __lifted_fields_129(long long mu_n, mere_vec_int* mu_data, mere_vec_int* mu_acc, long long mu_i) {
  return ({ __anon_101_env* __env = (__anon_101_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_101_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_101_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_data = mu_data; __env->mu_acc = mu_acc; (closure_int_closure_int_closure_int_unit){.env = __env, .fn = __anon_101_fn, .fn2 = __anon_101_fn2}; });
}

static int __lifted_copy_uq74_130(long long mu_width, long long mu_height, long long mu_bytes_per_sample, mere_vec_int* mu_pixels, mere_vec_int* mu_data, long long mu_start, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_width;
  __auto_type __mere_a1 = &mu_height;
  __auto_type __mere_a2 = &mu_bytes_per_sample;
  __auto_type __mere_a3 = &mu_pixels;
  __auto_type __mere_a4 = &mu_data;
  __auto_type __mere_a5 = &mu_start;
  __auto_type __mere_a6 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= (((mu_width * mu_height) * 3LL) * mu_bytes_per_sample)) ? 0 : ({ (void)(mere_vec_int_push(mu_pixels, mere_vec_int_get(mu_data, (mu_start + mu_i)))); ({ __auto_type __mt0 = mu_width; __auto_type __mt1 = mu_height; __auto_type __mt2 = mu_bytes_per_sample; __auto_type __mt3 = mu_pixels; __auto_type __mt4 = mu_data; __auto_type __mt5 = mu_start; __auto_type __mt6 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }));
}

static closure_str_str __lifted_join__rvfast_131(mere_vec_str* mu_t, long long mu_i) {
  return ({ __anon_102_env* __env = (__anon_102_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_102_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_102_env; __env->mu_i = mu_i; __env->mu_t = mu_t; (closure_str_str){.env = __env, .fn = __anon_102_fn}; });
}

static closure_str_str __lifted_join_132(mere_vec_str* mu_t, long long mu_i) {
  return ({ __anon_103_env* __env = (__anon_103_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_103_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_103_env; __env->mu_i = mu_i; __env->mu_t = mu_t; (closure_str_str){.env = __env, .fn = __anon_103_fn}; });
}

static int __lifted_copy_uq75_133(long long mu_width, long long mu_height, long long mu_bytes_per_sample, mere_vec_int* mu_pixels, mere_vec_int* mu_v, long long mu_start, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_width;
  __auto_type __mere_a1 = &mu_height;
  __auto_type __mere_a2 = &mu_bytes_per_sample;
  __auto_type __mere_a3 = &mu_pixels;
  __auto_type __mere_a4 = &mu_v;
  __auto_type __mere_a5 = &mu_start;
  __auto_type __mere_a6 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= (((mu_width * mu_height) * 3LL) * mu_bytes_per_sample)) ? 0 : ({ (void)(mere_vec_int_push(mu_pixels, mere_vec_int_get(mu_v, (mu_start + mu_i)))); ({ __auto_type __mt0 = mu_width; __auto_type __mt1 = mu_height; __auto_type __mt2 = mu_bytes_per_sample; __auto_type __mt3 = mu_pixels; __auto_type __mt4 = mu_v; __auto_type __mt5 = mu_start; __auto_type __mt6 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }));
}

static int __lifted_go_uq12_134(mere_vec_int* mu_v, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_v;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= 8LL) ? 0 : ({ __auto_type __let_tmp_b = ((mu_i == 0LL) ? 137LL : ((mu_i == 1LL) ? 80LL : ((mu_i == 2LL) ? 78LL : ((mu_i == 3LL) ? 71LL : ((mu_i == 4LL) ? 13LL : ((mu_i == 5LL) ? 10LL : ((mu_i == 6LL) ? 26LL : 10LL))))))); __auto_type mu_b = __let_tmp_b; (void)(mere_vec_int_push(mu_v, mu_b)); ({ __auto_type __mt0 = mu_v; __auto_type __mt1 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }));
}

long long __lifted_go_0__direct(long long mu_i, long long mu_c) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_i;
  __auto_type __mere_a1 = &mu_c;
  __mere_tail: ;
  return ((mu_i == 8LL) ? mu_c : ({ __auto_type __mt0 = (mu_i + 1LL); __auto_type __mt1 = ((((mu_c) & (1LL)) == 1LL) ? ((({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) ^ (3988292384LL)) : ({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go__rvfast_1__direct(long long mu_n, mere_vec_int* mu_bytes, long long mu_i, long long mu_crc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_bytes;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_crc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_crc : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_bytes; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get_unchecked(mu_bytes, mu_i); mu_crc_byte__direct(__da0, __da1); }); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go_uq1_2__direct(long long mu_n, mere_vec_int* mu_bytes, long long mu_i, long long mu_crc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_bytes;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_crc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_crc : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_bytes; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get(mu_bytes, mu_i); mu_crc_byte__direct(__da0, __da1); }); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go_uq2_3__direct(long long mu_n, mere_vec_int* mu_s, mere_vec_int* mu_data, long long mu_i, long long mu_acc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_s;
  __auto_type __mere_a2 = &mu_data;
  __auto_type __mere_a3 = &mu_i;
  __auto_type __mere_a4 = &mu_acc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_acc : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_s; __auto_type __mt2 = mu_data; __auto_type __mt3 = (mu_i + 1LL); __auto_type __mt4 = ((mu_acc) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; mu_br_bit__direct(__da0, __da1); })); long long __sn = (mu_i); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }))); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go__rvfast_uq3_9__direct(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_count, mere_vec_int* mu_syms, long long mu_len, long long mu_code0, long long mu_first, long long mu_index) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_s;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_count;
  __auto_type __mere_a3 = &mu_syms;
  __auto_type __mere_a4 = &mu_len;
  __auto_type __mere_a5 = &mu_code0;
  __auto_type __mere_a6 = &mu_first;
  __auto_type __mere_a7 = &mu_index;
  __mere_tail: ;
  return ((mu_len == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = ((mu_code0) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get_unchecked(mu_count, mu_len); __auto_type mu_c = __let_tmp_c; ((mu_code < (mu_first + mu_c)) ? mere_vec_int_get(mu_syms, (mu_index + (mu_code - mu_first))) : ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_count; __auto_type __mt3 = mu_syms; __auto_type __mt4 = (mu_len + 1LL); __auto_type __mt5 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __mt6 = ({ long long __sa = ((mu_first + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __mt7 = (mu_index + mu_c); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; goto __mere_tail; __mere_ret; })); }));
}
long long __lifted_go_uq4_10__direct(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_count, mere_vec_int* mu_syms, long long mu_len, long long mu_code0, long long mu_first, long long mu_index) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_s;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_count;
  __auto_type __mere_a3 = &mu_syms;
  __auto_type __mere_a4 = &mu_len;
  __auto_type __mere_a5 = &mu_code0;
  __auto_type __mere_a6 = &mu_first;
  __auto_type __mere_a7 = &mu_index;
  __mere_tail: ;
  return ((mu_len == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = ((mu_code0) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get(mu_count, mu_len); __auto_type mu_c = __let_tmp_c; ((mu_code < (mu_first + mu_c)) ? mere_vec_int_get(mu_syms, (mu_index + (mu_code - mu_first))) : ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_count; __auto_type __mt3 = mu_syms; __auto_type __mt4 = (mu_len + 1LL); __auto_type __mt5 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __mt6 = ({ long long __sa = ((mu_first + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __mt7 = (mu_index + mu_c); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; goto __mere_tail; __mere_ret; })); }));
}
int __lifted_chars__rvfast_36__direct(long long mu_len, mere_vec_int* mu_data, long long mu_d, mere_strbuf* mu_buf, long long mu_i, int mu_seen_sep) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_d;
  __auto_type __mere_a3 = &mu_buf;
  __auto_type __mere_a4 = &mu_i;
  __auto_type __mere_a5 = &mu_seen_sep;
  __mere_tail: ;
  return ((mu_i == mu_len) ? 0 : ({ __auto_type __let_tmp_b = mere_vec_int_get_unchecked(mu_data, (mu_d + mu_i)); __auto_type mu_b = __let_tmp_b; (((mu_b == 0LL) && (mu_seen_sep == 0)) ? ({ (void)(mere_strbuf_push(mu_buf, (__sl_30.s))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_d; __auto_type __mt3 = mu_buf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = 1; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }) : ({ (void)(mere_strbuf_push(mu_buf, __lang_char_at_chr(mu_b))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_d; __auto_type __mt3 = mu_buf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = mu_seen_sep; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); })); }));
}
int __lifted_chars_37__direct(long long mu_len, mere_vec_int* mu_data, long long mu_d, mere_strbuf* mu_buf, long long mu_i, int mu_seen_sep) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_len;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_d;
  __auto_type __mere_a3 = &mu_buf;
  __auto_type __mere_a4 = &mu_i;
  __auto_type __mere_a5 = &mu_seen_sep;
  __mere_tail: ;
  return ((mu_i == mu_len) ? 0 : ({ __auto_type __let_tmp_b = mere_vec_int_get(mu_data, (mu_d + mu_i)); __auto_type mu_b = __let_tmp_b; (((mu_b == 0LL) && (mu_seen_sep == 0)) ? ({ (void)(mere_strbuf_push(mu_buf, (__sl_30.s))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_d; __auto_type __mt3 = mu_buf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = 1; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); }) : ({ (void)(mere_strbuf_push(mu_buf, __lang_char_at_chr(mu_b))); ({ __auto_type __mt0 = mu_len; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_d; __auto_type __mt3 = mu_buf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = mu_seen_sep; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); })); }));
}
mere_bytebuf* __lifted_pass_43__direct(mere_bytebuf* mu_out, long long mu_width, long long mu_height, mere_vec_int* mu_raw, long long mu_bpp, long long mu_p_uq28, long long mu_off) {
  mere_bytebuf* __mere_ret;
  __auto_type __mere_a0 = &mu_out;
  __auto_type __mere_a1 = &mu_width;
  __auto_type __mere_a2 = &mu_height;
  __auto_type __mere_a3 = &mu_raw;
  __auto_type __mere_a4 = &mu_bpp;
  __auto_type __mere_a5 = &mu_p_uq28;
  __auto_type __mere_a6 = &mu_off;
  __mere_tail: ;
  return ((mu_p_uq28 == 7LL) ? mu_out : ({ __auto_type __let_tmp_pw = ({ __auto_type __da0 = mu_p_uq28; __auto_type __da1 = mu_width; mu_pass_width__direct(__da0, __da1); }); __auto_type mu_pw = __let_tmp_pw; __auto_type __let_tmp_ph = ({ __auto_type __da0 = mu_p_uq28; __auto_type __da1 = mu_height; mu_pass_height__direct(__da0, __da1); }); __auto_type mu_ph = __let_tmp_ph; (((mu_pw <= 0LL) || (mu_ph <= 0LL)) ? ({ __auto_type __mt0 = mu_out; __auto_type __mt1 = mu_width; __auto_type __mt2 = mu_height; __auto_type __mt3 = mu_raw; __auto_type __mt4 = mu_bpp; __auto_type __mt5 = (mu_p_uq28 + 1LL); __auto_type __mt6 = mu_off; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }) : ({ __auto_type __let_tmp_sub = ({ __auto_type __da0 = mu_raw; __auto_type __da1 = mu_off; __auto_type __da2 = mu_pw; __auto_type __da3 = mu_ph; __auto_type __da4 = mu_bpp; mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3, __da4); }); __auto_type mu_sub = __let_tmp_sub; ({ (void)(__lifted_place_44(mu_ph, mu_pw, mu_p_uq28, mu_bpp, mu_width, mu_out, mu_sub, 0LL)); ({ __auto_type __mt0 = mu_out; __auto_type __mt1 = mu_width; __auto_type __mt2 = mu_height; __auto_type __mt3 = mu_raw; __auto_type __mt4 = mu_bpp; __auto_type __mt5 = (mu_p_uq28 + 1LL); __auto_type __mt6 = (mu_off + (mu_ph * ((mu_pw * mu_bpp) + 1LL))); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }); }); })); }));
}
long long __lifted_go_uq31_49__direct(long long mu_i, long long mu_c) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_i;
  __auto_type __mere_a1 = &mu_c;
  __mere_tail: ;
  return ((mu_i == 8LL) ? mu_c : ({ __auto_type __mt0 = (mu_i + 1LL); __auto_type __mt1 = ((((mu_c) & (1LL)) == 1LL) ? ((({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) ^ (3988292384LL)) : ({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go__rvfast_uq32_50__direct(long long mu_n, mere_vec_int* mu_bytes, long long mu_i, long long mu_crc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_bytes;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_crc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_crc : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_bytes; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get_unchecked(mu_bytes, mu_i); mu_crc_byte__v2__direct(__da0, __da1); }); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go_uq33_51__direct(long long mu_n, mere_vec_int* mu_bytes, long long mu_i, long long mu_crc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_bytes;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_crc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_crc : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_bytes; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get(mu_bytes, mu_i); mu_crc_byte__v2__direct(__da0, __da1); }); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go_uq35_53__direct(long long mu_n, long long mu_v, long long mu_i, long long mu_acc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_acc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_acc : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_v; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ((({ long long __sa = (mu_acc); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (((({ long long __sa = (mu_v); long long __sn = (mu_i); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (1LL)))); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
tuple_int_int __lifted_best_58__direct(long long mu_max_back, long long mu_pos, long long mu_n, mere_vec_int* mu_data, long long mu_d, long long mu_blen, long long mu_bdist) {
  tuple_int_int __mere_ret;
  __auto_type __mere_a0 = &mu_max_back;
  __auto_type __mere_a1 = &mu_pos;
  __auto_type __mere_a2 = &mu_n;
  __auto_type __mere_a3 = &mu_data;
  __auto_type __mere_a4 = &mu_d;
  __auto_type __mere_a5 = &mu_blen;
  __auto_type __mere_a6 = &mu_bdist;
  __mere_tail: ;
  return ((mu_d > mu_max_back) ? ((tuple_int_int){.f0 = mu_blen, .f1 = mu_bdist}) : ({ __auto_type __let_tmp_start = (mu_pos - mu_d); __auto_type mu_start = __let_tmp_start; ({ __auto_type __let_tmp_l = (((((0LL >= 0LL) && (0LL <= 258LL)) && ((((mu_pos + 0LL) >= 0LL) && (((mu_pos + 0LL) + 1LL) <= mere_vec_int_len(mu_data))) && (((mu_pos + (258LL - 1LL)) >= 0LL) && (((mu_pos + (258LL - 1LL)) + 1LL) <= mere_vec_int_len(mu_data))))) && ((((mu_start + 0LL) >= 0LL) && (((mu_start + 0LL) + 1LL) <= mere_vec_int_len(mu_data))) && (((mu_start + (258LL - 1LL)) >= 0LL) && (((mu_start + (258LL - 1LL)) + 1LL) <= mere_vec_int_len(mu_data))))) ? __lifted_mlen__rvfast_59(mu_pos, mu_n, mu_data, mu_start, 0LL) : __lifted_mlen_60(mu_pos, mu_n, mu_data, mu_start, 0LL)); __auto_type mu_l = __let_tmp_l; ((mu_l > mu_blen) ? ({ __auto_type __mt0 = mu_max_back; __auto_type __mt1 = mu_pos; __auto_type __mt2 = mu_n; __auto_type __mt3 = mu_data; __auto_type __mt4 = (mu_d + 1LL); __auto_type __mt5 = mu_l; __auto_type __mt6 = mu_d; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }) : ({ __auto_type __mt0 = mu_max_back; __auto_type __mt1 = mu_pos; __auto_type __mt2 = mu_n; __auto_type __mt3 = mu_data; __auto_type __mt4 = (mu_d + 1LL); __auto_type __mt5 = mu_blen; __auto_type __mt6 = mu_bdist; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; })); }); }));
}
long long __lifted_go__rvfast_uq40_68__direct(long long mu_np, mere_vec_int* mu_alive, long long mu_skip, mere_vec_int* mu_nf, long long mu_i, long long mu_best_uq41) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_np;
  __auto_type __mere_a1 = &mu_alive;
  __auto_type __mere_a2 = &mu_skip;
  __auto_type __mere_a3 = &mu_nf;
  __auto_type __mere_a4 = &mu_i;
  __auto_type __mere_a5 = &mu_best_uq41;
  __mere_tail: ;
  return ((mu_i == mu_np) ? mu_best_uq41 : ((((mere_vec_int_get_unchecked(mu_alive, mu_i) == 1LL) && (mu_i != mu_skip)) && ((mu_best_uq41 < 0LL) || (mere_vec_int_get_unchecked(mu_nf, mu_i) < mere_vec_int_get(mu_nf, mu_best_uq41)))) ? ({ __auto_type __mt0 = mu_np; __auto_type __mt1 = mu_alive; __auto_type __mt2 = mu_skip; __auto_type __mt3 = mu_nf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = mu_i; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }) : ({ __auto_type __mt0 = mu_np; __auto_type __mt1 = mu_alive; __auto_type __mt2 = mu_skip; __auto_type __mt3 = mu_nf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = mu_best_uq41; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; })));
}
long long __lifted_go_uq42_69__direct(long long mu_np, mere_vec_int* mu_alive, long long mu_skip, mere_vec_int* mu_nf, long long mu_i, long long mu_best_uq43) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_np;
  __auto_type __mere_a1 = &mu_alive;
  __auto_type __mere_a2 = &mu_skip;
  __auto_type __mere_a3 = &mu_nf;
  __auto_type __mere_a4 = &mu_i;
  __auto_type __mere_a5 = &mu_best_uq43;
  __mere_tail: ;
  return ((mu_i == mu_np) ? mu_best_uq43 : ((((mere_vec_int_get(mu_alive, mu_i) == 1LL) && (mu_i != mu_skip)) && ((mu_best_uq43 < 0LL) || (mere_vec_int_get(mu_nf, mu_i) < mere_vec_int_get(mu_nf, mu_best_uq43)))) ? ({ __auto_type __mt0 = mu_np; __auto_type __mt1 = mu_alive; __auto_type __mt2 = mu_skip; __auto_type __mt3 = mu_nf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = mu_i; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }) : ({ __auto_type __mt0 = mu_np; __auto_type __mt1 = mu_alive; __auto_type __mt2 = mu_skip; __auto_type __mt3 = mu_nf; __auto_type __mt4 = (mu_i + 1LL); __auto_type __mt5 = mu_best_uq43; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; })));
}
int __lifted_assign_70__direct(mere_vec_int* mu_leafsym, mere_vec_int* mu_lengths, mere_vec_int* mu_nl, mere_vec_int* mu_nr, long long mu_node, long long mu_depth) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_leafsym;
  __auto_type __mere_a1 = &mu_lengths;
  __auto_type __mere_a2 = &mu_nl;
  __auto_type __mere_a3 = &mu_nr;
  __auto_type __mere_a4 = &mu_node;
  __auto_type __mere_a5 = &mu_depth;
  __mere_tail: ;
  return ({ __auto_type __let_tmp_sym = mere_vec_int_get(mu_leafsym, mu_node); __auto_type mu_sym = __let_tmp_sym; ((mu_sym >= 0LL) ? mere_vec_int_set(mu_lengths, mu_sym, mu_depth) : ({ (void)(({ __auto_type __ia0 = mere_vec_int_get(mu_nl, mu_node); __auto_type __ia1 = (mu_depth + 1LL); __lifted_assign_70__direct(mu_leafsym, mu_lengths, mu_nl, mu_nr, __ia0, __ia1); })); ({ __auto_type __mt0 = mu_leafsym; __auto_type __mt1 = mu_lengths; __auto_type __mt2 = mu_nl; __auto_type __mt3 = mu_nr; __auto_type __mt4 = mere_vec_int_get(mu_nr, mu_node); __auto_type __mt5 = (mu_depth + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; goto __mere_tail; __mere_ret; }); })); });
}
int __lifted_fill__rvfast_75__direct(long long mu_maxbits, mere_vec_int* mu_bl_count, mere_vec_int* mu_next_code, long long mu_bits, long long mu_code) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_maxbits;
  __auto_type __mere_a1 = &mu_bl_count;
  __auto_type __mere_a2 = &mu_next_code;
  __auto_type __mere_a3 = &mu_bits;
  __auto_type __mere_a4 = &mu_code;
  __mere_tail: ;
  return ((mu_bits > mu_maxbits) ? 0 : ({ __auto_type __let_tmp_c = ({ long long __sa = ((mu_code + mere_vec_int_get_unchecked(mu_bl_count, (mu_bits - 1LL)))); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type mu_c = __let_tmp_c; (void)(mere_vec_int_set_unchecked(mu_next_code, mu_bits, mu_c)); ({ __auto_type __mt0 = mu_maxbits; __auto_type __mt1 = mu_bl_count; __auto_type __mt2 = mu_next_code; __auto_type __mt3 = (mu_bits + 1LL); __auto_type __mt4 = mu_c; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}
int __lifted_fill_uq44_76__direct(long long mu_maxbits, mere_vec_int* mu_bl_count, mere_vec_int* mu_next_code, long long mu_bits, long long mu_code) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_maxbits;
  __auto_type __mere_a1 = &mu_bl_count;
  __auto_type __mere_a2 = &mu_next_code;
  __auto_type __mere_a3 = &mu_bits;
  __auto_type __mere_a4 = &mu_code;
  __mere_tail: ;
  return ((mu_bits > mu_maxbits) ? 0 : ({ __auto_type __let_tmp_c = ({ long long __sa = ((mu_code + mere_vec_int_get(mu_bl_count, (mu_bits - 1LL)))); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type mu_c = __let_tmp_c; (void)(mere_vec_int_set(mu_next_code, mu_bits, mu_c)); ({ __auto_type __mt0 = mu_maxbits; __auto_type __mt1 = mu_bl_count; __auto_type __mt2 = mu_next_code; __auto_type __mt3 = (mu_bits + 1LL); __auto_type __mt4 = mu_c; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}
long long __lifted_go__rvfast_uq45_80__direct(long long mu_n, mere_vec_int* mu_v, long long mu_i, long long mu_m) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_m;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_m : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_v; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ((mere_vec_int_get_unchecked(mu_v, mu_i) > mu_m) ? mere_vec_int_get_unchecked(mu_v, mu_i) : mu_m); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
long long __lifted_go_uq46_81__direct(long long mu_n, mere_vec_int* mu_v, long long mu_i, long long mu_m) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_v;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_m;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_m : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_v; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = ((mere_vec_int_get(mu_v, mu_i) > mu_m) ? mere_vec_int_get(mu_v, mu_i) : mu_m); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }));
}
long long __lifted_last_nz_87__direct(mere_vec_int* mu_v, long long mu_i, long long mu_floor) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_v;
  __auto_type __mere_a1 = &mu_i;
  __auto_type __mere_a2 = &mu_floor;
  __mere_tail: ;
  return ((mu_i < mu_floor) ? mu_floor : ((mere_vec_int_get(mu_v, mu_i) > 0LL) ? mu_i : ({ __auto_type __mt0 = mu_v; __auto_type __mt1 = (mu_i - 1LL); __auto_type __mt2 = mu_floor; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; })));
}
long long __lifted_go__rvfast_uq56_110__direct(long long mu_n, mere_vec_int* mu_data, long long mu_i, long long mu_a, long long mu_b) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_a;
  __auto_type __mere_a4 = &mu_b;
  __mere_tail: ;
  return ((mu_i == mu_n) ? ((mu_b * 65536LL) + mu_a) : ({ __auto_type __let_tmp_a2 = __lang_imod((mu_a + mere_vec_int_get_unchecked(mu_data, mu_i)), 65521LL); __auto_type mu_a2 = __let_tmp_a2; ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = mu_a2; __auto_type __mt4 = __lang_imod((mu_b + mu_a2), 65521LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}
long long __lifted_go_uq57_111__direct(long long mu_n, mere_vec_int* mu_data, long long mu_i, long long mu_a, long long mu_b) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_a;
  __auto_type __mere_a4 = &mu_b;
  __mere_tail: ;
  return ((mu_i == mu_n) ? ((mu_b * 65536LL) + mu_a) : ({ __auto_type __let_tmp_a2 = __lang_imod((mu_a + mere_vec_int_get(mu_data, mu_i)), 65521LL); __auto_type mu_a2 = __let_tmp_a2; ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = mu_a2; __auto_type __mt4 = __lang_imod((mu_b + mu_a2), 65521LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; goto __mere_tail; __mere_ret; }); }));
}
long long __lifted_go__rvfast_uq61_113__direct(long long mu_n, mere_vec_int* mu_row_uq60, long long mu_i, long long mu_acc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_row_uq60;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_acc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_acc : ({ __auto_type __let_tmp_b = mere_vec_int_get_unchecked(mu_row_uq60, mu_i); __auto_type mu_b = __let_tmp_b; ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_row_uq60; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = (mu_acc + ((mu_b < 128LL) ? mu_b : (256LL - mu_b))); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}
long long __lifted_go_uq62_114__direct(long long mu_n, mere_vec_int* mu_row_uq60, long long mu_i, long long mu_acc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_row_uq60;
  __auto_type __mere_a2 = &mu_i;
  __auto_type __mere_a3 = &mu_acc;
  __mere_tail: ;
  return ((mu_i == mu_n) ? mu_acc : ({ __auto_type __let_tmp_b = mere_vec_int_get(mu_row_uq60, mu_i); __auto_type mu_b = __let_tmp_b; ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_row_uq60; __auto_type __mt2 = (mu_i + 1LL); __auto_type __mt3 = (mu_acc + ((mu_b < 128LL) ? mu_b : (256LL - mu_b))); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }); }));
}
mere_vec_int* __lifted_go_uq64_115__direct(mere_vec_int* mu_row_uq63, mere_vec_int* mu_prev, long long mu_bpp, long long mu_kind, long long mu_bk, long long mu_bc, mere_vec_int* mu_best_uq65) {
  mere_vec_int* __mere_ret;
  __auto_type __mere_a0 = &mu_row_uq63;
  __auto_type __mere_a1 = &mu_prev;
  __auto_type __mere_a2 = &mu_bpp;
  __auto_type __mere_a3 = &mu_kind;
  __auto_type __mere_a4 = &mu_bk;
  __auto_type __mere_a5 = &mu_bc;
  __auto_type __mere_a6 = &mu_best_uq65;
  __mere_tail: ;
  return ((mu_kind == 5LL) ? ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(mere_vec_int_push(mu_out, mu_bk)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = mu_best_uq65; mu_push_all__direct(__da0, __da1); })); mu_out; }) : ({ __auto_type __let_tmp_cand = ({ __auto_type __da0 = mu_kind; __auto_type __da1 = mu_row_uq63; __auto_type __da2 = mu_prev; __auto_type __da3 = mu_bpp; mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_cand = __let_tmp_cand; __auto_type __let_tmp_c = ({ __auto_type __da0 = mu_cand; mu_row_cost__direct(__da0); }); __auto_type mu_c = __let_tmp_c; ((mu_c < mu_bc) ? ({ __auto_type __mt0 = mu_row_uq63; __auto_type __mt1 = mu_prev; __auto_type __mt2 = mu_bpp; __auto_type __mt3 = (mu_kind + 1LL); __auto_type __mt4 = mu_kind; __auto_type __mt5 = mu_c; __auto_type __mt6 = mu_cand; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }) : ({ __auto_type __mt0 = mu_row_uq63; __auto_type __mt1 = mu_prev; __auto_type __mt2 = mu_bpp; __auto_type __mt3 = (mu_kind + 1LL); __auto_type __mt4 = mu_bk; __auto_type __mt5 = mu_bc; __auto_type __mt6 = mu_best_uq65; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; })); }));
}
int __lifted_rows_122__direct(long long mu_height, long long mu_stride, int mu_grey, mere_vec_int* mu_pixels, long long mu_width, int mu_indexed, mere_vec_int* mu_pal, mere_vec_int* mu_raw, long long mu_bpp, long long mu_y, mere_vec_int* mu_prev) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_height;
  __auto_type __mere_a1 = &mu_stride;
  __auto_type __mere_a2 = &mu_grey;
  __auto_type __mere_a3 = &mu_pixels;
  __auto_type __mere_a4 = &mu_width;
  __auto_type __mere_a5 = &mu_indexed;
  __auto_type __mere_a6 = &mu_pal;
  __auto_type __mere_a7 = &mu_raw;
  __auto_type __mere_a8 = &mu_bpp;
  __auto_type __mere_a9 = &mu_y;
  __auto_type __mere_a10 = &mu_prev;
  __mere_tail: ;
  return ((mu_y == mu_height) ? 0 : ({ __auto_type __let_tmp_row = mere_vec_int_new(__lang_current_region); __auto_type mu_row = __let_tmp_row; ({ (void)(__lifted_col_uq71_123(mu_stride, mu_grey, mu_row, mu_pixels, mu_y, mu_width, mu_indexed, mu_pal, 0LL)); (void)(({ __auto_type __da0 = mu_raw; __auto_type __da1 = ({ __auto_type __da0 = mu_row; __auto_type __da1 = mu_prev; __auto_type __da2 = mu_bpp; mu_best_filter__Vec_int__Vec_int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); mu_push_all__direct(__da0, __da1); })); ({ __auto_type __mt0 = mu_height; __auto_type __mt1 = mu_stride; __auto_type __mt2 = mu_grey; __auto_type __mt3 = mu_pixels; __auto_type __mt4 = mu_width; __auto_type __mt5 = mu_indexed; __auto_type __mt6 = mu_pal; __auto_type __mt7 = mu_raw; __auto_type __mt8 = mu_bpp; __auto_type __mt9 = (mu_y + 1LL); __auto_type __mt10 = mu_row; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; *__mere_a7 = __mt7; *__mere_a8 = __mt8; *__mere_a9 = __mt9; *__mere_a10 = __mt10; goto __mere_tail; __mere_ret; }); }); }));
}
int __lifted_header_127__direct(mere_vec_int* mu_out, const char* mu_s, long long mu_i) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_out;
  __auto_type __mere_a1 = &mu_s;
  __auto_type __mere_a2 = &mu_i;
  __mere_tail: ;
  return ((mu_i >= ((long long) __lang_str_size(mu_s))) ? 0 : ({ (void)(mere_vec_int_push(mu_out, __lang_ord(__lang_char_at(mu_s, mu_i)))); ({ __auto_type __mt0 = mu_out; __auto_type __mt1 = mu_s; __auto_type __mt2 = (mu_i + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }));
}
int __lifted_fields_129__direct(long long mu_n, mere_vec_int* mu_data, mere_vec_int* mu_acc, long long mu_i, long long mu_got, long long mu_cur, long long mu_seen_uq73) {
  int __mere_ret;
  __auto_type __mere_a0 = &mu_n;
  __auto_type __mere_a1 = &mu_data;
  __auto_type __mere_a2 = &mu_acc;
  __auto_type __mere_a3 = &mu_i;
  __auto_type __mere_a4 = &mu_got;
  __auto_type __mere_a5 = &mu_cur;
  __auto_type __mere_a6 = &mu_seen_uq73;
  __mere_tail: ;
  return ((mu_i >= mu_n) ? 0 : ({ __auto_type __let_tmp_c = mere_vec_int_get(mu_data, mu_i); __auto_type mu_c = __let_tmp_c; (((mu_c >= 48LL) && (mu_c <= 57LL)) ? ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_acc; __auto_type __mt3 = (mu_i + 1LL); __auto_type __mt4 = mu_got; __auto_type __mt5 = (((mu_cur * 10LL) + mu_c) - 48LL); __auto_type __mt6 = 1LL; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }) : ((mu_seen_uq73 == 1LL) ? ({ (void)(mere_vec_int_push(mu_acc, mu_cur)); ((mu_got == 2LL) ? ({ (void)(mere_vec_int_push(mu_acc, (mu_i + 1LL))); 0; }) : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_acc; __auto_type __mt3 = (mu_i + 1LL); __auto_type __mt4 = (mu_got + 1LL); __auto_type __mt5 = 0LL; __auto_type __mt6 = 0LL; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; })); }) : ({ __auto_type __mt0 = mu_n; __auto_type __mt1 = mu_data; __auto_type __mt2 = mu_acc; __auto_type __mt3 = (mu_i + 1LL); __auto_type __mt4 = mu_got; __auto_type __mt5 = 0LL; __auto_type __mt6 = 0LL; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; *__mere_a4 = __mt4; *__mere_a5 = __mt5; *__mere_a6 = __mt6; goto __mere_tail; __mere_ret; }))); }));
}
const char* __lifted_join__rvfast_131__direct(mere_vec_str* mu_t, long long mu_i, const char* mu_acc) {
  const char* __mere_ret;
  __auto_type __mere_a0 = &mu_t;
  __auto_type __mere_a1 = &mu_i;
  __auto_type __mere_a2 = &mu_acc;
  __mere_tail: ;
  return ((mu_i >= mere_vec_str_len(mu_t)) ? mu_acc : ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = (mu_i + 1LL); __auto_type __mt2 = __lang_str_concat(__lang_str_concat(mu_acc, (__sl_31.s)), mere_vec_str_get_unchecked(mu_t, mu_i)); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }));
}
const char* __lifted_join_132__direct(mere_vec_str* mu_t, long long mu_i, const char* mu_acc) {
  const char* __mere_ret;
  __auto_type __mere_a0 = &mu_t;
  __auto_type __mere_a1 = &mu_i;
  __auto_type __mere_a2 = &mu_acc;
  __mere_tail: ;
  return ((mu_i >= mere_vec_str_len(mu_t)) ? mu_acc : ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = (mu_i + 1LL); __auto_type __mt2 = __lang_str_concat(__lang_str_concat(mu_acc, (__sl_31.s)), mere_vec_str_get(mu_t, mu_i)); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }));
}
static closure_int_closure_closure_int_closure_int_int_int mu_list_fold(list_int mu_xs) {
  return ({ __anon_2_env* __env = (__anon_2_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_2_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_2_env; __env->mu_xs = mu_xs; (closure_int_closure_closure_int_closure_int_int_int){.env = __env, .fn = __anon_2_fn, .fn2 = __anon_2_fn2}; });
}

static closure_int_closure_list_int_list_int mu__range_down(long long mu_a) {
  return ({ __anon_3_env* __env = (__anon_3_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_3_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_3_env; __env->mu_a = mu_a; (closure_int_closure_list_int_list_int){.env = __env, .fn = __anon_3_fn, .fn2 = __anon_3_fn2}; });
}

static closure_int_list_int mu_range(long long mu_a) {
  return ({ __anon_4_env* __env = (__anon_4_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_4_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_4_env; __env->mu_a = mu_a; (closure_int_list_int){.env = __env, .fn = __anon_4_fn}; });
}

static long long mu_list_sum(list_int mu_xs) {
  return ({ __auto_type __da0 = mu_xs; __auto_type __da1 = 0LL; __auto_type __da2 = ((closure_int_closure_int_int){.env = NULL, .fn = __anon_5_fn, .fn2 = __anon_5_fn2}); mu_list_fold__direct(__da0, __da1, __da2); });
}

static long long mu_list_product(list_int mu_xs) {
  return ({ __auto_type __da0 = mu_xs; __auto_type __da1 = 1LL; __auto_type __da2 = ((closure_int_closure_int_int){.env = NULL, .fn = __anon_6_fn, .fn2 = __anon_6_fn2}); mu_list_fold__direct(__da0, __da1, __da2); });
}

static closure_int_str mu__u8_nth(list_str mu_cs) {
  return ({ __anon_7_env* __env = (__anon_7_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_7_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_7_env; __env->mu_cs = mu_cs; (closure_int_str){.env = __env, .fn = __anon_7_fn}; });
}

static closure_int_str mu_utf8_at(const char* mu_s) {
  return ({ __anon_8_env* __env = (__anon_8_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_8_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_8_env; __env->mu_s = mu_s; (closure_int_str){.env = __env, .fn = __anon_8_fn}; });
}

static int mu__cp_bad(long long mu_n) {
  return (((mu_n < 0LL) || (mu_n > 1114111LL)) || ((mu_n >= 55296LL) && (mu_n <= 57343LL)));
}

static const char* mu_str_of_codepoint(long long mu_n) {
  return (({ __auto_type __da0 = mu_n; mu__cp_bad__direct(__da0); }) ? __lang_fail_str(__lang_str_concat((__sl_2.s), (__sl_1.s))) : ((mu_n < 128LL) ? __lang_char_at_chr(mu_n) : ((mu_n < 2048LL) ? ({ __auto_type __bc149 = __lang_char_at_chr(((192LL) | (({ long long __sa = (mu_n); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })))); __auto_type __bc150 = __lang_char_at_chr(((128LL) | (((mu_n) & (63LL))))); __auto_type __bc151 = __lang_str_concat(__bc149, __bc150); __bc151; }) : ((mu_n < 65536LL) ? ({ __auto_type __bc144 = __lang_char_at_chr(((224LL) | (({ long long __sa = (mu_n); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })))); __auto_type __bc145 = __lang_char_at_chr(((128LL) | (((({ long long __sa = (mu_n); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (63LL))))); __auto_type __bc146 = __lang_str_concat(__bc144, __bc145); __auto_type __bc147 = __lang_char_at_chr(((128LL) | (((mu_n) & (63LL))))); __auto_type __bc148 = __lang_str_concat(__bc146, __bc147); __bc148; }) : ({ __auto_type __bc137 = __lang_char_at_chr(((240LL) | (({ long long __sa = (mu_n); long long __sn = (18LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })))); __auto_type __bc138 = __lang_char_at_chr(((128LL) | (((({ long long __sa = (mu_n); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (63LL))))); __auto_type __bc139 = __lang_str_concat(__bc137, __bc138); __auto_type __bc140 = __lang_char_at_chr(((128LL) | (((({ long long __sa = (mu_n); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (63LL))))); __auto_type __bc141 = __lang_str_concat(__bc139, __bc140); __auto_type __bc142 = __lang_char_at_chr(((128LL) | (((mu_n) & (63LL))))); __auto_type __bc143 = __lang_str_concat(__bc141, __bc142); __bc143; })))));
}

static long long mu__cp_span(long long mu_b) {
  return ((mu_b < 128LL) ? 1LL : (((mu_b >= 192LL) && (mu_b <= 223LL)) ? 2LL : (((mu_b >= 224LL) && (mu_b <= 239LL)) ? 3LL : (((mu_b >= 240LL) && (mu_b <= 247LL)) ? 4LL : 1LL))));
}

static closure_int_int mu__cp_cont(const char* mu_s) {
  return ({ __anon_9_env* __env = (__anon_9_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_9_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_9_env; __env->mu_s = mu_s; (closure_int_int){.env = __env, .fn = __anon_9_fn}; });
}

static long long mu_codepoint_of(const char* mu_s) {
  return ({ __auto_type __let_tmp_n = ((long long) __lang_str_size(mu_s)); __auto_type mu_n = __let_tmp_n; ((mu_n == 0LL) ? __lang_fail_int(__lang_str_concat((__sl_2.s), (__sl_3.s))) : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at(mu_s, 0LL)); __auto_type mu_b0 = __let_tmp_b0; __auto_type __let_tmp_span = ({ __auto_type __da0 = mu_b0; mu__cp_span__direct(__da0); }); __auto_type mu_span = __let_tmp_span; ((mu_span != mu_n) ? __lang_fail_int(__lang_str_concat((__sl_2.s), (__sl_3.s))) : ({ __auto_type __let_tmp_cp = ((mu_span == 1LL) ? mu_b0 : ((mu_span == 2LL) ? ((({ long long __sa = (((mu_b0) & (31LL))); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); }))) : ((mu_span == 3LL) ? ((((({ long long __sa = (((mu_b0) & (15LL))); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); }))) : ((((({ long long __sa = (((mu_b0) & (7LL))); long long __sn = (18LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (((({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 3LL; mu__cp_cont__direct(__da0, __da1); })))))))); __auto_type mu_cp = __let_tmp_cp; __auto_type __let_tmp_shortest = ((mu_span == 1LL) ? (mu_cp < 128LL) : ((mu_span == 2LL) ? (mu_cp >= 128LL) : ((mu_span == 3LL) ? (mu_cp >= 2048LL) : (mu_cp >= 65536LL)))); __auto_type mu_shortest = __let_tmp_shortest; ((({ __auto_type __da0 = mu_cp; mu__cp_bad__direct(__da0); }) || (!(mu_shortest))) ? __lang_fail_int(__lang_str_concat((__sl_2.s), (__sl_3.s))) : mu_cp); })); })); });
}

static closure_int_int mu_codepoint_or(const char* mu_s) {
  return ({ __anon_10_env* __env = (__anon_10_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_10_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_10_env; __env->mu_s = mu_s; (closure_int_int){.env = __env, .fn = __anon_10_fn}; });
}

static closure_int_int mu_codepoint_at(const char* mu_s) {
  return ({ __anon_11_env* __env = (__anon_11_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_11_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_11_env; __env->mu_s = mu_s; (closure_int_int){.env = __env, .fn = __anon_11_fn}; });
}

static closure_int_closure_int_closure_str_str mu__u8_slice(list_str mu_cs) {
  return ({ __anon_12_env* __env = (__anon_12_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_12_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_12_env; __env->mu_cs = mu_cs; (closure_int_closure_int_closure_str_str){.env = __env, .fn = __anon_12_fn, .fn2 = __anon_12_fn2}; });
}

static closure_int_closure_int_str mu_utf8_sub(const char* mu_s) {
  return ({ __anon_13_env* __env = (__anon_13_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_13_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_13_env; __env->mu_s = mu_s; (closure_int_closure_int_str){.env = __env, .fn = __anon_13_fn, .fn2 = __anon_13_fn2}; });
}

static closure_str_str mu__u8_rev_join(list_str mu_cs) {
  return ({ __anon_14_env* __env = (__anon_14_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_14_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_14_env; __env->mu_cs = mu_cs; (closure_str_str){.env = __env, .fn = __anon_14_fn}; });
}

static const char* mu_utf8_rev(const char* mu_s) {
  return ({ __auto_type __da0 = __lang_utf8_chars(mu_s); __auto_type __da1 = (__sl_0.s); mu__u8_rev_join__direct(__da0, __da1); });
}

static long long mu__eaw_width(long long mu_cp) {
  return (((mu_cp >= 768LL) && (mu_cp <= 879LL)) ? 0LL : ((mu_cp == 12351LL) ? 1LL : (({ int __bc152 = ((mu_cp >= 4352LL)); int __bc153 = __bc152 ? (((mu_cp <= 4447LL)) ? 1 : 0) : 0; int __bc154 = __bc153 ? 1 : ((((mu_cp >= 11904LL) && (mu_cp <= 42191LL))) ? 1 : 0); int __bc155 = __bc154 ? 1 : ((((mu_cp >= 43360LL) && (mu_cp <= 43391LL))) ? 1 : 0); int __bc156 = __bc155 ? 1 : ((((mu_cp >= 44032LL) && (mu_cp <= 55203LL))) ? 1 : 0); int __bc157 = __bc156 ? 1 : ((((mu_cp >= 63744LL) && (mu_cp <= 64255LL))) ? 1 : 0); int __bc158 = __bc157 ? 1 : ((((mu_cp >= 65040LL) && (mu_cp <= 65049LL))) ? 1 : 0); int __bc159 = __bc158 ? 1 : ((((mu_cp >= 65072LL) && (mu_cp <= 65135LL))) ? 1 : 0); int __bc160 = __bc159 ? 1 : ((((mu_cp >= 65280LL) && (mu_cp <= 65376LL))) ? 1 : 0); int __bc161 = __bc160 ? 1 : ((((mu_cp >= 65504LL) && (mu_cp <= 65510LL))) ? 1 : 0); int __bc162 = __bc161 ? 1 : ((((mu_cp >= 127744LL) && (mu_cp <= 128767LL))) ? 1 : 0); int __bc163 = __bc162 ? 1 : ((((mu_cp >= 129280LL) && (mu_cp <= 129535LL))) ? 1 : 0); int __bc164 = __bc163 ? 1 : ((((mu_cp >= 131072LL) && (mu_cp <= 262141LL))) ? 1 : 0); __bc164; }) ? 2LL : 1LL)));
}

static closure_int_closure_int_closure_int_int mu__u8w_go(const char* mu_s) {
  return ({ __anon_15_env* __env = (__anon_15_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_15_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_15_env; __env->mu_s = mu_s; (closure_int_closure_int_closure_int_int){.env = __env, .fn = __anon_15_fn, .fn2 = __anon_15_fn2}; });
}

static long long mu_utf8_width(const char* mu_s) {
  return ({ __auto_type __da0 = mu_s; __auto_type __da1 = 0LL; __auto_type __da2 = ((long long) __lang_str_size(mu_s)); __auto_type __da3 = 0LL; mu__u8w_go__direct(__da0, __da1, __da2, __da3); });
}

static closure_int_str mu_pad_right(const char* mu_s) {
  return ({ __anon_16_env* __env = (__anon_16_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_16_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_16_env; __env->mu_s = mu_s; (closure_int_str){.env = __env, .fn = __anon_16_fn}; });
}

static closure_int_str mu_pad_left(const char* mu_s) {
  return ({ __anon_17_env* __env = (__anon_17_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_17_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_17_env; __env->mu_s = mu_s; (closure_int_str){.env = __env, .fn = __anon_17_fn}; });
}

static long long mu_sign(long long mu_n) {
  return ((mu_n > 0LL) ? 1LL : ((mu_n < 0LL) ? (0LL - 1LL) : 0LL));
}

static long long mu_incr(long long mu_n) {
  return (mu_n + 1LL);
}

static long long mu_decr(long long mu_n) {
  return (mu_n - 1LL);
}

static long long mu_square(long long mu_n) {
  return (mu_n * mu_n);
}

static long long mu_cube(long long mu_n) {
  return ((mu_n * mu_n) * mu_n);
}

static closure_int_int mu_sum_range(long long mu_a) {
  return ({ __anon_18_env* __env = (__anon_18_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_18_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_18_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_18_fn}; });
}

static closure_int_closure_int_int mu__pow_go(long long mu_acc) {
  return ({ __anon_19_env* __env = (__anon_19_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_19_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_19_env; __env->mu_acc = mu_acc; (closure_int_closure_int_int){.env = __env, .fn = __anon_19_fn, .fn2 = __anon_19_fn2}; });
}

static closure_int_int mu_pow(long long mu_b) {
  return ({ __anon_20_env* __env = (__anon_20_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_20_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_20_env; __env->mu_b = mu_b; (closure_int_int){.env = __env, .fn = __anon_20_fn}; });
}

static closure_int_int mu_lcm(long long mu_a) {
  return ({ __anon_21_env* __env = (__anon_21_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_21_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_21_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_21_fn}; });
}

static closure_int_tuple_int_int mu_divmod(long long mu_a) {
  return ({ __anon_22_env* __env = (__anon_22_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_22_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_22_env; __env->mu_a = mu_a; (closure_int_tuple_int_int){.env = __env, .fn = __anon_22_fn}; });
}

static closure_str_unit mu_assert(int mu_cond) {
  return ({ __anon_23_env* __env = (__anon_23_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_23_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_23_env; __env->mu_cond = mu_cond; (closure_str_unit){.env = __env, .fn = __anon_23_fn}; });
}

static closure_int_int mu_crc_byte(long long mu_crc) {
  return ({ __anon_24_env* __env = (__anon_24_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_24_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_24_env; __env->mu_crc = mu_crc; (closure_int_int){.env = __env, .fn = __anon_24_fn}; });
}

static long long mu_crc32_vec(mere_vec_int* mu_bytes) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_bytes); __auto_type mu_n = __let_tmp_n; ((((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_bytes))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32; __lifted_go__rvfast_1__direct(mu_n, mu_bytes, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32; __lifted_go_uq1_2__direct(mu_n, mu_bytes, __ia0, __ia1); }))) ^ (mu_mask32)); });
}

static mere_vec_int* mu_br_new__int__Vec_int(long long mu_pos) {
  return ({ __auto_type __let_tmp_s = mere_vec_int_new(__lang_current_region); __auto_type mu_s = __let_tmp_s; (void)(mere_vec_int_push(mu_s, mu_pos)); (void)(mere_vec_int_push(mu_s, 0LL)); (void)(mere_vec_int_push(mu_s, 0LL)); mu_s; });
}

static closure_Vec_int_int mu_br_bit(mere_vec_int* mu_s) {
  return ({ __anon_25_env* __env = (__anon_25_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_25_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_25_env; __env->mu_s = mu_s; (closure_Vec_int_int){.env = __env, .fn = __anon_25_fn}; });
}

static closure_Vec_int_closure_int_int mu_br_bits(mere_vec_int* mu_s) {
  return ({ __anon_26_env* __env = (__anon_26_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_26_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_26_env; __env->mu_s = mu_s; (closure_Vec_int_closure_int_int){.env = __env, .fn = __anon_26_fn, .fn2 = __anon_26_fn2}; });
}

static int mu_br_align(mere_vec_int* mu_s) {
  return ({ (void)(mere_vec_int_set(mu_s, 1LL, 0LL)); mere_vec_int_set(mu_s, 2LL, 0LL); });
}

static tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int(mere_vec_int* mu_lengths) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_lengths); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_count = mere_vec_int_new(__lang_current_region); __auto_type mu_count = __let_tmp_count; ({ (void)(__lifted_zc_4(mu_count, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_lengths))) ? __lifted_tally__rvfast_5(mu_n, mu_lengths, mu_count, 0LL) : __lifted_tally_6(mu_n, mu_lengths, mu_count, 0LL))); (void)(mere_vec_int_set(mu_count, 0LL, 0LL)); __auto_type __let_tmp_syms = mere_vec_int_new(__lang_current_region); __auto_type mu_syms = __let_tmp_syms; ({ (void)(__lifted_bylen_7(mu_n, mu_lengths, mu_syms, 1LL)); ((tuple_Vec_int_Vec_int){.f0 = mu_count, .f1 = mu_syms}); }); }); }); });
}

static closure_Vec_int_closure_Vec_int_closure_Vec_int_int mu_huff_decode(mere_vec_int* mu_s) {
  return ({ __anon_27_env* __env = (__anon_27_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_27_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_27_env; __env->mu_s = mu_s; (closure_Vec_int_closure_Vec_int_closure_Vec_int_int){.env = __env, .fn = __anon_27_fn, .fn2 = __anon_27_fn2}; });
}

static mere_vec_int* mu_vec_of(list_int mu_xs) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq5_11(mu_v, mu_xs); });
}

static mere_vec_int* mu_fixed_lit_lengths(int mu__u) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq6_12(mu_v, 0LL); });
}

static mere_vec_int* mu_fixed_dist_lengths(int mu__u) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq7_13(mu_v, 0LL); });
}

static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit mu_inflate_codes(mere_vec_int* mu_s) {
  return ({ __anon_28_env* __env = (__anon_28_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_28_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_28_env; __env->mu_s = mu_s; (closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_28_fn, .fn2 = __anon_28_fn2}; });
}

static closure_Vec_int_tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int(mere_vec_int* mu_s) {
  return ({ __anon_29_env* __env = (__anon_29_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_29_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_29_env; __env->mu_s = mu_s; (closure_Vec_int_tuple_Vec_int_Vec_int){.env = __env, .fn = __anon_29_fn}; });
}

static closure_int_Vec_int mu_inflate(mere_vec_int* mu_data) {
  return ({ __anon_30_env* __env = (__anon_30_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_30_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_30_env; __env->mu_data = mu_data; (closure_int_Vec_int){.env = __env, .fn = __anon_30_fn}; });
}

static long long mu_gzip_body_start(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_flg = mere_vec_int_get(mu_data, 3LL); __auto_type mu_flg = __let_tmp_flg; __auto_type __let_tmp_off10 = 10LL; __auto_type mu_off10 = __let_tmp_off10; __auto_type __let_tmp_off_extra = ((((mu_flg) & (4LL)) != 0LL) ? ({ __auto_type __let_tmp_xlen = ({ __auto_type __bc165 = mere_vec_int_get(mu_data, mu_off10); __auto_type __bc166 = ({ long long __sa = (mere_vec_int_get(mu_data, (mu_off10 + 1LL))); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc167 = (__bc165 + __bc166); __bc167; }); __auto_type mu_xlen = __let_tmp_xlen; ((mu_off10 + 2LL) + mu_xlen); }) : mu_off10); __auto_type mu_off_extra = __let_tmp_off_extra; ({ __auto_type __let_tmp_off_name = ((((mu_flg) & (8LL)) != 0LL) ? __lifted_skip_z_26(mu_data, mu_off_extra) : mu_off_extra); __auto_type mu_off_name = __let_tmp_off_name; __auto_type __let_tmp_off_comm = ((((mu_flg) & (16LL)) != 0LL) ? __lifted_skip_z_26(mu_data, mu_off_name) : mu_off_name); __auto_type mu_off_comm = __let_tmp_off_comm; ((((mu_flg) & (2LL)) != 0LL) ? (mu_off_comm + 2LL) : mu_off_comm); }); });
}

static int mu_is_gzip(mere_vec_int* mu_data) {
  return ((((mere_vec_int_len(mu_data) >= 18LL) && (mere_vec_int_get(mu_data, 0LL) == 31LL)) && (mere_vec_int_get(mu_data, 1LL) == 139LL)) && (mere_vec_int_get(mu_data, 2LL) == 8LL));
}

static mere_vec_int* mu_gunzip(mere_vec_int* mu_data) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = ({ __auto_type __da0 = mu_data; mu_gzip_body_start__direct(__da0); }); mu_inflate__direct(__da0, __da1); });
}

static closure_int_int mu_gzip_trailer_u32(mere_vec_int* mu_data) {
  return ({ __anon_31_env* __env = (__anon_31_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_31_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_31_env; __env->mu_data = mu_data; (closure_int_int){.env = __env, .fn = __anon_31_fn}; });
}

static closure_Vec_int_bool mu_gunzip_ok(mere_vec_int* mu_data) {
  return ({ __anon_32_env* __env = (__anon_32_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_32_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_32_env; __env->mu_data = mu_data; (closure_Vec_int_bool){.env = __env, .fn = __anon_32_fn}; });
}

static closure_int_Vec_int mu_new_vec(long long mu_n) {
  return ({ __anon_33_env* __env = (__anon_33_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_33_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_33_env; __env->mu_n = mu_n; (closure_int_Vec_int){.env = __env, .fn = __anon_33_fn}; });
}

static int mu_is_png(mere_vec_int* mu_data) {
  return ((mere_vec_int_len(mu_data) < 8LL) ? 0 : (((((0LL >= 0LL) && (0LL <= 8LL)) && (8LL <= mere_vec_int_len(mu_png_sig))) && (8LL <= mere_vec_int_len(mu_data))) ? __lifted_go__rvfast_uq13_28(mu_data, mu_png_sig, 0LL) : __lifted_go_uq14_29(mu_data, mu_png_sig, 0LL)));
}

static closure_int_int mu_u32be(mere_vec_int* mu_data) {
  return ({ __anon_34_env* __env = (__anon_34_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_34_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_34_env; __env->mu_data = mu_data; (closure_int_int){.env = __env, .fn = __anon_34_fn}; });
}

static closure_int_closure_int_closure_int_closure_int_closure_int_bool mu_type_is(mere_vec_int* mu_data) {
  return ({ __anon_35_env* __env = (__anon_35_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_35_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_35_env; __env->mu_data = mu_data; (closure_int_closure_int_closure_int_closure_int_closure_int_bool){.env = __env, .fn = __anon_35_fn, .fn2 = __anon_35_fn2}; });
}

static closure_int_bool mu_is_ihdr(mere_vec_int* mu_data) {
  return ({ __anon_36_env* __env = (__anon_36_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_36_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_36_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_36_fn}; });
}

static closure_int_bool mu_is_idat(mere_vec_int* mu_data) {
  return ({ __anon_37_env* __env = (__anon_37_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_37_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_37_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_37_fn}; });
}

static closure_int_bool mu_is_iend(mere_vec_int* mu_data) {
  return ({ __anon_38_env* __env = (__anon_38_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_38_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_38_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_38_fn}; });
}

static closure_int_bool mu_is_plte(mere_vec_int* mu_data) {
  return ({ __anon_39_env* __env = (__anon_39_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_39_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_39_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_39_fn}; });
}

static closure_int_bool mu_is_text(mere_vec_int* mu_data) {
  return ({ __anon_40_env* __env = (__anon_40_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_40_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_40_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_40_fn}; });
}

static closure_int_bool mu_is_gama(mere_vec_int* mu_data) {
  return ({ __anon_41_env* __env = (__anon_41_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_41_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_41_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_41_fn}; });
}

static closure_int_bool mu_is_trns(mere_vec_int* mu_data) {
  return ({ __anon_42_env* __env = (__anon_42_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_42_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_42_env; __env->mu_data = mu_data; (closure_int_bool){.env = __env, .fn = __anon_42_fn}; });
}

static mere_vec_int* mu_read_header__Vec_int__Vec_int(mere_vec_int* mu_data) {
  return __lifted_walk_30(mu_data, 8LL);
}

static long long mu_channels(long long mu_colour) {
  return ((mu_colour == 0LL) ? 1LL : ((mu_colour == 2LL) ? 3LL : ((mu_colour == 3LL) ? 1LL : ((mu_colour == 4LL) ? 2LL : ((mu_colour == 6LL) ? 4LL : 0LL)))));
}

static closure_int_int mu_pixel_bytes(long long mu_colour) {
  return ({ __anon_43_env* __env = (__anon_43_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_43_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_43_env; __env->mu_colour = mu_colour; (closure_int_int){.env = __env, .fn = __anon_43_fn}; });
}

static mere_vec_int* mu_palette(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq15_31(mu_n, mu_out, mu_data, 8LL); });
}

static mere_vec_int* mu_idat_bytes(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq17_33(mu_n, mu_out, mu_data, 8LL); });
}

static mere_vec_str* mu_text_chunks(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_str_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq19_35(mu_n, mu_out, mu_data, 8LL); });
}

static long long mu_gamma(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq20_38(mu_n, mu_data, 8LL); });
}

static int mu_has_transparency(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq21_39(mu_n, mu_data, 8LL); });
}

static closure_int_closure_int_int mu_paeth(long long mu_a) {
  return ({ __anon_44_env* __env = (__anon_44_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_44_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_44_env; __env->mu_a = mu_a; (closure_int_closure_int_int){.env = __env, .fn = __anon_44_fn, .fn2 = __anon_44_fn2}; });
}

static closure_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf(mere_vec_int* mu_raw) {
  return ({ __anon_45_env* __env = (__anon_45_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_45_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_45_env; __env->mu_raw = mu_raw; (closure_int_closure_int_closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_45_fn, .fn2 = __anon_45_fn2}; });
}

static closure_int_closure_int_closure_int_ByteBuf mu_unfilter__Vec_int__int__int__int__ByteBuf(mere_vec_int* mu_raw) {
  return ({ __anon_46_env* __env = (__anon_46_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_46_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_46_env; __env->mu_raw = mu_raw; (closure_int_closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_46_fn, .fn2 = __anon_46_fn2}; });
}

static int mu_is_interlaced(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 5LL) && (mere_vec_int_get(mu_h, 4LL) == 1LL)); });
}

static long long mu_adam7_xstart(long long mu_p_uq22) {
  return ((mu_p_uq22 == 0LL) ? 0LL : ((mu_p_uq22 == 1LL) ? 4LL : ((mu_p_uq22 == 2LL) ? 0LL : ((mu_p_uq22 == 3LL) ? 2LL : ((mu_p_uq22 == 4LL) ? 0LL : ((mu_p_uq22 == 5LL) ? 1LL : 0LL))))));
}

static long long mu_adam7_ystart(long long mu_p_uq23) {
  return ((mu_p_uq23 == 0LL) ? 0LL : ((mu_p_uq23 == 1LL) ? 0LL : ((mu_p_uq23 == 2LL) ? 4LL : ((mu_p_uq23 == 3LL) ? 0LL : ((mu_p_uq23 == 4LL) ? 2LL : ((mu_p_uq23 == 5LL) ? 0LL : 1LL))))));
}

static long long mu_adam7_xstep(long long mu_p_uq24) {
  return ((mu_p_uq24 == 0LL) ? 8LL : ((mu_p_uq24 == 1LL) ? 8LL : ((mu_p_uq24 == 2LL) ? 4LL : ((mu_p_uq24 == 3LL) ? 4LL : ((mu_p_uq24 == 4LL) ? 2LL : ((mu_p_uq24 == 5LL) ? 2LL : 1LL))))));
}

static long long mu_adam7_ystep(long long mu_p_uq25) {
  return ((mu_p_uq25 == 0LL) ? 8LL : ((mu_p_uq25 == 1LL) ? 8LL : ((mu_p_uq25 == 2LL) ? 8LL : ((mu_p_uq25 == 3LL) ? 4LL : ((mu_p_uq25 == 4LL) ? 4LL : ((mu_p_uq25 == 5LL) ? 2LL : 2LL))))));
}

static closure_int_int mu_ceil_div(long long mu_a) {
  return ({ __anon_47_env* __env = (__anon_47_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_47_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_47_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_47_fn}; });
}

static closure_int_int mu_pass_width(long long mu_p_uq26) {
  return ({ __anon_48_env* __env = (__anon_48_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_48_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_48_env; __env->mu_p_uq26 = mu_p_uq26; (closure_int_int){.env = __env, .fn = __anon_48_fn}; });
}

static closure_int_int mu_pass_height(long long mu_p_uq27) {
  return ({ __anon_49_env* __env = (__anon_49_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_49_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_49_env; __env->mu_p_uq27 = mu_p_uq27; (closure_int_int){.env = __env, .fn = __anon_49_fn}; });
}

static closure_int_closure_int_closure_int_ByteBuf mu_deinterlace__Vec_int__int__int__int__ByteBuf(mere_vec_int* mu_raw) {
  return ({ __anon_50_env* __env = (__anon_50_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_50_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_50_env; __env->mu_raw = mu_raw; (closure_int_closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_50_fn, .fn2 = __anon_50_fn2}; });
}

static mere_bytebuf* mu_decode__Vec_int__ByteBuf(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 0LL) ? mere_bytebuf_new(__lang_current_region, 0LL) : ({ __auto_type __let_tmp_width = mere_vec_int_get(mu_h, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_h, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_bpp = ({ __auto_type __da0 = mere_vec_int_get(mu_h, 3LL); __auto_type __da1 = mere_vec_int_get(mu_h, 2LL); mu_pixel_bytes__direct(__da0, __da1); }); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_raw = ({ __auto_type __da0 = ({ __auto_type __da0 = mu_data; mu_idat_bytes__direct(__da0); }); __auto_type __da1 = 2LL; mu_inflate__direct(__da0, __da1); }); __auto_type mu_raw = __let_tmp_raw; ((mere_vec_int_get(mu_h, 4LL) == 1LL) ? ({ __auto_type __da0 = mu_raw; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = mu_bpp; mu_deinterlace__Vec_int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3); }) : ({ __auto_type __da0 = mu_raw; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = mu_bpp; mu_unfilter__Vec_int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3); })); })); });
}

static mere_vec_int* mu_rgb8(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; __auto_type __let_tmp_width = mere_vec_int_get(mu_h, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_h, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_depth = mere_vec_int_get(mu_h, 2LL); __auto_type mu_depth = __let_tmp_depth; __auto_type __let_tmp_colour = mere_vec_int_get(mu_h, 3LL); __auto_type mu_colour = __let_tmp_colour; __auto_type __let_tmp_bpp = ({ __auto_type __da0 = mu_colour; __auto_type __da1 = mu_depth; mu_pixel_bytes__direct(__da0, __da1); }); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_step = ((mu_depth == 16LL) ? 2LL : 1LL); __auto_type mu_step = __let_tmp_step; __auto_type __let_tmp_pixels = ({ __auto_type __da0 = mu_data; mu_decode__Vec_int__ByteBuf__direct(__lang_current_region, __da0); }); __auto_type mu_pixels = __let_tmp_pixels; __auto_type __let_tmp_pal = ({ __auto_type __da0 = mu_data; mu_palette__direct(__da0); }); __auto_type mu_pal = __let_tmp_pal; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __lifted_go_uq29_47(mu_width, mu_height, mu_out, mu_bpp, mu_pixels, mu_colour, mu_pal, mu_step, 0LL); });
}

static mere_vec_int* mu_vec_of__v2(list_int mu_xs) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq30_48(mu_v, mu_xs); });
}

static closure_int_int mu_crc_byte__v2(long long mu_crc) {
  return ({ __anon_51_env* __env = (__anon_51_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_51_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_51_env; __env->mu_crc = mu_crc; (closure_int_int){.env = __env, .fn = __anon_51_fn}; });
}

static long long mu_crc32_vec__v2(mere_vec_int* mu_bytes) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_bytes); __auto_type mu_n = __let_tmp_n; ((((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_bytes))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32__v2; __lifted_go__rvfast_uq32_50__direct(mu_n, mu_bytes, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32__v2; __lifted_go_uq33_51__direct(mu_n, mu_bytes, __ia0, __ia1); }))) ^ (mu_mask32__v2)); });
}

static mere_vec_int* mu_bw_new__unit__Vec_int(int mu__u) {
  return ({ __auto_type __let_tmp_s = mere_vec_int_new(__lang_current_region); __auto_type mu_s = __let_tmp_s; (void)(mere_vec_int_push(mu_s, 0LL)); (void)(mere_vec_int_push(mu_s, 0LL)); mu_s; });
}

static closure_Vec_int_closure_int_closure_int_unit mu_bw_bits(mere_vec_int* mu_s) {
  return ({ __anon_52_env* __env = (__anon_52_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_52_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_52_env; __env->mu_s = mu_s; (closure_Vec_int_closure_int_closure_int_unit){.env = __env, .fn = __anon_52_fn, .fn2 = __anon_52_fn2}; });
}

static closure_Vec_int_unit mu_bw_flush(mere_vec_int* mu_s) {
  return ({ __anon_53_env* __env = (__anon_53_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_53_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_53_env; __env->mu_s = mu_s; (closure_Vec_int_unit){.env = __env, .fn = __anon_53_fn}; });
}

static closure_int_int mu_rev_bits(long long mu_v) {
  return ({ __anon_54_env* __env = (__anon_54_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_54_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_54_env; __env->mu_v = mu_v; (closure_int_int){.env = __env, .fn = __anon_54_fn}; });
}

static tuple_int_int_int mu_len_encode(long long mu_len) {
  return ((((((0LL >= 0LL) && (0LL <= 28LL)) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_g_len_base))) && ((((28LL - 1LL) + 1LL) >= 0LL) && ((((28LL - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_g_len_base))))) && (28LL <= mere_vec_int_len(mu_g_len_extra))) && (28LL <= mere_vec_int_len(mu_g_len_base))) ? __lifted_go__rvfast_uq36_54(mu_g_len_base, mu_g_len_extra, mu_len, 0LL) : __lifted_go_uq37_55(mu_g_len_base, mu_g_len_extra, mu_len, 0LL));
}

static tuple_int_int_int mu_dist_encode(long long mu_dist) {
  return ((((((0LL >= 0LL) && (0LL <= 29LL)) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_g_dist_base))) && ((((29LL - 1LL) + 1LL) >= 0LL) && ((((29LL - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_g_dist_base))))) && (29LL <= mere_vec_int_len(mu_g_dist_extra))) && (29LL <= mere_vec_int_len(mu_g_dist_base))) ? __lifted_go__rvfast_uq38_56(mu_g_dist_extra, mu_dist, mu_g_dist_base, 0LL) : __lifted_go_uq39_57(mu_g_dist_extra, mu_dist, mu_g_dist_base, 0LL));
}

static closure_int_closure_int_tuple_int_int mu_find_match(mere_vec_int* mu_data) {
  return ({ __anon_55_env* __env = (__anon_55_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_55_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_55_env; __env->mu_data = mu_data; (closure_int_closure_int_tuple_int_int){.env = __env, .fn = __anon_55_fn, .fn2 = __anon_55_fn2}; });
}

static closure_int_closure_int_Vec_int mu_build_lengths__Vec_int__int__int__Vec_int(mere_vec_int* mu_freq) {
  return ({ __anon_56_env* __env = (__anon_56_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_56_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_56_env; __env->mu_freq = mu_freq; (closure_int_closure_int_Vec_int){.env = __env, .fn = __anon_56_fn, .fn2 = __anon_56_fn2}; });
}

static closure_int_closure_int_Vec_int mu_canon_codes__Vec_int__int__int__Vec_int(mere_vec_int* mu_lengths) {
  return ({ __anon_57_env* __env = (__anon_57_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_57_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_57_env; __env->mu_lengths = mu_lengths; (closure_int_closure_int_Vec_int){.env = __env, .fn = __anon_57_fn, .fn2 = __anon_57_fn2}; });
}

static closure_int_int mu_vmax(mere_vec_int* mu_v) {
  return ({ __anon_58_env* __env = (__anon_58_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_58_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_58_env; __env->mu_v = mu_v; (closure_int_int){.env = __env, .fn = __anon_58_fn}; });
}

static mere_vec_int* mu_tokenize__Vec_int__Vec_int(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_toks = mere_vec_int_new(__lang_current_region); __auto_type mu_toks = __let_tmp_toks; ({ (void)(__lifted_go_uq47_82(mu_n, mu_data, mu_toks, 0LL)); mu_toks; }); });
}

static mere_vec_int* mu_deflate_dynamic__Vec_int__Vec_int(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_toks = ({ __auto_type __da0 = mu_data; mu_tokenize__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_toks = __let_tmp_toks; __auto_type __let_tmp_nt = mere_vec_int_len(mu_toks); __auto_type mu_nt = __let_tmp_nt; __auto_type __let_tmp_lfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_lfreq = __let_tmp_lfreq; ({ (void)(__lifted_zl_uq48_83(mu_lfreq, 0LL)); __auto_type __let_tmp_dfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_dfreq = __let_tmp_dfreq; ({ (void)(__lifted_zd_84(mu_dfreq, 0LL)); ({ (void)(((((((0LL >= 0LL) && (0LL <= mu_nt)) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 2LL) >= 0LL) && ((((mu_nt - 1LL) + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 1LL) >= 0LL) && ((((mu_nt - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && (mu_nt <= mere_vec_int_len(mu_toks))) ? __lifted_count__rvfast_85(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL) : __lifted_count_86(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL))); (void)(mere_vec_int_set(mu_lfreq, 256LL, (mere_vec_int_get(mu_lfreq, 256LL) + 1LL))); __auto_type __let_tmp_llen = ({ __auto_type __da0 = mu_lfreq; __auto_type __da1 = 286LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_llen = __let_tmp_llen; __auto_type __let_tmp_dlen = ({ __auto_type __da0 = mu_dfreq; __auto_type __da1 = 30LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_dlen = __let_tmp_dlen; (void)(((({ __auto_type __da0 = mu_dlen; __auto_type __da1 = 30LL; mu_vmax__direct(__da0, __da1); }) == 0LL) ? mere_vec_int_set(mu_dlen, 0LL, 1LL) : 0)); __auto_type __let_tmp_lcode = ({ __auto_type __da0 = mu_llen; __auto_type __da1 = 286LL; __auto_type __da2 = 15LL; mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_lcode = __let_tmp_lcode; __auto_type __let_tmp_dcode = ({ __auto_type __da0 = mu_dlen; __auto_type __da1 = 30LL; __auto_type __da2 = 15LL; mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_dcode = __let_tmp_dcode; ({ __auto_type __let_tmp_hlit = (({ __auto_type __ia0 = mu_llen; __auto_type __ia1 = 285LL; __auto_type __ia2 = 256LL; __lifted_last_nz_87__direct(__ia0, __ia1, __ia2); }) + 1LL); __auto_type mu_hlit = __let_tmp_hlit; __auto_type __let_tmp_hdist = (({ __auto_type __ia0 = mu_dlen; __auto_type __ia1 = 29LL; __auto_type __ia2 = 0LL; __lifted_last_nz_87__direct(__ia0, __ia1, __ia2); }) + 1LL); __auto_type mu_hdist = __let_tmp_hdist; __auto_type __let_tmp_cl_syms = mere_vec_int_new(__lang_current_region); __auto_type mu_cl_syms = __let_tmp_cl_syms; __auto_type __let_tmp_combined = mere_vec_int_new(__lang_current_region); __auto_type mu_combined = __let_tmp_combined; ({ (void)(__lifted_cc1_88(mu_hlit, mu_combined, mu_llen, 0LL)); ({ (void)(__lifted_cc2_89(mu_hdist, mu_combined, mu_dlen, 0LL)); __auto_type __let_tmp_cn = mere_vec_int_len(mu_combined); __auto_type mu_cn = __let_tmp_cn; ({ (void)(__lifted_rle_90(mu_cn, mu_combined, mu_cl_syms, 0LL)); __auto_type __let_tmp_clfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_clfreq = __let_tmp_clfreq; ({ (void)(__lifted_zc_uq49_94(mu_clfreq, 0LL)); __auto_type __let_tmp_cs_n = mere_vec_int_len(mu_cl_syms); __auto_type mu_cs_n = __let_tmp_cs_n; ({ (void)(((((0LL >= 0LL) && (0LL <= mu_cs_n)) && (mu_cs_n <= mere_vec_int_len(mu_cl_syms))) ? __lifted_ccount__rvfast_95(mu_cs_n, mu_cl_syms, mu_clfreq, 0LL) : __lifted_ccount_96(mu_cs_n, mu_cl_syms, mu_clfreq, 0LL))); __auto_type __let_tmp_cllen = ({ __auto_type __da0 = mu_clfreq; __auto_type __da1 = 19LL; __auto_type __da2 = 7LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_cllen = __let_tmp_cllen; __auto_type __let_tmp_clcode = ({ __auto_type __da0 = mu_cllen; __auto_type __da1 = 19LL; __auto_type __da2 = 7LL; mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_clcode = __let_tmp_clcode; ({ __auto_type __let_tmp_hclen = __lifted_last_cl_97(mu_cllen, mu_cl_order, 18LL); __auto_type mu_hclen = __let_tmp_hclen; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_s = ({ __auto_type __da0 = 0; mu_bw_new__unit__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_s = __let_tmp_s; (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = 1LL; __auto_type __da3 = 1LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = 2LL; __auto_type __da3 = 2LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = (mu_hlit - 257LL); __auto_type __da3 = 5LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = (mu_hdist - 1LL); __auto_type __da3 = 5LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = (mu_hclen - 4LL); __auto_type __da3 = 4LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); ({ (void)(__lifted_emit_cl_98(mu_hclen, mu_s, mu_out, mu_cllen, mu_cl_order, 0LL)); ({ (void)(__lifted_emit_rle_99(mu_cs_n, mu_cl_syms, mu_s, mu_out, mu_clcode, mu_cllen, 0LL)); ({ (void)(__lifted_emit_data_100(mu_nt, mu_toks, mu_s, mu_out, mu_lcode, mu_llen, mu_dcode, mu_dlen, 0LL)); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = ({ __auto_type __da0 = mere_vec_int_get(mu_lcode, 256LL); __auto_type __da1 = mere_vec_int_get(mu_llen, 256LL); mu_rev_bits__direct(__da0, __da1); }); __auto_type __da3 = mere_vec_int_get(mu_llen, 256LL); mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; mu_bw_flush__direct(__da0, __da1); })); mu_out; }); }); }); }); }); }); }); }); }); }); }); }); }); });
}

static mere_vec_int* mu_deflate_stored__Vec_int__Vec_int(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; ({ (void)(((mu_n == 0LL) ? ({ (void)(mere_vec_int_push(mu_out, 1LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 255LL)); (void)(mere_vec_int_push(mu_out, 255LL)); 0; }) : __lifted_blocks_101(mu_n, mu_out, mu_data, 0LL))); mu_out; }); });
}

static mere_vec_int* mu_deflate_body__Vec_int__Vec_int(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_toks = ({ __auto_type __da0 = mu_data; mu_tokenize__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_toks = __let_tmp_toks; __auto_type __let_tmp_nt = mere_vec_int_len(mu_toks); __auto_type mu_nt = __let_tmp_nt; __auto_type __let_tmp_lfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_lfreq = __let_tmp_lfreq; ({ (void)(__lifted_zl_uq50_103(mu_lfreq, 0LL)); __auto_type __let_tmp_dfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_dfreq = __let_tmp_dfreq; ({ (void)(__lifted_zd_uq51_104(mu_dfreq, 0LL)); ({ (void)(((((((0LL >= 0LL) && (0LL <= mu_nt)) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 2LL) >= 0LL) && ((((mu_nt - 1LL) + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 1LL) >= 0LL) && ((((mu_nt - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && (mu_nt <= mere_vec_int_len(mu_toks))) ? __lifted_count__rvfast_uq52_105(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL) : __lifted_count_uq53_106(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL))); (void)(mere_vec_int_set(mu_lfreq, 256LL, (mere_vec_int_get(mu_lfreq, 256LL) + 1LL))); __auto_type __let_tmp_llen = ({ __auto_type __da0 = mu_lfreq; __auto_type __da1 = 286LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_llen = __let_tmp_llen; __auto_type __let_tmp_dlen = ({ __auto_type __da0 = mu_dfreq; __auto_type __da1 = 30LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_dlen = __let_tmp_dlen; (((({ __auto_type __da0 = mu_llen; __auto_type __da1 = 286LL; mu_vmax__direct(__da0, __da1); }) > 15LL) || (({ __auto_type __da0 = mu_dlen; __auto_type __da1 = 30LL; mu_vmax__direct(__da0, __da1); }) > 15LL)) ? ({ __auto_type __da0 = mu_data; mu_deflate_stored__Vec_int__Vec_int__direct(__lang_current_region, __da0); }) : ({ __auto_type __da0 = mu_data; mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_current_region, __da0); })); }); }); }); });
}

static mere_vec_int* mu_deflate_gzip(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(mere_vec_int_push(mu_out, 31LL)); (void)(mere_vec_int_push(mu_out, 139LL)); (void)(mere_vec_int_push(mu_out, 8LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 3LL)); __auto_type __let_tmp_body = ({ __auto_type __da0 = mu_data; mu_deflate_body__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_body = __let_tmp_body; __auto_type __let_tmp_bn = mere_vec_int_len(mu_body); __auto_type mu_bn = __let_tmp_bn; ({ (void)(__lifted_cp_uq54_107(mu_bn, mu_out, mu_body, 0LL)); (void)(__lifted_push_u32_108(mu_out, ({ __auto_type __da0 = mu_data; mu_crc32_vec__v2__direct(__da0); }))); (void)(__lifted_push_u32_108(mu_out, ((mere_vec_int_len(mu_data)) & (mu_mask32__v2)))); mu_out; }); });
}

static closure_int_unit mu_bytes_push4be(mere_vec_int* mu_out) {
  return ({ __anon_59_env* __env = (__anon_59_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_59_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_59_env; __env->mu_out = mu_out; (closure_int_unit){.env = __env, .fn = __anon_59_fn}; });
}

static closure_Vec_int_unit mu_push_all(mere_vec_int* mu_out) {
  return ({ __anon_60_env* __env = (__anon_60_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_60_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_60_env; __env->mu_out = mu_out; (closure_Vec_int_unit){.env = __env, .fn = __anon_60_fn}; });
}

static long long mu_adler32(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; ((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_data))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 1LL; __auto_type __ia2 = 0LL; __lifted_go__rvfast_uq56_110__direct(mu_n, mu_data, __ia0, __ia1, __ia2); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 1LL; __auto_type __ia2 = 0LL; __lifted_go_uq57_111__direct(mu_n, mu_data, __ia0, __ia1, __ia2); })); });
}

static closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit mu_chunk(mere_vec_int* mu_out) {
  return ({ __anon_61_env* __env = (__anon_61_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_61_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_61_env; __env->mu_out = mu_out; (closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_61_fn, .fn2 = __anon_61_fn2}; });
}

static closure_int_closure_int_int mu_paeth_e(long long mu_a) {
  return ({ __anon_62_env* __env = (__anon_62_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_62_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_62_env; __env->mu_a = mu_a; (closure_int_closure_int_int){.env = __env, .fn = __anon_62_fn, .fn2 = __anon_62_fn2}; });
}

static closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_filter_row(long long mu_kind) {
  return ({ __anon_63_env* __env = (__anon_63_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_63_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_63_env; __env->mu_kind = mu_kind; (closure_Vec_int_closure_Vec_int_closure_int_Vec_int){.env = __env, .fn = __anon_63_fn, .fn2 = __anon_63_fn2}; });
}

static long long mu_row_cost(mere_vec_int* mu_row_uq60) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_row_uq60); __auto_type mu_n = __let_tmp_n; ((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_row_uq60))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go__rvfast_uq61_113__direct(mu_n, mu_row_uq60, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq62_114__direct(mu_n, mu_row_uq60, __ia0, __ia1); })); });
}

static closure_Vec_int_closure_int_Vec_int mu_best_filter__Vec_int__Vec_int__int__Vec_int(mere_vec_int* mu_row_uq63) {
  return ({ __anon_64_env* __env = (__anon_64_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_64_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_64_env; __env->mu_row_uq63 = mu_row_uq63; (closure_Vec_int_closure_int_Vec_int){.env = __env, .fn = __anon_64_fn, .fn2 = __anon_64_fn2}; });
}

static closure_int_bool mu_all_grey(mere_vec_int* mu_pixels) {
  return ({ __anon_65_env* __env = (__anon_65_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_65_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_65_env; __env->mu_pixels = mu_pixels; (closure_int_bool){.env = __env, .fn = __anon_65_fn}; });
}

static closure_int_Vec_int mu_collect_palette(mere_vec_int* mu_pixels) {
  return ({ __anon_66_env* __env = (__anon_66_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_66_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_66_env; __env->mu_pixels = mu_pixels; (closure_int_Vec_int){.env = __env, .fn = __anon_66_fn}; });
}

static closure_int_closure_int_closure_int_int mu_palette_index(mere_vec_int* mu_pal) {
  return ({ __anon_67_env* __env = (__anon_67_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_67_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_67_env; __env->mu_pal = mu_pal; (closure_int_closure_int_closure_int_int){.env = __env, .fn = __anon_67_fn, .fn2 = __anon_67_fn2}; });
}

static closure_int_closure_int_closure_int_Vec_int mu_encode_at_depth__Vec_int__int__int__int__Vec_int(mere_vec_int* mu_pixels) {
  return ({ __anon_68_env* __env = (__anon_68_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_68_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_68_env; __env->mu_pixels = mu_pixels; (closure_int_closure_int_closure_int_Vec_int){.env = __env, .fn = __anon_68_fn, .fn2 = __anon_68_fn2}; });
}

static closure_int_closure_int_Vec_int mu_encode_rgb8__Vec_int__int__int__Vec_int(mere_vec_int* mu_pixels) {
  return ({ __anon_69_env* __env = (__anon_69_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_69_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_69_env; __env->mu_pixels = mu_pixels; (closure_int_closure_int_Vec_int){.env = __env, .fn = __anon_69_fn, .fn2 = __anon_69_fn2}; });
}

static closure_int_closure_int_Vec_int mu_encode16__Vec_int__int__int__Vec_int(mere_vec_int* mu_pixels) {
  return ({ __anon_70_env* __env = (__anon_70_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_70_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_70_env; __env->mu_pixels = mu_pixels; (closure_int_closure_int_Vec_int){.env = __env, .fn = __anon_70_fn, .fn2 = __anon_70_fn2}; });
}

static const char* mu_colour_name(long long mu_c) {
  return ((mu_c == 0LL) ? (__sl_13.s) : ((mu_c == 2LL) ? (__sl_12.s) : ((mu_c == 3LL) ? (__sl_11.s) : ((mu_c == 4LL) ? (__sl_10.s) : ((mu_c == 6LL) ? (__sl_9.s) : (__sl_8.s))))));
}

static int mu_info(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 0LL) ? ({ const char* __ps = (__sl_19.s);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }) : ({ __auto_type __let_tmp_g = ({ __auto_type __da0 = mu_data; mu_gamma__direct(__da0); }); __auto_type mu_g = __let_tmp_g; (void)(({ const char* __ps = ({ __auto_type __bc168 = show_int(mere_vec_int_get(mu_h, 0LL)); __auto_type __bc169 = (__sl_14.s); __auto_type __bc170 = __lang_str_concat(__bc168, __bc169); __auto_type __bc171 = show_int(mere_vec_int_get(mu_h, 1LL)); __auto_type __bc172 = __lang_str_concat(__bc170, __bc171); __auto_type __bc173 = (__sl_4.s); __auto_type __bc174 = __lang_str_concat(__bc172, __bc173); __auto_type __bc175 = show_int(mere_vec_int_get(mu_h, 2LL)); __auto_type __bc176 = __lang_str_concat(__bc174, __bc175); __auto_type __bc177 = (__sl_15.s); __auto_type __bc178 = __lang_str_concat(__bc176, __bc177); __auto_type __bc179 = ({ __auto_type __da0 = mere_vec_int_get(mu_h, 3LL); mu_colour_name__direct(__da0); }); __auto_type __bc180 = __lang_str_concat(__bc178, __bc179); __auto_type __bc181 = ((mere_vec_int_get(mu_h, 4LL) == 1LL) ? (__sl_16.s) : (__sl_0.s)); __auto_type __bc182 = __lang_str_concat(__bc180, __bc181); __auto_type __bc183 = (({ __auto_type __da0 = mu_data; mu_has_transparency__direct(__da0); }) ? (__sl_17.s) : (__sl_0.s)); __auto_type __bc184 = __lang_str_concat(__bc182, __bc183); __auto_type __bc185 = ((mu_g > 0LL) ? __lang_str_concat((__sl_18.s), show_int(mu_g)) : (__sl_0.s)); __auto_type __bc186 = __lang_str_concat(__bc184, __bc185); __bc186; });            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; })); __auto_type __let_tmp_t = ({ __auto_type __da0 = mu_data; mu_text_chunks__direct(__da0); }); __auto_type mu_t = __let_tmp_t; ((((0LL >= 0LL) && (0LL <= mere_vec_str_len(mu_t))) && (mere_vec_str_len(mu_t) <= mere_vec_str_len(mu_t))) ? __lifted_show__rvfast_125(mu_t, 0LL) : __lifted_show_126(mu_t, 0LL)); })); });
}

static mere_vec_int* mu_ppm_bytes(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; ({ (void)(({ __auto_type __ia0 = ({ __auto_type __bc187 = (__sl_20.s); __auto_type __bc188 = show_int(mere_vec_int_get(mu_h, 0LL)); __auto_type __bc189 = __lang_str_concat(__bc187, __bc188); __auto_type __bc190 = (__sl_4.s); __auto_type __bc191 = __lang_str_concat(__bc189, __bc190); __auto_type __bc192 = show_int(mere_vec_int_get(mu_h, 1LL)); __auto_type __bc193 = __lang_str_concat(__bc191, __bc192); __auto_type __bc194 = (__sl_21.s); __auto_type __bc195 = __lang_str_concat(__bc193, __bc194); __bc195; }); __auto_type __ia1 = 0LL; __lifted_header_127__direct(mu_out, __ia0, __ia1); })); __auto_type __let_tmp_rgb = ({ __auto_type __da0 = mu_data; mu_rgb8__direct(__da0); }); __auto_type mu_rgb = __let_tmp_rgb; __auto_type __let_tmp_n = mere_vec_int_len(mu_rgb); __auto_type mu_n = __let_tmp_n; __lifted_go_uq72_128(mu_n, mu_out, mu_rgb, 0LL); }); });
}

static mere_vec_int* mu_read_ppm__Vec_int__Vec_int(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_acc = mere_vec_int_new(__lang_current_region); __auto_type mu_acc = __let_tmp_acc; ({ (void)(({ __auto_type __ia0 = 2LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct(mu_n, mu_data, mu_acc, __ia0, __ia1, __ia2, __ia3); })); mu_acc; }); });
}

static mere_vec_int* mu_load(const char* mu_path) {
  return __lang_vec_of_bytes(__lang_read_bytes(mu_path), __lang_current_region);
}

static closure_str_unit mu_encode_cmd(const char* mu_path) {
  return ({ __anon_71_env* __env = (__anon_71_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_71_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_71_env; __env->mu_path = mu_path; (closure_str_unit){.env = __env, .fn = __anon_71_fn}; });
}

static int mu___mere_user_main(int mu__u) {
  return ({ __auto_type __scrut = ((void)(0), __lang_args()); (((list_str__tag(__scrut) == 1) && (((strcmp(((list_str__node(__scrut)->payload.Cons).f0), "encode") == 0)) && (((list_str__tag((list_str__node(__scrut)->payload.Cons).f1) == 1) && ((list_str__tag((list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f1) == 1))))))) ? ({ __auto_type mu_path = (list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f0; __auto_type mu_out = (list_str__node((list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f1)->payload.Cons).f0; ({ __auto_type __da0 = mu_path; __auto_type __da1 = mu_out; mu_encode_cmd__direct(__da0, __da1); }); }) : ((((list_str__tag(__scrut) == 1) && ((list_str__tag((list_str__node(__scrut)->payload.Cons).f1) == 1)))) ? ({ __auto_type mu_cmd = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_path = (list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f0; __auto_type mu_rest = (list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f1; ({ __auto_type __let_tmp_data = ({ __auto_type __da0 = mu_path; mu_load__direct(__da0); }); __auto_type mu_data = __let_tmp_data; ((({ __auto_type __da0 = mu_data; mu_is_png__direct(__da0); }) == 0) ? ({ const char* __ps = (__sl_29.s);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }) : ((__lang_str_cmp(mu_cmd, (__sl_28.s)) == 0) ? ({ __auto_type __da0 = mu_data; mu_info__direct(__da0); }) : ((__lang_str_cmp(mu_cmd, (__sl_27.s)) == 0) ? ({ __auto_type __scrut = mu_rest; (list_str__tag(__scrut) == 1) ? ({ __auto_type mu_out = (list_str__node(__scrut)->payload.Cons).f0; ({ __auto_type __let_tmp_b = __lang_bytes_of_vec(({ __auto_type __da0 = mu_data; mu_ppm_bytes__direct(__da0); })); __auto_type mu_b = __let_tmp_b; (void)(__lang_write_bytes(mu_out, mu_b)); ({ const char* __ps = __lang_str_concat(__lang_str_concat(show_int(((long long)(mu_b)->len)), (__sl_26.s)), mu_out);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }); }) : ((list_str__tag(__scrut) == 0) ? ({ ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))); }) : ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; })))); }); }) : ((((list_str__tag(__scrut) == 1) && ((list_str__tag((list_str__node(__scrut)->payload.Cons).f1) == 0)))) ? ({ __auto_type mu_cmd = (list_str__node(__scrut)->payload.Cons).f0; ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }) : ((1) ? ({ ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))))); });
}

static const char* mu_png_info(mere_bytes* mu_data) {
  return ({ __auto_type __let_tmp_v = __lang_vec_of_bytes(mu_data, __lang_current_region); __auto_type mu_v = __let_tmp_v; ((({ __auto_type __da0 = mu_v; mu_is_png__direct(__da0); }) == 0) ? __lang_fail_str(__lang_str_concat((__sl_2.s), (__sl_29.s))) : ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_v; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 0LL) ? __lang_fail_str(__lang_str_concat((__sl_2.s), (__sl_19.s))) : ({ __auto_type __let_tmp_g = ({ __auto_type __da0 = mu_v; mu_gamma__direct(__da0); }); __auto_type mu_g = __let_tmp_g; __auto_type __let_tmp_head = ({ __auto_type __bc196 = show_int(mere_vec_int_get(mu_h, 0LL)); __auto_type __bc197 = (__sl_14.s); __auto_type __bc198 = __lang_str_concat(__bc196, __bc197); __auto_type __bc199 = show_int(mere_vec_int_get(mu_h, 1LL)); __auto_type __bc200 = __lang_str_concat(__bc198, __bc199); __auto_type __bc201 = (__sl_4.s); __auto_type __bc202 = __lang_str_concat(__bc200, __bc201); __auto_type __bc203 = show_int(mere_vec_int_get(mu_h, 2LL)); __auto_type __bc204 = __lang_str_concat(__bc202, __bc203); __auto_type __bc205 = (__sl_15.s); __auto_type __bc206 = __lang_str_concat(__bc204, __bc205); __auto_type __bc207 = ({ __auto_type __da0 = mere_vec_int_get(mu_h, 3LL); mu_colour_name__direct(__da0); }); __auto_type __bc208 = __lang_str_concat(__bc206, __bc207); __auto_type __bc209 = ((mere_vec_int_get(mu_h, 4LL) == 1LL) ? (__sl_16.s) : (__sl_0.s)); __auto_type __bc210 = __lang_str_concat(__bc208, __bc209); __auto_type __bc211 = (({ __auto_type __da0 = mu_v; mu_has_transparency__direct(__da0); }) ? (__sl_17.s) : (__sl_0.s)); __auto_type __bc212 = __lang_str_concat(__bc210, __bc211); __auto_type __bc213 = ((mu_g > 0LL) ? __lang_str_concat((__sl_18.s), show_int(mu_g)) : (__sl_0.s)); __auto_type __bc214 = __lang_str_concat(__bc212, __bc213); __bc214; }); __auto_type mu_head = __let_tmp_head; __auto_type __let_tmp_t = ({ __auto_type __da0 = mu_v; mu_text_chunks__direct(__da0); }); __auto_type mu_t = __let_tmp_t; ((((0LL >= 0LL) && (0LL <= mere_vec_str_len(mu_t))) && (mere_vec_str_len(mu_t) <= mere_vec_str_len(mu_t))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_head; __lifted_join__rvfast_131__direct(mu_t, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_head; __lifted_join_132__direct(mu_t, __ia0, __ia1); })); })); })); });
}

static mere_bytes* mu_png_to_ppm(mere_bytes* mu_data) {
  return ({ __auto_type __let_tmp_v = __lang_vec_of_bytes(mu_data, __lang_current_region); __auto_type mu_v = __let_tmp_v; ((({ __auto_type __da0 = mu_v; mu_is_png__direct(__da0); }) == 0) ? ({ __lang_fail_impl(__lang_str_concat((__sl_2.s), (__sl_29.s))); (mere_bytes*)0; }) : __lang_bytes_of_vec(({ __auto_type __da0 = mu_v; mu_ppm_bytes__direct(__da0); }))); });
}

static mere_bytes* mu_ppm_to_png(mere_bytes* mu_data) {
  return ({ __auto_type __let_tmp_v = __lang_vec_of_bytes(mu_data, __lang_current_region); __auto_type mu_v = __let_tmp_v; __auto_type __let_tmp_f = ({ __auto_type __da0 = mu_v; mu_read_ppm__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_f = __let_tmp_f; ((mere_vec_int_len(mu_f) < 4LL) ? ({ __lang_fail_impl(__lang_str_concat((__sl_2.s), (__sl_25.s))); (mere_bytes*)0; }) : ({ __auto_type __let_tmp_width = mere_vec_int_get(mu_f, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_f, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_maxval = mere_vec_int_get(mu_f, 2LL); __auto_type mu_maxval = __let_tmp_maxval; __auto_type __let_tmp_start = mere_vec_int_get(mu_f, 3LL); __auto_type mu_start = __let_tmp_start; __auto_type __let_tmp_wide = (mu_maxval > 255LL); __auto_type mu_wide = __let_tmp_wide; __auto_type __let_tmp_bytes_per_sample = (mu_wide ? 2LL : 1LL); __auto_type mu_bytes_per_sample = __let_tmp_bytes_per_sample; __auto_type __let_tmp_pixels = mere_vec_int_new(__lang_current_region); __auto_type mu_pixels = __let_tmp_pixels; ({ (void)(__lifted_copy_uq75_133(mu_width, mu_height, mu_bytes_per_sample, mu_pixels, mu_v, mu_start, 0LL)); __lang_bytes_of_vec((mu_wide ? ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode16__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }) : ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode_rgb8__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }))); }); })); });
}

static long long mu_list_fold__direct(list_int mu_xs, long long mu_acc, closure_int_closure_int_int mu_f) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_xs;
  __auto_type __mere_a1 = &mu_acc;
  __auto_type __mere_a2 = &mu_f;
  __mere_tail: ;
  return ({ __auto_type __scrut = mu_xs; (list_int__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_int__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_int__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_int__node(__scrut)->payload.Cons).f1; ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = ({ __auto_type __c2 = mu_f; __auto_type __a0 = mu_acc; __typeof__(__c2.fn(__c2.env, __a0)) __i2; if (!__c2.fn2) __i2 = __c2.fn(__c2.env, __a0); __auto_type __a1 = mu_h; __c2.fn2 ? __c2.fn2(__c2.env, __a0, __a1) : __i2.fn(__i2.env, __a1); }); __auto_type __mt2 = mu_f; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))); });
}

static list_int mu__range_down__direct(long long mu_a, long long mu_i, list_int mu_acc) {
  list_int __mere_ret;
  __auto_type __mere_a0 = &mu_a;
  __auto_type __mere_a1 = &mu_i;
  __auto_type __mere_a2 = &mu_acc;
  __mere_tail: ;
  return ((mu_i < mu_a) ? mu_acc : ({ __auto_type __mt0 = mu_a; __auto_type __mt1 = (mu_i - 1LL); __auto_type __mt2 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = mu_i, .f1 = mu_acc}); list_int__mk(1, __p); }); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }));
}

static list_int mu_range__direct(long long mu_a, long long mu_b) {
  return ({ __auto_type __da0 = mu_a; __auto_type __da1 = mu_b; __auto_type __da2 = list_int__mk(0, &__mshared_list_int_t0); mu__range_down__direct(__da0, __da1, __da2); });
}

static long long mu_list_sum__direct(list_int mu_xs) {
  return ({ __auto_type __da0 = mu_xs; __auto_type __da1 = 0LL; __auto_type __da2 = ((closure_int_closure_int_int){.env = NULL, .fn = __anon_0_fn, .fn2 = __anon_0_fn2}); mu_list_fold__direct(__da0, __da1, __da2); });
}

static long long mu_list_product__direct(list_int mu_xs) {
  return ({ __auto_type __da0 = mu_xs; __auto_type __da1 = 1LL; __auto_type __da2 = ((closure_int_closure_int_int){.env = NULL, .fn = __anon_1_fn, .fn2 = __anon_1_fn2}); mu_list_fold__direct(__da0, __da1, __da2); });
}

static const char* mu__u8_nth__direct(list_str mu_cs, long long mu_i) {
  const char* __mere_ret;
  __auto_type __mere_a0 = &mu_cs;
  __auto_type __mere_a1 = &mu_i;
  __mere_tail: ;
  return ({ __auto_type __scrut = mu_cs; (list_str__tag(__scrut) == 0) ? ({ (__sl_0.s); }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; ((mu_i == 0LL) ? mu_h : ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = (mu_i - 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; })); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}

static const char* mu_utf8_at__direct(const char* mu_s, long long mu_i) {
  return ({ __auto_type __da0 = __lang_utf8_chars(mu_s); __auto_type __da1 = mu_i; mu__u8_nth__direct(__da0, __da1); });
}

static int mu__cp_bad__direct(long long mu_n) {
  return (((mu_n < 0LL) || (mu_n > 1114111LL)) || ((mu_n >= 55296LL) && (mu_n <= 57343LL)));
}

static const char* mu_str_of_codepoint__direct(long long mu_n) {
  return (({ __auto_type __da0 = mu_n; mu__cp_bad__direct(__da0); }) ? __lang_fail_str(__lang_str_concat((__sl_2.s), (__sl_1.s))) : ((mu_n < 128LL) ? __lang_char_at_chr(mu_n) : ((mu_n < 2048LL) ? ({ __auto_type __bc13 = __lang_char_at_chr(((192LL) | (({ long long __sa = (mu_n); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })))); __auto_type __bc14 = __lang_char_at_chr(((128LL) | (((mu_n) & (63LL))))); __auto_type __bc15 = __lang_str_concat(__bc13, __bc14); __bc15; }) : ((mu_n < 65536LL) ? ({ __auto_type __bc8 = __lang_char_at_chr(((224LL) | (({ long long __sa = (mu_n); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })))); __auto_type __bc9 = __lang_char_at_chr(((128LL) | (((({ long long __sa = (mu_n); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (63LL))))); __auto_type __bc10 = __lang_str_concat(__bc8, __bc9); __auto_type __bc11 = __lang_char_at_chr(((128LL) | (((mu_n) & (63LL))))); __auto_type __bc12 = __lang_str_concat(__bc10, __bc11); __bc12; }) : ({ __auto_type __bc1 = __lang_char_at_chr(((240LL) | (({ long long __sa = (mu_n); long long __sn = (18LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })))); __auto_type __bc2 = __lang_char_at_chr(((128LL) | (((({ long long __sa = (mu_n); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (63LL))))); __auto_type __bc3 = __lang_str_concat(__bc1, __bc2); __auto_type __bc4 = __lang_char_at_chr(((128LL) | (((({ long long __sa = (mu_n); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (63LL))))); __auto_type __bc5 = __lang_str_concat(__bc3, __bc4); __auto_type __bc6 = __lang_char_at_chr(((128LL) | (((mu_n) & (63LL))))); __auto_type __bc7 = __lang_str_concat(__bc5, __bc6); __bc7; })))));
}

static long long mu__cp_span__direct(long long mu_b) {
  return ((mu_b < 128LL) ? 1LL : (((mu_b >= 192LL) && (mu_b <= 223LL)) ? 2LL : (((mu_b >= 224LL) && (mu_b <= 239LL)) ? 3LL : (((mu_b >= 240LL) && (mu_b <= 247LL)) ? 4LL : 1LL))));
}

static long long mu__cp_cont__direct(const char* mu_s, long long mu_i) {
  return ((__lang_ord(__lang_char_at(mu_s, mu_i))) & (63LL));
}

static long long mu_codepoint_of__direct(const char* mu_s) {
  return ({ __auto_type __let_tmp_n = ((long long) __lang_str_size(mu_s)); __auto_type mu_n = __let_tmp_n; ((mu_n == 0LL) ? __lang_fail_int(__lang_str_concat((__sl_2.s), (__sl_3.s))) : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at(mu_s, 0LL)); __auto_type mu_b0 = __let_tmp_b0; __auto_type __let_tmp_span = ({ __auto_type __da0 = mu_b0; mu__cp_span__direct(__da0); }); __auto_type mu_span = __let_tmp_span; ((mu_span != mu_n) ? __lang_fail_int(__lang_str_concat((__sl_2.s), (__sl_3.s))) : ({ __auto_type __let_tmp_cp = ((mu_span == 1LL) ? mu_b0 : ((mu_span == 2LL) ? ((({ long long __sa = (((mu_b0) & (31LL))); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); }))) : ((mu_span == 3LL) ? ((((({ long long __sa = (((mu_b0) & (15LL))); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); }))) : ((((({ long long __sa = (((mu_b0) & (7LL))); long long __sn = (18LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (((({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 3LL; mu__cp_cont__direct(__da0, __da1); })))))))); __auto_type mu_cp = __let_tmp_cp; __auto_type __let_tmp_shortest = ((mu_span == 1LL) ? (mu_cp < 128LL) : ((mu_span == 2LL) ? (mu_cp >= 128LL) : ((mu_span == 3LL) ? (mu_cp >= 2048LL) : (mu_cp >= 65536LL)))); __auto_type mu_shortest = __let_tmp_shortest; ((({ __auto_type __da0 = mu_cp; mu__cp_bad__direct(__da0); }) || (!(mu_shortest))) ? __lang_fail_int(__lang_str_concat((__sl_2.s), (__sl_3.s))) : mu_cp); })); })); });
}

static long long mu_codepoint_or__direct(const char* mu_s, long long mu_d) {
  return ({ __auto_type __let_tmp_n = ((long long) __lang_str_size(mu_s)); __auto_type mu_n = __let_tmp_n; ((mu_n == 0LL) ? mu_d : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at(mu_s, 0LL)); __auto_type mu_b0 = __let_tmp_b0; __auto_type __let_tmp_span = ({ __auto_type __da0 = mu_b0; mu__cp_span__direct(__da0); }); __auto_type mu_span = __let_tmp_span; ((mu_span != mu_n) ? mu_d : ({ __auto_type __let_tmp_cp = ((mu_span == 1LL) ? mu_b0 : ((mu_span == 2LL) ? ((({ long long __sa = (((mu_b0) & (31LL))); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); }))) : ((mu_span == 3LL) ? ((((({ long long __sa = (((mu_b0) & (15LL))); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); }))) : ((((({ long long __sa = (((mu_b0) & (7LL))); long long __sn = (18LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (((({ long long __sa = (({ __auto_type __da0 = mu_s; __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = mu_s; __auto_type __da1 = 3LL; mu__cp_cont__direct(__da0, __da1); })))))))); __auto_type mu_cp = __let_tmp_cp; __auto_type __let_tmp_shortest = ((mu_span == 1LL) ? (mu_cp < 128LL) : ((mu_span == 2LL) ? (mu_cp >= 128LL) : ((mu_span == 3LL) ? (mu_cp >= 2048LL) : (mu_cp >= 65536LL)))); __auto_type mu_shortest = __let_tmp_shortest; ((({ __auto_type __da0 = mu_cp; mu__cp_bad__direct(__da0); }) || (!(mu_shortest))) ? mu_d : mu_cp); })); })); });
}

static long long mu_codepoint_at__direct(const char* mu_s, long long mu_i) {
  return ({ __auto_type __da0 = ({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_i; mu_utf8_at__direct(__da0, __da1); }); mu_codepoint_of__direct(__da0); });
}

static const char* mu__u8_slice__direct(list_str mu_cs, long long mu_start, long long mu_len, const char* mu_acc) {
  const char* __mere_ret;
  __auto_type __mere_a0 = &mu_cs;
  __auto_type __mere_a1 = &mu_start;
  __auto_type __mere_a2 = &mu_len;
  __auto_type __mere_a3 = &mu_acc;
  __mere_tail: ;
  return ({ __auto_type __scrut = mu_cs; (list_str__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; ((mu_start > 0LL) ? ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = (mu_start - 1LL); __auto_type __mt2 = mu_len; __auto_type __mt3 = mu_acc; *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }) : ((mu_len > 0LL) ? ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = 0LL; __auto_type __mt2 = (mu_len - 1LL); __auto_type __mt3 = __lang_str_concat(mu_acc, mu_h); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }) : mu_acc)); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}

static const char* mu_utf8_sub__direct(const char* mu_s, long long mu_start, long long mu_len) {
  return ({ __auto_type __da0 = __lang_utf8_chars(mu_s); __auto_type __da1 = mu_start; __auto_type __da2 = mu_len; __auto_type __da3 = (__sl_0.s); mu__u8_slice__direct(__da0, __da1, __da2, __da3); });
}

static const char* mu__u8_rev_join__direct(list_str mu_cs, const char* mu_acc) {
  const char* __mere_ret;
  __auto_type __mere_a0 = &mu_cs;
  __auto_type __mere_a1 = &mu_acc;
  __mere_tail: ;
  return ({ __auto_type __scrut = mu_cs; (list_str__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; ({ __auto_type __mt0 = mu_t; __auto_type __mt1 = __lang_str_concat(mu_h, mu_acc); *__mere_a0 = __mt0; *__mere_a1 = __mt1; goto __mere_tail; __mere_ret; }); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}

static const char* mu_utf8_rev__direct(const char* mu_s) {
  return ({ __auto_type __da0 = __lang_utf8_chars(mu_s); __auto_type __da1 = (__sl_0.s); mu__u8_rev_join__direct(__da0, __da1); });
}

static long long mu__eaw_width__direct(long long mu_cp) {
  return (((mu_cp >= 768LL) && (mu_cp <= 879LL)) ? 0LL : ((mu_cp == 12351LL) ? 1LL : (({ int __bc16 = ((mu_cp >= 4352LL)); int __bc17 = __bc16 ? (((mu_cp <= 4447LL)) ? 1 : 0) : 0; int __bc18 = __bc17 ? 1 : ((((mu_cp >= 11904LL) && (mu_cp <= 42191LL))) ? 1 : 0); int __bc19 = __bc18 ? 1 : ((((mu_cp >= 43360LL) && (mu_cp <= 43391LL))) ? 1 : 0); int __bc20 = __bc19 ? 1 : ((((mu_cp >= 44032LL) && (mu_cp <= 55203LL))) ? 1 : 0); int __bc21 = __bc20 ? 1 : ((((mu_cp >= 63744LL) && (mu_cp <= 64255LL))) ? 1 : 0); int __bc22 = __bc21 ? 1 : ((((mu_cp >= 65040LL) && (mu_cp <= 65049LL))) ? 1 : 0); int __bc23 = __bc22 ? 1 : ((((mu_cp >= 65072LL) && (mu_cp <= 65135LL))) ? 1 : 0); int __bc24 = __bc23 ? 1 : ((((mu_cp >= 65280LL) && (mu_cp <= 65376LL))) ? 1 : 0); int __bc25 = __bc24 ? 1 : ((((mu_cp >= 65504LL) && (mu_cp <= 65510LL))) ? 1 : 0); int __bc26 = __bc25 ? 1 : ((((mu_cp >= 127744LL) && (mu_cp <= 128767LL))) ? 1 : 0); int __bc27 = __bc26 ? 1 : ((((mu_cp >= 129280LL) && (mu_cp <= 129535LL))) ? 1 : 0); int __bc28 = __bc27 ? 1 : ((((mu_cp >= 131072LL) && (mu_cp <= 262141LL))) ? 1 : 0); __bc28; }) ? 2LL : 1LL)));
}

static long long mu__u8w_go__direct(const char* mu_s, long long mu_i, long long mu_n, long long mu_acc) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_s;
  __auto_type __mere_a1 = &mu_i;
  __auto_type __mere_a2 = &mu_n;
  __auto_type __mere_a3 = &mu_acc;
  __mere_tail: ;
  return ((mu_i >= mu_n) ? mu_acc : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at(mu_s, mu_i)); __auto_type mu_b0 = __let_tmp_b0; ((mu_b0 < 194LL) ? ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = (mu_i + 1LL); __auto_type __mt2 = mu_n; __auto_type __mt3 = (mu_acc + 1LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }) : ((mu_b0 < 224LL) ? (((mu_i + 1LL) < mu_n) ? ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = (mu_i + 2LL); __auto_type __mt2 = mu_n; __auto_type __mt3 = (mu_acc + ({ __auto_type __da0 = ((__lang_imod(mu_b0, 32LL) * 64LL) + __lang_imod(__lang_ord(__lang_char_at(mu_s, (mu_i + 1LL))), 64LL)); mu__eaw_width__direct(__da0); })); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }) : (mu_acc + 1LL)) : ((mu_b0 < 240LL) ? (((mu_i + 2LL) < mu_n) ? ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = (mu_i + 3LL); __auto_type __mt2 = mu_n; __auto_type __mt3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc40 = mu_b0; __auto_type __bc41 = 16LL; __auto_type __bc42 = __lang_imod(__bc40, __bc41); __auto_type __bc43 = 4096LL; __auto_type __bc44 = (__bc42 * __bc43); __auto_type __bc45 = (__lang_imod(__lang_ord(__lang_char_at(mu_s, (mu_i + 1LL))), 64LL) * 64LL); __auto_type __bc46 = (__bc44 + __bc45); __auto_type __bc47 = __lang_imod(__lang_ord(__lang_char_at(mu_s, (mu_i + 2LL))), 64LL); __auto_type __bc48 = (__bc46 + __bc47); __bc48; }); mu__eaw_width__direct(__da0); })); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }) : (mu_acc + 1LL)) : (((mu_i + 3LL) < mu_n) ? ({ __auto_type __mt0 = mu_s; __auto_type __mt1 = (mu_i + 4LL); __auto_type __mt2 = mu_n; __auto_type __mt3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc29 = mu_b0; __auto_type __bc30 = 8LL; __auto_type __bc31 = __lang_imod(__bc29, __bc30); __auto_type __bc32 = 262144LL; __auto_type __bc33 = (__bc31 * __bc32); __auto_type __bc34 = (__lang_imod(__lang_ord(__lang_char_at(mu_s, (mu_i + 1LL))), 64LL) * 4096LL); __auto_type __bc35 = (__bc33 + __bc34); __auto_type __bc36 = (__lang_imod(__lang_ord(__lang_char_at(mu_s, (mu_i + 2LL))), 64LL) * 64LL); __auto_type __bc37 = (__bc35 + __bc36); __auto_type __bc38 = __lang_imod(__lang_ord(__lang_char_at(mu_s, (mu_i + 3LL))), 64LL); __auto_type __bc39 = (__bc37 + __bc38); __bc39; }); mu__eaw_width__direct(__da0); })); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; *__mere_a3 = __mt3; goto __mere_tail; __mere_ret; }) : (mu_acc + 1LL))))); }));
}

static long long mu_utf8_width__direct(const char* mu_s) {
  return ({ __auto_type __da0 = mu_s; __auto_type __da1 = 0LL; __auto_type __da2 = ((long long) __lang_str_size(mu_s)); __auto_type __da3 = 0LL; mu__u8w_go__direct(__da0, __da1, __da2, __da3); });
}

static const char* mu_pad_right__direct(const char* mu_s, long long mu_w) {
  return ({ __auto_type __let_tmp_d = (mu_w - ({ __auto_type __da0 = mu_s; mu_utf8_width__direct(__da0); })); __auto_type mu_d = __let_tmp_d; ((mu_d <= 0LL) ? mu_s : __lang_str_concat(mu_s, __lang_str_repeat((__sl_4.s), mu_d))); });
}

static const char* mu_pad_left__direct(const char* mu_s, long long mu_w) {
  return ({ __auto_type __let_tmp_d = (mu_w - ({ __auto_type __da0 = mu_s; mu_utf8_width__direct(__da0); })); __auto_type mu_d = __let_tmp_d; ((mu_d <= 0LL) ? mu_s : __lang_str_concat(__lang_str_repeat((__sl_4.s), mu_d), mu_s)); });
}

static long long mu_sign__direct(long long mu_n) {
  return ((mu_n > 0LL) ? 1LL : ((mu_n < 0LL) ? (0LL - 1LL) : 0LL));
}

static long long mu_incr__direct(long long mu_n) {
  return (mu_n + 1LL);
}

static long long mu_decr__direct(long long mu_n) {
  return (mu_n - 1LL);
}

static long long mu_square__direct(long long mu_n) {
  return (mu_n * mu_n);
}

static long long mu_cube__direct(long long mu_n) {
  return ((mu_n * mu_n) * mu_n);
}

static long long mu_sum_range__direct(long long mu_a, long long mu_b) {
  return ((mu_a > mu_b) ? 0LL : ({ __auto_type __let_tmp_n = ((mu_b - mu_a) + 1LL); __auto_type mu_n = __let_tmp_n; ((__lang_imod(mu_n, 2LL) == 0LL) ? (__lang_idiv(mu_n, 2LL) * (mu_a + mu_b)) : (mu_n * __lang_idiv((mu_a + mu_b), 2LL))); }));
}

static long long mu__pow_go__direct(long long mu_acc, long long mu_sq, long long mu_n) {
  long long __mere_ret;
  __auto_type __mere_a0 = &mu_acc;
  __auto_type __mere_a1 = &mu_sq;
  __auto_type __mere_a2 = &mu_n;
  __mere_tail: ;
  return ((mu_n == 0LL) ? mu_acc : ((__lang_imod(mu_n, 2LL) == 1LL) ? ({ __auto_type __mt0 = (mu_acc * mu_sq); __auto_type __mt1 = (mu_sq * mu_sq); __auto_type __mt2 = __lang_idiv(mu_n, 2LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; }) : ({ __auto_type __mt0 = mu_acc; __auto_type __mt1 = (mu_sq * mu_sq); __auto_type __mt2 = __lang_idiv(mu_n, 2LL); *__mere_a0 = __mt0; *__mere_a1 = __mt1; *__mere_a2 = __mt2; goto __mere_tail; __mere_ret; })));
}

static long long mu_pow__direct(long long mu_b, long long mu_e) {
  return ((mu_e < 0LL) ? __lang_fail_int(__lang_str_concat((__sl_2.s), __lang_str_concat((__sl_5.s), show_int(mu_e)))) : ({ __auto_type __da0 = 1LL; __auto_type __da1 = mu_b; __auto_type __da2 = mu_e; mu__pow_go__direct(__da0, __da1, __da2); }));
}

static long long mu_lcm__direct(long long mu_a, long long mu_b) {
  return (((mu_a == 0LL) || (mu_b == 0LL)) ? 0LL : ({ long long __a = ((__lang_idiv(mu_a, __lang_gcd(mu_a, mu_b)) * mu_b)); __a < 0 ? -__a : __a; }));
}

static tuple_int_int mu_divmod__direct(long long mu_a, long long mu_b) {
  return ((mu_b == 0LL) ? ({ __lang_fail_impl(__lang_str_concat((__sl_2.s), (__sl_6.s))); (tuple_int_int){0}; }) : ((tuple_int_int){.f0 = __lang_idiv(mu_a, mu_b), .f1 = __lang_imod(mu_a, mu_b)}));
}

static int mu_assert__direct(int mu_cond, const char* mu_msg) {
  return (mu_cond ? 0 : __lang_fail_int(__lang_str_concat((__sl_2.s), __lang_str_concat((__sl_7.s), mu_msg))));
}

static long long mu_crc_byte__direct(long long mu_crc, long long mu_b) {
  return ({ __auto_type __let_tmp_x = ((mu_crc) ^ (mu_b)); __auto_type mu_x = __let_tmp_x; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_x; __lifted_go_0__direct(__ia0, __ia1); }); });
}

static long long mu_crc32_vec__direct(mere_vec_int* mu_bytes) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_bytes); __auto_type mu_n = __let_tmp_n; ((((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_bytes))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32; __lifted_go__rvfast_1__direct(mu_n, mu_bytes, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32; __lifted_go_uq1_2__direct(mu_n, mu_bytes, __ia0, __ia1); }))) ^ (mu_mask32)); });
}

static mere_vec_int* mu_br_new__int__Vec_int__direct(__lang_region* __rp4010, long long mu_pos) {
  return ({ __auto_type __let_tmp_s = mere_vec_int_new(__rp4010); __auto_type mu_s = __let_tmp_s; (void)(mere_vec_int_push(mu_s, mu_pos)); (void)(mere_vec_int_push(mu_s, 0LL)); (void)(mere_vec_int_push(mu_s, 0LL)); mu_s; });
}

static long long mu_br_bit__direct(mere_vec_int* mu_s, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_cnt = mere_vec_int_get(mu_s, 2LL); __auto_type mu_cnt = __let_tmp_cnt; (void)(((mu_cnt == 0LL) ? ({ __auto_type __let_tmp_p = mere_vec_int_get(mu_s, 0LL); __auto_type mu_p = __let_tmp_p; (void)(mere_vec_int_set(mu_s, 1LL, mere_vec_int_get(mu_data, mu_p))); (void)(mere_vec_int_set(mu_s, 0LL, (mu_p + 1LL))); mere_vec_int_set(mu_s, 2LL, 8LL); }) : 0)); __auto_type __let_tmp_buf = mere_vec_int_get(mu_s, 1LL); __auto_type mu_buf = __let_tmp_buf; (void)(mere_vec_int_set(mu_s, 1LL, ({ long long __sa = (mu_buf); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); }))); (void)(mere_vec_int_set(mu_s, 2LL, (mere_vec_int_get(mu_s, 2LL) - 1LL))); ((mu_buf) & (1LL)); });
}

static long long mu_br_bits__direct(mere_vec_int* mu_s, mere_vec_int* mu_data, long long mu_n) {
  return ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq2_3__direct(mu_n, mu_s, mu_data, __ia0, __ia1); });
}

static int mu_br_align__direct(mere_vec_int* mu_s) {
  return ({ (void)(mere_vec_int_set(mu_s, 1LL, 0LL)); mere_vec_int_set(mu_s, 2LL, 0LL); });
}

static tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_region* __rp4060, __lang_region* __rp4091, mere_vec_int* mu_lengths) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_lengths); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_count = mere_vec_int_new(__rp4060); __auto_type mu_count = __let_tmp_count; ({ (void)(__lifted_zc_4(mu_count, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_lengths))) ? __lifted_tally__rvfast_5(mu_n, mu_lengths, mu_count, 0LL) : __lifted_tally_6(mu_n, mu_lengths, mu_count, 0LL))); (void)(mere_vec_int_set(mu_count, 0LL, 0LL)); __auto_type __let_tmp_syms = mere_vec_int_new(__rp4091); __auto_type mu_syms = __let_tmp_syms; ({ (void)(__lifted_bylen_7(mu_n, mu_lengths, mu_syms, 1LL)); ((tuple_Vec_int_Vec_int){.f0 = mu_count, .f1 = mu_syms}); }); }); }); });
}

static long long mu_huff_decode__direct(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_count, mere_vec_int* mu_syms) {
  return ((((1LL >= 0LL) && (1LL <= 16LL)) && (16LL <= mere_vec_int_len(mu_count))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go__rvfast_uq3_9__direct(mu_s, mu_data, mu_count, mu_syms, __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go_uq4_10__direct(mu_s, mu_data, mu_count, mu_syms, __ia0, __ia1, __ia2, __ia3); }));
}

static mere_vec_int* mu_vec_of__direct(list_int mu_xs) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq5_11(mu_v, mu_xs); });
}

static mere_vec_int* mu_fixed_lit_lengths__direct(int mu__u) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq6_12(mu_v, 0LL); });
}

static mere_vec_int* mu_fixed_dist_lengths__direct(int mu__u) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq7_13(mu_v, 0LL); });
}

static int mu_inflate_codes__direct(mere_vec_int* mu_s, mere_vec_int* mu_data, mere_vec_int* mu_out, mere_vec_int* mu_lcount, mere_vec_int* mu_lsyms, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms) {
  return __lifted_loop_14(mu_s, mu_data, mu_lcount, mu_lsyms, mu_out, mu_len_base, mu_len_extra, mu_dcount, mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}

static tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_region* __rp4446, __lang_region* __rp4450, mere_vec_int* mu_s, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_hlit = (({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 5LL; mu_br_bits__direct(__da0, __da1, __da2); }) + 257LL); __auto_type mu_hlit = __let_tmp_hlit; __auto_type __let_tmp_hdist = (({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 5LL; mu_br_bits__direct(__da0, __da1, __da2); }) + 1LL); __auto_type mu_hdist = __let_tmp_hdist; __auto_type __let_tmp_hclen = (({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_data; __auto_type __da2 = 4LL; mu_br_bits__direct(__da0, __da1, __da2); }) + 4LL); __auto_type mu_hclen = __let_tmp_hclen; __auto_type __let_tmp_cl_lengths = mere_vec_int_new(__lang_current_region); __auto_type mu_cl_lengths = __let_tmp_cl_lengths; ({ (void)(__lifted_zero19_16(mu_cl_lengths, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= mu_hclen)) && (mu_hclen <= mere_vec_int_len(mu_clen_order))) ? __lifted_rd__rvfast_17(mu_hclen, mu_cl_lengths, mu_clen_order, mu_s, mu_data, 0LL) : __lifted_rd_18(mu_hclen, mu_cl_lengths, mu_clen_order, mu_s, mu_data, 0LL))); ({ __auto_type __let_tup = ({ __auto_type __da0 = mu_cl_lengths; mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0); }); __auto_type mu_clc = (__let_tup).f0; __auto_type mu_cls = (__let_tup).f1; ({ __auto_type __let_tmp_lengths = mere_vec_int_new(__lang_current_region); __auto_type mu_lengths = __let_tmp_lengths; __auto_type __let_tmp_total = (mu_hlit + mu_hdist); __auto_type mu_total = __let_tmp_total; ({ (void)(__lifted_fill_19(mu_lengths, mu_total, mu_s, mu_data, mu_clc, mu_cls, 0)); __auto_type __let_tmp_llen = mere_vec_int_new(__rp4446); __auto_type mu_llen = __let_tmp_llen; __auto_type __let_tmp_dlen = mere_vec_int_new(__rp4450); __auto_type mu_dlen = __let_tmp_dlen; ({ (void)(__lifted_split_23(mu_total, mu_hlit, mu_llen, mu_lengths, mu_dlen, 0LL)); ((tuple_Vec_int_Vec_int){.f0 = mu_llen, .f1 = mu_dlen}); }); }); }); }); }); }); });
}

static mere_vec_int* mu_inflate__direct(mere_vec_int* mu_data, long long mu_start) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_s = ({ __auto_type __da0 = mu_start; mu_br_new__int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_s = __let_tmp_s; __lifted_block_24(mu_s, mu_data, mu_out, 0); });
}

static long long mu_gzip_body_start__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_flg = mere_vec_int_get(mu_data, 3LL); __auto_type mu_flg = __let_tmp_flg; __auto_type __let_tmp_off10 = 10LL; __auto_type mu_off10 = __let_tmp_off10; __auto_type __let_tmp_off_extra = ((((mu_flg) & (4LL)) != 0LL) ? ({ __auto_type __let_tmp_xlen = ({ __auto_type __bc49 = mere_vec_int_get(mu_data, mu_off10); __auto_type __bc50 = ({ long long __sa = (mere_vec_int_get(mu_data, (mu_off10 + 1LL))); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc51 = (__bc49 + __bc50); __bc51; }); __auto_type mu_xlen = __let_tmp_xlen; ((mu_off10 + 2LL) + mu_xlen); }) : mu_off10); __auto_type mu_off_extra = __let_tmp_off_extra; ({ __auto_type __let_tmp_off_name = ((((mu_flg) & (8LL)) != 0LL) ? __lifted_skip_z_26(mu_data, mu_off_extra) : mu_off_extra); __auto_type mu_off_name = __let_tmp_off_name; __auto_type __let_tmp_off_comm = ((((mu_flg) & (16LL)) != 0LL) ? __lifted_skip_z_26(mu_data, mu_off_name) : mu_off_name); __auto_type mu_off_comm = __let_tmp_off_comm; ((((mu_flg) & (2LL)) != 0LL) ? (mu_off_comm + 2LL) : mu_off_comm); }); });
}

static int mu_is_gzip__direct(mere_vec_int* mu_data) {
  return ((((mere_vec_int_len(mu_data) >= 18LL) && (mere_vec_int_get(mu_data, 0LL) == 31LL)) && (mere_vec_int_get(mu_data, 1LL) == 139LL)) && (mere_vec_int_get(mu_data, 2LL) == 8LL));
}

static mere_vec_int* mu_gunzip__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = ({ __auto_type __da0 = mu_data; mu_gzip_body_start__direct(__da0); }); mu_inflate__direct(__da0, __da1); });
}

static long long mu_gzip_trailer_u32__direct(mere_vec_int* mu_data, long long mu_o) {
  return ({ __auto_type __bc52 = mere_vec_int_get(mu_data, mu_o); __auto_type __bc53 = ({ long long __sa = (mere_vec_int_get(mu_data, (mu_o + 1LL))); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc54 = (__bc52 + __bc53); __auto_type __bc55 = ({ long long __sa = (mere_vec_int_get(mu_data, (mu_o + 2LL))); long long __sn = (16LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc56 = (__bc54 + __bc55); __auto_type __bc57 = ({ long long __sa = (mere_vec_int_get(mu_data, (mu_o + 3LL))); long long __sn = (24LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc58 = (__bc56 + __bc57); __bc58; });
}

static int mu_gunzip_ok__direct(mere_vec_int* mu_data, mere_vec_int* mu_out) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; ((({ __auto_type __da0 = mu_out; mu_crc32_vec__direct(__da0); }) == ({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_n - 8LL); mu_gzip_trailer_u32__direct(__da0, __da1); })) && (((mere_vec_int_len(mu_out)) & (mu_mask32)) == ({ __auto_type __da0 = mu_data; __auto_type __da1 = (mu_n - 4LL); mu_gzip_trailer_u32__direct(__da0, __da1); }))); });
}

static mere_vec_int* mu_new_vec__direct(__lang_region* __rp4583, long long mu_n, long long mu_x) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__rp4583); __auto_type mu_v = __let_tmp_v; ({ (void)(__lifted_go_uq11_27(mu_n, mu_v, mu_x, 0LL)); mu_v; }); });
}

static int mu_is_png__direct(mere_vec_int* mu_data) {
  return ((mere_vec_int_len(mu_data) < 8LL) ? 0 : (((((0LL >= 0LL) && (0LL <= 8LL)) && (8LL <= mere_vec_int_len(mu_png_sig))) && (8LL <= mere_vec_int_len(mu_data))) ? __lifted_go__rvfast_uq13_28(mu_data, mu_png_sig, 0LL) : __lifted_go_uq14_29(mu_data, mu_png_sig, 0LL)));
}

static long long mu_u32be__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __bc59 = mere_vec_int_get(mu_data, mu_i); __auto_type __bc60 = 16777216LL; __auto_type __bc61 = (__bc59 * __bc60); __auto_type __bc62 = (mere_vec_int_get(mu_data, (mu_i + 1LL)) * 65536LL); __auto_type __bc63 = (__bc61 + __bc62); __auto_type __bc64 = (mere_vec_int_get(mu_data, (mu_i + 2LL)) * 256LL); __auto_type __bc65 = (__bc63 + __bc64); __auto_type __bc66 = mere_vec_int_get(mu_data, (mu_i + 3LL)); __auto_type __bc67 = (__bc65 + __bc66); __bc67; });
}

static int mu_type_is__direct(mere_vec_int* mu_data, long long mu_i, long long mu_a, long long mu_b, long long mu_c, long long mu_d) {
  return ((((mere_vec_int_get(mu_data, mu_i) == mu_a) && (mere_vec_int_get(mu_data, (mu_i + 1LL)) == mu_b)) && (mere_vec_int_get(mu_data, (mu_i + 2LL)) == mu_c)) && (mere_vec_int_get(mu_data, (mu_i + 3LL)) == mu_d));
}

static int mu_is_ihdr__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 73LL; __auto_type __da3 = 72LL; __auto_type __da4 = 68LL; __auto_type __da5 = 82LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static int mu_is_idat__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 73LL; __auto_type __da3 = 68LL; __auto_type __da4 = 65LL; __auto_type __da5 = 84LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static int mu_is_iend__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 73LL; __auto_type __da3 = 69LL; __auto_type __da4 = 78LL; __auto_type __da5 = 68LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static int mu_is_plte__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 80LL; __auto_type __da3 = 76LL; __auto_type __da4 = 84LL; __auto_type __da5 = 69LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static int mu_is_text__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 116LL; __auto_type __da3 = 69LL; __auto_type __da4 = 88LL; __auto_type __da5 = 116LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static int mu_is_gama__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 103LL; __auto_type __da3 = 65LL; __auto_type __da4 = 77LL; __auto_type __da5 = 65LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static int mu_is_trns__direct(mere_vec_int* mu_data, long long mu_i) {
  return ({ __auto_type __da0 = mu_data; __auto_type __da1 = mu_i; __auto_type __da2 = 116LL; __auto_type __da3 = 82LL; __auto_type __da4 = 78LL; __auto_type __da5 = 83LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}

static mere_vec_int* mu_read_header__Vec_int__Vec_int__direct(__lang_region* __rp4699, mere_vec_int* mu_data) {
  return __lifted_walk_30(mu_data, 8LL);
}

static long long mu_channels__direct(long long mu_colour) {
  return ((mu_colour == 0LL) ? 1LL : ((mu_colour == 2LL) ? 3LL : ((mu_colour == 3LL) ? 1LL : ((mu_colour == 4LL) ? 2LL : ((mu_colour == 6LL) ? 4LL : 0LL)))));
}

static long long mu_pixel_bytes__direct(long long mu_colour, long long mu_depth) {
  return ({ __auto_type __bc68 = ({ __auto_type __da0 = mu_colour; mu_channels__direct(__da0); }); __auto_type __bc69 = ((mu_depth == 16LL) ? 2LL : 1LL); __auto_type __bc70 = (__bc68 * __bc69); __bc70; });
}

static mere_vec_int* mu_palette__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq15_31(mu_n, mu_out, mu_data, 8LL); });
}

static mere_vec_int* mu_idat_bytes__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq17_33(mu_n, mu_out, mu_data, 8LL); });
}

static mere_vec_str* mu_text_chunks__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_str_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq19_35(mu_n, mu_out, mu_data, 8LL); });
}

static long long mu_gamma__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq20_38(mu_n, mu_data, 8LL); });
}

static int mu_has_transparency__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __lifted_walk_uq21_39(mu_n, mu_data, 8LL); });
}

static long long mu_paeth__direct(long long mu_a, long long mu_b, long long mu_c) {
  return ({ __auto_type __let_tmp_p = ((mu_a + mu_b) - mu_c); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_pa = ((mu_p >= mu_a) ? (mu_p - mu_a) : (mu_a - mu_p)); __auto_type mu_pa = __let_tmp_pa; __auto_type __let_tmp_pb = ((mu_p >= mu_b) ? (mu_p - mu_b) : (mu_b - mu_p)); __auto_type mu_pb = __let_tmp_pb; __auto_type __let_tmp_pc = ((mu_p >= mu_c) ? (mu_p - mu_c) : (mu_c - mu_p)); __auto_type mu_pc = __let_tmp_pc; (((mu_pa <= mu_pb) && (mu_pa <= mu_pc)) ? mu_a : ((mu_pb <= mu_pc) ? mu_b : mu_c)); });
}

static mere_bytebuf* mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_region* __rp4848, mere_vec_int* mu_raw, long long mu_start, long long mu_width, long long mu_height, long long mu_bpp) {
  return ({ __auto_type __let_tmp_stride = (mu_width * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (mu_stride * mu_height)); __auto_type mu_out = __let_tmp_out; __lifted_row_40(mu_height, mu_out, mu_start, mu_stride, mu_raw, mu_bpp, 0LL); });
}

static mere_bytebuf* mu_unfilter__Vec_int__int__int__int__ByteBuf__direct(__lang_region* __rp4853, mere_vec_int* mu_raw, long long mu_width, long long mu_height, long long mu_bpp) {
  return ({ __auto_type __da0 = mu_raw; __auto_type __da1 = 0LL; __auto_type __da2 = mu_width; __auto_type __da3 = mu_height; __auto_type __da4 = mu_bpp; mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__rp4853, __da0, __da1, __da2, __da3, __da4); });
}

static int mu_is_interlaced__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 5LL) && (mere_vec_int_get(mu_h, 4LL) == 1LL)); });
}

static long long mu_adam7_xstart__direct(long long mu_p_uq22) {
  return ((mu_p_uq22 == 0LL) ? 0LL : ((mu_p_uq22 == 1LL) ? 4LL : ((mu_p_uq22 == 2LL) ? 0LL : ((mu_p_uq22 == 3LL) ? 2LL : ((mu_p_uq22 == 4LL) ? 0LL : ((mu_p_uq22 == 5LL) ? 1LL : 0LL))))));
}

static long long mu_adam7_ystart__direct(long long mu_p_uq23) {
  return ((mu_p_uq23 == 0LL) ? 0LL : ((mu_p_uq23 == 1LL) ? 0LL : ((mu_p_uq23 == 2LL) ? 4LL : ((mu_p_uq23 == 3LL) ? 0LL : ((mu_p_uq23 == 4LL) ? 2LL : ((mu_p_uq23 == 5LL) ? 0LL : 1LL))))));
}

static long long mu_adam7_xstep__direct(long long mu_p_uq24) {
  return ((mu_p_uq24 == 0LL) ? 8LL : ((mu_p_uq24 == 1LL) ? 8LL : ((mu_p_uq24 == 2LL) ? 4LL : ((mu_p_uq24 == 3LL) ? 4LL : ((mu_p_uq24 == 4LL) ? 2LL : ((mu_p_uq24 == 5LL) ? 2LL : 1LL))))));
}

static long long mu_adam7_ystep__direct(long long mu_p_uq25) {
  return ((mu_p_uq25 == 0LL) ? 8LL : ((mu_p_uq25 == 1LL) ? 8LL : ((mu_p_uq25 == 2LL) ? 8LL : ((mu_p_uq25 == 3LL) ? 4LL : ((mu_p_uq25 == 4LL) ? 4LL : ((mu_p_uq25 == 5LL) ? 2LL : 2LL))))));
}

static long long mu_ceil_div__direct(long long mu_a, long long mu_b) {
  return __lang_idiv(((mu_a + mu_b) - 1LL), mu_b);
}

static long long mu_pass_width__direct(long long mu_p_uq26, long long mu_width) {
  return ({ __auto_type __da0 = (mu_width - ({ __auto_type __da0 = mu_p_uq26; mu_adam7_xstart__direct(__da0); })); __auto_type __da1 = ({ __auto_type __da0 = mu_p_uq26; mu_adam7_xstep__direct(__da0); }); mu_ceil_div__direct(__da0, __da1); });
}

static long long mu_pass_height__direct(long long mu_p_uq27, long long mu_height) {
  return ({ __auto_type __da0 = (mu_height - ({ __auto_type __da0 = mu_p_uq27; mu_adam7_ystart__direct(__da0); })); __auto_type __da1 = ({ __auto_type __da0 = mu_p_uq27; mu_adam7_ystep__direct(__da0); }); mu_ceil_div__direct(__da0, __da1); });
}

static mere_bytebuf* mu_deinterlace__Vec_int__int__int__int__ByteBuf__direct(__lang_region* __rp4899, mere_vec_int* mu_raw, long long mu_width, long long mu_height, long long mu_bpp) {
  return ({ __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, ((mu_width * mu_height) * mu_bpp)); __auto_type mu_out = __let_tmp_out; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_pass_43__direct(mu_out, mu_width, mu_height, mu_raw, mu_bpp, __ia0, __ia1); }); });
}

static mere_bytebuf* mu_decode__Vec_int__ByteBuf__direct(__lang_region* __rp4923, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 0LL) ? mere_bytebuf_new(__rp4923, 0LL) : ({ __auto_type __let_tmp_width = mere_vec_int_get(mu_h, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_h, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_bpp = ({ __auto_type __da0 = mere_vec_int_get(mu_h, 3LL); __auto_type __da1 = mere_vec_int_get(mu_h, 2LL); mu_pixel_bytes__direct(__da0, __da1); }); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_raw = ({ __auto_type __da0 = ({ __auto_type __da0 = mu_data; mu_idat_bytes__direct(__da0); }); __auto_type __da1 = 2LL; mu_inflate__direct(__da0, __da1); }); __auto_type mu_raw = __let_tmp_raw; ((mere_vec_int_get(mu_h, 4LL) == 1LL) ? ({ __auto_type __da0 = mu_raw; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = mu_bpp; mu_deinterlace__Vec_int__int__int__int__ByteBuf__direct(__rp4923, __da0, __da1, __da2, __da3); }) : ({ __auto_type __da0 = mu_raw; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = mu_bpp; mu_unfilter__Vec_int__int__int__int__ByteBuf__direct(__rp4923, __da0, __da1, __da2, __da3); })); })); });
}

static mere_vec_int* mu_rgb8__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; __auto_type __let_tmp_width = mere_vec_int_get(mu_h, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_h, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_depth = mere_vec_int_get(mu_h, 2LL); __auto_type mu_depth = __let_tmp_depth; __auto_type __let_tmp_colour = mere_vec_int_get(mu_h, 3LL); __auto_type mu_colour = __let_tmp_colour; __auto_type __let_tmp_bpp = ({ __auto_type __da0 = mu_colour; __auto_type __da1 = mu_depth; mu_pixel_bytes__direct(__da0, __da1); }); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_step = ((mu_depth == 16LL) ? 2LL : 1LL); __auto_type mu_step = __let_tmp_step; __auto_type __let_tmp_pixels = ({ __auto_type __da0 = mu_data; mu_decode__Vec_int__ByteBuf__direct(__lang_current_region, __da0); }); __auto_type mu_pixels = __let_tmp_pixels; __auto_type __let_tmp_pal = ({ __auto_type __da0 = mu_data; mu_palette__direct(__da0); }); __auto_type mu_pal = __let_tmp_pal; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __lifted_go_uq29_47(mu_width, mu_height, mu_out, mu_bpp, mu_pixels, mu_colour, mu_pal, mu_step, 0LL); });
}

static mere_vec_int* mu_vec_of__v2__direct(list_int mu_xs) {
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; __lifted_go_uq30_48(mu_v, mu_xs); });
}

static long long mu_crc_byte__v2__direct(long long mu_crc, long long mu_b) {
  return ({ __auto_type __let_tmp_x = ((mu_crc) ^ (mu_b)); __auto_type mu_x = __let_tmp_x; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_x; __lifted_go_uq31_49__direct(__ia0, __ia1); }); });
}

static long long mu_crc32_vec__v2__direct(mere_vec_int* mu_bytes) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_bytes); __auto_type mu_n = __let_tmp_n; ((((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_bytes))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32__v2; __lifted_go__rvfast_uq32_50__direct(mu_n, mu_bytes, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_mask32__v2; __lifted_go_uq33_51__direct(mu_n, mu_bytes, __ia0, __ia1); }))) ^ (mu_mask32__v2)); });
}

static mere_vec_int* mu_bw_new__unit__Vec_int__direct(__lang_region* __rp5155, int mu__u) {
  return ({ __auto_type __let_tmp_s = mere_vec_int_new(__rp5155); __auto_type mu_s = __let_tmp_s; (void)(mere_vec_int_push(mu_s, 0LL)); (void)(mere_vec_int_push(mu_s, 0LL)); mu_s; });
}

static int mu_bw_bits__direct(mere_vec_int* mu_s, mere_vec_int* mu_out, long long mu_v, long long mu_n) {
  return __lifted_go_uq34_52(mu_n, mu_v, mu_s, mu_out, 0LL);
}

static int mu_bw_flush__direct(mere_vec_int* mu_s, mere_vec_int* mu_out) {
  return ((mere_vec_int_get(mu_s, 1LL) > 0LL) ? ({ (void)(mere_vec_int_push(mu_out, ((mere_vec_int_get(mu_s, 0LL)) & (255LL)))); (void)(mere_vec_int_set(mu_s, 0LL, 0LL)); mere_vec_int_set(mu_s, 1LL, 0LL); }) : 0);
}

static long long mu_rev_bits__direct(long long mu_v, long long mu_n) {
  return ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq35_53__direct(mu_n, mu_v, __ia0, __ia1); });
}

static tuple_int_int_int mu_len_encode__direct(long long mu_len) {
  return ((((((0LL >= 0LL) && (0LL <= 28LL)) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_g_len_base))) && ((((28LL - 1LL) + 1LL) >= 0LL) && ((((28LL - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_g_len_base))))) && (28LL <= mere_vec_int_len(mu_g_len_extra))) && (28LL <= mere_vec_int_len(mu_g_len_base))) ? __lifted_go__rvfast_uq36_54(mu_g_len_base, mu_g_len_extra, mu_len, 0LL) : __lifted_go_uq37_55(mu_g_len_base, mu_g_len_extra, mu_len, 0LL));
}

static tuple_int_int_int mu_dist_encode__direct(long long mu_dist) {
  return ((((((0LL >= 0LL) && (0LL <= 29LL)) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_g_dist_base))) && ((((29LL - 1LL) + 1LL) >= 0LL) && ((((29LL - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_g_dist_base))))) && (29LL <= mere_vec_int_len(mu_g_dist_extra))) && (29LL <= mere_vec_int_len(mu_g_dist_base))) ? __lifted_go__rvfast_uq38_56(mu_g_dist_extra, mu_dist, mu_g_dist_base, 0LL) : __lifted_go_uq39_57(mu_g_dist_extra, mu_dist, mu_g_dist_base, 0LL));
}

static tuple_int_int mu_find_match__direct(mere_vec_int* mu_data, long long mu_pos, long long mu_n) {
  return ({ __auto_type __let_tmp_max_back = ((mu_pos > 32768LL) ? 32768LL : mu_pos); __auto_type mu_max_back = __let_tmp_max_back; ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __lifted_best_58__direct(mu_max_back, mu_pos, mu_n, mu_data, __ia0, __ia1, __ia2); }); });
}

static mere_vec_int* mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_region* __rp5340, mere_vec_int* mu_freq, long long mu_nsym, long long mu_maxbits) {
  return ({ __auto_type __let_tmp_nf = mere_vec_int_new(__lang_current_region); __auto_type mu_nf = __let_tmp_nf; __auto_type __let_tmp_nl = mere_vec_int_new(__lang_current_region); __auto_type mu_nl = __let_tmp_nl; __auto_type __let_tmp_nr = mere_vec_int_new(__lang_current_region); __auto_type mu_nr = __let_tmp_nr; ({ (void)(__lifted_seed_61(mu_nsym, mu_freq, mu_nf, mu_nl, mu_nr, 0LL)); __auto_type __let_tmp_syms = mere_vec_int_new(__lang_current_region); __auto_type mu_syms = __let_tmp_syms; ({ (void)(__lifted_seed2_62(mu_nsym, mu_freq, mu_syms, 0LL)); __auto_type __let_tmp_nleaves = mere_vec_int_len(mu_nf); __auto_type mu_nleaves = __let_tmp_nleaves; __auto_type __let_tmp_lengths = mere_vec_int_new(__rp5340); __auto_type mu_lengths = __let_tmp_lengths; ({ (void)(__lifted_zl_63(mu_nsym, mu_lengths, 0LL)); ((mu_nleaves == 0LL) ? mu_lengths : ((mu_nleaves == 1LL) ? ({ (void)(mere_vec_int_set(mu_lengths, mere_vec_int_get(mu_syms, 0LL), 1LL)); mu_lengths; }) : ({ __auto_type __let_tmp_alive = mere_vec_int_new(__lang_current_region); __auto_type mu_alive = __let_tmp_alive; ({ (void)(__lifted_al_64(mu_nleaves, mu_alive, 0LL)); __auto_type __let_tmp_leafsym = mere_vec_int_new(__lang_current_region); __auto_type mu_leafsym = __let_tmp_leafsym; ({ (void)(__lifted_ls_65(mu_nleaves, mu_leafsym, mu_syms, 0LL)); ({ (void)(__lifted_merge_66(mu_nf, mu_alive, mu_nl, mu_nr, mu_leafsym, mu_nleaves)); __auto_type __let_tmp_root = (mere_vec_int_len(mu_nf) - 1LL); __auto_type mu_root = __let_tmp_root; ({ (void)(({ __auto_type __ia0 = mu_root; __auto_type __ia1 = 0LL; __lifted_assign_70__direct(mu_leafsym, mu_lengths, mu_nl, mu_nr, __ia0, __ia1); })); (void)(mu_maxbits); mu_lengths; }); }); }); }); }))); }); }); }); });
}

static mere_vec_int* mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_region* __rp5499, mere_vec_int* mu_lengths, long long mu_n, long long mu_maxbits) {
  return ({ __auto_type __let_tmp_bl_count = mere_vec_int_new(__lang_current_region); __auto_type mu_bl_count = __let_tmp_bl_count; ({ (void)(__lifted_z_71(mu_maxbits, mu_bl_count, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_lengths))) ? __lifted_cnt__rvfast_72(mu_n, mu_lengths, mu_bl_count, 0LL) : __lifted_cnt_73(mu_n, mu_lengths, mu_bl_count, 0LL))); __auto_type __let_tmp_next_code = mere_vec_int_new(__lang_current_region); __auto_type mu_next_code = __let_tmp_next_code; ({ (void)(__lifted_z2_74(mu_maxbits, mu_next_code, 0LL)); ({ (void)((((((1LL >= 0LL) && (1LL <= (mu_maxbits + 1LL))) && ((mu_maxbits + 1LL) <= mere_vec_int_len(mu_next_code))) && ((((1LL - 1LL) >= 0LL) && (((1LL - 1LL) + 1LL) <= mere_vec_int_len(mu_bl_count))) && (((((mu_maxbits + 1LL) - 1LL) - 1LL) >= 0LL) && (((((mu_maxbits + 1LL) - 1LL) - 1LL) + 1LL) <= mere_vec_int_len(mu_bl_count))))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __lifted_fill__rvfast_75__direct(mu_maxbits, mu_bl_count, mu_next_code, __ia0, __ia1); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __lifted_fill_uq44_76__direct(mu_maxbits, mu_bl_count, mu_next_code, __ia0, __ia1); }))); __auto_type __let_tmp_codes = mere_vec_int_new(__rp5499); __auto_type mu_codes = __let_tmp_codes; ({ (void)(__lifted_z3_77(mu_n, mu_codes, 0LL)); ({ (void)((((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_codes))) && (mu_n <= mere_vec_int_len(mu_lengths))) ? __lifted_asg__rvfast_78(mu_n, mu_lengths, mu_next_code, mu_codes, 0LL) : __lifted_asg_79(mu_n, mu_lengths, mu_next_code, mu_codes, 0LL))); mu_codes; }); }); }); }); }); }); });
}

static long long mu_vmax__direct(mere_vec_int* mu_v, long long mu_n) {
  return ((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_v))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go__rvfast_uq45_80__direct(mu_n, mu_v, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq46_81__direct(mu_n, mu_v, __ia0, __ia1); }));
}

static mere_vec_int* mu_tokenize__Vec_int__Vec_int__direct(__lang_region* __rp5580, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_toks = mere_vec_int_new(__rp5580); __auto_type mu_toks = __let_tmp_toks; ({ (void)(__lifted_go_uq47_82(mu_n, mu_data, mu_toks, 0LL)); mu_toks; }); });
}

static mere_vec_int* mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_region* __rp5839, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_toks = ({ __auto_type __da0 = mu_data; mu_tokenize__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_toks = __let_tmp_toks; __auto_type __let_tmp_nt = mere_vec_int_len(mu_toks); __auto_type mu_nt = __let_tmp_nt; __auto_type __let_tmp_lfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_lfreq = __let_tmp_lfreq; ({ (void)(__lifted_zl_uq48_83(mu_lfreq, 0LL)); __auto_type __let_tmp_dfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_dfreq = __let_tmp_dfreq; ({ (void)(__lifted_zd_84(mu_dfreq, 0LL)); ({ (void)(((((((0LL >= 0LL) && (0LL <= mu_nt)) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 2LL) >= 0LL) && ((((mu_nt - 1LL) + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 1LL) >= 0LL) && ((((mu_nt - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && (mu_nt <= mere_vec_int_len(mu_toks))) ? __lifted_count__rvfast_85(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL) : __lifted_count_86(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL))); (void)(mere_vec_int_set(mu_lfreq, 256LL, (mere_vec_int_get(mu_lfreq, 256LL) + 1LL))); __auto_type __let_tmp_llen = ({ __auto_type __da0 = mu_lfreq; __auto_type __da1 = 286LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_llen = __let_tmp_llen; __auto_type __let_tmp_dlen = ({ __auto_type __da0 = mu_dfreq; __auto_type __da1 = 30LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_dlen = __let_tmp_dlen; (void)(((({ __auto_type __da0 = mu_dlen; __auto_type __da1 = 30LL; mu_vmax__direct(__da0, __da1); }) == 0LL) ? mere_vec_int_set(mu_dlen, 0LL, 1LL) : 0)); __auto_type __let_tmp_lcode = ({ __auto_type __da0 = mu_llen; __auto_type __da1 = 286LL; __auto_type __da2 = 15LL; mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_lcode = __let_tmp_lcode; __auto_type __let_tmp_dcode = ({ __auto_type __da0 = mu_dlen; __auto_type __da1 = 30LL; __auto_type __da2 = 15LL; mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_dcode = __let_tmp_dcode; ({ __auto_type __let_tmp_hlit = (({ __auto_type __ia0 = mu_llen; __auto_type __ia1 = 285LL; __auto_type __ia2 = 256LL; __lifted_last_nz_87__direct(__ia0, __ia1, __ia2); }) + 1LL); __auto_type mu_hlit = __let_tmp_hlit; __auto_type __let_tmp_hdist = (({ __auto_type __ia0 = mu_dlen; __auto_type __ia1 = 29LL; __auto_type __ia2 = 0LL; __lifted_last_nz_87__direct(__ia0, __ia1, __ia2); }) + 1LL); __auto_type mu_hdist = __let_tmp_hdist; __auto_type __let_tmp_cl_syms = mere_vec_int_new(__lang_current_region); __auto_type mu_cl_syms = __let_tmp_cl_syms; __auto_type __let_tmp_combined = mere_vec_int_new(__lang_current_region); __auto_type mu_combined = __let_tmp_combined; ({ (void)(__lifted_cc1_88(mu_hlit, mu_combined, mu_llen, 0LL)); ({ (void)(__lifted_cc2_89(mu_hdist, mu_combined, mu_dlen, 0LL)); __auto_type __let_tmp_cn = mere_vec_int_len(mu_combined); __auto_type mu_cn = __let_tmp_cn; ({ (void)(__lifted_rle_90(mu_cn, mu_combined, mu_cl_syms, 0LL)); __auto_type __let_tmp_clfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_clfreq = __let_tmp_clfreq; ({ (void)(__lifted_zc_uq49_94(mu_clfreq, 0LL)); __auto_type __let_tmp_cs_n = mere_vec_int_len(mu_cl_syms); __auto_type mu_cs_n = __let_tmp_cs_n; ({ (void)(((((0LL >= 0LL) && (0LL <= mu_cs_n)) && (mu_cs_n <= mere_vec_int_len(mu_cl_syms))) ? __lifted_ccount__rvfast_95(mu_cs_n, mu_cl_syms, mu_clfreq, 0LL) : __lifted_ccount_96(mu_cs_n, mu_cl_syms, mu_clfreq, 0LL))); __auto_type __let_tmp_cllen = ({ __auto_type __da0 = mu_clfreq; __auto_type __da1 = 19LL; __auto_type __da2 = 7LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_cllen = __let_tmp_cllen; __auto_type __let_tmp_clcode = ({ __auto_type __da0 = mu_cllen; __auto_type __da1 = 19LL; __auto_type __da2 = 7LL; mu_canon_codes__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_clcode = __let_tmp_clcode; ({ __auto_type __let_tmp_hclen = __lifted_last_cl_97(mu_cllen, mu_cl_order, 18LL); __auto_type mu_hclen = __let_tmp_hclen; __auto_type __let_tmp_out = mere_vec_int_new(__rp5839); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_s = ({ __auto_type __da0 = 0; mu_bw_new__unit__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_s = __let_tmp_s; (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = 1LL; __auto_type __da3 = 1LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = 2LL; __auto_type __da3 = 2LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = (mu_hlit - 257LL); __auto_type __da3 = 5LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = (mu_hdist - 1LL); __auto_type __da3 = 5LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = (mu_hclen - 4LL); __auto_type __da3 = 4LL; mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); ({ (void)(__lifted_emit_cl_98(mu_hclen, mu_s, mu_out, mu_cllen, mu_cl_order, 0LL)); ({ (void)(__lifted_emit_rle_99(mu_cs_n, mu_cl_syms, mu_s, mu_out, mu_clcode, mu_cllen, 0LL)); ({ (void)(__lifted_emit_data_100(mu_nt, mu_toks, mu_s, mu_out, mu_lcode, mu_llen, mu_dcode, mu_dlen, 0LL)); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; __auto_type __da2 = ({ __auto_type __da0 = mere_vec_int_get(mu_lcode, 256LL); __auto_type __da1 = mere_vec_int_get(mu_llen, 256LL); mu_rev_bits__direct(__da0, __da1); }); __auto_type __da3 = mere_vec_int_get(mu_llen, 256LL); mu_bw_bits__direct(__da0, __da1, __da2, __da3); })); (void)(({ __auto_type __da0 = mu_s; __auto_type __da1 = mu_out; mu_bw_flush__direct(__da0, __da1); })); mu_out; }); }); }); }); }); }); }); }); }); }); }); }); }); });
}

static mere_vec_int* mu_deflate_stored__Vec_int__Vec_int__direct(__lang_region* __rp5938, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__rp5938); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; ({ (void)(((mu_n == 0LL) ? ({ (void)(mere_vec_int_push(mu_out, 1LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 255LL)); (void)(mere_vec_int_push(mu_out, 255LL)); 0; }) : __lifted_blocks_101(mu_n, mu_out, mu_data, 0LL))); mu_out; }); });
}

static mere_vec_int* mu_deflate_body__Vec_int__Vec_int__direct(__lang_region* __rp6075, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_toks = ({ __auto_type __da0 = mu_data; mu_tokenize__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_toks = __let_tmp_toks; __auto_type __let_tmp_nt = mere_vec_int_len(mu_toks); __auto_type mu_nt = __let_tmp_nt; __auto_type __let_tmp_lfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_lfreq = __let_tmp_lfreq; ({ (void)(__lifted_zl_uq50_103(mu_lfreq, 0LL)); __auto_type __let_tmp_dfreq = mere_vec_int_new(__lang_current_region); __auto_type mu_dfreq = __let_tmp_dfreq; ({ (void)(__lifted_zd_uq51_104(mu_dfreq, 0LL)); ({ (void)(((((((0LL >= 0LL) && (0LL <= mu_nt)) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 2LL) >= 0LL) && ((((mu_nt - 1LL) + 2LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))) && ((((mu_nt - 1LL) + 1LL) >= 0LL) && ((((mu_nt - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_toks))))) && (mu_nt <= mere_vec_int_len(mu_toks))) ? __lifted_count__rvfast_uq52_105(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL) : __lifted_count_uq53_106(mu_nt, mu_toks, mu_lfreq, mu_dfreq, 0LL))); (void)(mere_vec_int_set(mu_lfreq, 256LL, (mere_vec_int_get(mu_lfreq, 256LL) + 1LL))); __auto_type __let_tmp_llen = ({ __auto_type __da0 = mu_lfreq; __auto_type __da1 = 286LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_llen = __let_tmp_llen; __auto_type __let_tmp_dlen = ({ __auto_type __da0 = mu_dfreq; __auto_type __da1 = 30LL; __auto_type __da2 = 15LL; mu_build_lengths__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); __auto_type mu_dlen = __let_tmp_dlen; (((({ __auto_type __da0 = mu_llen; __auto_type __da1 = 286LL; mu_vmax__direct(__da0, __da1); }) > 15LL) || (({ __auto_type __da0 = mu_dlen; __auto_type __da1 = 30LL; mu_vmax__direct(__da0, __da1); }) > 15LL)) ? ({ __auto_type __da0 = mu_data; mu_deflate_stored__Vec_int__Vec_int__direct(__rp6075, __da0); }) : ({ __auto_type __da0 = mu_data; mu_deflate_dynamic__Vec_int__Vec_int__direct(__rp6075, __da0); })); }); }); }); });
}

static mere_vec_int* mu_deflate_gzip__direct(__lang_region* __rp6080, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__rp6080); __auto_type mu_out = __let_tmp_out; (void)(mere_vec_int_push(mu_out, 31LL)); (void)(mere_vec_int_push(mu_out, 139LL)); (void)(mere_vec_int_push(mu_out, 8LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 0LL)); (void)(mere_vec_int_push(mu_out, 3LL)); __auto_type __let_tmp_body = ({ __auto_type __da0 = mu_data; mu_deflate_body__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_body = __let_tmp_body; __auto_type __let_tmp_bn = mere_vec_int_len(mu_body); __auto_type mu_bn = __let_tmp_bn; ({ (void)(__lifted_cp_uq54_107(mu_bn, mu_out, mu_body, 0LL)); (void)(__lifted_push_u32_108(mu_out, ({ __auto_type __da0 = mu_data; mu_crc32_vec__v2__direct(__da0); }))); (void)(__lifted_push_u32_108(mu_out, ((mere_vec_int_len(mu_data)) & (mu_mask32__v2)))); mu_out; }); });
}

static int mu_bytes_push4be__direct(mere_vec_int* mu_out, long long mu_v) {
  return ({ (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_v); long long __sn = (24LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_v); long long __sn = (16LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); (void)(mere_vec_int_push(mu_out, ((({ long long __sa = (mu_v); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); mere_vec_int_push(mu_out, ((mu_v) & (255LL))); });
}

static int mu_push_all__direct(mere_vec_int* mu_out, mere_vec_int* mu_src) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_src); __auto_type mu_n = __let_tmp_n; __lifted_go_uq55_109(mu_n, mu_out, mu_src, 0LL); });
}

static long long mu_adler32__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; ((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_data))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 1LL; __auto_type __ia2 = 0LL; __lifted_go__rvfast_uq56_110__direct(mu_n, mu_data, __ia0, __ia1, __ia2); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 1LL; __auto_type __ia2 = 0LL; __lifted_go_uq57_111__direct(mu_n, mu_data, __ia0, __ia1, __ia2); })); });
}

static int mu_chunk__direct(mere_vec_int* mu_out, long long mu_t0, long long mu_t1, long long mu_t2, long long mu_t3, mere_vec_int* mu_body) {
  return ({ (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, mu_t0)); (void)(mere_vec_int_push(mu_tagged, mu_t1)); (void)(mere_vec_int_push(mu_tagged, mu_t2)); (void)(mere_vec_int_push(mu_tagged, mu_t3)); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = mu_out; __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}

static long long mu_paeth_e__direct(long long mu_a, long long mu_b, long long mu_c) {
  return ({ __auto_type __let_tmp_p = ((mu_a + mu_b) - mu_c); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_pa = ((mu_p >= mu_a) ? (mu_p - mu_a) : (mu_a - mu_p)); __auto_type mu_pa = __let_tmp_pa; __auto_type __let_tmp_pb = ((mu_p >= mu_b) ? (mu_p - mu_b) : (mu_b - mu_p)); __auto_type mu_pb = __let_tmp_pb; __auto_type __let_tmp_pc = ((mu_p >= mu_c) ? (mu_p - mu_c) : (mu_c - mu_p)); __auto_type mu_pc = __let_tmp_pc; (((mu_pa <= mu_pb) && (mu_pa <= mu_pc)) ? mu_a : ((mu_pb <= mu_pc) ? mu_b : mu_c)); });
}

static mere_vec_int* mu_filter_row__direct(long long mu_kind, mere_vec_int* mu_row_uq58, mere_vec_int* mu_prev, long long mu_bpp) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_row_uq58); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __lifted_go_uq59_112(mu_n, mu_out, mu_row_uq58, mu_bpp, mu_prev, mu_kind, 0LL); });
}

static long long mu_row_cost__direct(mere_vec_int* mu_row_uq60) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_row_uq60); __auto_type mu_n = __let_tmp_n; ((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_row_uq60))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go__rvfast_uq61_113__direct(mu_n, mu_row_uq60, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq62_114__direct(mu_n, mu_row_uq60, __ia0, __ia1); })); });
}

static mere_vec_int* mu_best_filter__Vec_int__Vec_int__int__Vec_int__direct(__lang_region* __rp6275, mere_vec_int* mu_row_uq63, mere_vec_int* mu_prev, long long mu_bpp) {
  return ({ __auto_type __let_tmp_first = ({ __auto_type __da0 = 0LL; __auto_type __da1 = mu_row_uq63; __auto_type __da2 = mu_prev; __auto_type __da3 = mu_bpp; mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_first = __let_tmp_first; ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = ({ __auto_type __da0 = mu_first; mu_row_cost__direct(__da0); }); __auto_type __ia3 = mu_first; __lifted_go_uq64_115__direct(mu_row_uq63, mu_prev, mu_bpp, __ia0, __ia1, __ia2, __ia3); }); });
}

static int mu_all_grey__direct(mere_vec_int* mu_pixels, long long mu_n) {
  return __lifted_go_uq66_116(mu_n, mu_pixels, 0LL);
}

static mere_vec_int* mu_collect_palette__direct(mere_vec_int* mu_pixels, long long mu_n) {
  return ({ __auto_type __let_tmp_pal = mere_vec_int_new(__lang_current_region); __auto_type mu_pal = __let_tmp_pal; __lifted_go_uq67_117(mu_n, mu_pal, mu_pixels, 0LL); });
}

static long long mu_palette_index__direct(mere_vec_int* mu_pal, long long mu_r, long long mu_g, long long mu_b) {
  return ((((((0LL >= 0LL) && (0LL <= mere_vec_int_len(mu_pal))) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len(mu_pal))) && ((((mere_vec_int_len(mu_pal) - 1LL) + 2LL) >= 0LL) && ((((mere_vec_int_len(mu_pal) - 1LL) + 2LL) + 1LL) <= mere_vec_int_len(mu_pal))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len(mu_pal))) && ((((mere_vec_int_len(mu_pal) - 1LL) + 1LL) >= 0LL) && ((((mere_vec_int_len(mu_pal) - 1LL) + 1LL) + 1LL) <= mere_vec_int_len(mu_pal))))) && (mere_vec_int_len(mu_pal) <= mere_vec_int_len(mu_pal))) ? __lifted_go__rvfast_uq68_120(mu_pal, mu_r, mu_g, mu_b, 0LL) : __lifted_go_uq69_121(mu_pal, mu_r, mu_g, mu_b, 0LL));
}

static mere_vec_int* mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__lang_region* __rp6484, mere_vec_int* mu_pixels, long long mu_width, long long mu_height, long long mu_depth) {
  return ({ __auto_type __let_tmp_count = (mu_width * mu_height); __auto_type mu_count = __let_tmp_count; __auto_type __let_tmp_eight = (mu_depth == 8LL); __auto_type mu_eight = __let_tmp_eight; __auto_type __let_tmp_grey = (mu_eight && ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_count; mu_all_grey__direct(__da0, __da1); })); __auto_type mu_grey = __let_tmp_grey; __auto_type __let_tmp_pal = ((mu_grey || (mu_eight == 0)) ? mere_vec_int_new(__lang_current_region) : ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_count; mu_collect_palette__direct(__da0, __da1); })); __auto_type mu_pal = __let_tmp_pal; __auto_type __let_tmp_indexed = ((((mu_grey == 0) && mu_eight) && (mere_vec_int_len(mu_pal) <= 768LL)) && (mere_vec_int_len(mu_pal) > 0LL)); __auto_type mu_indexed = __let_tmp_indexed; __auto_type __let_tmp_bpp = ((mu_grey || mu_indexed) ? 1LL : (mu_eight ? 3LL : 6LL)); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_stride = (mu_width * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_raw = mere_vec_int_new(__lang_current_region); __auto_type mu_raw = __let_tmp_raw; ({ (void)(({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mere_vec_int_new(__lang_current_region); __lifted_rows_122__direct(mu_height, mu_stride, mu_grey, mu_pixels, mu_width, mu_indexed, mu_pal, mu_raw, mu_bpp, __ia0, __ia1); })); __auto_type __let_tmp_z = mere_vec_int_new(__lang_current_region); __auto_type mu_z = __let_tmp_z; (void)(mere_vec_int_push(mu_z, 120LL)); (void)(mere_vec_int_push(mu_z, 1LL)); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_adler32__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_out = mere_vec_int_new(__rp6484); __auto_type mu_out = __let_tmp_out; (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = ({ __auto_type __let_tmp_sig = mere_vec_int_new(__lang_current_region); __auto_type mu_sig = __let_tmp_sig; __lifted_go_uq70_124(mu_sig, 0LL); }); mu_push_all__direct(__da0, __da1); })); __auto_type __let_tmp_ihdr = mere_vec_int_new(__lang_current_region); __auto_type mu_ihdr = __let_tmp_ihdr; (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = mu_width; mu_bytes_push4be__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = mu_height; mu_bytes_push4be__direct(__da0, __da1); })); (void)(mere_vec_int_push(mu_ihdr, mu_depth)); (void)(mere_vec_int_push(mu_ihdr, (mu_grey ? 0LL : (mu_indexed ? 3LL : 2LL)))); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 72LL; __auto_type __da3 = 68LL; __auto_type __da4 = 82LL; __auto_type __da5 = mu_ihdr; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)((mu_indexed ? ({ __auto_type __da0 = mu_out; __auto_type __da1 = 80LL; __auto_type __da2 = 76LL; __auto_type __da3 = 84LL; __auto_type __da4 = 69LL; __auto_type __da5 = mu_pal; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); }) : 0)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 68LL; __auto_type __da3 = 65LL; __auto_type __da4 = 84LL; __auto_type __da5 = mu_z; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 69LL; __auto_type __da3 = 78LL; __auto_type __da4 = 68LL; __auto_type __da5 = mere_vec_int_new(__lang_current_region); mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); mu_out; }); });
}

static mere_vec_int* mu_encode_rgb8__Vec_int__int__int__Vec_int__direct(__lang_region* __rp6529, mere_vec_int* mu_pixels, long long mu_width, long long mu_height) {
  return ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = 8LL; mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__rp6529, __da0, __da1, __da2, __da3); });
}

static mere_vec_int* mu_encode16__Vec_int__int__int__Vec_int__direct(__lang_region* __rp6534, mere_vec_int* mu_pixels, long long mu_width, long long mu_height) {
  return ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = 16LL; mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__rp6534, __da0, __da1, __da2, __da3); });
}

static const char* mu_colour_name__direct(long long mu_c) {
  return ((mu_c == 0LL) ? (__sl_13.s) : ((mu_c == 2LL) ? (__sl_12.s) : ((mu_c == 3LL) ? (__sl_11.s) : ((mu_c == 4LL) ? (__sl_10.s) : ((mu_c == 6LL) ? (__sl_9.s) : (__sl_8.s))))));
}

static int mu_info__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 0LL) ? ({ const char* __ps = (__sl_19.s);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }) : ({ __auto_type __let_tmp_g = ({ __auto_type __da0 = mu_data; mu_gamma__direct(__da0); }); __auto_type mu_g = __let_tmp_g; (void)(({ const char* __ps = ({ __auto_type __bc71 = show_int(mere_vec_int_get(mu_h, 0LL)); __auto_type __bc72 = (__sl_14.s); __auto_type __bc73 = __lang_str_concat(__bc71, __bc72); __auto_type __bc74 = show_int(mere_vec_int_get(mu_h, 1LL)); __auto_type __bc75 = __lang_str_concat(__bc73, __bc74); __auto_type __bc76 = (__sl_4.s); __auto_type __bc77 = __lang_str_concat(__bc75, __bc76); __auto_type __bc78 = show_int(mere_vec_int_get(mu_h, 2LL)); __auto_type __bc79 = __lang_str_concat(__bc77, __bc78); __auto_type __bc80 = (__sl_15.s); __auto_type __bc81 = __lang_str_concat(__bc79, __bc80); __auto_type __bc82 = ({ __auto_type __da0 = mere_vec_int_get(mu_h, 3LL); mu_colour_name__direct(__da0); }); __auto_type __bc83 = __lang_str_concat(__bc81, __bc82); __auto_type __bc84 = ((mere_vec_int_get(mu_h, 4LL) == 1LL) ? (__sl_16.s) : (__sl_0.s)); __auto_type __bc85 = __lang_str_concat(__bc83, __bc84); __auto_type __bc86 = (({ __auto_type __da0 = mu_data; mu_has_transparency__direct(__da0); }) ? (__sl_17.s) : (__sl_0.s)); __auto_type __bc87 = __lang_str_concat(__bc85, __bc86); __auto_type __bc88 = ((mu_g > 0LL) ? __lang_str_concat((__sl_18.s), show_int(mu_g)) : (__sl_0.s)); __auto_type __bc89 = __lang_str_concat(__bc87, __bc88); __bc89; });            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; })); __auto_type __let_tmp_t = ({ __auto_type __da0 = mu_data; mu_text_chunks__direct(__da0); }); __auto_type mu_t = __let_tmp_t; ((((0LL >= 0LL) && (0LL <= mere_vec_str_len(mu_t))) && (mere_vec_str_len(mu_t) <= mere_vec_str_len(mu_t))) ? __lifted_show__rvfast_125(mu_t, 0LL) : __lifted_show_126(mu_t, 0LL)); })); });
}

static mere_vec_int* mu_ppm_bytes__direct(mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_data; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; ({ (void)(({ __auto_type __ia0 = ({ __auto_type __bc90 = (__sl_20.s); __auto_type __bc91 = show_int(mere_vec_int_get(mu_h, 0LL)); __auto_type __bc92 = __lang_str_concat(__bc90, __bc91); __auto_type __bc93 = (__sl_4.s); __auto_type __bc94 = __lang_str_concat(__bc92, __bc93); __auto_type __bc95 = show_int(mere_vec_int_get(mu_h, 1LL)); __auto_type __bc96 = __lang_str_concat(__bc94, __bc95); __auto_type __bc97 = (__sl_21.s); __auto_type __bc98 = __lang_str_concat(__bc96, __bc97); __bc98; }); __auto_type __ia1 = 0LL; __lifted_header_127__direct(mu_out, __ia0, __ia1); })); __auto_type __let_tmp_rgb = ({ __auto_type __da0 = mu_data; mu_rgb8__direct(__da0); }); __auto_type mu_rgb = __let_tmp_rgb; __auto_type __let_tmp_n = mere_vec_int_len(mu_rgb); __auto_type mu_n = __let_tmp_n; __lifted_go_uq72_128(mu_n, mu_out, mu_rgb, 0LL); }); });
}

static mere_vec_int* mu_read_ppm__Vec_int__Vec_int__direct(__lang_region* __rp6610, mere_vec_int* mu_data) {
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_data); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_acc = mere_vec_int_new(__rp6610); __auto_type mu_acc = __let_tmp_acc; ({ (void)(({ __auto_type __ia0 = 2LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct(mu_n, mu_data, mu_acc, __ia0, __ia1, __ia2, __ia3); })); mu_acc; }); });
}

static mere_vec_int* mu_load__direct(const char* mu_path) {
  return __lang_vec_of_bytes(__lang_read_bytes(mu_path), __lang_current_region);
}

static int mu_encode_cmd__direct(const char* mu_path, const char* mu_out) {
  return ({ __auto_type __let_tmp_data = ({ __auto_type __da0 = mu_path; mu_load__direct(__da0); }); __auto_type mu_data = __let_tmp_data; __auto_type __let_tmp_f = ({ __auto_type __da0 = mu_data; mu_read_ppm__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_f = __let_tmp_f; ((mere_vec_int_len(mu_f) < 4LL) ? ({ const char* __ps = (__sl_25.s);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }) : ({ __auto_type __let_tmp_width = mere_vec_int_get(mu_f, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_f, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_maxval = mere_vec_int_get(mu_f, 2LL); __auto_type mu_maxval = __let_tmp_maxval; __auto_type __let_tmp_start = mere_vec_int_get(mu_f, 3LL); __auto_type mu_start = __let_tmp_start; __auto_type __let_tmp_wide = (mu_maxval > 255LL); __auto_type mu_wide = __let_tmp_wide; __auto_type __let_tmp_bytes_per_sample = (mu_wide ? 2LL : 1LL); __auto_type mu_bytes_per_sample = __let_tmp_bytes_per_sample; __auto_type __let_tmp_pixels = mere_vec_int_new(__lang_current_region); __auto_type mu_pixels = __let_tmp_pixels; ({ (void)(__lifted_copy_uq74_130(mu_width, mu_height, mu_bytes_per_sample, mu_pixels, mu_data, mu_start, 0LL)); __auto_type __let_tmp_png = __lang_bytes_of_vec((mu_wide ? ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode16__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }) : ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode_rgb8__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }))); __auto_type mu_png = __let_tmp_png; (void)(__lang_write_bytes(mu_out, mu_png)); ({ const char* __ps = ({ __auto_type __bc99 = show_int(mu_width); __auto_type __bc100 = (__sl_14.s); __auto_type __bc101 = __lang_str_concat(__bc99, __bc100); __auto_type __bc102 = show_int(mu_height); __auto_type __bc103 = __lang_str_concat(__bc101, __bc102); __auto_type __bc104 = (__sl_4.s); __auto_type __bc105 = __lang_str_concat(__bc103, __bc104); __auto_type __bc106 = show_int((mu_wide ? 16LL : 8LL)); __auto_type __bc107 = __lang_str_concat(__bc105, __bc106); __auto_type __bc108 = (__sl_22.s); __auto_type __bc109 = __lang_str_concat(__bc107, __bc108); __auto_type __bc110 = mu_out; __auto_type __bc111 = __lang_str_concat(__bc109, __bc110); __auto_type __bc112 = (__sl_23.s); __auto_type __bc113 = __lang_str_concat(__bc111, __bc112); __auto_type __bc114 = show_int(((long long)(mu_png)->len)); __auto_type __bc115 = __lang_str_concat(__bc113, __bc114); __auto_type __bc116 = (__sl_24.s); __auto_type __bc117 = __lang_str_concat(__bc115, __bc116); __bc117; });            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }); })); });
}

static int mu___mere_user_main__direct(int mu__u) {
  return ({ __auto_type __scrut = ((void)(0), __lang_args()); (((list_str__tag(__scrut) == 1) && (((strcmp(((list_str__node(__scrut)->payload.Cons).f0), "encode") == 0)) && (((list_str__tag((list_str__node(__scrut)->payload.Cons).f1) == 1) && ((list_str__tag((list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f1) == 1))))))) ? ({ __auto_type mu_path = (list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f0; __auto_type mu_out = (list_str__node((list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f1)->payload.Cons).f0; ({ __auto_type __da0 = mu_path; __auto_type __da1 = mu_out; mu_encode_cmd__direct(__da0, __da1); }); }) : ((((list_str__tag(__scrut) == 1) && ((list_str__tag((list_str__node(__scrut)->payload.Cons).f1) == 1)))) ? ({ __auto_type mu_cmd = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_path = (list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f0; __auto_type mu_rest = (list_str__node((list_str__node(__scrut)->payload.Cons).f1)->payload.Cons).f1; ({ __auto_type __let_tmp_data = ({ __auto_type __da0 = mu_path; mu_load__direct(__da0); }); __auto_type mu_data = __let_tmp_data; ((({ __auto_type __da0 = mu_data; mu_is_png__direct(__da0); }) == 0) ? ({ const char* __ps = (__sl_29.s);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }) : ((__lang_str_cmp(mu_cmd, (__sl_28.s)) == 0) ? ({ __auto_type __da0 = mu_data; mu_info__direct(__da0); }) : ((__lang_str_cmp(mu_cmd, (__sl_27.s)) == 0) ? ({ __auto_type __scrut = mu_rest; (list_str__tag(__scrut) == 1) ? ({ __auto_type mu_out = (list_str__node(__scrut)->payload.Cons).f0; ({ __auto_type __let_tmp_b = __lang_bytes_of_vec(({ __auto_type __da0 = mu_data; mu_ppm_bytes__direct(__da0); })); __auto_type mu_b = __let_tmp_b; (void)(__lang_write_bytes(mu_out, mu_b)); ({ const char* __ps = __lang_str_concat(__lang_str_concat(show_int(((long long)(mu_b)->len)), (__sl_26.s)), mu_out);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }); }) : ((list_str__tag(__scrut) == 0) ? ({ ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))); }) : ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; })))); }); }) : ((((list_str__tag(__scrut) == 1) && ((list_str__tag((list_str__node(__scrut)->payload.Cons).f1) == 0)))) ? ({ __auto_type mu_cmd = (list_str__node(__scrut)->payload.Cons).f0; ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }) : ((1) ? ({ ({ const char* __ps = mu_usage;            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))))); });
}

static const char* mu_png_info__direct(mere_bytes* mu_data) {
  return ({ __auto_type __let_tmp_v = __lang_vec_of_bytes(mu_data, __lang_current_region); __auto_type mu_v = __let_tmp_v; ((({ __auto_type __da0 = mu_v; mu_is_png__direct(__da0); }) == 0) ? __lang_fail_str(__lang_str_concat((__sl_2.s), (__sl_29.s))) : ({ __auto_type __let_tmp_h = ({ __auto_type __da0 = mu_v; mu_read_header__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_h = __let_tmp_h; ((mere_vec_int_len(mu_h) == 0LL) ? __lang_fail_str(__lang_str_concat((__sl_2.s), (__sl_19.s))) : ({ __auto_type __let_tmp_g = ({ __auto_type __da0 = mu_v; mu_gamma__direct(__da0); }); __auto_type mu_g = __let_tmp_g; __auto_type __let_tmp_head = ({ __auto_type __bc118 = show_int(mere_vec_int_get(mu_h, 0LL)); __auto_type __bc119 = (__sl_14.s); __auto_type __bc120 = __lang_str_concat(__bc118, __bc119); __auto_type __bc121 = show_int(mere_vec_int_get(mu_h, 1LL)); __auto_type __bc122 = __lang_str_concat(__bc120, __bc121); __auto_type __bc123 = (__sl_4.s); __auto_type __bc124 = __lang_str_concat(__bc122, __bc123); __auto_type __bc125 = show_int(mere_vec_int_get(mu_h, 2LL)); __auto_type __bc126 = __lang_str_concat(__bc124, __bc125); __auto_type __bc127 = (__sl_15.s); __auto_type __bc128 = __lang_str_concat(__bc126, __bc127); __auto_type __bc129 = ({ __auto_type __da0 = mere_vec_int_get(mu_h, 3LL); mu_colour_name__direct(__da0); }); __auto_type __bc130 = __lang_str_concat(__bc128, __bc129); __auto_type __bc131 = ((mere_vec_int_get(mu_h, 4LL) == 1LL) ? (__sl_16.s) : (__sl_0.s)); __auto_type __bc132 = __lang_str_concat(__bc130, __bc131); __auto_type __bc133 = (({ __auto_type __da0 = mu_v; mu_has_transparency__direct(__da0); }) ? (__sl_17.s) : (__sl_0.s)); __auto_type __bc134 = __lang_str_concat(__bc132, __bc133); __auto_type __bc135 = ((mu_g > 0LL) ? __lang_str_concat((__sl_18.s), show_int(mu_g)) : (__sl_0.s)); __auto_type __bc136 = __lang_str_concat(__bc134, __bc135); __bc136; }); __auto_type mu_head = __let_tmp_head; __auto_type __let_tmp_t = ({ __auto_type __da0 = mu_v; mu_text_chunks__direct(__da0); }); __auto_type mu_t = __let_tmp_t; ((((0LL >= 0LL) && (0LL <= mere_vec_str_len(mu_t))) && (mere_vec_str_len(mu_t) <= mere_vec_str_len(mu_t))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_head; __lifted_join__rvfast_131__direct(mu_t, __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_head; __lifted_join_132__direct(mu_t, __ia0, __ia1); })); })); })); });
}

static mere_bytes* mu_png_to_ppm__direct(mere_bytes* mu_data) {
  return ({ __auto_type __let_tmp_v = __lang_vec_of_bytes(mu_data, __lang_current_region); __auto_type mu_v = __let_tmp_v; ((({ __auto_type __da0 = mu_v; mu_is_png__direct(__da0); }) == 0) ? ({ __lang_fail_impl(__lang_str_concat((__sl_2.s), (__sl_29.s))); (mere_bytes*)0; }) : __lang_bytes_of_vec(({ __auto_type __da0 = mu_v; mu_ppm_bytes__direct(__da0); }))); });
}

static mere_bytes* mu_ppm_to_png__direct(mere_bytes* mu_data) {
  return ({ __auto_type __let_tmp_v = __lang_vec_of_bytes(mu_data, __lang_current_region); __auto_type mu_v = __let_tmp_v; __auto_type __let_tmp_f = ({ __auto_type __da0 = mu_v; mu_read_ppm__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_f = __let_tmp_f; ((mere_vec_int_len(mu_f) < 4LL) ? ({ __lang_fail_impl(__lang_str_concat((__sl_2.s), (__sl_25.s))); (mere_bytes*)0; }) : ({ __auto_type __let_tmp_width = mere_vec_int_get(mu_f, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_f, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_maxval = mere_vec_int_get(mu_f, 2LL); __auto_type mu_maxval = __let_tmp_maxval; __auto_type __let_tmp_start = mere_vec_int_get(mu_f, 3LL); __auto_type mu_start = __let_tmp_start; __auto_type __let_tmp_wide = (mu_maxval > 255LL); __auto_type mu_wide = __let_tmp_wide; __auto_type __let_tmp_bytes_per_sample = (mu_wide ? 2LL : 1LL); __auto_type mu_bytes_per_sample = __let_tmp_bytes_per_sample; __auto_type __let_tmp_pixels = mere_vec_int_new(__lang_current_region); __auto_type mu_pixels = __let_tmp_pixels; ({ (void)(__lifted_copy_uq75_133(mu_width, mu_height, mu_bytes_per_sample, mu_pixels, mu_v, mu_start, 0LL)); __lang_bytes_of_vec((mu_wide ? ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode16__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }) : ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode_rgb8__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }))); }); })); });
}

static closure_int_int __anon_0_fn(void* __env_self_void, long long mu_a) {
  (void)__env_self_void;
  return ({ __anon_104_env* __env = (__anon_104_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_104_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_104_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_104_fn}; });
}
static long long __anon_0_fn2(void* __env_self_void, long long mu_a, long long mu_x) {
  (void)__env_self_void;
  return (mu_a + mu_x);
}
static closure_int_int __anon_1_fn(void* __env_self_void, long long mu_a) {
  (void)__env_self_void;
  return ({ __anon_105_env* __env = (__anon_105_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_105_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_105_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_105_fn}; });
}
static long long __anon_1_fn2(void* __env_self_void, long long mu_a, long long mu_x) {
  (void)__env_self_void;
  return (mu_a * mu_x);
}
static closure_closure_int_closure_int_int_int __anon_2_fn(void* __env_self_void, long long mu_acc) {
  __anon_2_env* __env_self = (__anon_2_env*)__env_self_void;
  return ({ __anon_106_env* __env = (__anon_106_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_106_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_106_env; __env->mu_xs = (__env_self->mu_xs); __env->mu_acc = mu_acc; (closure_closure_int_closure_int_int_int){.env = __env, .fn = __anon_106_fn}; });
}
static long long __anon_2_fn2(void* __env_self_void, long long mu_acc, closure_int_closure_int_int mu_f) {
  __anon_2_env* __env_self = (__anon_2_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_xs); (list_int__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_int__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_int__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_int__node(__scrut)->payload.Cons).f1; ({ __auto_type __da0 = mu_t; __auto_type __da1 = ({ __auto_type __c2 = mu_f; __auto_type __a0 = mu_acc; __typeof__(__c2.fn(__c2.env, __a0)) __i2; if (!__c2.fn2) __i2 = __c2.fn(__c2.env, __a0); __auto_type __a1 = mu_h; __c2.fn2 ? __c2.fn2(__c2.env, __a0, __a1) : __i2.fn(__i2.env, __a1); }); __auto_type __da2 = mu_f; mu_list_fold__direct(__da0, __da1, __da2); }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))); });
}
static closure_list_int_list_int __anon_3_fn(void* __env_self_void, long long mu_i) {
  __anon_3_env* __env_self = (__anon_3_env*)__env_self_void;
  return ({ __anon_107_env* __env = (__anon_107_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_107_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_107_env; __env->mu_i = mu_i; __env->mu_a = (__env_self->mu_a); (closure_list_int_list_int){.env = __env, .fn = __anon_107_fn}; });
}
static list_int __anon_3_fn2(void* __env_self_void, long long mu_i, list_int mu_acc) {
  __anon_3_env* __env_self = (__anon_3_env*)__env_self_void;
  return ((mu_i < (__env_self->mu_a)) ? mu_acc : ({ __auto_type __da0 = (__env_self->mu_a); __auto_type __da1 = (mu_i - 1LL); __auto_type __da2 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = mu_i, .f1 = mu_acc}); list_int__mk(1, __p); }); mu__range_down__direct(__da0, __da1, __da2); }));
}
static list_int __anon_4_fn(void* __env_self_void, long long mu_b) {
  __anon_4_env* __env_self = (__anon_4_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_a); __auto_type __da1 = mu_b; __auto_type __da2 = list_int__mk(0, &__mshared_list_int_t0); mu__range_down__direct(__da0, __da1, __da2); });
}
static closure_int_int __anon_5_fn(void* __env_self_void, long long mu_a) {
  (void)__env_self_void;
  return ({ __anon_108_env* __env = (__anon_108_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_108_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_108_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_108_fn}; });
}
static long long __anon_5_fn2(void* __env_self_void, long long mu_a, long long mu_x) {
  (void)__env_self_void;
  return (mu_a + mu_x);
}
static closure_int_int __anon_6_fn(void* __env_self_void, long long mu_a) {
  (void)__env_self_void;
  return ({ __anon_109_env* __env = (__anon_109_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_109_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_109_env; __env->mu_a = mu_a; (closure_int_int){.env = __env, .fn = __anon_109_fn}; });
}
static long long __anon_6_fn2(void* __env_self_void, long long mu_a, long long mu_x) {
  (void)__env_self_void;
  return (mu_a * mu_x);
}
static const char* __anon_7_fn(void* __env_self_void, long long mu_i) {
  __anon_7_env* __env_self = (__anon_7_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_cs); (list_str__tag(__scrut) == 0) ? ({ (__sl_0.s); }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; ((mu_i == 0LL) ? mu_h : ({ __auto_type __da0 = mu_t; __auto_type __da1 = (mu_i - 1LL); mu__u8_nth__direct(__da0, __da1); })); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}
static const char* __anon_8_fn(void* __env_self_void, long long mu_i) {
  __anon_8_env* __env_self = (__anon_8_env*)__env_self_void;
  return ({ __auto_type __da0 = __lang_utf8_chars((__env_self->mu_s)); __auto_type __da1 = mu_i; mu__u8_nth__direct(__da0, __da1); });
}
static long long __anon_9_fn(void* __env_self_void, long long mu_i) {
  __anon_9_env* __env_self = (__anon_9_env*)__env_self_void;
  return ((__lang_ord(__lang_char_at((__env_self->mu_s), mu_i))) & (63LL));
}
static long long __anon_10_fn(void* __env_self_void, long long mu_d) {
  __anon_10_env* __env_self = (__anon_10_env*)__env_self_void;
  return ({ __auto_type __let_tmp_n = ((long long) __lang_str_size((__env_self->mu_s))); __auto_type mu_n = __let_tmp_n; ((mu_n == 0LL) ? mu_d : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at((__env_self->mu_s), 0LL)); __auto_type mu_b0 = __let_tmp_b0; __auto_type __let_tmp_span = ({ __auto_type __da0 = mu_b0; mu__cp_span__direct(__da0); }); __auto_type mu_span = __let_tmp_span; ((mu_span != mu_n) ? mu_d : ({ __auto_type __let_tmp_cp = ((mu_span == 1LL) ? mu_b0 : ((mu_span == 2LL) ? ((({ long long __sa = (((mu_b0) & (31LL))); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); }))) : ((mu_span == 3LL) ? ((((({ long long __sa = (((mu_b0) & (15LL))); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); }))) : ((((({ long long __sa = (((mu_b0) & (7LL))); long long __sn = (18LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ long long __sa = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = 1LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (12LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })))) | (((({ long long __sa = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = 2LL; mu__cp_cont__direct(__da0, __da1); })); long long __sn = (6LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = 3LL; mu__cp_cont__direct(__da0, __da1); })))))))); __auto_type mu_cp = __let_tmp_cp; __auto_type __let_tmp_shortest = ((mu_span == 1LL) ? (mu_cp < 128LL) : ((mu_span == 2LL) ? (mu_cp >= 128LL) : ((mu_span == 3LL) ? (mu_cp >= 2048LL) : (mu_cp >= 65536LL)))); __auto_type mu_shortest = __let_tmp_shortest; ((({ __auto_type __da0 = mu_cp; mu__cp_bad__direct(__da0); }) || (!(mu_shortest))) ? mu_d : mu_cp); })); })); });
}
static long long __anon_11_fn(void* __env_self_void, long long mu_i) {
  __anon_11_env* __env_self = (__anon_11_env*)__env_self_void;
  return ({ __auto_type __da0 = ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = mu_i; mu_utf8_at__direct(__da0, __da1); }); mu_codepoint_of__direct(__da0); });
}
static closure_int_closure_str_str __anon_12_fn(void* __env_self_void, long long mu_start) {
  __anon_12_env* __env_self = (__anon_12_env*)__env_self_void;
  return ({ __anon_110_env* __env = (__anon_110_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_110_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_110_env; __env->mu_cs = (__env_self->mu_cs); __env->mu_start = mu_start; (closure_int_closure_str_str){.env = __env, .fn = __anon_110_fn, .fn2 = __anon_110_fn2}; });
}
static closure_str_str __anon_12_fn2(void* __env_self_void, long long mu_start, long long mu_len) {
  __anon_12_env* __env_self = (__anon_12_env*)__env_self_void;
  return ({ __anon_111_env* __env = (__anon_111_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_111_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_111_env; __env->mu_cs = (__env_self->mu_cs); __env->mu_start = mu_start; __env->mu_len = mu_len; (closure_str_str){.env = __env, .fn = __anon_111_fn}; });
}
static closure_int_str __anon_13_fn(void* __env_self_void, long long mu_start) {
  __anon_13_env* __env_self = (__anon_13_env*)__env_self_void;
  return ({ __anon_112_env* __env = (__anon_112_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_112_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_112_env; __env->mu_s = (__env_self->mu_s); __env->mu_start = mu_start; (closure_int_str){.env = __env, .fn = __anon_112_fn}; });
}
static const char* __anon_13_fn2(void* __env_self_void, long long mu_start, long long mu_len) {
  __anon_13_env* __env_self = (__anon_13_env*)__env_self_void;
  return ({ __auto_type __da0 = __lang_utf8_chars((__env_self->mu_s)); __auto_type __da1 = mu_start; __auto_type __da2 = mu_len; __auto_type __da3 = (__sl_0.s); mu__u8_slice__direct(__da0, __da1, __da2, __da3); });
}
static const char* __anon_14_fn(void* __env_self_void, const char* mu_acc) {
  __anon_14_env* __env_self = (__anon_14_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_cs); (list_str__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; ({ __auto_type __da0 = mu_t; __auto_type __da1 = __lang_str_concat(mu_h, mu_acc); mu__u8_rev_join__direct(__da0, __da1); }); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}
static closure_int_closure_int_int __anon_15_fn(void* __env_self_void, long long mu_i) {
  __anon_15_env* __env_self = (__anon_15_env*)__env_self_void;
  return ({ __anon_113_env* __env = (__anon_113_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_113_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_113_env; __env->mu_i = mu_i; __env->mu_s = (__env_self->mu_s); (closure_int_closure_int_int){.env = __env, .fn = __anon_113_fn, .fn2 = __anon_113_fn2}; });
}
static closure_int_int __anon_15_fn2(void* __env_self_void, long long mu_i, long long mu_n) {
  __anon_15_env* __env_self = (__anon_15_env*)__env_self_void;
  return ({ __anon_114_env* __env = (__anon_114_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_114_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_114_env; __env->mu_i = mu_i; __env->mu_n = mu_n; __env->mu_s = (__env_self->mu_s); (closure_int_int){.env = __env, .fn = __anon_114_fn}; });
}
static const char* __anon_16_fn(void* __env_self_void, long long mu_w) {
  __anon_16_env* __env_self = (__anon_16_env*)__env_self_void;
  return ({ __auto_type __let_tmp_d = (mu_w - ({ __auto_type __da0 = (__env_self->mu_s); mu_utf8_width__direct(__da0); })); __auto_type mu_d = __let_tmp_d; ((mu_d <= 0LL) ? (__env_self->mu_s) : __lang_str_concat((__env_self->mu_s), __lang_str_repeat((__sl_4.s), mu_d))); });
}
static const char* __anon_17_fn(void* __env_self_void, long long mu_w) {
  __anon_17_env* __env_self = (__anon_17_env*)__env_self_void;
  return ({ __auto_type __let_tmp_d = (mu_w - ({ __auto_type __da0 = (__env_self->mu_s); mu_utf8_width__direct(__da0); })); __auto_type mu_d = __let_tmp_d; ((mu_d <= 0LL) ? (__env_self->mu_s) : __lang_str_concat(__lang_str_repeat((__sl_4.s), mu_d), (__env_self->mu_s))); });
}
static long long __anon_18_fn(void* __env_self_void, long long mu_b) {
  __anon_18_env* __env_self = (__anon_18_env*)__env_self_void;
  return (((__env_self->mu_a) > mu_b) ? 0LL : ({ __auto_type __let_tmp_n = ((mu_b - (__env_self->mu_a)) + 1LL); __auto_type mu_n = __let_tmp_n; ((__lang_imod(mu_n, 2LL) == 0LL) ? (__lang_idiv(mu_n, 2LL) * ((__env_self->mu_a) + mu_b)) : (mu_n * __lang_idiv(((__env_self->mu_a) + mu_b), 2LL))); }));
}
static closure_int_int __anon_19_fn(void* __env_self_void, long long mu_sq) {
  __anon_19_env* __env_self = (__anon_19_env*)__env_self_void;
  return ({ __anon_115_env* __env = (__anon_115_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_115_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_115_env; __env->mu_acc = (__env_self->mu_acc); __env->mu_sq = mu_sq; (closure_int_int){.env = __env, .fn = __anon_115_fn}; });
}
static long long __anon_19_fn2(void* __env_self_void, long long mu_sq, long long mu_n) {
  __anon_19_env* __env_self = (__anon_19_env*)__env_self_void;
  return ((mu_n == 0LL) ? (__env_self->mu_acc) : ((__lang_imod(mu_n, 2LL) == 1LL) ? ({ __auto_type __da0 = ((__env_self->mu_acc) * mu_sq); __auto_type __da1 = (mu_sq * mu_sq); __auto_type __da2 = __lang_idiv(mu_n, 2LL); mu__pow_go__direct(__da0, __da1, __da2); }) : ({ __auto_type __da0 = (__env_self->mu_acc); __auto_type __da1 = (mu_sq * mu_sq); __auto_type __da2 = __lang_idiv(mu_n, 2LL); mu__pow_go__direct(__da0, __da1, __da2); })));
}
static long long __anon_20_fn(void* __env_self_void, long long mu_e) {
  __anon_20_env* __env_self = (__anon_20_env*)__env_self_void;
  return ((mu_e < 0LL) ? __lang_fail_int(__lang_str_concat((__sl_2.s), __lang_str_concat((__sl_5.s), show_int(mu_e)))) : ({ __auto_type __da0 = 1LL; __auto_type __da1 = (__env_self->mu_b); __auto_type __da2 = mu_e; mu__pow_go__direct(__da0, __da1, __da2); }));
}
static long long __anon_21_fn(void* __env_self_void, long long mu_b) {
  __anon_21_env* __env_self = (__anon_21_env*)__env_self_void;
  return ((((__env_self->mu_a) == 0LL) || (mu_b == 0LL)) ? 0LL : ({ long long __a = ((__lang_idiv((__env_self->mu_a), __lang_gcd((__env_self->mu_a), mu_b)) * mu_b)); __a < 0 ? -__a : __a; }));
}
static tuple_int_int __anon_22_fn(void* __env_self_void, long long mu_b) {
  __anon_22_env* __env_self = (__anon_22_env*)__env_self_void;
  return ((mu_b == 0LL) ? ({ __lang_fail_impl(__lang_str_concat((__sl_2.s), (__sl_6.s))); (tuple_int_int){0}; }) : ((tuple_int_int){.f0 = __lang_idiv((__env_self->mu_a), mu_b), .f1 = __lang_imod((__env_self->mu_a), mu_b)}));
}
static int __anon_23_fn(void* __env_self_void, const char* mu_msg) {
  __anon_23_env* __env_self = (__anon_23_env*)__env_self_void;
  return ((__env_self->mu_cond) ? 0 : __lang_fail_int(__lang_str_concat((__sl_2.s), __lang_str_concat((__sl_7.s), mu_msg))));
}
static long long __anon_24_fn(void* __env_self_void, long long mu_b) {
  __anon_24_env* __env_self = (__anon_24_env*)__env_self_void;
  return ({ __auto_type __let_tmp_x = (((__env_self->mu_crc)) ^ (mu_b)); __auto_type mu_x = __let_tmp_x; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_x; __lifted_go_0__direct(__ia0, __ia1); }); });
}
static long long __anon_25_fn(void* __env_self_void, mere_vec_int* mu_data) {
  __anon_25_env* __env_self = (__anon_25_env*)__env_self_void;
  return ({ __auto_type __let_tmp_cnt = mere_vec_int_get((__env_self->mu_s), 2LL); __auto_type mu_cnt = __let_tmp_cnt; (void)(((mu_cnt == 0LL) ? ({ __auto_type __let_tmp_p = mere_vec_int_get((__env_self->mu_s), 0LL); __auto_type mu_p = __let_tmp_p; (void)(mere_vec_int_set((__env_self->mu_s), 1LL, mere_vec_int_get(mu_data, mu_p))); (void)(mere_vec_int_set((__env_self->mu_s), 0LL, (mu_p + 1LL))); mere_vec_int_set((__env_self->mu_s), 2LL, 8LL); }) : 0)); __auto_type __let_tmp_buf = mere_vec_int_get((__env_self->mu_s), 1LL); __auto_type mu_buf = __let_tmp_buf; (void)(mere_vec_int_set((__env_self->mu_s), 1LL, ({ long long __sa = (mu_buf); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); }))); (void)(mere_vec_int_set((__env_self->mu_s), 2LL, (mere_vec_int_get((__env_self->mu_s), 2LL) - 1LL))); ((mu_buf) & (1LL)); });
}
static closure_int_int __anon_26_fn(void* __env_self_void, mere_vec_int* mu_data) {
  __anon_26_env* __env_self = (__anon_26_env*)__env_self_void;
  return ({ __anon_116_env* __env = (__anon_116_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_116_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_116_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = mu_data; (closure_int_int){.env = __env, .fn = __anon_116_fn}; });
}
static long long __anon_26_fn2(void* __env_self_void, mere_vec_int* mu_data, long long mu_n) {
  __anon_26_env* __env_self = (__anon_26_env*)__env_self_void;
  return ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq2_3__direct(mu_n, (__env_self->mu_s), mu_data, __ia0, __ia1); });
}
static closure_Vec_int_closure_Vec_int_int __anon_27_fn(void* __env_self_void, mere_vec_int* mu_data) {
  __anon_27_env* __env_self = (__anon_27_env*)__env_self_void;
  return ({ __anon_117_env* __env = (__anon_117_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_117_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_117_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = mu_data; (closure_Vec_int_closure_Vec_int_int){.env = __env, .fn = __anon_117_fn, .fn2 = __anon_117_fn2}; });
}
static closure_Vec_int_int __anon_27_fn2(void* __env_self_void, mere_vec_int* mu_data, mere_vec_int* mu_count) {
  __anon_27_env* __env_self = (__anon_27_env*)__env_self_void;
  return ({ __anon_118_env* __env = (__anon_118_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_118_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_118_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = mu_data; __env->mu_count = mu_count; (closure_Vec_int_int){.env = __env, .fn = __anon_118_fn}; });
}
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_28_fn(void* __env_self_void, mere_vec_int* mu_data) {
  __anon_28_env* __env_self = (__anon_28_env*)__env_self_void;
  return ({ __anon_119_env* __env = (__anon_119_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_119_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_119_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = mu_data; (closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_119_fn, .fn2 = __anon_119_fn2}; });
}
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_28_fn2(void* __env_self_void, mere_vec_int* mu_data, mere_vec_int* mu_out) {
  __anon_28_env* __env_self = (__anon_28_env*)__env_self_void;
  return ({ __anon_120_env* __env = (__anon_120_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_120_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_120_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = mu_data; __env->mu_out = mu_out; (closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_120_fn, .fn2 = __anon_120_fn2}; });
}
static tuple_Vec_int_Vec_int __anon_29_fn(void* __env_self_void, mere_vec_int* mu_data) {
  __anon_29_env* __env_self = (__anon_29_env*)__env_self_void;
  return ({ __auto_type __let_tmp_hlit = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = mu_data; __auto_type __da2 = 5LL; mu_br_bits__direct(__da0, __da1, __da2); }) + 257LL); __auto_type mu_hlit = __let_tmp_hlit; __auto_type __let_tmp_hdist = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = mu_data; __auto_type __da2 = 5LL; mu_br_bits__direct(__da0, __da1, __da2); }) + 1LL); __auto_type mu_hdist = __let_tmp_hdist; __auto_type __let_tmp_hclen = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = mu_data; __auto_type __da2 = 4LL; mu_br_bits__direct(__da0, __da1, __da2); }) + 4LL); __auto_type mu_hclen = __let_tmp_hclen; __auto_type __let_tmp_cl_lengths = mere_vec_int_new(__lang_current_region); __auto_type mu_cl_lengths = __let_tmp_cl_lengths; ({ (void)(__lifted_zero19_16(mu_cl_lengths, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= mu_hclen)) && (mu_hclen <= mere_vec_int_len(mu_clen_order))) ? __lifted_rd__rvfast_17(mu_hclen, mu_cl_lengths, mu_clen_order, (__env_self->mu_s), mu_data, 0LL) : __lifted_rd_18(mu_hclen, mu_cl_lengths, mu_clen_order, (__env_self->mu_s), mu_data, 0LL))); ({ __auto_type __let_tup = ({ __auto_type __da0 = mu_cl_lengths; mu_huff_build__Vec_int__tuple_Vec_int_Vec_int__direct(__lang_current_region, __lang_current_region, __da0); }); __auto_type mu_clc = (__let_tup).f0; __auto_type mu_cls = (__let_tup).f1; ({ __auto_type __let_tmp_lengths = mere_vec_int_new(__lang_current_region); __auto_type mu_lengths = __let_tmp_lengths; __auto_type __let_tmp_total = (mu_hlit + mu_hdist); __auto_type mu_total = __let_tmp_total; ({ (void)(__lifted_fill_19(mu_lengths, mu_total, (__env_self->mu_s), mu_data, mu_clc, mu_cls, 0)); __auto_type __let_tmp_llen = mere_vec_int_new(__lang_current_region); __auto_type mu_llen = __let_tmp_llen; __auto_type __let_tmp_dlen = mere_vec_int_new(__lang_current_region); __auto_type mu_dlen = __let_tmp_dlen; ({ (void)(__lifted_split_23(mu_total, mu_hlit, mu_llen, mu_lengths, mu_dlen, 0LL)); ((tuple_Vec_int_Vec_int){.f0 = mu_llen, .f1 = mu_dlen}); }); }); }); }); }); }); });
}
static mere_vec_int* __anon_30_fn(void* __env_self_void, long long mu_start) {
  __anon_30_env* __env_self = (__anon_30_env*)__env_self_void;
  return ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __auto_type __let_tmp_s = ({ __auto_type __da0 = mu_start; mu_br_new__int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_s = __let_tmp_s; __lifted_block_24(mu_s, (__env_self->mu_data), mu_out, 0); });
}
static long long __anon_31_fn(void* __env_self_void, long long mu_o) {
  __anon_31_env* __env_self = (__anon_31_env*)__env_self_void;
  return ({ __auto_type __bc233 = mere_vec_int_get((__env_self->mu_data), mu_o); __auto_type __bc234 = ({ long long __sa = (mere_vec_int_get((__env_self->mu_data), (mu_o + 1LL))); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc235 = (__bc233 + __bc234); __auto_type __bc236 = ({ long long __sa = (mere_vec_int_get((__env_self->mu_data), (mu_o + 2LL))); long long __sn = (16LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc237 = (__bc235 + __bc236); __auto_type __bc238 = ({ long long __sa = (mere_vec_int_get((__env_self->mu_data), (mu_o + 3LL))); long long __sn = (24LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __bc239 = (__bc237 + __bc238); __bc239; });
}
static int __anon_32_fn(void* __env_self_void, mere_vec_int* mu_out) {
  __anon_32_env* __env_self = (__anon_32_env*)__env_self_void;
  return ({ __auto_type __let_tmp_n = mere_vec_int_len((__env_self->mu_data)); __auto_type mu_n = __let_tmp_n; ((({ __auto_type __da0 = mu_out; mu_crc32_vec__direct(__da0); }) == ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = (mu_n - 8LL); mu_gzip_trailer_u32__direct(__da0, __da1); })) && (((mere_vec_int_len(mu_out)) & (mu_mask32)) == ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = (mu_n - 4LL); mu_gzip_trailer_u32__direct(__da0, __da1); }))); });
}
static mere_vec_int* __anon_33_fn(void* __env_self_void, long long mu_x) {
  __anon_33_env* __env_self = (__anon_33_env*)__env_self_void;
  return ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; ({ (void)(__lifted_go_uq11_27((__env_self->mu_n), mu_v, mu_x, 0LL)); mu_v; }); });
}
static long long __anon_34_fn(void* __env_self_void, long long mu_i) {
  __anon_34_env* __env_self = (__anon_34_env*)__env_self_void;
  return ({ __auto_type __bc240 = mere_vec_int_get((__env_self->mu_data), mu_i); __auto_type __bc241 = 16777216LL; __auto_type __bc242 = (__bc240 * __bc241); __auto_type __bc243 = (mere_vec_int_get((__env_self->mu_data), (mu_i + 1LL)) * 65536LL); __auto_type __bc244 = (__bc242 + __bc243); __auto_type __bc245 = (mere_vec_int_get((__env_self->mu_data), (mu_i + 2LL)) * 256LL); __auto_type __bc246 = (__bc244 + __bc245); __auto_type __bc247 = mere_vec_int_get((__env_self->mu_data), (mu_i + 3LL)); __auto_type __bc248 = (__bc246 + __bc247); __bc248; });
}
static closure_int_closure_int_closure_int_closure_int_bool __anon_35_fn(void* __env_self_void, long long mu_i) {
  __anon_35_env* __env_self = (__anon_35_env*)__env_self_void;
  return ({ __anon_121_env* __env = (__anon_121_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_121_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_121_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = mu_i; (closure_int_closure_int_closure_int_closure_int_bool){.env = __env, .fn = __anon_121_fn, .fn2 = __anon_121_fn2}; });
}
static closure_int_closure_int_closure_int_bool __anon_35_fn2(void* __env_self_void, long long mu_i, long long mu_a) {
  __anon_35_env* __env_self = (__anon_35_env*)__env_self_void;
  return ({ __anon_122_env* __env = (__anon_122_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_122_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_122_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = mu_i; __env->mu_a = mu_a; (closure_int_closure_int_closure_int_bool){.env = __env, .fn = __anon_122_fn, .fn2 = __anon_122_fn2}; });
}
static int __anon_36_fn(void* __env_self_void, long long mu_i) {
  __anon_36_env* __env_self = (__anon_36_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 73LL; __auto_type __da3 = 72LL; __auto_type __da4 = 68LL; __auto_type __da5 = 82LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static int __anon_37_fn(void* __env_self_void, long long mu_i) {
  __anon_37_env* __env_self = (__anon_37_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 73LL; __auto_type __da3 = 68LL; __auto_type __da4 = 65LL; __auto_type __da5 = 84LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static int __anon_38_fn(void* __env_self_void, long long mu_i) {
  __anon_38_env* __env_self = (__anon_38_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 73LL; __auto_type __da3 = 69LL; __auto_type __da4 = 78LL; __auto_type __da5 = 68LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static int __anon_39_fn(void* __env_self_void, long long mu_i) {
  __anon_39_env* __env_self = (__anon_39_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 80LL; __auto_type __da3 = 76LL; __auto_type __da4 = 84LL; __auto_type __da5 = 69LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static int __anon_40_fn(void* __env_self_void, long long mu_i) {
  __anon_40_env* __env_self = (__anon_40_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 116LL; __auto_type __da3 = 69LL; __auto_type __da4 = 88LL; __auto_type __da5 = 116LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static int __anon_41_fn(void* __env_self_void, long long mu_i) {
  __anon_41_env* __env_self = (__anon_41_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 103LL; __auto_type __da3 = 65LL; __auto_type __da4 = 77LL; __auto_type __da5 = 65LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static int __anon_42_fn(void* __env_self_void, long long mu_i) {
  __anon_42_env* __env_self = (__anon_42_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_data); __auto_type __da1 = mu_i; __auto_type __da2 = 116LL; __auto_type __da3 = 82LL; __auto_type __da4 = 78LL; __auto_type __da5 = 83LL; mu_type_is__direct(__da0, __da1, __da2, __da3, __da4, __da5); });
}
static long long __anon_43_fn(void* __env_self_void, long long mu_depth) {
  __anon_43_env* __env_self = (__anon_43_env*)__env_self_void;
  return ({ __auto_type __bc249 = ({ __auto_type __da0 = (__env_self->mu_colour); mu_channels__direct(__da0); }); __auto_type __bc250 = ((mu_depth == 16LL) ? 2LL : 1LL); __auto_type __bc251 = (__bc249 * __bc250); __bc251; });
}
static closure_int_int __anon_44_fn(void* __env_self_void, long long mu_b) {
  __anon_44_env* __env_self = (__anon_44_env*)__env_self_void;
  return ({ __anon_123_env* __env = (__anon_123_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_123_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_123_env; __env->mu_a = (__env_self->mu_a); __env->mu_b = mu_b; (closure_int_int){.env = __env, .fn = __anon_123_fn}; });
}
static long long __anon_44_fn2(void* __env_self_void, long long mu_b, long long mu_c) {
  __anon_44_env* __env_self = (__anon_44_env*)__env_self_void;
  return ({ __auto_type __let_tmp_p = (((__env_self->mu_a) + mu_b) - mu_c); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_pa = ((mu_p >= (__env_self->mu_a)) ? (mu_p - (__env_self->mu_a)) : ((__env_self->mu_a) - mu_p)); __auto_type mu_pa = __let_tmp_pa; __auto_type __let_tmp_pb = ((mu_p >= mu_b) ? (mu_p - mu_b) : (mu_b - mu_p)); __auto_type mu_pb = __let_tmp_pb; __auto_type __let_tmp_pc = ((mu_p >= mu_c) ? (mu_p - mu_c) : (mu_c - mu_p)); __auto_type mu_pc = __let_tmp_pc; (((mu_pa <= mu_pb) && (mu_pa <= mu_pc)) ? (__env_self->mu_a) : ((mu_pb <= mu_pc) ? mu_b : mu_c)); });
}
static closure_int_closure_int_closure_int_ByteBuf __anon_45_fn(void* __env_self_void, long long mu_start) {
  __anon_45_env* __env_self = (__anon_45_env*)__env_self_void;
  return ({ __anon_124_env* __env = (__anon_124_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_124_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_124_env; __env->mu_start = mu_start; __env->mu_raw = (__env_self->mu_raw); (closure_int_closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_124_fn, .fn2 = __anon_124_fn2}; });
}
static closure_int_closure_int_ByteBuf __anon_45_fn2(void* __env_self_void, long long mu_start, long long mu_width) {
  __anon_45_env* __env_self = (__anon_45_env*)__env_self_void;
  return ({ __anon_125_env* __env = (__anon_125_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_125_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_125_env; __env->mu_width = mu_width; __env->mu_start = mu_start; __env->mu_raw = (__env_self->mu_raw); (closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_125_fn, .fn2 = __anon_125_fn2}; });
}
static closure_int_closure_int_ByteBuf __anon_46_fn(void* __env_self_void, long long mu_width) {
  __anon_46_env* __env_self = (__anon_46_env*)__env_self_void;
  return ({ __anon_126_env* __env = (__anon_126_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_126_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_126_env; __env->mu_raw = (__env_self->mu_raw); __env->mu_width = mu_width; (closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_126_fn, .fn2 = __anon_126_fn2}; });
}
static closure_int_ByteBuf __anon_46_fn2(void* __env_self_void, long long mu_width, long long mu_height) {
  __anon_46_env* __env_self = (__anon_46_env*)__env_self_void;
  return ({ __anon_127_env* __env = (__anon_127_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_127_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_127_env; __env->mu_raw = (__env_self->mu_raw); __env->mu_width = mu_width; __env->mu_height = mu_height; (closure_int_ByteBuf){.env = __env, .fn = __anon_127_fn}; });
}
static long long __anon_47_fn(void* __env_self_void, long long mu_b) {
  __anon_47_env* __env_self = (__anon_47_env*)__env_self_void;
  return __lang_idiv((((__env_self->mu_a) + mu_b) - 1LL), mu_b);
}
static long long __anon_48_fn(void* __env_self_void, long long mu_width) {
  __anon_48_env* __env_self = (__anon_48_env*)__env_self_void;
  return ({ __auto_type __da0 = (mu_width - ({ __auto_type __da0 = (__env_self->mu_p_uq26); mu_adam7_xstart__direct(__da0); })); __auto_type __da1 = ({ __auto_type __da0 = (__env_self->mu_p_uq26); mu_adam7_xstep__direct(__da0); }); mu_ceil_div__direct(__da0, __da1); });
}
static long long __anon_49_fn(void* __env_self_void, long long mu_height) {
  __anon_49_env* __env_self = (__anon_49_env*)__env_self_void;
  return ({ __auto_type __da0 = (mu_height - ({ __auto_type __da0 = (__env_self->mu_p_uq27); mu_adam7_ystart__direct(__da0); })); __auto_type __da1 = ({ __auto_type __da0 = (__env_self->mu_p_uq27); mu_adam7_ystep__direct(__da0); }); mu_ceil_div__direct(__da0, __da1); });
}
static closure_int_closure_int_ByteBuf __anon_50_fn(void* __env_self_void, long long mu_width) {
  __anon_50_env* __env_self = (__anon_50_env*)__env_self_void;
  return ({ __anon_128_env* __env = (__anon_128_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_128_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_128_env; __env->mu_width = mu_width; __env->mu_raw = (__env_self->mu_raw); (closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_128_fn, .fn2 = __anon_128_fn2}; });
}
static closure_int_ByteBuf __anon_50_fn2(void* __env_self_void, long long mu_width, long long mu_height) {
  __anon_50_env* __env_self = (__anon_50_env*)__env_self_void;
  return ({ __anon_129_env* __env = (__anon_129_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_129_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_129_env; __env->mu_width = mu_width; __env->mu_height = mu_height; __env->mu_raw = (__env_self->mu_raw); (closure_int_ByteBuf){.env = __env, .fn = __anon_129_fn}; });
}
static long long __anon_51_fn(void* __env_self_void, long long mu_b) {
  __anon_51_env* __env_self = (__anon_51_env*)__env_self_void;
  return ({ __auto_type __let_tmp_x = (((__env_self->mu_crc)) ^ (mu_b)); __auto_type mu_x = __let_tmp_x; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mu_x; __lifted_go_uq31_49__direct(__ia0, __ia1); }); });
}
static closure_int_closure_int_unit __anon_52_fn(void* __env_self_void, mere_vec_int* mu_out) {
  __anon_52_env* __env_self = (__anon_52_env*)__env_self_void;
  return ({ __anon_130_env* __env = (__anon_130_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_130_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_130_env; __env->mu_s = (__env_self->mu_s); __env->mu_out = mu_out; (closure_int_closure_int_unit){.env = __env, .fn = __anon_130_fn, .fn2 = __anon_130_fn2}; });
}
static closure_int_unit __anon_52_fn2(void* __env_self_void, mere_vec_int* mu_out, long long mu_v) {
  __anon_52_env* __env_self = (__anon_52_env*)__env_self_void;
  return ({ __anon_131_env* __env = (__anon_131_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_131_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_131_env; __env->mu_v = mu_v; __env->mu_s = (__env_self->mu_s); __env->mu_out = mu_out; (closure_int_unit){.env = __env, .fn = __anon_131_fn}; });
}
static int __anon_53_fn(void* __env_self_void, mere_vec_int* mu_out) {
  __anon_53_env* __env_self = (__anon_53_env*)__env_self_void;
  return ((mere_vec_int_get((__env_self->mu_s), 1LL) > 0LL) ? ({ (void)(mere_vec_int_push(mu_out, ((mere_vec_int_get((__env_self->mu_s), 0LL)) & (255LL)))); (void)(mere_vec_int_set((__env_self->mu_s), 0LL, 0LL)); mere_vec_int_set((__env_self->mu_s), 1LL, 0LL); }) : 0);
}
static long long __anon_54_fn(void* __env_self_void, long long mu_n) {
  __anon_54_env* __env_self = (__anon_54_env*)__env_self_void;
  return ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq35_53__direct(mu_n, (__env_self->mu_v), __ia0, __ia1); });
}
static closure_int_tuple_int_int __anon_55_fn(void* __env_self_void, long long mu_pos) {
  __anon_55_env* __env_self = (__anon_55_env*)__env_self_void;
  return ({ __anon_132_env* __env = (__anon_132_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_132_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_132_env; __env->mu_pos = mu_pos; __env->mu_data = (__env_self->mu_data); (closure_int_tuple_int_int){.env = __env, .fn = __anon_132_fn}; });
}
static tuple_int_int __anon_55_fn2(void* __env_self_void, long long mu_pos, long long mu_n) {
  __anon_55_env* __env_self = (__anon_55_env*)__env_self_void;
  return ({ __auto_type __let_tmp_max_back = ((mu_pos > 32768LL) ? 32768LL : mu_pos); __auto_type mu_max_back = __let_tmp_max_back; ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __lifted_best_58__direct(mu_max_back, mu_pos, mu_n, (__env_self->mu_data), __ia0, __ia1, __ia2); }); });
}
static closure_int_Vec_int __anon_56_fn(void* __env_self_void, long long mu_nsym) {
  __anon_56_env* __env_self = (__anon_56_env*)__env_self_void;
  return ({ __anon_133_env* __env = (__anon_133_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_133_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_133_env; __env->mu_nsym = mu_nsym; __env->mu_freq = (__env_self->mu_freq); (closure_int_Vec_int){.env = __env, .fn = __anon_133_fn}; });
}
static mere_vec_int* __anon_56_fn2(void* __env_self_void, long long mu_nsym, long long mu_maxbits) {
  __anon_56_env* __env_self = (__anon_56_env*)__env_self_void;
  return ({ __auto_type __let_tmp_nf = mere_vec_int_new(__lang_current_region); __auto_type mu_nf = __let_tmp_nf; __auto_type __let_tmp_nl = mere_vec_int_new(__lang_current_region); __auto_type mu_nl = __let_tmp_nl; __auto_type __let_tmp_nr = mere_vec_int_new(__lang_current_region); __auto_type mu_nr = __let_tmp_nr; ({ (void)(__lifted_seed_61(mu_nsym, (__env_self->mu_freq), mu_nf, mu_nl, mu_nr, 0LL)); __auto_type __let_tmp_syms = mere_vec_int_new(__lang_current_region); __auto_type mu_syms = __let_tmp_syms; ({ (void)(__lifted_seed2_62(mu_nsym, (__env_self->mu_freq), mu_syms, 0LL)); __auto_type __let_tmp_nleaves = mere_vec_int_len(mu_nf); __auto_type mu_nleaves = __let_tmp_nleaves; __auto_type __let_tmp_lengths = mere_vec_int_new(__lang_current_region); __auto_type mu_lengths = __let_tmp_lengths; ({ (void)(__lifted_zl_63(mu_nsym, mu_lengths, 0LL)); ((mu_nleaves == 0LL) ? mu_lengths : ((mu_nleaves == 1LL) ? ({ (void)(mere_vec_int_set(mu_lengths, mere_vec_int_get(mu_syms, 0LL), 1LL)); mu_lengths; }) : ({ __auto_type __let_tmp_alive = mere_vec_int_new(__lang_current_region); __auto_type mu_alive = __let_tmp_alive; ({ (void)(__lifted_al_64(mu_nleaves, mu_alive, 0LL)); __auto_type __let_tmp_leafsym = mere_vec_int_new(__lang_current_region); __auto_type mu_leafsym = __let_tmp_leafsym; ({ (void)(__lifted_ls_65(mu_nleaves, mu_leafsym, mu_syms, 0LL)); ({ (void)(__lifted_merge_66(mu_nf, mu_alive, mu_nl, mu_nr, mu_leafsym, mu_nleaves)); __auto_type __let_tmp_root = (mere_vec_int_len(mu_nf) - 1LL); __auto_type mu_root = __let_tmp_root; ({ (void)(({ __auto_type __ia0 = mu_root; __auto_type __ia1 = 0LL; __lifted_assign_70__direct(mu_leafsym, mu_lengths, mu_nl, mu_nr, __ia0, __ia1); })); (void)(mu_maxbits); mu_lengths; }); }); }); }); }))); }); }); }); });
}
static closure_int_Vec_int __anon_57_fn(void* __env_self_void, long long mu_n) {
  __anon_57_env* __env_self = (__anon_57_env*)__env_self_void;
  return ({ __anon_134_env* __env = (__anon_134_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_134_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_134_env; __env->mu_n = mu_n; __env->mu_lengths = (__env_self->mu_lengths); (closure_int_Vec_int){.env = __env, .fn = __anon_134_fn}; });
}
static mere_vec_int* __anon_57_fn2(void* __env_self_void, long long mu_n, long long mu_maxbits) {
  __anon_57_env* __env_self = (__anon_57_env*)__env_self_void;
  return ({ __auto_type __let_tmp_bl_count = mere_vec_int_new(__lang_current_region); __auto_type mu_bl_count = __let_tmp_bl_count; ({ (void)(__lifted_z_71(mu_maxbits, mu_bl_count, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len((__env_self->mu_lengths)))) ? __lifted_cnt__rvfast_72(mu_n, (__env_self->mu_lengths), mu_bl_count, 0LL) : __lifted_cnt_73(mu_n, (__env_self->mu_lengths), mu_bl_count, 0LL))); __auto_type __let_tmp_next_code = mere_vec_int_new(__lang_current_region); __auto_type mu_next_code = __let_tmp_next_code; ({ (void)(__lifted_z2_74(mu_maxbits, mu_next_code, 0LL)); ({ (void)((((((1LL >= 0LL) && (1LL <= (mu_maxbits + 1LL))) && ((mu_maxbits + 1LL) <= mere_vec_int_len(mu_next_code))) && ((((1LL - 1LL) >= 0LL) && (((1LL - 1LL) + 1LL) <= mere_vec_int_len(mu_bl_count))) && (((((mu_maxbits + 1LL) - 1LL) - 1LL) >= 0LL) && (((((mu_maxbits + 1LL) - 1LL) - 1LL) + 1LL) <= mere_vec_int_len(mu_bl_count))))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __lifted_fill__rvfast_75__direct(mu_maxbits, mu_bl_count, mu_next_code, __ia0, __ia1); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __lifted_fill_uq44_76__direct(mu_maxbits, mu_bl_count, mu_next_code, __ia0, __ia1); }))); __auto_type __let_tmp_codes = mere_vec_int_new(__lang_current_region); __auto_type mu_codes = __let_tmp_codes; ({ (void)(__lifted_z3_77(mu_n, mu_codes, 0LL)); ({ (void)((((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len(mu_codes))) && (mu_n <= mere_vec_int_len((__env_self->mu_lengths)))) ? __lifted_asg__rvfast_78(mu_n, (__env_self->mu_lengths), mu_next_code, mu_codes, 0LL) : __lifted_asg_79(mu_n, (__env_self->mu_lengths), mu_next_code, mu_codes, 0LL))); mu_codes; }); }); }); }); }); }); });
}
static long long __anon_58_fn(void* __env_self_void, long long mu_n) {
  __anon_58_env* __env_self = (__anon_58_env*)__env_self_void;
  return ((((0LL >= 0LL) && (0LL <= mu_n)) && (mu_n <= mere_vec_int_len((__env_self->mu_v)))) ? ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go__rvfast_uq45_80__direct(mu_n, (__env_self->mu_v), __ia0, __ia1); }) : ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq46_81__direct(mu_n, (__env_self->mu_v), __ia0, __ia1); }));
}
static int __anon_59_fn(void* __env_self_void, long long mu_v) {
  __anon_59_env* __env_self = (__anon_59_env*)__env_self_void;
  return ({ (void)(mere_vec_int_push((__env_self->mu_out), ((({ long long __sa = (mu_v); long long __sn = (24LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); (void)(mere_vec_int_push((__env_self->mu_out), ((({ long long __sa = (mu_v); long long __sn = (16LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); (void)(mere_vec_int_push((__env_self->mu_out), ((({ long long __sa = (mu_v); long long __sn = (8LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (255LL)))); mere_vec_int_push((__env_self->mu_out), ((mu_v) & (255LL))); });
}
static int __anon_60_fn(void* __env_self_void, mere_vec_int* mu_src) {
  __anon_60_env* __env_self = (__anon_60_env*)__env_self_void;
  return ({ __auto_type __let_tmp_n = mere_vec_int_len(mu_src); __auto_type mu_n = __let_tmp_n; __lifted_go_uq55_109(mu_n, (__env_self->mu_out), mu_src, 0LL); });
}
static closure_int_closure_int_closure_int_closure_Vec_int_unit __anon_61_fn(void* __env_self_void, long long mu_t0) {
  __anon_61_env* __env_self = (__anon_61_env*)__env_self_void;
  return ({ __anon_135_env* __env = (__anon_135_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_135_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_135_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = mu_t0; (closure_int_closure_int_closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_135_fn, .fn2 = __anon_135_fn2}; });
}
static closure_int_closure_int_closure_Vec_int_unit __anon_61_fn2(void* __env_self_void, long long mu_t0, long long mu_t1) {
  __anon_61_env* __env_self = (__anon_61_env*)__env_self_void;
  return ({ __anon_136_env* __env = (__anon_136_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_136_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_136_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = mu_t0; __env->mu_t1 = mu_t1; (closure_int_closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_136_fn, .fn2 = __anon_136_fn2}; });
}
static closure_int_int __anon_62_fn(void* __env_self_void, long long mu_b) {
  __anon_62_env* __env_self = (__anon_62_env*)__env_self_void;
  return ({ __anon_137_env* __env = (__anon_137_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_137_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_137_env; __env->mu_a = (__env_self->mu_a); __env->mu_b = mu_b; (closure_int_int){.env = __env, .fn = __anon_137_fn}; });
}
static long long __anon_62_fn2(void* __env_self_void, long long mu_b, long long mu_c) {
  __anon_62_env* __env_self = (__anon_62_env*)__env_self_void;
  return ({ __auto_type __let_tmp_p = (((__env_self->mu_a) + mu_b) - mu_c); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_pa = ((mu_p >= (__env_self->mu_a)) ? (mu_p - (__env_self->mu_a)) : ((__env_self->mu_a) - mu_p)); __auto_type mu_pa = __let_tmp_pa; __auto_type __let_tmp_pb = ((mu_p >= mu_b) ? (mu_p - mu_b) : (mu_b - mu_p)); __auto_type mu_pb = __let_tmp_pb; __auto_type __let_tmp_pc = ((mu_p >= mu_c) ? (mu_p - mu_c) : (mu_c - mu_p)); __auto_type mu_pc = __let_tmp_pc; (((mu_pa <= mu_pb) && (mu_pa <= mu_pc)) ? (__env_self->mu_a) : ((mu_pb <= mu_pc) ? mu_b : mu_c)); });
}
static closure_Vec_int_closure_int_Vec_int __anon_63_fn(void* __env_self_void, mere_vec_int* mu_row_uq58) {
  __anon_63_env* __env_self = (__anon_63_env*)__env_self_void;
  return ({ __anon_138_env* __env = (__anon_138_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_138_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_138_env; __env->mu_row_uq58 = mu_row_uq58; __env->mu_kind = (__env_self->mu_kind); (closure_Vec_int_closure_int_Vec_int){.env = __env, .fn = __anon_138_fn, .fn2 = __anon_138_fn2}; });
}
static closure_int_Vec_int __anon_63_fn2(void* __env_self_void, mere_vec_int* mu_row_uq58, mere_vec_int* mu_prev) {
  __anon_63_env* __env_self = (__anon_63_env*)__env_self_void;
  return ({ __anon_139_env* __env = (__anon_139_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_139_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_139_env; __env->mu_row_uq58 = mu_row_uq58; __env->mu_prev = mu_prev; __env->mu_kind = (__env_self->mu_kind); (closure_int_Vec_int){.env = __env, .fn = __anon_139_fn}; });
}
static closure_int_Vec_int __anon_64_fn(void* __env_self_void, mere_vec_int* mu_prev) {
  __anon_64_env* __env_self = (__anon_64_env*)__env_self_void;
  return ({ __anon_140_env* __env = (__anon_140_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_140_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_140_env; __env->mu_row_uq63 = (__env_self->mu_row_uq63); __env->mu_prev = mu_prev; (closure_int_Vec_int){.env = __env, .fn = __anon_140_fn}; });
}
static mere_vec_int* __anon_64_fn2(void* __env_self_void, mere_vec_int* mu_prev, long long mu_bpp) {
  __anon_64_env* __env_self = (__anon_64_env*)__env_self_void;
  return ({ __auto_type __let_tmp_first = ({ __auto_type __da0 = 0LL; __auto_type __da1 = (__env_self->mu_row_uq63); __auto_type __da2 = mu_prev; __auto_type __da3 = mu_bpp; mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_first = __let_tmp_first; ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = ({ __auto_type __da0 = mu_first; mu_row_cost__direct(__da0); }); __auto_type __ia3 = mu_first; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), mu_prev, mu_bpp, __ia0, __ia1, __ia2, __ia3); }); });
}
static int __anon_65_fn(void* __env_self_void, long long mu_n) {
  __anon_65_env* __env_self = (__anon_65_env*)__env_self_void;
  return __lifted_go_uq66_116(mu_n, (__env_self->mu_pixels), 0LL);
}
static mere_vec_int* __anon_66_fn(void* __env_self_void, long long mu_n) {
  __anon_66_env* __env_self = (__anon_66_env*)__env_self_void;
  return ({ __auto_type __let_tmp_pal = mere_vec_int_new(__lang_current_region); __auto_type mu_pal = __let_tmp_pal; __lifted_go_uq67_117(mu_n, mu_pal, (__env_self->mu_pixels), 0LL); });
}
static closure_int_closure_int_int __anon_67_fn(void* __env_self_void, long long mu_r) {
  __anon_67_env* __env_self = (__anon_67_env*)__env_self_void;
  return ({ __anon_141_env* __env = (__anon_141_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_141_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_141_env; __env->mu_pal = (__env_self->mu_pal); __env->mu_r = mu_r; (closure_int_closure_int_int){.env = __env, .fn = __anon_141_fn, .fn2 = __anon_141_fn2}; });
}
static closure_int_int __anon_67_fn2(void* __env_self_void, long long mu_r, long long mu_g) {
  __anon_67_env* __env_self = (__anon_67_env*)__env_self_void;
  return ({ __anon_142_env* __env = (__anon_142_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_142_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_142_env; __env->mu_pal = (__env_self->mu_pal); __env->mu_r = mu_r; __env->mu_g = mu_g; (closure_int_int){.env = __env, .fn = __anon_142_fn}; });
}
static closure_int_closure_int_Vec_int __anon_68_fn(void* __env_self_void, long long mu_width) {
  __anon_68_env* __env_self = (__anon_68_env*)__env_self_void;
  return ({ __anon_143_env* __env = (__anon_143_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_143_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_143_env; __env->mu_width = mu_width; __env->mu_pixels = (__env_self->mu_pixels); (closure_int_closure_int_Vec_int){.env = __env, .fn = __anon_143_fn, .fn2 = __anon_143_fn2}; });
}
static closure_int_Vec_int __anon_68_fn2(void* __env_self_void, long long mu_width, long long mu_height) {
  __anon_68_env* __env_self = (__anon_68_env*)__env_self_void;
  return ({ __anon_144_env* __env = (__anon_144_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_144_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_144_env; __env->mu_width = mu_width; __env->mu_height = mu_height; __env->mu_pixels = (__env_self->mu_pixels); (closure_int_Vec_int){.env = __env, .fn = __anon_144_fn}; });
}
static closure_int_Vec_int __anon_69_fn(void* __env_self_void, long long mu_width) {
  __anon_69_env* __env_self = (__anon_69_env*)__env_self_void;
  return ({ __anon_145_env* __env = (__anon_145_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_145_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_145_env; __env->mu_pixels = (__env_self->mu_pixels); __env->mu_width = mu_width; (closure_int_Vec_int){.env = __env, .fn = __anon_145_fn}; });
}
static mere_vec_int* __anon_69_fn2(void* __env_self_void, long long mu_width, long long mu_height) {
  __anon_69_env* __env_self = (__anon_69_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = 8LL; mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2, __da3); });
}
static closure_int_Vec_int __anon_70_fn(void* __env_self_void, long long mu_width) {
  __anon_70_env* __env_self = (__anon_70_env*)__env_self_void;
  return ({ __anon_146_env* __env = (__anon_146_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_146_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_146_env; __env->mu_pixels = (__env_self->mu_pixels); __env->mu_width = mu_width; (closure_int_Vec_int){.env = __env, .fn = __anon_146_fn}; });
}
static mere_vec_int* __anon_70_fn2(void* __env_self_void, long long mu_width, long long mu_height) {
  __anon_70_env* __env_self = (__anon_70_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; __auto_type __da3 = 16LL; mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2, __da3); });
}
static int __anon_71_fn(void* __env_self_void, const char* mu_out) {
  __anon_71_env* __env_self = (__anon_71_env*)__env_self_void;
  return ({ __auto_type __let_tmp_data = ({ __auto_type __da0 = (__env_self->mu_path); mu_load__direct(__da0); }); __auto_type mu_data = __let_tmp_data; __auto_type __let_tmp_f = ({ __auto_type __da0 = mu_data; mu_read_ppm__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); __auto_type mu_f = __let_tmp_f; ((mere_vec_int_len(mu_f) < 4LL) ? ({ const char* __ps = (__sl_25.s);            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }) : ({ __auto_type __let_tmp_width = mere_vec_int_get(mu_f, 0LL); __auto_type mu_width = __let_tmp_width; __auto_type __let_tmp_height = mere_vec_int_get(mu_f, 1LL); __auto_type mu_height = __let_tmp_height; __auto_type __let_tmp_maxval = mere_vec_int_get(mu_f, 2LL); __auto_type mu_maxval = __let_tmp_maxval; __auto_type __let_tmp_start = mere_vec_int_get(mu_f, 3LL); __auto_type mu_start = __let_tmp_start; __auto_type __let_tmp_wide = (mu_maxval > 255LL); __auto_type mu_wide = __let_tmp_wide; __auto_type __let_tmp_bytes_per_sample = (mu_wide ? 2LL : 1LL); __auto_type mu_bytes_per_sample = __let_tmp_bytes_per_sample; __auto_type __let_tmp_pixels = mere_vec_int_new(__lang_current_region); __auto_type mu_pixels = __let_tmp_pixels; ({ (void)(__lifted_copy_uq74_130(mu_width, mu_height, mu_bytes_per_sample, mu_pixels, mu_data, mu_start, 0LL)); __auto_type __let_tmp_png = __lang_bytes_of_vec((mu_wide ? ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode16__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }) : ({ __auto_type __da0 = mu_pixels; __auto_type __da1 = mu_width; __auto_type __da2 = mu_height; mu_encode_rgb8__Vec_int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }))); __auto_type mu_png = __let_tmp_png; (void)(__lang_write_bytes(mu_out, mu_png)); ({ const char* __ps = ({ __auto_type __bc252 = show_int(mu_width); __auto_type __bc253 = (__sl_14.s); __auto_type __bc254 = __lang_str_concat(__bc252, __bc253); __auto_type __bc255 = show_int(mu_height); __auto_type __bc256 = __lang_str_concat(__bc254, __bc255); __auto_type __bc257 = (__sl_4.s); __auto_type __bc258 = __lang_str_concat(__bc256, __bc257); __auto_type __bc259 = show_int((mu_wide ? 16LL : 8LL)); __auto_type __bc260 = __lang_str_concat(__bc258, __bc259); __auto_type __bc261 = (__sl_22.s); __auto_type __bc262 = __lang_str_concat(__bc260, __bc261); __auto_type __bc263 = mu_out; __auto_type __bc264 = __lang_str_concat(__bc262, __bc263); __auto_type __bc265 = (__sl_23.s); __auto_type __bc266 = __lang_str_concat(__bc264, __bc265); __auto_type __bc267 = show_int(((long long)(mu_png)->len)); __auto_type __bc268 = __lang_str_concat(__bc266, __bc267); __auto_type __bc269 = (__sl_24.s); __auto_type __bc270 = __lang_str_concat(__bc268, __bc269); __bc270; });            fwrite(__ps, 1, __lang_str_size(__ps), stdout);            fputc('\n', stdout); 0; }); }); })); });
}
static long long __anon_72_fn(void* __env_self_void, long long mu_c) {
  __anon_72_env* __env_self = (__anon_72_env*)__env_self_void;
  return (((__env_self->mu_i) == 8LL) ? mu_c : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((((mu_c) & (1LL)) == 1LL) ? ((({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) ^ (3988292384LL)) : ({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })); __lifted_go_0__direct(__ia0, __ia1); }));
}
static long long __anon_73_fn(void* __env_self_void, long long mu_crc) {
  __anon_73_env* __env_self = (__anon_73_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_crc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get_unchecked((__env_self->mu_bytes), (__env_self->mu_i)); mu_crc_byte__direct(__da0, __da1); }); __lifted_go__rvfast_1__direct((__env_self->mu_n), (__env_self->mu_bytes), __ia0, __ia1); }));
}
static long long __anon_74_fn(void* __env_self_void, long long mu_crc) {
  __anon_74_env* __env_self = (__anon_74_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_crc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get((__env_self->mu_bytes), (__env_self->mu_i)); mu_crc_byte__direct(__da0, __da1); }); __lifted_go_uq1_2__direct((__env_self->mu_n), (__env_self->mu_bytes), __ia0, __ia1); }));
}
static long long __anon_75_fn(void* __env_self_void, long long mu_acc) {
  __anon_75_env* __env_self = (__anon_75_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_acc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((mu_acc) | (({ long long __sa = (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); })); long long __sn = ((__env_self->mu_i)); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }))); __lifted_go_uq2_3__direct((__env_self->mu_n), (__env_self->mu_s), (__env_self->mu_data), __ia0, __ia1); }));
}
static closure_int_closure_int_int __anon_76_fn(void* __env_self_void, long long mu_code0) {
  __anon_76_env* __env_self = (__anon_76_env*)__env_self_void;
  return ({ __anon_147_env* __env = (__anon_147_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_147_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_147_env; __env->mu_len = (__env_self->mu_len); __env->mu_code0 = mu_code0; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = (__env_self->mu_count); __env->mu_syms = (__env_self->mu_syms); (closure_int_closure_int_int){.env = __env, .fn = __anon_147_fn, .fn2 = __anon_147_fn2}; });
}
static closure_int_int __anon_76_fn2(void* __env_self_void, long long mu_code0, long long mu_first) {
  __anon_76_env* __env_self = (__anon_76_env*)__env_self_void;
  return ({ __anon_148_env* __env = (__anon_148_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_148_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_148_env; __env->mu_len = (__env_self->mu_len); __env->mu_code0 = mu_code0; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = (__env_self->mu_count); __env->mu_first = mu_first; __env->mu_syms = (__env_self->mu_syms); (closure_int_int){.env = __env, .fn = __anon_148_fn}; });
}
static closure_int_closure_int_int __anon_77_fn(void* __env_self_void, long long mu_code0) {
  __anon_77_env* __env_self = (__anon_77_env*)__env_self_void;
  return ({ __anon_149_env* __env = (__anon_149_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_149_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_149_env; __env->mu_len = (__env_self->mu_len); __env->mu_code0 = mu_code0; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = (__env_self->mu_count); __env->mu_syms = (__env_self->mu_syms); (closure_int_closure_int_int){.env = __env, .fn = __anon_149_fn, .fn2 = __anon_149_fn2}; });
}
static closure_int_int __anon_77_fn2(void* __env_self_void, long long mu_code0, long long mu_first) {
  __anon_77_env* __env_self = (__anon_77_env*)__env_self_void;
  return ({ __anon_150_env* __env = (__anon_150_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_150_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_150_env; __env->mu_len = (__env_self->mu_len); __env->mu_code0 = mu_code0; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = (__env_self->mu_count); __env->mu_first = mu_first; __env->mu_syms = (__env_self->mu_syms); (closure_int_int){.env = __env, .fn = __anon_150_fn}; });
}
static int __anon_78_fn(void* __env_self_void, int mu_seen_sep) {
  __anon_78_env* __env_self = (__anon_78_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_len)) ? 0 : ({ __auto_type __let_tmp_b = mere_vec_int_get_unchecked((__env_self->mu_data), ((__env_self->mu_d) + (__env_self->mu_i))); __auto_type mu_b = __let_tmp_b; (((mu_b == 0LL) && (mu_seen_sep == 0)) ? ({ (void)(mere_strbuf_push((__env_self->mu_buf), (__sl_30.s))); ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = 1; __lifted_chars__rvfast_36__direct((__env_self->mu_len), (__env_self->mu_data), (__env_self->mu_d), (__env_self->mu_buf), __ia0, __ia1); }); }) : ({ (void)(mere_strbuf_push((__env_self->mu_buf), __lang_char_at_chr(mu_b))); ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_seen_sep; __lifted_chars__rvfast_36__direct((__env_self->mu_len), (__env_self->mu_data), (__env_self->mu_d), (__env_self->mu_buf), __ia0, __ia1); }); })); }));
}
static int __anon_79_fn(void* __env_self_void, int mu_seen_sep) {
  __anon_79_env* __env_self = (__anon_79_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_len)) ? 0 : ({ __auto_type __let_tmp_b = mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_d) + (__env_self->mu_i))); __auto_type mu_b = __let_tmp_b; (((mu_b == 0LL) && (mu_seen_sep == 0)) ? ({ (void)(mere_strbuf_push((__env_self->mu_buf), (__sl_30.s))); ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = 1; __lifted_chars_37__direct((__env_self->mu_len), (__env_self->mu_data), (__env_self->mu_d), (__env_self->mu_buf), __ia0, __ia1); }); }) : ({ (void)(mere_strbuf_push((__env_self->mu_buf), __lang_char_at_chr(mu_b))); ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_seen_sep; __lifted_chars_37__direct((__env_self->mu_len), (__env_self->mu_data), (__env_self->mu_d), (__env_self->mu_buf), __ia0, __ia1); }); })); }));
}
static mere_bytebuf* __anon_80_fn(void* __env_self_void, long long mu_off) {
  __anon_80_env* __env_self = (__anon_80_env*)__env_self_void;
  return (((__env_self->mu_p_uq28) == 7LL) ? (__env_self->mu_out) : ({ __auto_type __let_tmp_pw = ({ __auto_type __da0 = (__env_self->mu_p_uq28); __auto_type __da1 = (__env_self->mu_width); mu_pass_width__direct(__da0, __da1); }); __auto_type mu_pw = __let_tmp_pw; __auto_type __let_tmp_ph = ({ __auto_type __da0 = (__env_self->mu_p_uq28); __auto_type __da1 = (__env_self->mu_height); mu_pass_height__direct(__da0, __da1); }); __auto_type mu_ph = __let_tmp_ph; (((mu_pw <= 0LL) || (mu_ph <= 0LL)) ? ({ __auto_type __ia0 = ((__env_self->mu_p_uq28) + 1LL); __auto_type __ia1 = mu_off; __lifted_pass_43__direct((__env_self->mu_out), (__env_self->mu_width), (__env_self->mu_height), (__env_self->mu_raw), (__env_self->mu_bpp), __ia0, __ia1); }) : ({ __auto_type __let_tmp_sub = ({ __auto_type __da0 = (__env_self->mu_raw); __auto_type __da1 = mu_off; __auto_type __da2 = mu_pw; __auto_type __da3 = mu_ph; __auto_type __da4 = (__env_self->mu_bpp); mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3, __da4); }); __auto_type mu_sub = __let_tmp_sub; ({ (void)(__lifted_place_44(mu_ph, mu_pw, (__env_self->mu_p_uq28), (__env_self->mu_bpp), (__env_self->mu_width), (__env_self->mu_out), mu_sub, 0LL)); ({ __auto_type __ia0 = ((__env_self->mu_p_uq28) + 1LL); __auto_type __ia1 = (mu_off + (mu_ph * ((mu_pw * (__env_self->mu_bpp)) + 1LL))); __lifted_pass_43__direct((__env_self->mu_out), (__env_self->mu_width), (__env_self->mu_height), (__env_self->mu_raw), (__env_self->mu_bpp), __ia0, __ia1); }); }); })); }));
}
static long long __anon_81_fn(void* __env_self_void, long long mu_c) {
  __anon_81_env* __env_self = (__anon_81_env*)__env_self_void;
  return (((__env_self->mu_i) == 8LL) ? mu_c : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((((mu_c) & (1LL)) == 1LL) ? ((({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) ^ (3988292384LL)) : ({ long long __sa = (mu_c); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })); __lifted_go_uq31_49__direct(__ia0, __ia1); }));
}
static long long __anon_82_fn(void* __env_self_void, long long mu_crc) {
  __anon_82_env* __env_self = (__anon_82_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_crc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get_unchecked((__env_self->mu_bytes), (__env_self->mu_i)); mu_crc_byte__v2__direct(__da0, __da1); }); __lifted_go__rvfast_uq32_50__direct((__env_self->mu_n), (__env_self->mu_bytes), __ia0, __ia1); }));
}
static long long __anon_83_fn(void* __env_self_void, long long mu_crc) {
  __anon_83_env* __env_self = (__anon_83_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_crc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ({ __auto_type __da0 = mu_crc; __auto_type __da1 = mere_vec_int_get((__env_self->mu_bytes), (__env_self->mu_i)); mu_crc_byte__v2__direct(__da0, __da1); }); __lifted_go_uq33_51__direct((__env_self->mu_n), (__env_self->mu_bytes), __ia0, __ia1); }));
}
static long long __anon_84_fn(void* __env_self_void, long long mu_acc) {
  __anon_84_env* __env_self = (__anon_84_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_acc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((({ long long __sa = (mu_acc); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); })) | (((({ long long __sa = ((__env_self->mu_v)); long long __sn = ((__env_self->mu_i)); ((unsigned long long)__sn > 63ULL) ? (__sa < 0 ? -1LL : 0LL) : (__sa >> __sn); })) & (1LL)))); __lifted_go_uq35_53__direct((__env_self->mu_n), (__env_self->mu_v), __ia0, __ia1); }));
}
static closure_int_tuple_int_int __anon_85_fn(void* __env_self_void, long long mu_blen) {
  __anon_85_env* __env_self = (__anon_85_env*)__env_self_void;
  return ({ __anon_151_env* __env = (__anon_151_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_151_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_151_env; __env->mu_d = (__env_self->mu_d); __env->mu_max_back = (__env_self->mu_max_back); __env->mu_blen = mu_blen; __env->mu_pos = (__env_self->mu_pos); __env->mu_n = (__env_self->mu_n); __env->mu_data = (__env_self->mu_data); (closure_int_tuple_int_int){.env = __env, .fn = __anon_151_fn}; });
}
static tuple_int_int __anon_85_fn2(void* __env_self_void, long long mu_blen, long long mu_bdist) {
  __anon_85_env* __env_self = (__anon_85_env*)__env_self_void;
  return (((__env_self->mu_d) > (__env_self->mu_max_back)) ? ((tuple_int_int){.f0 = mu_blen, .f1 = mu_bdist}) : ({ __auto_type __let_tmp_start = ((__env_self->mu_pos) - (__env_self->mu_d)); __auto_type mu_start = __let_tmp_start; ({ __auto_type __let_tmp_l = (((((0LL >= 0LL) && (0LL <= 258LL)) && (((((__env_self->mu_pos) + 0LL) >= 0LL) && ((((__env_self->mu_pos) + 0LL) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))) && ((((__env_self->mu_pos) + (258LL - 1LL)) >= 0LL) && ((((__env_self->mu_pos) + (258LL - 1LL)) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))))) && ((((mu_start + 0LL) >= 0LL) && (((mu_start + 0LL) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))) && (((mu_start + (258LL - 1LL)) >= 0LL) && (((mu_start + (258LL - 1LL)) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))))) ? __lifted_mlen__rvfast_59((__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), mu_start, 0LL) : __lifted_mlen_60((__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), mu_start, 0LL)); __auto_type mu_l = __let_tmp_l; ((mu_l > mu_blen) ? ({ __auto_type __ia0 = ((__env_self->mu_d) + 1LL); __auto_type __ia1 = mu_l; __auto_type __ia2 = (__env_self->mu_d); __lifted_best_58__direct((__env_self->mu_max_back), (__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); }) : ({ __auto_type __ia0 = ((__env_self->mu_d) + 1LL); __auto_type __ia1 = mu_blen; __auto_type __ia2 = mu_bdist; __lifted_best_58__direct((__env_self->mu_max_back), (__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); })); }); }));
}
static long long __anon_86_fn(void* __env_self_void, long long mu_best_uq41) {
  __anon_86_env* __env_self = (__anon_86_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_np)) ? mu_best_uq41 : ((((mere_vec_int_get_unchecked((__env_self->mu_alive), (__env_self->mu_i)) == 1LL) && ((__env_self->mu_i) != (__env_self->mu_skip))) && ((mu_best_uq41 < 0LL) || (mere_vec_int_get_unchecked((__env_self->mu_nf), (__env_self->mu_i)) < mere_vec_int_get((__env_self->mu_nf), mu_best_uq41)))) ? ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_i); __lifted_go__rvfast_uq40_68__direct((__env_self->mu_np), (__env_self->mu_alive), (__env_self->mu_skip), (__env_self->mu_nf), __ia0, __ia1); }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_best_uq41; __lifted_go__rvfast_uq40_68__direct((__env_self->mu_np), (__env_self->mu_alive), (__env_self->mu_skip), (__env_self->mu_nf), __ia0, __ia1); })));
}
static long long __anon_87_fn(void* __env_self_void, long long mu_best_uq43) {
  __anon_87_env* __env_self = (__anon_87_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_np)) ? mu_best_uq43 : ((((mere_vec_int_get((__env_self->mu_alive), (__env_self->mu_i)) == 1LL) && ((__env_self->mu_i) != (__env_self->mu_skip))) && ((mu_best_uq43 < 0LL) || (mere_vec_int_get((__env_self->mu_nf), (__env_self->mu_i)) < mere_vec_int_get((__env_self->mu_nf), mu_best_uq43)))) ? ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_i); __lifted_go_uq42_69__direct((__env_self->mu_np), (__env_self->mu_alive), (__env_self->mu_skip), (__env_self->mu_nf), __ia0, __ia1); }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_best_uq43; __lifted_go_uq42_69__direct((__env_self->mu_np), (__env_self->mu_alive), (__env_self->mu_skip), (__env_self->mu_nf), __ia0, __ia1); })));
}
static int __anon_88_fn(void* __env_self_void, long long mu_depth) {
  __anon_88_env* __env_self = (__anon_88_env*)__env_self_void;
  return ({ __auto_type __let_tmp_sym = mere_vec_int_get((__env_self->mu_leafsym), (__env_self->mu_node)); __auto_type mu_sym = __let_tmp_sym; ((mu_sym >= 0LL) ? mere_vec_int_set((__env_self->mu_lengths), mu_sym, mu_depth) : ({ (void)(({ __auto_type __ia0 = mere_vec_int_get((__env_self->mu_nl), (__env_self->mu_node)); __auto_type __ia1 = (mu_depth + 1LL); __lifted_assign_70__direct((__env_self->mu_leafsym), (__env_self->mu_lengths), (__env_self->mu_nl), (__env_self->mu_nr), __ia0, __ia1); })); ({ __auto_type __ia0 = mere_vec_int_get((__env_self->mu_nr), (__env_self->mu_node)); __auto_type __ia1 = (mu_depth + 1LL); __lifted_assign_70__direct((__env_self->mu_leafsym), (__env_self->mu_lengths), (__env_self->mu_nl), (__env_self->mu_nr), __ia0, __ia1); }); })); });
}
static int __anon_89_fn(void* __env_self_void, long long mu_code) {
  __anon_89_env* __env_self = (__anon_89_env*)__env_self_void;
  return (((__env_self->mu_bits) > (__env_self->mu_maxbits)) ? 0 : ({ __auto_type __let_tmp_c = ({ long long __sa = ((mu_code + mere_vec_int_get_unchecked((__env_self->mu_bl_count), ((__env_self->mu_bits) - 1LL)))); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type mu_c = __let_tmp_c; (void)(mere_vec_int_set_unchecked((__env_self->mu_next_code), (__env_self->mu_bits), mu_c)); ({ __auto_type __ia0 = ((__env_self->mu_bits) + 1LL); __auto_type __ia1 = mu_c; __lifted_fill__rvfast_75__direct((__env_self->mu_maxbits), (__env_self->mu_bl_count), (__env_self->mu_next_code), __ia0, __ia1); }); }));
}
static int __anon_90_fn(void* __env_self_void, long long mu_code) {
  __anon_90_env* __env_self = (__anon_90_env*)__env_self_void;
  return (((__env_self->mu_bits) > (__env_self->mu_maxbits)) ? 0 : ({ __auto_type __let_tmp_c = ({ long long __sa = ((mu_code + mere_vec_int_get((__env_self->mu_bl_count), ((__env_self->mu_bits) - 1LL)))); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type mu_c = __let_tmp_c; (void)(mere_vec_int_set((__env_self->mu_next_code), (__env_self->mu_bits), mu_c)); ({ __auto_type __ia0 = ((__env_self->mu_bits) + 1LL); __auto_type __ia1 = mu_c; __lifted_fill_uq44_76__direct((__env_self->mu_maxbits), (__env_self->mu_bl_count), (__env_self->mu_next_code), __ia0, __ia1); }); }));
}
static long long __anon_91_fn(void* __env_self_void, long long mu_m) {
  __anon_91_env* __env_self = (__anon_91_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_m : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((mere_vec_int_get_unchecked((__env_self->mu_v), (__env_self->mu_i)) > mu_m) ? mere_vec_int_get_unchecked((__env_self->mu_v), (__env_self->mu_i)) : mu_m); __lifted_go__rvfast_uq45_80__direct((__env_self->mu_n), (__env_self->mu_v), __ia0, __ia1); }));
}
static long long __anon_92_fn(void* __env_self_void, long long mu_m) {
  __anon_92_env* __env_self = (__anon_92_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_m : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((mere_vec_int_get((__env_self->mu_v), (__env_self->mu_i)) > mu_m) ? mere_vec_int_get((__env_self->mu_v), (__env_self->mu_i)) : mu_m); __lifted_go_uq46_81__direct((__env_self->mu_n), (__env_self->mu_v), __ia0, __ia1); }));
}
static closure_int_int __anon_93_fn(void* __env_self_void, long long mu_i) {
  __anon_93_env* __env_self = (__anon_93_env*)__env_self_void;
  return ({ __anon_152_env* __env = (__anon_152_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_152_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_152_env; __env->mu_i = mu_i; __env->mu_v = (__env_self->mu_v); (closure_int_int){.env = __env, .fn = __anon_152_fn}; });
}
static long long __anon_93_fn2(void* __env_self_void, long long mu_i, long long mu_floor) {
  __anon_93_env* __env_self = (__anon_93_env*)__env_self_void;
  return ((mu_i < mu_floor) ? mu_floor : ((mere_vec_int_get((__env_self->mu_v), mu_i) > 0LL) ? mu_i : ({ __auto_type __ia0 = (__env_self->mu_v); __auto_type __ia1 = (mu_i - 1LL); __auto_type __ia2 = mu_floor; __lifted_last_nz_87__direct(__ia0, __ia1, __ia2); })));
}
static closure_int_int __anon_94_fn(void* __env_self_void, long long mu_a) {
  __anon_94_env* __env_self = (__anon_94_env*)__env_self_void;
  return ({ __anon_153_env* __env = (__anon_153_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_153_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_153_env; __env->mu_i = (__env_self->mu_i); __env->mu_n = (__env_self->mu_n); __env->mu_a = mu_a; __env->mu_data = (__env_self->mu_data); (closure_int_int){.env = __env, .fn = __anon_153_fn}; });
}
static long long __anon_94_fn2(void* __env_self_void, long long mu_a, long long mu_b) {
  __anon_94_env* __env_self = (__anon_94_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? ((mu_b * 65536LL) + mu_a) : ({ __auto_type __let_tmp_a2 = __lang_imod((mu_a + mere_vec_int_get_unchecked((__env_self->mu_data), (__env_self->mu_i))), 65521LL); __auto_type mu_a2 = __let_tmp_a2; ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_a2; __auto_type __ia2 = __lang_imod((mu_b + mu_a2), 65521LL); __lifted_go__rvfast_uq56_110__direct((__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); }); }));
}
static closure_int_int __anon_95_fn(void* __env_self_void, long long mu_a) {
  __anon_95_env* __env_self = (__anon_95_env*)__env_self_void;
  return ({ __anon_154_env* __env = (__anon_154_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_154_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_154_env; __env->mu_i = (__env_self->mu_i); __env->mu_n = (__env_self->mu_n); __env->mu_a = mu_a; __env->mu_data = (__env_self->mu_data); (closure_int_int){.env = __env, .fn = __anon_154_fn}; });
}
static long long __anon_95_fn2(void* __env_self_void, long long mu_a, long long mu_b) {
  __anon_95_env* __env_self = (__anon_95_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? ((mu_b * 65536LL) + mu_a) : ({ __auto_type __let_tmp_a2 = __lang_imod((mu_a + mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i))), 65521LL); __auto_type mu_a2 = __let_tmp_a2; ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_a2; __auto_type __ia2 = __lang_imod((mu_b + mu_a2), 65521LL); __lifted_go_uq57_111__direct((__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); }); }));
}
static long long __anon_96_fn(void* __env_self_void, long long mu_acc) {
  __anon_96_env* __env_self = (__anon_96_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_acc : ({ __auto_type __let_tmp_b = mere_vec_int_get_unchecked((__env_self->mu_row_uq60), (__env_self->mu_i)); __auto_type mu_b = __let_tmp_b; ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (mu_acc + ((mu_b < 128LL) ? mu_b : (256LL - mu_b))); __lifted_go__rvfast_uq61_113__direct((__env_self->mu_n), (__env_self->mu_row_uq60), __ia0, __ia1); }); }));
}
static long long __anon_97_fn(void* __env_self_void, long long mu_acc) {
  __anon_97_env* __env_self = (__anon_97_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? mu_acc : ({ __auto_type __let_tmp_b = mere_vec_int_get((__env_self->mu_row_uq60), (__env_self->mu_i)); __auto_type mu_b = __let_tmp_b; ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (mu_acc + ((mu_b < 128LL) ? mu_b : (256LL - mu_b))); __lifted_go_uq62_114__direct((__env_self->mu_n), (__env_self->mu_row_uq60), __ia0, __ia1); }); }));
}
static closure_int_closure_Vec_int_Vec_int __anon_98_fn(void* __env_self_void, long long mu_bk) {
  __anon_98_env* __env_self = (__anon_98_env*)__env_self_void;
  return ({ __anon_155_env* __env = (__anon_155_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_155_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_155_env; __env->mu_kind = (__env_self->mu_kind); __env->mu_bk = mu_bk; __env->mu_row_uq63 = (__env_self->mu_row_uq63); __env->mu_prev = (__env_self->mu_prev); __env->mu_bpp = (__env_self->mu_bpp); (closure_int_closure_Vec_int_Vec_int){.env = __env, .fn = __anon_155_fn, .fn2 = __anon_155_fn2}; });
}
static closure_Vec_int_Vec_int __anon_98_fn2(void* __env_self_void, long long mu_bk, long long mu_bc) {
  __anon_98_env* __env_self = (__anon_98_env*)__env_self_void;
  return ({ __anon_156_env* __env = (__anon_156_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_156_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_156_env; __env->mu_kind = (__env_self->mu_kind); __env->mu_bk = mu_bk; __env->mu_row_uq63 = (__env_self->mu_row_uq63); __env->mu_prev = (__env_self->mu_prev); __env->mu_bpp = (__env_self->mu_bpp); __env->mu_bc = mu_bc; (closure_Vec_int_Vec_int){.env = __env, .fn = __anon_156_fn}; });
}
static int __anon_99_fn(void* __env_self_void, mere_vec_int* mu_prev) {
  __anon_99_env* __env_self = (__anon_99_env*)__env_self_void;
  return (((__env_self->mu_y) == (__env_self->mu_height)) ? 0 : ({ __auto_type __let_tmp_row = mere_vec_int_new(__lang_current_region); __auto_type mu_row = __let_tmp_row; ({ (void)(__lifted_col_uq71_123((__env_self->mu_stride), (__env_self->mu_grey), mu_row, (__env_self->mu_pixels), (__env_self->mu_y), (__env_self->mu_width), (__env_self->mu_indexed), (__env_self->mu_pal), 0LL)); (void)(({ __auto_type __da0 = (__env_self->mu_raw); __auto_type __da1 = ({ __auto_type __da0 = mu_row; __auto_type __da1 = mu_prev; __auto_type __da2 = (__env_self->mu_bpp); mu_best_filter__Vec_int__Vec_int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2); }); mu_push_all__direct(__da0, __da1); })); ({ __auto_type __ia0 = ((__env_self->mu_y) + 1LL); __auto_type __ia1 = mu_row; __lifted_rows_122__direct((__env_self->mu_height), (__env_self->mu_stride), (__env_self->mu_grey), (__env_self->mu_pixels), (__env_self->mu_width), (__env_self->mu_indexed), (__env_self->mu_pal), (__env_self->mu_raw), (__env_self->mu_bpp), __ia0, __ia1); }); }); }));
}
static int __anon_100_fn(void* __env_self_void, long long mu_i) {
  __anon_100_env* __env_self = (__anon_100_env*)__env_self_void;
  return ((mu_i >= ((long long) __lang_str_size((__env_self->mu_s)))) ? 0 : ({ (void)(mere_vec_int_push((__env_self->mu_out), __lang_ord(__lang_char_at((__env_self->mu_s), mu_i)))); ({ __auto_type __ia0 = (__env_self->mu_s); __auto_type __ia1 = (mu_i + 1LL); __lifted_header_127__direct((__env_self->mu_out), __ia0, __ia1); }); }));
}
static closure_int_closure_int_unit __anon_101_fn(void* __env_self_void, long long mu_got) {
  __anon_101_env* __env_self = (__anon_101_env*)__env_self_void;
  return ({ __anon_157_env* __env = (__anon_157_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_157_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_157_env; __env->mu_i = (__env_self->mu_i); __env->mu_n = (__env_self->mu_n); __env->mu_data = (__env_self->mu_data); __env->mu_got = mu_got; __env->mu_acc = (__env_self->mu_acc); (closure_int_closure_int_unit){.env = __env, .fn = __anon_157_fn, .fn2 = __anon_157_fn2}; });
}
static closure_int_unit __anon_101_fn2(void* __env_self_void, long long mu_got, long long mu_cur) {
  __anon_101_env* __env_self = (__anon_101_env*)__env_self_void;
  return ({ __anon_158_env* __env = (__anon_158_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_158_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_158_env; __env->mu_i = (__env_self->mu_i); __env->mu_n = (__env_self->mu_n); __env->mu_data = (__env_self->mu_data); __env->mu_got = mu_got; __env->mu_cur = mu_cur; __env->mu_acc = (__env_self->mu_acc); (closure_int_unit){.env = __env, .fn = __anon_158_fn}; });
}
static const char* __anon_102_fn(void* __env_self_void, const char* mu_acc) {
  __anon_102_env* __env_self = (__anon_102_env*)__env_self_void;
  return (((__env_self->mu_i) >= mere_vec_str_len((__env_self->mu_t))) ? mu_acc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = __lang_str_concat(__lang_str_concat(mu_acc, (__sl_31.s)), mere_vec_str_get_unchecked((__env_self->mu_t), (__env_self->mu_i))); __lifted_join__rvfast_131__direct((__env_self->mu_t), __ia0, __ia1); }));
}
static const char* __anon_103_fn(void* __env_self_void, const char* mu_acc) {
  __anon_103_env* __env_self = (__anon_103_env*)__env_self_void;
  return (((__env_self->mu_i) >= mere_vec_str_len((__env_self->mu_t))) ? mu_acc : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = __lang_str_concat(__lang_str_concat(mu_acc, (__sl_31.s)), mere_vec_str_get((__env_self->mu_t), (__env_self->mu_i))); __lifted_join_132__direct((__env_self->mu_t), __ia0, __ia1); }));
}
static long long __anon_104_fn(void* __env_self_void, long long mu_x) {
  __anon_104_env* __env_self = (__anon_104_env*)__env_self_void;
  return ((__env_self->mu_a) + mu_x);
}
static long long __anon_105_fn(void* __env_self_void, long long mu_x) {
  __anon_105_env* __env_self = (__anon_105_env*)__env_self_void;
  return ((__env_self->mu_a) * mu_x);
}
static long long __anon_106_fn(void* __env_self_void, closure_int_closure_int_int mu_f) {
  __anon_106_env* __env_self = (__anon_106_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_xs); (list_int__tag(__scrut) == 0) ? ({ (__env_self->mu_acc); }) : ((list_int__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_int__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_int__node(__scrut)->payload.Cons).f1; ({ __auto_type __da0 = mu_t; __auto_type __da1 = ({ __auto_type __c2 = mu_f; __auto_type __a0 = (__env_self->mu_acc); __typeof__(__c2.fn(__c2.env, __a0)) __i2; if (!__c2.fn2) __i2 = __c2.fn(__c2.env, __a0); __auto_type __a1 = mu_h; __c2.fn2 ? __c2.fn2(__c2.env, __a0, __a1) : __i2.fn(__i2.env, __a1); }); __auto_type __da2 = mu_f; mu_list_fold__direct(__da0, __da1, __da2); }); }) : (({ __lang_fail_impl("no matching arm in match"); 0; }))); });
}
static list_int __anon_107_fn(void* __env_self_void, list_int mu_acc) {
  __anon_107_env* __env_self = (__anon_107_env*)__env_self_void;
  return (((__env_self->mu_i) < (__env_self->mu_a)) ? mu_acc : ({ __auto_type __da0 = (__env_self->mu_a); __auto_type __da1 = ((__env_self->mu_i) - 1LL); __auto_type __da2 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = (__env_self->mu_i), .f1 = mu_acc}); list_int__mk(1, __p); }); mu__range_down__direct(__da0, __da1, __da2); }));
}
static long long __anon_108_fn(void* __env_self_void, long long mu_x) {
  __anon_108_env* __env_self = (__anon_108_env*)__env_self_void;
  return ((__env_self->mu_a) + mu_x);
}
static long long __anon_109_fn(void* __env_self_void, long long mu_x) {
  __anon_109_env* __env_self = (__anon_109_env*)__env_self_void;
  return ((__env_self->mu_a) * mu_x);
}
static closure_str_str __anon_110_fn(void* __env_self_void, long long mu_len) {
  __anon_110_env* __env_self = (__anon_110_env*)__env_self_void;
  return ({ __anon_159_env* __env = (__anon_159_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_159_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_159_env; __env->mu_cs = (__env_self->mu_cs); __env->mu_start = (__env_self->mu_start); __env->mu_len = mu_len; (closure_str_str){.env = __env, .fn = __anon_159_fn}; });
}
static const char* __anon_110_fn2(void* __env_self_void, long long mu_len, const char* mu_acc) {
  __anon_110_env* __env_self = (__anon_110_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_cs); (list_str__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; (((__env_self->mu_start) > 0LL) ? ({ __auto_type __da0 = mu_t; __auto_type __da1 = ((__env_self->mu_start) - 1LL); __auto_type __da2 = mu_len; __auto_type __da3 = mu_acc; mu__u8_slice__direct(__da0, __da1, __da2, __da3); }) : ((mu_len > 0LL) ? ({ __auto_type __da0 = mu_t; __auto_type __da1 = 0LL; __auto_type __da2 = (mu_len - 1LL); __auto_type __da3 = __lang_str_concat(mu_acc, mu_h); mu__u8_slice__direct(__da0, __da1, __da2, __da3); }) : mu_acc)); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}
static const char* __anon_111_fn(void* __env_self_void, const char* mu_acc) {
  __anon_111_env* __env_self = (__anon_111_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_cs); (list_str__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; (((__env_self->mu_start) > 0LL) ? ({ __auto_type __da0 = mu_t; __auto_type __da1 = ((__env_self->mu_start) - 1LL); __auto_type __da2 = (__env_self->mu_len); __auto_type __da3 = mu_acc; mu__u8_slice__direct(__da0, __da1, __da2, __da3); }) : (((__env_self->mu_len) > 0LL) ? ({ __auto_type __da0 = mu_t; __auto_type __da1 = 0LL; __auto_type __da2 = ((__env_self->mu_len) - 1LL); __auto_type __da3 = __lang_str_concat(mu_acc, mu_h); mu__u8_slice__direct(__da0, __da1, __da2, __da3); }) : mu_acc)); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}
static const char* __anon_112_fn(void* __env_self_void, long long mu_len) {
  __anon_112_env* __env_self = (__anon_112_env*)__env_self_void;
  return ({ __auto_type __da0 = __lang_utf8_chars((__env_self->mu_s)); __auto_type __da1 = (__env_self->mu_start); __auto_type __da2 = mu_len; __auto_type __da3 = (__sl_0.s); mu__u8_slice__direct(__da0, __da1, __da2, __da3); });
}
static closure_int_int __anon_113_fn(void* __env_self_void, long long mu_n) {
  __anon_113_env* __env_self = (__anon_113_env*)__env_self_void;
  return ({ __anon_160_env* __env = (__anon_160_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_160_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_160_env; __env->mu_i = (__env_self->mu_i); __env->mu_n = mu_n; __env->mu_s = (__env_self->mu_s); (closure_int_int){.env = __env, .fn = __anon_160_fn}; });
}
static long long __anon_113_fn2(void* __env_self_void, long long mu_n, long long mu_acc) {
  __anon_113_env* __env_self = (__anon_113_env*)__env_self_void;
  return (((__env_self->mu_i) >= mu_n) ? mu_acc : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at((__env_self->mu_s), (__env_self->mu_i))); __auto_type mu_b0 = __let_tmp_b0; ((mu_b0 < 194LL) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 1LL); __auto_type __da2 = mu_n; __auto_type __da3 = (mu_acc + 1LL); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : ((mu_b0 < 224LL) ? ((((__env_self->mu_i) + 1LL) < mu_n) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 2LL); __auto_type __da2 = mu_n; __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ((__lang_imod(mu_b0, 32LL) * 64LL) + __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL)); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL)) : ((mu_b0 < 240LL) ? ((((__env_self->mu_i) + 2LL) < mu_n) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 3LL); __auto_type __da2 = mu_n; __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc282 = mu_b0; __auto_type __bc283 = 16LL; __auto_type __bc284 = __lang_imod(__bc282, __bc283); __auto_type __bc285 = 4096LL; __auto_type __bc286 = (__bc284 * __bc285); __auto_type __bc287 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL) * 64LL); __auto_type __bc288 = (__bc286 + __bc287); __auto_type __bc289 = __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 2LL))), 64LL); __auto_type __bc290 = (__bc288 + __bc289); __bc290; }); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL)) : ((((__env_self->mu_i) + 3LL) < mu_n) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 4LL); __auto_type __da2 = mu_n; __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc271 = mu_b0; __auto_type __bc272 = 8LL; __auto_type __bc273 = __lang_imod(__bc271, __bc272); __auto_type __bc274 = 262144LL; __auto_type __bc275 = (__bc273 * __bc274); __auto_type __bc276 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL) * 4096LL); __auto_type __bc277 = (__bc275 + __bc276); __auto_type __bc278 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 2LL))), 64LL) * 64LL); __auto_type __bc279 = (__bc277 + __bc278); __auto_type __bc280 = __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 3LL))), 64LL); __auto_type __bc281 = (__bc279 + __bc280); __bc281; }); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL))))); }));
}
static long long __anon_114_fn(void* __env_self_void, long long mu_acc) {
  __anon_114_env* __env_self = (__anon_114_env*)__env_self_void;
  return (((__env_self->mu_i) >= (__env_self->mu_n)) ? mu_acc : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at((__env_self->mu_s), (__env_self->mu_i))); __auto_type mu_b0 = __let_tmp_b0; ((mu_b0 < 194LL) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 1LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + 1LL); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : ((mu_b0 < 224LL) ? ((((__env_self->mu_i) + 1LL) < (__env_self->mu_n)) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 2LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ((__lang_imod(mu_b0, 32LL) * 64LL) + __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL)); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL)) : ((mu_b0 < 240LL) ? ((((__env_self->mu_i) + 2LL) < (__env_self->mu_n)) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 3LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc302 = mu_b0; __auto_type __bc303 = 16LL; __auto_type __bc304 = __lang_imod(__bc302, __bc303); __auto_type __bc305 = 4096LL; __auto_type __bc306 = (__bc304 * __bc305); __auto_type __bc307 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL) * 64LL); __auto_type __bc308 = (__bc306 + __bc307); __auto_type __bc309 = __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 2LL))), 64LL); __auto_type __bc310 = (__bc308 + __bc309); __bc310; }); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL)) : ((((__env_self->mu_i) + 3LL) < (__env_self->mu_n)) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 4LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc291 = mu_b0; __auto_type __bc292 = 8LL; __auto_type __bc293 = __lang_imod(__bc291, __bc292); __auto_type __bc294 = 262144LL; __auto_type __bc295 = (__bc293 * __bc294); __auto_type __bc296 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL) * 4096LL); __auto_type __bc297 = (__bc295 + __bc296); __auto_type __bc298 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 2LL))), 64LL) * 64LL); __auto_type __bc299 = (__bc297 + __bc298); __auto_type __bc300 = __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 3LL))), 64LL); __auto_type __bc301 = (__bc299 + __bc300); __bc301; }); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL))))); }));
}
static long long __anon_115_fn(void* __env_self_void, long long mu_n) {
  __anon_115_env* __env_self = (__anon_115_env*)__env_self_void;
  return ((mu_n == 0LL) ? (__env_self->mu_acc) : ((__lang_imod(mu_n, 2LL) == 1LL) ? ({ __auto_type __da0 = ((__env_self->mu_acc) * (__env_self->mu_sq)); __auto_type __da1 = ((__env_self->mu_sq) * (__env_self->mu_sq)); __auto_type __da2 = __lang_idiv(mu_n, 2LL); mu__pow_go__direct(__da0, __da1, __da2); }) : ({ __auto_type __da0 = (__env_self->mu_acc); __auto_type __da1 = ((__env_self->mu_sq) * (__env_self->mu_sq)); __auto_type __da2 = __lang_idiv(mu_n, 2LL); mu__pow_go__direct(__da0, __da1, __da2); })));
}
static long long __anon_116_fn(void* __env_self_void, long long mu_n) {
  __anon_116_env* __env_self = (__anon_116_env*)__env_self_void;
  return ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_go_uq2_3__direct(mu_n, (__env_self->mu_s), (__env_self->mu_data), __ia0, __ia1); });
}
static closure_Vec_int_int __anon_117_fn(void* __env_self_void, mere_vec_int* mu_count) {
  __anon_117_env* __env_self = (__anon_117_env*)__env_self_void;
  return ({ __anon_161_env* __env = (__anon_161_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_161_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_161_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = mu_count; (closure_Vec_int_int){.env = __env, .fn = __anon_161_fn}; });
}
static long long __anon_117_fn2(void* __env_self_void, mere_vec_int* mu_count, mere_vec_int* mu_syms) {
  __anon_117_env* __env_self = (__anon_117_env*)__env_self_void;
  return ((((1LL >= 0LL) && (1LL <= 16LL)) && (16LL <= mere_vec_int_len(mu_count))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go__rvfast_uq3_9__direct((__env_self->mu_s), (__env_self->mu_data), mu_count, mu_syms, __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go_uq4_10__direct((__env_self->mu_s), (__env_self->mu_data), mu_count, mu_syms, __ia0, __ia1, __ia2, __ia3); }));
}
static long long __anon_118_fn(void* __env_self_void, mere_vec_int* mu_syms) {
  __anon_118_env* __env_self = (__anon_118_env*)__env_self_void;
  return ((((1LL >= 0LL) && (1LL <= 16LL)) && (16LL <= mere_vec_int_len((__env_self->mu_count)))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go__rvfast_uq3_9__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), mu_syms, __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go_uq4_10__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), mu_syms, __ia0, __ia1, __ia2, __ia3); }));
}
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_119_fn(void* __env_self_void, mere_vec_int* mu_out) {
  __anon_119_env* __env_self = (__anon_119_env*)__env_self_void;
  return ({ __anon_162_env* __env = (__anon_162_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_162_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_162_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_out = mu_out; (closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_162_fn, .fn2 = __anon_162_fn2}; });
}
static closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_119_fn2(void* __env_self_void, mere_vec_int* mu_out, mere_vec_int* mu_lcount) {
  __anon_119_env* __env_self = (__anon_119_env*)__env_self_void;
  return ({ __anon_163_env* __env = (__anon_163_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_163_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_163_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = mu_lcount; __env->mu_out = mu_out; (closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_163_fn, .fn2 = __anon_163_fn2}; });
}
static closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_120_fn(void* __env_self_void, mere_vec_int* mu_lcount) {
  __anon_120_env* __env_self = (__anon_120_env*)__env_self_void;
  return ({ __anon_164_env* __env = (__anon_164_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_164_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_164_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = mu_lcount; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_164_fn, .fn2 = __anon_164_fn2}; });
}
static closure_Vec_int_closure_Vec_int_unit __anon_120_fn2(void* __env_self_void, mere_vec_int* mu_lcount, mere_vec_int* mu_lsyms) {
  __anon_120_env* __env_self = (__anon_120_env*)__env_self_void;
  return ({ __anon_165_env* __env = (__anon_165_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_165_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_165_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = mu_lcount; __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_165_fn, .fn2 = __anon_165_fn2}; });
}
static closure_int_closure_int_closure_int_bool __anon_121_fn(void* __env_self_void, long long mu_a) {
  __anon_121_env* __env_self = (__anon_121_env*)__env_self_void;
  return ({ __anon_166_env* __env = (__anon_166_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_166_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_166_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = mu_a; (closure_int_closure_int_closure_int_bool){.env = __env, .fn = __anon_166_fn, .fn2 = __anon_166_fn2}; });
}
static closure_int_closure_int_bool __anon_121_fn2(void* __env_self_void, long long mu_a, long long mu_b) {
  __anon_121_env* __env_self = (__anon_121_env*)__env_self_void;
  return ({ __anon_167_env* __env = (__anon_167_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_167_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_167_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = mu_a; __env->mu_b = mu_b; (closure_int_closure_int_bool){.env = __env, .fn = __anon_167_fn, .fn2 = __anon_167_fn2}; });
}
static closure_int_closure_int_bool __anon_122_fn(void* __env_self_void, long long mu_b) {
  __anon_122_env* __env_self = (__anon_122_env*)__env_self_void;
  return ({ __anon_168_env* __env = (__anon_168_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_168_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_168_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = mu_b; (closure_int_closure_int_bool){.env = __env, .fn = __anon_168_fn, .fn2 = __anon_168_fn2}; });
}
static closure_int_bool __anon_122_fn2(void* __env_self_void, long long mu_b, long long mu_c) {
  __anon_122_env* __env_self = (__anon_122_env*)__env_self_void;
  return ({ __anon_169_env* __env = (__anon_169_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_169_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_169_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = mu_b; __env->mu_c = mu_c; (closure_int_bool){.env = __env, .fn = __anon_169_fn}; });
}
static long long __anon_123_fn(void* __env_self_void, long long mu_c) {
  __anon_123_env* __env_self = (__anon_123_env*)__env_self_void;
  return ({ __auto_type __let_tmp_p = (((__env_self->mu_a) + (__env_self->mu_b)) - mu_c); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_pa = ((mu_p >= (__env_self->mu_a)) ? (mu_p - (__env_self->mu_a)) : ((__env_self->mu_a) - mu_p)); __auto_type mu_pa = __let_tmp_pa; __auto_type __let_tmp_pb = ((mu_p >= (__env_self->mu_b)) ? (mu_p - (__env_self->mu_b)) : ((__env_self->mu_b) - mu_p)); __auto_type mu_pb = __let_tmp_pb; __auto_type __let_tmp_pc = ((mu_p >= mu_c) ? (mu_p - mu_c) : (mu_c - mu_p)); __auto_type mu_pc = __let_tmp_pc; (((mu_pa <= mu_pb) && (mu_pa <= mu_pc)) ? (__env_self->mu_a) : ((mu_pb <= mu_pc) ? (__env_self->mu_b) : mu_c)); });
}
static closure_int_closure_int_ByteBuf __anon_124_fn(void* __env_self_void, long long mu_width) {
  __anon_124_env* __env_self = (__anon_124_env*)__env_self_void;
  return ({ __anon_170_env* __env = (__anon_170_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_170_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_170_env; __env->mu_width = mu_width; __env->mu_start = (__env_self->mu_start); __env->mu_raw = (__env_self->mu_raw); (closure_int_closure_int_ByteBuf){.env = __env, .fn = __anon_170_fn, .fn2 = __anon_170_fn2}; });
}
static closure_int_ByteBuf __anon_124_fn2(void* __env_self_void, long long mu_width, long long mu_height) {
  __anon_124_env* __env_self = (__anon_124_env*)__env_self_void;
  return ({ __anon_171_env* __env = (__anon_171_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_171_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_171_env; __env->mu_width = mu_width; __env->mu_height = mu_height; __env->mu_start = (__env_self->mu_start); __env->mu_raw = (__env_self->mu_raw); (closure_int_ByteBuf){.env = __env, .fn = __anon_171_fn}; });
}
static closure_int_ByteBuf __anon_125_fn(void* __env_self_void, long long mu_height) {
  __anon_125_env* __env_self = (__anon_125_env*)__env_self_void;
  return ({ __anon_172_env* __env = (__anon_172_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_172_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_172_env; __env->mu_width = (__env_self->mu_width); __env->mu_height = mu_height; __env->mu_start = (__env_self->mu_start); __env->mu_raw = (__env_self->mu_raw); (closure_int_ByteBuf){.env = __env, .fn = __anon_172_fn}; });
}
static mere_bytebuf* __anon_125_fn2(void* __env_self_void, long long mu_height, long long mu_bpp) {
  __anon_125_env* __env_self = (__anon_125_env*)__env_self_void;
  return ({ __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (mu_stride * mu_height)); __auto_type mu_out = __let_tmp_out; __lifted_row_40(mu_height, mu_out, (__env_self->mu_start), mu_stride, (__env_self->mu_raw), mu_bpp, 0LL); });
}
static closure_int_ByteBuf __anon_126_fn(void* __env_self_void, long long mu_height) {
  __anon_126_env* __env_self = (__anon_126_env*)__env_self_void;
  return ({ __anon_173_env* __env = (__anon_173_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_173_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_173_env; __env->mu_raw = (__env_self->mu_raw); __env->mu_width = (__env_self->mu_width); __env->mu_height = mu_height; (closure_int_ByteBuf){.env = __env, .fn = __anon_173_fn}; });
}
static mere_bytebuf* __anon_126_fn2(void* __env_self_void, long long mu_height, long long mu_bpp) {
  __anon_126_env* __env_self = (__anon_126_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_raw); __auto_type __da1 = 0LL; __auto_type __da2 = (__env_self->mu_width); __auto_type __da3 = mu_height; __auto_type __da4 = mu_bpp; mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3, __da4); });
}
static mere_bytebuf* __anon_127_fn(void* __env_self_void, long long mu_bpp) {
  __anon_127_env* __env_self = (__anon_127_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_raw); __auto_type __da1 = 0LL; __auto_type __da2 = (__env_self->mu_width); __auto_type __da3 = (__env_self->mu_height); __auto_type __da4 = mu_bpp; mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3, __da4); });
}
static closure_int_ByteBuf __anon_128_fn(void* __env_self_void, long long mu_height) {
  __anon_128_env* __env_self = (__anon_128_env*)__env_self_void;
  return ({ __anon_174_env* __env = (__anon_174_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_174_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_174_env; __env->mu_width = (__env_self->mu_width); __env->mu_height = mu_height; __env->mu_raw = (__env_self->mu_raw); (closure_int_ByteBuf){.env = __env, .fn = __anon_174_fn}; });
}
static mere_bytebuf* __anon_128_fn2(void* __env_self_void, long long mu_height, long long mu_bpp) {
  __anon_128_env* __env_self = (__anon_128_env*)__env_self_void;
  return ({ __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (((__env_self->mu_width) * mu_height) * mu_bpp)); __auto_type mu_out = __let_tmp_out; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_pass_43__direct(mu_out, (__env_self->mu_width), mu_height, (__env_self->mu_raw), mu_bpp, __ia0, __ia1); }); });
}
static mere_bytebuf* __anon_129_fn(void* __env_self_void, long long mu_bpp) {
  __anon_129_env* __env_self = (__anon_129_env*)__env_self_void;
  return ({ __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (((__env_self->mu_width) * (__env_self->mu_height)) * mu_bpp)); __auto_type mu_out = __let_tmp_out; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_pass_43__direct(mu_out, (__env_self->mu_width), (__env_self->mu_height), (__env_self->mu_raw), mu_bpp, __ia0, __ia1); }); });
}
static closure_int_unit __anon_130_fn(void* __env_self_void, long long mu_v) {
  __anon_130_env* __env_self = (__anon_130_env*)__env_self_void;
  return ({ __anon_175_env* __env = (__anon_175_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_175_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_175_env; __env->mu_v = mu_v; __env->mu_s = (__env_self->mu_s); __env->mu_out = (__env_self->mu_out); (closure_int_unit){.env = __env, .fn = __anon_175_fn}; });
}
static int __anon_130_fn2(void* __env_self_void, long long mu_v, long long mu_n) {
  __anon_130_env* __env_self = (__anon_130_env*)__env_self_void;
  return __lifted_go_uq34_52(mu_n, mu_v, (__env_self->mu_s), (__env_self->mu_out), 0LL);
}
static int __anon_131_fn(void* __env_self_void, long long mu_n) {
  __anon_131_env* __env_self = (__anon_131_env*)__env_self_void;
  return __lifted_go_uq34_52(mu_n, (__env_self->mu_v), (__env_self->mu_s), (__env_self->mu_out), 0LL);
}
static tuple_int_int __anon_132_fn(void* __env_self_void, long long mu_n) {
  __anon_132_env* __env_self = (__anon_132_env*)__env_self_void;
  return ({ __auto_type __let_tmp_max_back = (((__env_self->mu_pos) > 32768LL) ? 32768LL : (__env_self->mu_pos)); __auto_type mu_max_back = __let_tmp_max_back; ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __lifted_best_58__direct(mu_max_back, (__env_self->mu_pos), mu_n, (__env_self->mu_data), __ia0, __ia1, __ia2); }); });
}
static mere_vec_int* __anon_133_fn(void* __env_self_void, long long mu_maxbits) {
  __anon_133_env* __env_self = (__anon_133_env*)__env_self_void;
  return ({ __auto_type __let_tmp_nf = mere_vec_int_new(__lang_current_region); __auto_type mu_nf = __let_tmp_nf; __auto_type __let_tmp_nl = mere_vec_int_new(__lang_current_region); __auto_type mu_nl = __let_tmp_nl; __auto_type __let_tmp_nr = mere_vec_int_new(__lang_current_region); __auto_type mu_nr = __let_tmp_nr; ({ (void)(__lifted_seed_61((__env_self->mu_nsym), (__env_self->mu_freq), mu_nf, mu_nl, mu_nr, 0LL)); __auto_type __let_tmp_syms = mere_vec_int_new(__lang_current_region); __auto_type mu_syms = __let_tmp_syms; ({ (void)(__lifted_seed2_62((__env_self->mu_nsym), (__env_self->mu_freq), mu_syms, 0LL)); __auto_type __let_tmp_nleaves = mere_vec_int_len(mu_nf); __auto_type mu_nleaves = __let_tmp_nleaves; __auto_type __let_tmp_lengths = mere_vec_int_new(__lang_current_region); __auto_type mu_lengths = __let_tmp_lengths; ({ (void)(__lifted_zl_63((__env_self->mu_nsym), mu_lengths, 0LL)); ((mu_nleaves == 0LL) ? mu_lengths : ((mu_nleaves == 1LL) ? ({ (void)(mere_vec_int_set(mu_lengths, mere_vec_int_get(mu_syms, 0LL), 1LL)); mu_lengths; }) : ({ __auto_type __let_tmp_alive = mere_vec_int_new(__lang_current_region); __auto_type mu_alive = __let_tmp_alive; ({ (void)(__lifted_al_64(mu_nleaves, mu_alive, 0LL)); __auto_type __let_tmp_leafsym = mere_vec_int_new(__lang_current_region); __auto_type mu_leafsym = __let_tmp_leafsym; ({ (void)(__lifted_ls_65(mu_nleaves, mu_leafsym, mu_syms, 0LL)); ({ (void)(__lifted_merge_66(mu_nf, mu_alive, mu_nl, mu_nr, mu_leafsym, mu_nleaves)); __auto_type __let_tmp_root = (mere_vec_int_len(mu_nf) - 1LL); __auto_type mu_root = __let_tmp_root; ({ (void)(({ __auto_type __ia0 = mu_root; __auto_type __ia1 = 0LL; __lifted_assign_70__direct(mu_leafsym, mu_lengths, mu_nl, mu_nr, __ia0, __ia1); })); (void)(mu_maxbits); mu_lengths; }); }); }); }); }))); }); }); }); });
}
static mere_vec_int* __anon_134_fn(void* __env_self_void, long long mu_maxbits) {
  __anon_134_env* __env_self = (__anon_134_env*)__env_self_void;
  return ({ __auto_type __let_tmp_bl_count = mere_vec_int_new(__lang_current_region); __auto_type mu_bl_count = __let_tmp_bl_count; ({ (void)(__lifted_z_71(mu_maxbits, mu_bl_count, 0LL)); ({ (void)(((((0LL >= 0LL) && (0LL <= (__env_self->mu_n))) && ((__env_self->mu_n) <= mere_vec_int_len((__env_self->mu_lengths)))) ? __lifted_cnt__rvfast_72((__env_self->mu_n), (__env_self->mu_lengths), mu_bl_count, 0LL) : __lifted_cnt_73((__env_self->mu_n), (__env_self->mu_lengths), mu_bl_count, 0LL))); __auto_type __let_tmp_next_code = mere_vec_int_new(__lang_current_region); __auto_type mu_next_code = __let_tmp_next_code; ({ (void)(__lifted_z2_74(mu_maxbits, mu_next_code, 0LL)); ({ (void)((((((1LL >= 0LL) && (1LL <= (mu_maxbits + 1LL))) && ((mu_maxbits + 1LL) <= mere_vec_int_len(mu_next_code))) && ((((1LL - 1LL) >= 0LL) && (((1LL - 1LL) + 1LL) <= mere_vec_int_len(mu_bl_count))) && (((((mu_maxbits + 1LL) - 1LL) - 1LL) >= 0LL) && (((((mu_maxbits + 1LL) - 1LL) - 1LL) + 1LL) <= mere_vec_int_len(mu_bl_count))))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __lifted_fill__rvfast_75__direct(mu_maxbits, mu_bl_count, mu_next_code, __ia0, __ia1); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __lifted_fill_uq44_76__direct(mu_maxbits, mu_bl_count, mu_next_code, __ia0, __ia1); }))); __auto_type __let_tmp_codes = mere_vec_int_new(__lang_current_region); __auto_type mu_codes = __let_tmp_codes; ({ (void)(__lifted_z3_77((__env_self->mu_n), mu_codes, 0LL)); ({ (void)((((((0LL >= 0LL) && (0LL <= (__env_self->mu_n))) && ((__env_self->mu_n) <= mere_vec_int_len(mu_codes))) && ((__env_self->mu_n) <= mere_vec_int_len((__env_self->mu_lengths)))) ? __lifted_asg__rvfast_78((__env_self->mu_n), (__env_self->mu_lengths), mu_next_code, mu_codes, 0LL) : __lifted_asg_79((__env_self->mu_n), (__env_self->mu_lengths), mu_next_code, mu_codes, 0LL))); mu_codes; }); }); }); }); }); }); });
}
static closure_int_closure_int_closure_Vec_int_unit __anon_135_fn(void* __env_self_void, long long mu_t1) {
  __anon_135_env* __env_self = (__anon_135_env*)__env_self_void;
  return ({ __anon_176_env* __env = (__anon_176_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_176_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_176_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = mu_t1; (closure_int_closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_176_fn, .fn2 = __anon_176_fn2}; });
}
static closure_int_closure_Vec_int_unit __anon_135_fn2(void* __env_self_void, long long mu_t1, long long mu_t2) {
  __anon_135_env* __env_self = (__anon_135_env*)__env_self_void;
  return ({ __anon_177_env* __env = (__anon_177_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_177_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_177_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = mu_t1; __env->mu_t2 = mu_t2; (closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_177_fn, .fn2 = __anon_177_fn2}; });
}
static closure_int_closure_Vec_int_unit __anon_136_fn(void* __env_self_void, long long mu_t2) {
  __anon_136_env* __env_self = (__anon_136_env*)__env_self_void;
  return ({ __anon_178_env* __env = (__anon_178_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_178_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_178_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = mu_t2; (closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_178_fn, .fn2 = __anon_178_fn2}; });
}
static closure_Vec_int_unit __anon_136_fn2(void* __env_self_void, long long mu_t2, long long mu_t3) {
  __anon_136_env* __env_self = (__anon_136_env*)__env_self_void;
  return ({ __anon_179_env* __env = (__anon_179_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_179_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_179_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = mu_t2; __env->mu_t3 = mu_t3; (closure_Vec_int_unit){.env = __env, .fn = __anon_179_fn}; });
}
static long long __anon_137_fn(void* __env_self_void, long long mu_c) {
  __anon_137_env* __env_self = (__anon_137_env*)__env_self_void;
  return ({ __auto_type __let_tmp_p = (((__env_self->mu_a) + (__env_self->mu_b)) - mu_c); __auto_type mu_p = __let_tmp_p; __auto_type __let_tmp_pa = ((mu_p >= (__env_self->mu_a)) ? (mu_p - (__env_self->mu_a)) : ((__env_self->mu_a) - mu_p)); __auto_type mu_pa = __let_tmp_pa; __auto_type __let_tmp_pb = ((mu_p >= (__env_self->mu_b)) ? (mu_p - (__env_self->mu_b)) : ((__env_self->mu_b) - mu_p)); __auto_type mu_pb = __let_tmp_pb; __auto_type __let_tmp_pc = ((mu_p >= mu_c) ? (mu_p - mu_c) : (mu_c - mu_p)); __auto_type mu_pc = __let_tmp_pc; (((mu_pa <= mu_pb) && (mu_pa <= mu_pc)) ? (__env_self->mu_a) : ((mu_pb <= mu_pc) ? (__env_self->mu_b) : mu_c)); });
}
static closure_int_Vec_int __anon_138_fn(void* __env_self_void, mere_vec_int* mu_prev) {
  __anon_138_env* __env_self = (__anon_138_env*)__env_self_void;
  return ({ __anon_180_env* __env = (__anon_180_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_180_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_180_env; __env->mu_row_uq58 = (__env_self->mu_row_uq58); __env->mu_prev = mu_prev; __env->mu_kind = (__env_self->mu_kind); (closure_int_Vec_int){.env = __env, .fn = __anon_180_fn}; });
}
static mere_vec_int* __anon_138_fn2(void* __env_self_void, mere_vec_int* mu_prev, long long mu_bpp) {
  __anon_138_env* __env_self = (__anon_138_env*)__env_self_void;
  return ({ __auto_type __let_tmp_n = mere_vec_int_len((__env_self->mu_row_uq58)); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __lifted_go_uq59_112(mu_n, mu_out, (__env_self->mu_row_uq58), mu_bpp, mu_prev, (__env_self->mu_kind), 0LL); });
}
static mere_vec_int* __anon_139_fn(void* __env_self_void, long long mu_bpp) {
  __anon_139_env* __env_self = (__anon_139_env*)__env_self_void;
  return ({ __auto_type __let_tmp_n = mere_vec_int_len((__env_self->mu_row_uq58)); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __lifted_go_uq59_112(mu_n, mu_out, (__env_self->mu_row_uq58), mu_bpp, (__env_self->mu_prev), (__env_self->mu_kind), 0LL); });
}
static mere_vec_int* __anon_140_fn(void* __env_self_void, long long mu_bpp) {
  __anon_140_env* __env_self = (__anon_140_env*)__env_self_void;
  return ({ __auto_type __let_tmp_first = ({ __auto_type __da0 = 0LL; __auto_type __da1 = (__env_self->mu_row_uq63); __auto_type __da2 = (__env_self->mu_prev); __auto_type __da3 = mu_bpp; mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_first = __let_tmp_first; ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = ({ __auto_type __da0 = mu_first; mu_row_cost__direct(__da0); }); __auto_type __ia3 = mu_first; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), mu_bpp, __ia0, __ia1, __ia2, __ia3); }); });
}
static closure_int_int __anon_141_fn(void* __env_self_void, long long mu_g) {
  __anon_141_env* __env_self = (__anon_141_env*)__env_self_void;
  return ({ __anon_181_env* __env = (__anon_181_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_181_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_181_env; __env->mu_pal = (__env_self->mu_pal); __env->mu_r = (__env_self->mu_r); __env->mu_g = mu_g; (closure_int_int){.env = __env, .fn = __anon_181_fn}; });
}
static long long __anon_141_fn2(void* __env_self_void, long long mu_g, long long mu_b) {
  __anon_141_env* __env_self = (__anon_141_env*)__env_self_void;
  return ((((((0LL >= 0LL) && (0LL <= mere_vec_int_len((__env_self->mu_pal)))) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 2LL) >= 0LL) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 2LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 1LL) >= 0LL) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 1LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))))) && (mere_vec_int_len((__env_self->mu_pal)) <= mere_vec_int_len((__env_self->mu_pal)))) ? __lifted_go__rvfast_uq68_120((__env_self->mu_pal), (__env_self->mu_r), mu_g, mu_b, 0LL) : __lifted_go_uq69_121((__env_self->mu_pal), (__env_self->mu_r), mu_g, mu_b, 0LL));
}
static long long __anon_142_fn(void* __env_self_void, long long mu_b) {
  __anon_142_env* __env_self = (__anon_142_env*)__env_self_void;
  return ((((((0LL >= 0LL) && (0LL <= mere_vec_int_len((__env_self->mu_pal)))) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 2LL) >= 0LL) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 2LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 1LL) >= 0LL) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 1LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))))) && (mere_vec_int_len((__env_self->mu_pal)) <= mere_vec_int_len((__env_self->mu_pal)))) ? __lifted_go__rvfast_uq68_120((__env_self->mu_pal), (__env_self->mu_r), (__env_self->mu_g), mu_b, 0LL) : __lifted_go_uq69_121((__env_self->mu_pal), (__env_self->mu_r), (__env_self->mu_g), mu_b, 0LL));
}
static closure_int_Vec_int __anon_143_fn(void* __env_self_void, long long mu_height) {
  __anon_143_env* __env_self = (__anon_143_env*)__env_self_void;
  return ({ __anon_182_env* __env = (__anon_182_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_182_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_182_env; __env->mu_width = (__env_self->mu_width); __env->mu_height = mu_height; __env->mu_pixels = (__env_self->mu_pixels); (closure_int_Vec_int){.env = __env, .fn = __anon_182_fn}; });
}
static mere_vec_int* __anon_143_fn2(void* __env_self_void, long long mu_height, long long mu_depth) {
  __anon_143_env* __env_self = (__anon_143_env*)__env_self_void;
  return ({ __auto_type __let_tmp_count = ((__env_self->mu_width) * mu_height); __auto_type mu_count = __let_tmp_count; __auto_type __let_tmp_eight = (mu_depth == 8LL); __auto_type mu_eight = __let_tmp_eight; __auto_type __let_tmp_grey = (mu_eight && ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_count; mu_all_grey__direct(__da0, __da1); })); __auto_type mu_grey = __let_tmp_grey; __auto_type __let_tmp_pal = ((mu_grey || (mu_eight == 0)) ? mere_vec_int_new(__lang_current_region) : ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_count; mu_collect_palette__direct(__da0, __da1); })); __auto_type mu_pal = __let_tmp_pal; __auto_type __let_tmp_indexed = ((((mu_grey == 0) && mu_eight) && (mere_vec_int_len(mu_pal) <= 768LL)) && (mere_vec_int_len(mu_pal) > 0LL)); __auto_type mu_indexed = __let_tmp_indexed; __auto_type __let_tmp_bpp = ((mu_grey || mu_indexed) ? 1LL : (mu_eight ? 3LL : 6LL)); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_raw = mere_vec_int_new(__lang_current_region); __auto_type mu_raw = __let_tmp_raw; ({ (void)(({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mere_vec_int_new(__lang_current_region); __lifted_rows_122__direct(mu_height, mu_stride, mu_grey, (__env_self->mu_pixels), (__env_self->mu_width), mu_indexed, mu_pal, mu_raw, mu_bpp, __ia0, __ia1); })); __auto_type __let_tmp_z = mere_vec_int_new(__lang_current_region); __auto_type mu_z = __let_tmp_z; (void)(mere_vec_int_push(mu_z, 120LL)); (void)(mere_vec_int_push(mu_z, 1LL)); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_adler32__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = ({ __auto_type __let_tmp_sig = mere_vec_int_new(__lang_current_region); __auto_type mu_sig = __let_tmp_sig; __lifted_go_uq70_124(mu_sig, 0LL); }); mu_push_all__direct(__da0, __da1); })); __auto_type __let_tmp_ihdr = mere_vec_int_new(__lang_current_region); __auto_type mu_ihdr = __let_tmp_ihdr; (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = (__env_self->mu_width); mu_bytes_push4be__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = mu_height; mu_bytes_push4be__direct(__da0, __da1); })); (void)(mere_vec_int_push(mu_ihdr, mu_depth)); (void)(mere_vec_int_push(mu_ihdr, (mu_grey ? 0LL : (mu_indexed ? 3LL : 2LL)))); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 72LL; __auto_type __da3 = 68LL; __auto_type __da4 = 82LL; __auto_type __da5 = mu_ihdr; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)((mu_indexed ? ({ __auto_type __da0 = mu_out; __auto_type __da1 = 80LL; __auto_type __da2 = 76LL; __auto_type __da3 = 84LL; __auto_type __da4 = 69LL; __auto_type __da5 = mu_pal; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); }) : 0)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 68LL; __auto_type __da3 = 65LL; __auto_type __da4 = 84LL; __auto_type __da5 = mu_z; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 69LL; __auto_type __da3 = 78LL; __auto_type __da4 = 68LL; __auto_type __da5 = mere_vec_int_new(__lang_current_region); mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); mu_out; }); });
}
static mere_vec_int* __anon_144_fn(void* __env_self_void, long long mu_depth) {
  __anon_144_env* __env_self = (__anon_144_env*)__env_self_void;
  return ({ __auto_type __let_tmp_count = ((__env_self->mu_width) * (__env_self->mu_height)); __auto_type mu_count = __let_tmp_count; __auto_type __let_tmp_eight = (mu_depth == 8LL); __auto_type mu_eight = __let_tmp_eight; __auto_type __let_tmp_grey = (mu_eight && ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_count; mu_all_grey__direct(__da0, __da1); })); __auto_type mu_grey = __let_tmp_grey; __auto_type __let_tmp_pal = ((mu_grey || (mu_eight == 0)) ? mere_vec_int_new(__lang_current_region) : ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_count; mu_collect_palette__direct(__da0, __da1); })); __auto_type mu_pal = __let_tmp_pal; __auto_type __let_tmp_indexed = ((((mu_grey == 0) && mu_eight) && (mere_vec_int_len(mu_pal) <= 768LL)) && (mere_vec_int_len(mu_pal) > 0LL)); __auto_type mu_indexed = __let_tmp_indexed; __auto_type __let_tmp_bpp = ((mu_grey || mu_indexed) ? 1LL : (mu_eight ? 3LL : 6LL)); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_raw = mere_vec_int_new(__lang_current_region); __auto_type mu_raw = __let_tmp_raw; ({ (void)(({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mere_vec_int_new(__lang_current_region); __lifted_rows_122__direct((__env_self->mu_height), mu_stride, mu_grey, (__env_self->mu_pixels), (__env_self->mu_width), mu_indexed, mu_pal, mu_raw, mu_bpp, __ia0, __ia1); })); __auto_type __let_tmp_z = mere_vec_int_new(__lang_current_region); __auto_type mu_z = __let_tmp_z; (void)(mere_vec_int_push(mu_z, 120LL)); (void)(mere_vec_int_push(mu_z, 1LL)); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_adler32__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = ({ __auto_type __let_tmp_sig = mere_vec_int_new(__lang_current_region); __auto_type mu_sig = __let_tmp_sig; __lifted_go_uq70_124(mu_sig, 0LL); }); mu_push_all__direct(__da0, __da1); })); __auto_type __let_tmp_ihdr = mere_vec_int_new(__lang_current_region); __auto_type mu_ihdr = __let_tmp_ihdr; (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = (__env_self->mu_width); mu_bytes_push4be__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = (__env_self->mu_height); mu_bytes_push4be__direct(__da0, __da1); })); (void)(mere_vec_int_push(mu_ihdr, mu_depth)); (void)(mere_vec_int_push(mu_ihdr, (mu_grey ? 0LL : (mu_indexed ? 3LL : 2LL)))); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 72LL; __auto_type __da3 = 68LL; __auto_type __da4 = 82LL; __auto_type __da5 = mu_ihdr; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)((mu_indexed ? ({ __auto_type __da0 = mu_out; __auto_type __da1 = 80LL; __auto_type __da2 = 76LL; __auto_type __da3 = 84LL; __auto_type __da4 = 69LL; __auto_type __da5 = mu_pal; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); }) : 0)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 68LL; __auto_type __da3 = 65LL; __auto_type __da4 = 84LL; __auto_type __da5 = mu_z; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 69LL; __auto_type __da3 = 78LL; __auto_type __da4 = 68LL; __auto_type __da5 = mere_vec_int_new(__lang_current_region); mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); mu_out; }); });
}
static mere_vec_int* __anon_145_fn(void* __env_self_void, long long mu_height) {
  __anon_145_env* __env_self = (__anon_145_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = (__env_self->mu_width); __auto_type __da2 = mu_height; __auto_type __da3 = 8LL; mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2, __da3); });
}
static mere_vec_int* __anon_146_fn(void* __env_self_void, long long mu_height) {
  __anon_146_env* __env_self = (__anon_146_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = (__env_self->mu_width); __auto_type __da2 = mu_height; __auto_type __da3 = 16LL; mu_encode_at_depth__Vec_int__int__int__int__Vec_int__direct(__lang_current_region, __da0, __da1, __da2, __da3); });
}
static closure_int_int __anon_147_fn(void* __env_self_void, long long mu_first) {
  __anon_147_env* __env_self = (__anon_147_env*)__env_self_void;
  return ({ __anon_183_env* __env = (__anon_183_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_183_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_183_env; __env->mu_len = (__env_self->mu_len); __env->mu_code0 = (__env_self->mu_code0); __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = (__env_self->mu_count); __env->mu_first = mu_first; __env->mu_syms = (__env_self->mu_syms); (closure_int_int){.env = __env, .fn = __anon_183_fn}; });
}
static long long __anon_147_fn2(void* __env_self_void, long long mu_first, long long mu_index) {
  __anon_147_env* __env_self = (__anon_147_env*)__env_self_void;
  return (((__env_self->mu_len) == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = (((__env_self->mu_code0)) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get_unchecked((__env_self->mu_count), (__env_self->mu_len)); __auto_type mu_c = __let_tmp_c; ((mu_code < (mu_first + mu_c)) ? mere_vec_int_get((__env_self->mu_syms), (mu_index + (mu_code - mu_first))) : ({ __auto_type __ia0 = ((__env_self->mu_len) + 1LL); __auto_type __ia1 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia2 = ({ long long __sa = ((mu_first + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia3 = (mu_index + mu_c); __lifted_go__rvfast_uq3_9__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), (__env_self->mu_syms), __ia0, __ia1, __ia2, __ia3); })); }));
}
static long long __anon_148_fn(void* __env_self_void, long long mu_index) {
  __anon_148_env* __env_self = (__anon_148_env*)__env_self_void;
  return (((__env_self->mu_len) == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = (((__env_self->mu_code0)) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get_unchecked((__env_self->mu_count), (__env_self->mu_len)); __auto_type mu_c = __let_tmp_c; ((mu_code < ((__env_self->mu_first) + mu_c)) ? mere_vec_int_get((__env_self->mu_syms), (mu_index + (mu_code - (__env_self->mu_first)))) : ({ __auto_type __ia0 = ((__env_self->mu_len) + 1LL); __auto_type __ia1 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia2 = ({ long long __sa = (((__env_self->mu_first) + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia3 = (mu_index + mu_c); __lifted_go__rvfast_uq3_9__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), (__env_self->mu_syms), __ia0, __ia1, __ia2, __ia3); })); }));
}
static closure_int_int __anon_149_fn(void* __env_self_void, long long mu_first) {
  __anon_149_env* __env_self = (__anon_149_env*)__env_self_void;
  return ({ __anon_184_env* __env = (__anon_184_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_184_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_184_env; __env->mu_len = (__env_self->mu_len); __env->mu_code0 = (__env_self->mu_code0); __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_count = (__env_self->mu_count); __env->mu_first = mu_first; __env->mu_syms = (__env_self->mu_syms); (closure_int_int){.env = __env, .fn = __anon_184_fn}; });
}
static long long __anon_149_fn2(void* __env_self_void, long long mu_first, long long mu_index) {
  __anon_149_env* __env_self = (__anon_149_env*)__env_self_void;
  return (((__env_self->mu_len) == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = (((__env_self->mu_code0)) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get((__env_self->mu_count), (__env_self->mu_len)); __auto_type mu_c = __let_tmp_c; ((mu_code < (mu_first + mu_c)) ? mere_vec_int_get((__env_self->mu_syms), (mu_index + (mu_code - mu_first))) : ({ __auto_type __ia0 = ((__env_self->mu_len) + 1LL); __auto_type __ia1 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia2 = ({ long long __sa = ((mu_first + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia3 = (mu_index + mu_c); __lifted_go_uq4_10__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), (__env_self->mu_syms), __ia0, __ia1, __ia2, __ia3); })); }));
}
static long long __anon_150_fn(void* __env_self_void, long long mu_index) {
  __anon_150_env* __env_self = (__anon_150_env*)__env_self_void;
  return (((__env_self->mu_len) == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = (((__env_self->mu_code0)) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get((__env_self->mu_count), (__env_self->mu_len)); __auto_type mu_c = __let_tmp_c; ((mu_code < ((__env_self->mu_first) + mu_c)) ? mere_vec_int_get((__env_self->mu_syms), (mu_index + (mu_code - (__env_self->mu_first)))) : ({ __auto_type __ia0 = ((__env_self->mu_len) + 1LL); __auto_type __ia1 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia2 = ({ long long __sa = (((__env_self->mu_first) + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia3 = (mu_index + mu_c); __lifted_go_uq4_10__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), (__env_self->mu_syms), __ia0, __ia1, __ia2, __ia3); })); }));
}
static tuple_int_int __anon_151_fn(void* __env_self_void, long long mu_bdist) {
  __anon_151_env* __env_self = (__anon_151_env*)__env_self_void;
  return (((__env_self->mu_d) > (__env_self->mu_max_back)) ? ((tuple_int_int){.f0 = (__env_self->mu_blen), .f1 = mu_bdist}) : ({ __auto_type __let_tmp_start = ((__env_self->mu_pos) - (__env_self->mu_d)); __auto_type mu_start = __let_tmp_start; ({ __auto_type __let_tmp_l = (((((0LL >= 0LL) && (0LL <= 258LL)) && (((((__env_self->mu_pos) + 0LL) >= 0LL) && ((((__env_self->mu_pos) + 0LL) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))) && ((((__env_self->mu_pos) + (258LL - 1LL)) >= 0LL) && ((((__env_self->mu_pos) + (258LL - 1LL)) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))))) && ((((mu_start + 0LL) >= 0LL) && (((mu_start + 0LL) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))) && (((mu_start + (258LL - 1LL)) >= 0LL) && (((mu_start + (258LL - 1LL)) + 1LL) <= mere_vec_int_len((__env_self->mu_data)))))) ? __lifted_mlen__rvfast_59((__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), mu_start, 0LL) : __lifted_mlen_60((__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), mu_start, 0LL)); __auto_type mu_l = __let_tmp_l; ((mu_l > (__env_self->mu_blen)) ? ({ __auto_type __ia0 = ((__env_self->mu_d) + 1LL); __auto_type __ia1 = mu_l; __auto_type __ia2 = (__env_self->mu_d); __lifted_best_58__direct((__env_self->mu_max_back), (__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); }) : ({ __auto_type __ia0 = ((__env_self->mu_d) + 1LL); __auto_type __ia1 = (__env_self->mu_blen); __auto_type __ia2 = mu_bdist; __lifted_best_58__direct((__env_self->mu_max_back), (__env_self->mu_pos), (__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); })); }); }));
}
static long long __anon_152_fn(void* __env_self_void, long long mu_floor) {
  __anon_152_env* __env_self = (__anon_152_env*)__env_self_void;
  return (((__env_self->mu_i) < mu_floor) ? mu_floor : ((mere_vec_int_get((__env_self->mu_v), (__env_self->mu_i)) > 0LL) ? (__env_self->mu_i) : ({ __auto_type __ia0 = (__env_self->mu_v); __auto_type __ia1 = ((__env_self->mu_i) - 1LL); __auto_type __ia2 = mu_floor; __lifted_last_nz_87__direct(__ia0, __ia1, __ia2); })));
}
static long long __anon_153_fn(void* __env_self_void, long long mu_b) {
  __anon_153_env* __env_self = (__anon_153_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? ((mu_b * 65536LL) + (__env_self->mu_a)) : ({ __auto_type __let_tmp_a2 = __lang_imod(((__env_self->mu_a) + mere_vec_int_get_unchecked((__env_self->mu_data), (__env_self->mu_i))), 65521LL); __auto_type mu_a2 = __let_tmp_a2; ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_a2; __auto_type __ia2 = __lang_imod((mu_b + mu_a2), 65521LL); __lifted_go__rvfast_uq56_110__direct((__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); }); }));
}
static long long __anon_154_fn(void* __env_self_void, long long mu_b) {
  __anon_154_env* __env_self = (__anon_154_env*)__env_self_void;
  return (((__env_self->mu_i) == (__env_self->mu_n)) ? ((mu_b * 65536LL) + (__env_self->mu_a)) : ({ __auto_type __let_tmp_a2 = __lang_imod(((__env_self->mu_a) + mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i))), 65521LL); __auto_type mu_a2 = __let_tmp_a2; ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = mu_a2; __auto_type __ia2 = __lang_imod((mu_b + mu_a2), 65521LL); __lifted_go_uq57_111__direct((__env_self->mu_n), (__env_self->mu_data), __ia0, __ia1, __ia2); }); }));
}
static closure_Vec_int_Vec_int __anon_155_fn(void* __env_self_void, long long mu_bc) {
  __anon_155_env* __env_self = (__anon_155_env*)__env_self_void;
  return ({ __anon_185_env* __env = (__anon_185_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_185_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_185_env; __env->mu_kind = (__env_self->mu_kind); __env->mu_bk = (__env_self->mu_bk); __env->mu_row_uq63 = (__env_self->mu_row_uq63); __env->mu_prev = (__env_self->mu_prev); __env->mu_bpp = (__env_self->mu_bpp); __env->mu_bc = mu_bc; (closure_Vec_int_Vec_int){.env = __env, .fn = __anon_185_fn}; });
}
static mere_vec_int* __anon_155_fn2(void* __env_self_void, long long mu_bc, mere_vec_int* mu_best_uq65) {
  __anon_155_env* __env_self = (__anon_155_env*)__env_self_void;
  return (((__env_self->mu_kind) == 5LL) ? ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(mere_vec_int_push(mu_out, (__env_self->mu_bk))); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = mu_best_uq65; mu_push_all__direct(__da0, __da1); })); mu_out; }) : ({ __auto_type __let_tmp_cand = ({ __auto_type __da0 = (__env_self->mu_kind); __auto_type __da1 = (__env_self->mu_row_uq63); __auto_type __da2 = (__env_self->mu_prev); __auto_type __da3 = (__env_self->mu_bpp); mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_cand = __let_tmp_cand; __auto_type __let_tmp_c = ({ __auto_type __da0 = mu_cand; mu_row_cost__direct(__da0); }); __auto_type mu_c = __let_tmp_c; ((mu_c < mu_bc) ? ({ __auto_type __ia0 = ((__env_self->mu_kind) + 1LL); __auto_type __ia1 = (__env_self->mu_kind); __auto_type __ia2 = mu_c; __auto_type __ia3 = mu_cand; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), (__env_self->mu_bpp), __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = ((__env_self->mu_kind) + 1LL); __auto_type __ia1 = (__env_self->mu_bk); __auto_type __ia2 = mu_bc; __auto_type __ia3 = mu_best_uq65; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), (__env_self->mu_bpp), __ia0, __ia1, __ia2, __ia3); })); }));
}
static mere_vec_int* __anon_156_fn(void* __env_self_void, mere_vec_int* mu_best_uq65) {
  __anon_156_env* __env_self = (__anon_156_env*)__env_self_void;
  return (((__env_self->mu_kind) == 5LL) ? ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(mere_vec_int_push(mu_out, (__env_self->mu_bk))); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = mu_best_uq65; mu_push_all__direct(__da0, __da1); })); mu_out; }) : ({ __auto_type __let_tmp_cand = ({ __auto_type __da0 = (__env_self->mu_kind); __auto_type __da1 = (__env_self->mu_row_uq63); __auto_type __da2 = (__env_self->mu_prev); __auto_type __da3 = (__env_self->mu_bpp); mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_cand = __let_tmp_cand; __auto_type __let_tmp_c = ({ __auto_type __da0 = mu_cand; mu_row_cost__direct(__da0); }); __auto_type mu_c = __let_tmp_c; ((mu_c < (__env_self->mu_bc)) ? ({ __auto_type __ia0 = ((__env_self->mu_kind) + 1LL); __auto_type __ia1 = (__env_self->mu_kind); __auto_type __ia2 = mu_c; __auto_type __ia3 = mu_cand; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), (__env_self->mu_bpp), __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = ((__env_self->mu_kind) + 1LL); __auto_type __ia1 = (__env_self->mu_bk); __auto_type __ia2 = (__env_self->mu_bc); __auto_type __ia3 = mu_best_uq65; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), (__env_self->mu_bpp), __ia0, __ia1, __ia2, __ia3); })); }));
}
static closure_int_unit __anon_157_fn(void* __env_self_void, long long mu_cur) {
  __anon_157_env* __env_self = (__anon_157_env*)__env_self_void;
  return ({ __anon_186_env* __env = (__anon_186_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_186_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_186_env; __env->mu_i = (__env_self->mu_i); __env->mu_n = (__env_self->mu_n); __env->mu_data = (__env_self->mu_data); __env->mu_got = (__env_self->mu_got); __env->mu_cur = mu_cur; __env->mu_acc = (__env_self->mu_acc); (closure_int_unit){.env = __env, .fn = __anon_186_fn}; });
}
static int __anon_157_fn2(void* __env_self_void, long long mu_cur, long long mu_seen_uq73) {
  __anon_157_env* __env_self = (__anon_157_env*)__env_self_void;
  return (((__env_self->mu_i) >= (__env_self->mu_n)) ? 0 : ({ __auto_type __let_tmp_c = mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)); __auto_type mu_c = __let_tmp_c; (((mu_c >= 48LL) && (mu_c <= 57LL)) ? ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_got); __auto_type __ia2 = (((mu_cur * 10LL) + mu_c) - 48LL); __auto_type __ia3 = 1LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); }) : ((mu_seen_uq73 == 1LL) ? ({ (void)(mere_vec_int_push((__env_self->mu_acc), mu_cur)); (((__env_self->mu_got) == 2LL) ? ({ (void)(mere_vec_int_push((__env_self->mu_acc), ((__env_self->mu_i) + 1LL))); 0; }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((__env_self->mu_got) + 1LL); __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); })); }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_got); __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); }))); }));
}
static int __anon_158_fn(void* __env_self_void, long long mu_seen_uq73) {
  __anon_158_env* __env_self = (__anon_158_env*)__env_self_void;
  return (((__env_self->mu_i) >= (__env_self->mu_n)) ? 0 : ({ __auto_type __let_tmp_c = mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)); __auto_type mu_c = __let_tmp_c; (((mu_c >= 48LL) && (mu_c <= 57LL)) ? ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_got); __auto_type __ia2 = ((((__env_self->mu_cur) * 10LL) + mu_c) - 48LL); __auto_type __ia3 = 1LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); }) : ((mu_seen_uq73 == 1LL) ? ({ (void)(mere_vec_int_push((__env_self->mu_acc), (__env_self->mu_cur))); (((__env_self->mu_got) == 2LL) ? ({ (void)(mere_vec_int_push((__env_self->mu_acc), ((__env_self->mu_i) + 1LL))); 0; }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((__env_self->mu_got) + 1LL); __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); })); }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_got); __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); }))); }));
}
static const char* __anon_159_fn(void* __env_self_void, const char* mu_acc) {
  __anon_159_env* __env_self = (__anon_159_env*)__env_self_void;
  return ({ __auto_type __scrut = (__env_self->mu_cs); (list_str__tag(__scrut) == 0) ? ({ mu_acc; }) : ((list_str__tag(__scrut) == 1) ? ({ __auto_type mu_h = (list_str__node(__scrut)->payload.Cons).f0; __auto_type mu_t = (list_str__node(__scrut)->payload.Cons).f1; (((__env_self->mu_start) > 0LL) ? ({ __auto_type __da0 = mu_t; __auto_type __da1 = ((__env_self->mu_start) - 1LL); __auto_type __da2 = (__env_self->mu_len); __auto_type __da3 = mu_acc; mu__u8_slice__direct(__da0, __da1, __da2, __da3); }) : (((__env_self->mu_len) > 0LL) ? ({ __auto_type __da0 = mu_t; __auto_type __da1 = 0LL; __auto_type __da2 = ((__env_self->mu_len) - 1LL); __auto_type __da3 = __lang_str_concat(mu_acc, mu_h); mu__u8_slice__direct(__da0, __da1, __da2, __da3); }) : mu_acc)); }) : (({ __lang_fail_impl("no matching arm in match"); ""; }))); });
}
static long long __anon_160_fn(void* __env_self_void, long long mu_acc) {
  __anon_160_env* __env_self = (__anon_160_env*)__env_self_void;
  return (((__env_self->mu_i) >= (__env_self->mu_n)) ? mu_acc : ({ __auto_type __let_tmp_b0 = __lang_ord(__lang_char_at((__env_self->mu_s), (__env_self->mu_i))); __auto_type mu_b0 = __let_tmp_b0; ((mu_b0 < 194LL) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 1LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + 1LL); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : ((mu_b0 < 224LL) ? ((((__env_self->mu_i) + 1LL) < (__env_self->mu_n)) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 2LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ((__lang_imod(mu_b0, 32LL) * 64LL) + __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL)); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL)) : ((mu_b0 < 240LL) ? ((((__env_self->mu_i) + 2LL) < (__env_self->mu_n)) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 3LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc322 = mu_b0; __auto_type __bc323 = 16LL; __auto_type __bc324 = __lang_imod(__bc322, __bc323); __auto_type __bc325 = 4096LL; __auto_type __bc326 = (__bc324 * __bc325); __auto_type __bc327 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL) * 64LL); __auto_type __bc328 = (__bc326 + __bc327); __auto_type __bc329 = __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 2LL))), 64LL); __auto_type __bc330 = (__bc328 + __bc329); __bc330; }); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL)) : ((((__env_self->mu_i) + 3LL) < (__env_self->mu_n)) ? ({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = ((__env_self->mu_i) + 4LL); __auto_type __da2 = (__env_self->mu_n); __auto_type __da3 = (mu_acc + ({ __auto_type __da0 = ({ __auto_type __bc311 = mu_b0; __auto_type __bc312 = 8LL; __auto_type __bc313 = __lang_imod(__bc311, __bc312); __auto_type __bc314 = 262144LL; __auto_type __bc315 = (__bc313 * __bc314); __auto_type __bc316 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 1LL))), 64LL) * 4096LL); __auto_type __bc317 = (__bc315 + __bc316); __auto_type __bc318 = (__lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 2LL))), 64LL) * 64LL); __auto_type __bc319 = (__bc317 + __bc318); __auto_type __bc320 = __lang_imod(__lang_ord(__lang_char_at((__env_self->mu_s), ((__env_self->mu_i) + 3LL))), 64LL); __auto_type __bc321 = (__bc319 + __bc320); __bc321; }); mu__eaw_width__direct(__da0); })); mu__u8w_go__direct(__da0, __da1, __da2, __da3); }) : (mu_acc + 1LL))))); }));
}
static long long __anon_161_fn(void* __env_self_void, mere_vec_int* mu_syms) {
  __anon_161_env* __env_self = (__anon_161_env*)__env_self_void;
  return ((((1LL >= 0LL) && (1LL <= 16LL)) && (16LL <= mere_vec_int_len((__env_self->mu_count)))) ? ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go__rvfast_uq3_9__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), mu_syms, __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = 1LL; __auto_type __ia1 = 0LL; __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_go_uq4_10__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), mu_syms, __ia0, __ia1, __ia2, __ia3); }));
}
static closure_Vec_int_closure_Vec_int_closure_Vec_int_unit __anon_162_fn(void* __env_self_void, mere_vec_int* mu_lcount) {
  __anon_162_env* __env_self = (__anon_162_env*)__env_self_void;
  return ({ __anon_187_env* __env = (__anon_187_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_187_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_187_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = mu_lcount; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_187_fn, .fn2 = __anon_187_fn2}; });
}
static closure_Vec_int_closure_Vec_int_unit __anon_162_fn2(void* __env_self_void, mere_vec_int* mu_lcount, mere_vec_int* mu_lsyms) {
  __anon_162_env* __env_self = (__anon_162_env*)__env_self_void;
  return ({ __anon_188_env* __env = (__anon_188_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_188_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_188_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = mu_lcount; __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_188_fn, .fn2 = __anon_188_fn2}; });
}
static closure_Vec_int_closure_Vec_int_unit __anon_163_fn(void* __env_self_void, mere_vec_int* mu_lsyms) {
  __anon_163_env* __env_self = (__anon_163_env*)__env_self_void;
  return ({ __anon_189_env* __env = (__anon_189_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_189_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_189_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_189_fn, .fn2 = __anon_189_fn2}; });
}
static closure_Vec_int_unit __anon_163_fn2(void* __env_self_void, mere_vec_int* mu_lsyms, mere_vec_int* mu_dcount) {
  __anon_163_env* __env_self = (__anon_163_env*)__env_self_void;
  return ({ __anon_190_env* __env = (__anon_190_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_190_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_190_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_190_fn}; });
}
static closure_Vec_int_closure_Vec_int_unit __anon_164_fn(void* __env_self_void, mere_vec_int* mu_lsyms) {
  __anon_164_env* __env_self = (__anon_164_env*)__env_self_void;
  return ({ __anon_191_env* __env = (__anon_191_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_191_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_191_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_191_fn, .fn2 = __anon_191_fn2}; });
}
static closure_Vec_int_unit __anon_164_fn2(void* __env_self_void, mere_vec_int* mu_lsyms, mere_vec_int* mu_dcount) {
  __anon_164_env* __env_self = (__anon_164_env*)__env_self_void;
  return ({ __anon_192_env* __env = (__anon_192_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_192_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_192_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_192_fn}; });
}
static closure_Vec_int_unit __anon_165_fn(void* __env_self_void, mere_vec_int* mu_dcount) {
  __anon_165_env* __env_self = (__anon_165_env*)__env_self_void;
  return ({ __anon_193_env* __env = (__anon_193_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_193_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_193_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = (__env_self->mu_lsyms); __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_193_fn}; });
}
static int __anon_165_fn2(void* __env_self_void, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms) {
  __anon_165_env* __env_self = (__anon_165_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, mu_dcount, mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static closure_int_closure_int_bool __anon_166_fn(void* __env_self_void, long long mu_b) {
  __anon_166_env* __env_self = (__anon_166_env*)__env_self_void;
  return ({ __anon_194_env* __env = (__anon_194_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_194_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_194_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = mu_b; (closure_int_closure_int_bool){.env = __env, .fn = __anon_194_fn, .fn2 = __anon_194_fn2}; });
}
static closure_int_bool __anon_166_fn2(void* __env_self_void, long long mu_b, long long mu_c) {
  __anon_166_env* __env_self = (__anon_166_env*)__env_self_void;
  return ({ __anon_195_env* __env = (__anon_195_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_195_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_195_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = mu_b; __env->mu_c = mu_c; (closure_int_bool){.env = __env, .fn = __anon_195_fn}; });
}
static closure_int_bool __anon_167_fn(void* __env_self_void, long long mu_c) {
  __anon_167_env* __env_self = (__anon_167_env*)__env_self_void;
  return ({ __anon_196_env* __env = (__anon_196_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_196_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_196_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = (__env_self->mu_b); __env->mu_c = mu_c; (closure_int_bool){.env = __env, .fn = __anon_196_fn}; });
}
static int __anon_167_fn2(void* __env_self_void, long long mu_c, long long mu_d) {
  __anon_167_env* __env_self = (__anon_167_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == mu_c)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static closure_int_bool __anon_168_fn(void* __env_self_void, long long mu_c) {
  __anon_168_env* __env_self = (__anon_168_env*)__env_self_void;
  return ({ __anon_197_env* __env = (__anon_197_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_197_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_197_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = (__env_self->mu_b); __env->mu_c = mu_c; (closure_int_bool){.env = __env, .fn = __anon_197_fn}; });
}
static int __anon_168_fn2(void* __env_self_void, long long mu_c, long long mu_d) {
  __anon_168_env* __env_self = (__anon_168_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == mu_c)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static int __anon_169_fn(void* __env_self_void, long long mu_d) {
  __anon_169_env* __env_self = (__anon_169_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == (__env_self->mu_c))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static closure_int_ByteBuf __anon_170_fn(void* __env_self_void, long long mu_height) {
  __anon_170_env* __env_self = (__anon_170_env*)__env_self_void;
  return ({ __anon_198_env* __env = (__anon_198_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_198_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_198_env; __env->mu_width = (__env_self->mu_width); __env->mu_height = mu_height; __env->mu_start = (__env_self->mu_start); __env->mu_raw = (__env_self->mu_raw); (closure_int_ByteBuf){.env = __env, .fn = __anon_198_fn}; });
}
static mere_bytebuf* __anon_170_fn2(void* __env_self_void, long long mu_height, long long mu_bpp) {
  __anon_170_env* __env_self = (__anon_170_env*)__env_self_void;
  return ({ __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (mu_stride * mu_height)); __auto_type mu_out = __let_tmp_out; __lifted_row_40(mu_height, mu_out, (__env_self->mu_start), mu_stride, (__env_self->mu_raw), mu_bpp, 0LL); });
}
static mere_bytebuf* __anon_171_fn(void* __env_self_void, long long mu_bpp) {
  __anon_171_env* __env_self = (__anon_171_env*)__env_self_void;
  return ({ __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (mu_stride * (__env_self->mu_height))); __auto_type mu_out = __let_tmp_out; __lifted_row_40((__env_self->mu_height), mu_out, (__env_self->mu_start), mu_stride, (__env_self->mu_raw), mu_bpp, 0LL); });
}
static mere_bytebuf* __anon_172_fn(void* __env_self_void, long long mu_bpp) {
  __anon_172_env* __env_self = (__anon_172_env*)__env_self_void;
  return ({ __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (mu_stride * (__env_self->mu_height))); __auto_type mu_out = __let_tmp_out; __lifted_row_40((__env_self->mu_height), mu_out, (__env_self->mu_start), mu_stride, (__env_self->mu_raw), mu_bpp, 0LL); });
}
static mere_bytebuf* __anon_173_fn(void* __env_self_void, long long mu_bpp) {
  __anon_173_env* __env_self = (__anon_173_env*)__env_self_void;
  return ({ __auto_type __da0 = (__env_self->mu_raw); __auto_type __da1 = 0LL; __auto_type __da2 = (__env_self->mu_width); __auto_type __da3 = (__env_self->mu_height); __auto_type __da4 = mu_bpp; mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf__direct(__lang_current_region, __da0, __da1, __da2, __da3, __da4); });
}
static mere_bytebuf* __anon_174_fn(void* __env_self_void, long long mu_bpp) {
  __anon_174_env* __env_self = (__anon_174_env*)__env_self_void;
  return ({ __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (((__env_self->mu_width) * (__env_self->mu_height)) * mu_bpp)); __auto_type mu_out = __let_tmp_out; ({ __auto_type __ia0 = 0LL; __auto_type __ia1 = 0LL; __lifted_pass_43__direct(mu_out, (__env_self->mu_width), (__env_self->mu_height), (__env_self->mu_raw), mu_bpp, __ia0, __ia1); }); });
}
static int __anon_175_fn(void* __env_self_void, long long mu_n) {
  __anon_175_env* __env_self = (__anon_175_env*)__env_self_void;
  return __lifted_go_uq34_52(mu_n, (__env_self->mu_v), (__env_self->mu_s), (__env_self->mu_out), 0LL);
}
static closure_int_closure_Vec_int_unit __anon_176_fn(void* __env_self_void, long long mu_t2) {
  __anon_176_env* __env_self = (__anon_176_env*)__env_self_void;
  return ({ __anon_199_env* __env = (__anon_199_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_199_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_199_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = mu_t2; (closure_int_closure_Vec_int_unit){.env = __env, .fn = __anon_199_fn, .fn2 = __anon_199_fn2}; });
}
static closure_Vec_int_unit __anon_176_fn2(void* __env_self_void, long long mu_t2, long long mu_t3) {
  __anon_176_env* __env_self = (__anon_176_env*)__env_self_void;
  return ({ __anon_200_env* __env = (__anon_200_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_200_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_200_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = mu_t2; __env->mu_t3 = mu_t3; (closure_Vec_int_unit){.env = __env, .fn = __anon_200_fn}; });
}
static closure_Vec_int_unit __anon_177_fn(void* __env_self_void, long long mu_t3) {
  __anon_177_env* __env_self = (__anon_177_env*)__env_self_void;
  return ({ __anon_201_env* __env = (__anon_201_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_201_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_201_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = (__env_self->mu_t2); __env->mu_t3 = mu_t3; (closure_Vec_int_unit){.env = __env, .fn = __anon_201_fn}; });
}
static int __anon_177_fn2(void* __env_self_void, long long mu_t3, mere_vec_int* mu_body) {
  __anon_177_env* __env_self = (__anon_177_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, mu_t3)); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static closure_Vec_int_unit __anon_178_fn(void* __env_self_void, long long mu_t3) {
  __anon_178_env* __env_self = (__anon_178_env*)__env_self_void;
  return ({ __anon_202_env* __env = (__anon_202_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_202_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_202_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = (__env_self->mu_t2); __env->mu_t3 = mu_t3; (closure_Vec_int_unit){.env = __env, .fn = __anon_202_fn}; });
}
static int __anon_178_fn2(void* __env_self_void, long long mu_t3, mere_vec_int* mu_body) {
  __anon_178_env* __env_self = (__anon_178_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, mu_t3)); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static int __anon_179_fn(void* __env_self_void, mere_vec_int* mu_body) {
  __anon_179_env* __env_self = (__anon_179_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t3))); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static mere_vec_int* __anon_180_fn(void* __env_self_void, long long mu_bpp) {
  __anon_180_env* __env_self = (__anon_180_env*)__env_self_void;
  return ({ __auto_type __let_tmp_n = mere_vec_int_len((__env_self->mu_row_uq58)); __auto_type mu_n = __let_tmp_n; __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; __lifted_go_uq59_112(mu_n, mu_out, (__env_self->mu_row_uq58), mu_bpp, (__env_self->mu_prev), (__env_self->mu_kind), 0LL); });
}
static long long __anon_181_fn(void* __env_self_void, long long mu_b) {
  __anon_181_env* __env_self = (__anon_181_env*)__env_self_void;
  return ((((((0LL >= 0LL) && (0LL <= mere_vec_int_len((__env_self->mu_pal)))) && ((((0LL + 2LL) >= 0LL) && (((0LL + 2LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 2LL) >= 0LL) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 2LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))))) && ((((0LL + 1LL) >= 0LL) && (((0LL + 1LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 1LL) >= 0LL) && ((((mere_vec_int_len((__env_self->mu_pal)) - 1LL) + 1LL) + 1LL) <= mere_vec_int_len((__env_self->mu_pal)))))) && (mere_vec_int_len((__env_self->mu_pal)) <= mere_vec_int_len((__env_self->mu_pal)))) ? __lifted_go__rvfast_uq68_120((__env_self->mu_pal), (__env_self->mu_r), (__env_self->mu_g), mu_b, 0LL) : __lifted_go_uq69_121((__env_self->mu_pal), (__env_self->mu_r), (__env_self->mu_g), mu_b, 0LL));
}
static mere_vec_int* __anon_182_fn(void* __env_self_void, long long mu_depth) {
  __anon_182_env* __env_self = (__anon_182_env*)__env_self_void;
  return ({ __auto_type __let_tmp_count = ((__env_self->mu_width) * (__env_self->mu_height)); __auto_type mu_count = __let_tmp_count; __auto_type __let_tmp_eight = (mu_depth == 8LL); __auto_type mu_eight = __let_tmp_eight; __auto_type __let_tmp_grey = (mu_eight && ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_count; mu_all_grey__direct(__da0, __da1); })); __auto_type mu_grey = __let_tmp_grey; __auto_type __let_tmp_pal = ((mu_grey || (mu_eight == 0)) ? mere_vec_int_new(__lang_current_region) : ({ __auto_type __da0 = (__env_self->mu_pixels); __auto_type __da1 = mu_count; mu_collect_palette__direct(__da0, __da1); })); __auto_type mu_pal = __let_tmp_pal; __auto_type __let_tmp_indexed = ((((mu_grey == 0) && mu_eight) && (mere_vec_int_len(mu_pal) <= 768LL)) && (mere_vec_int_len(mu_pal) > 0LL)); __auto_type mu_indexed = __let_tmp_indexed; __auto_type __let_tmp_bpp = ((mu_grey || mu_indexed) ? 1LL : (mu_eight ? 3LL : 6LL)); __auto_type mu_bpp = __let_tmp_bpp; __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_raw = mere_vec_int_new(__lang_current_region); __auto_type mu_raw = __let_tmp_raw; ({ (void)(({ __auto_type __ia0 = 0LL; __auto_type __ia1 = mere_vec_int_new(__lang_current_region); __lifted_rows_122__direct((__env_self->mu_height), mu_stride, mu_grey, (__env_self->mu_pixels), (__env_self->mu_width), mu_indexed, mu_pal, mu_raw, mu_bpp, __ia0, __ia1); })); __auto_type __let_tmp_z = mere_vec_int_new(__lang_current_region); __auto_type mu_z = __let_tmp_z; (void)(mere_vec_int_push(mu_z, 120LL)); (void)(mere_vec_int_push(mu_z, 1LL)); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_deflate_dynamic__Vec_int__Vec_int__direct(__lang_current_region, __da0); }); mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_z; __auto_type __da1 = ({ __auto_type __da0 = mu_raw; mu_adler32__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = ({ __auto_type __let_tmp_sig = mere_vec_int_new(__lang_current_region); __auto_type mu_sig = __let_tmp_sig; __lifted_go_uq70_124(mu_sig, 0LL); }); mu_push_all__direct(__da0, __da1); })); __auto_type __let_tmp_ihdr = mere_vec_int_new(__lang_current_region); __auto_type mu_ihdr = __let_tmp_ihdr; (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = (__env_self->mu_width); mu_bytes_push4be__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = mu_ihdr; __auto_type __da1 = (__env_self->mu_height); mu_bytes_push4be__direct(__da0, __da1); })); (void)(mere_vec_int_push(mu_ihdr, mu_depth)); (void)(mere_vec_int_push(mu_ihdr, (mu_grey ? 0LL : (mu_indexed ? 3LL : 2LL)))); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(mere_vec_int_push(mu_ihdr, 0LL)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 72LL; __auto_type __da3 = 68LL; __auto_type __da4 = 82LL; __auto_type __da5 = mu_ihdr; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)((mu_indexed ? ({ __auto_type __da0 = mu_out; __auto_type __da1 = 80LL; __auto_type __da2 = 76LL; __auto_type __da3 = 84LL; __auto_type __da4 = 69LL; __auto_type __da5 = mu_pal; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); }) : 0)); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 68LL; __auto_type __da3 = 65LL; __auto_type __da4 = 84LL; __auto_type __da5 = mu_z; mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = 73LL; __auto_type __da2 = 69LL; __auto_type __da3 = 78LL; __auto_type __da4 = 68LL; __auto_type __da5 = mere_vec_int_new(__lang_current_region); mu_chunk__direct(__da0, __da1, __da2, __da3, __da4, __da5); })); mu_out; }); });
}
static long long __anon_183_fn(void* __env_self_void, long long mu_index) {
  __anon_183_env* __env_self = (__anon_183_env*)__env_self_void;
  return (((__env_self->mu_len) == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = (((__env_self->mu_code0)) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get_unchecked((__env_self->mu_count), (__env_self->mu_len)); __auto_type mu_c = __let_tmp_c; ((mu_code < ((__env_self->mu_first) + mu_c)) ? mere_vec_int_get((__env_self->mu_syms), (mu_index + (mu_code - (__env_self->mu_first)))) : ({ __auto_type __ia0 = ((__env_self->mu_len) + 1LL); __auto_type __ia1 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia2 = ({ long long __sa = (((__env_self->mu_first) + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia3 = (mu_index + mu_c); __lifted_go__rvfast_uq3_9__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), (__env_self->mu_syms), __ia0, __ia1, __ia2, __ia3); })); }));
}
static long long __anon_184_fn(void* __env_self_void, long long mu_index) {
  __anon_184_env* __env_self = (__anon_184_env*)__env_self_void;
  return (((__env_self->mu_len) == 16LL) ? (0LL - 1LL) : ({ __auto_type __let_tmp_code = (((__env_self->mu_code0)) | (({ __auto_type __da0 = (__env_self->mu_s); __auto_type __da1 = (__env_self->mu_data); mu_br_bit__direct(__da0, __da1); }))); __auto_type mu_code = __let_tmp_code; __auto_type __let_tmp_c = mere_vec_int_get((__env_self->mu_count), (__env_self->mu_len)); __auto_type mu_c = __let_tmp_c; ((mu_code < ((__env_self->mu_first) + mu_c)) ? mere_vec_int_get((__env_self->mu_syms), (mu_index + (mu_code - (__env_self->mu_first)))) : ({ __auto_type __ia0 = ((__env_self->mu_len) + 1LL); __auto_type __ia1 = ({ long long __sa = (mu_code); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia2 = ({ long long __sa = (((__env_self->mu_first) + mu_c)); long long __sn = (1LL); ((unsigned long long)__sn > 63ULL) ? 0LL : (long long)((unsigned long long)__sa << __sn); }); __auto_type __ia3 = (mu_index + mu_c); __lifted_go_uq4_10__direct((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_count), (__env_self->mu_syms), __ia0, __ia1, __ia2, __ia3); })); }));
}
static mere_vec_int* __anon_185_fn(void* __env_self_void, mere_vec_int* mu_best_uq65) {
  __anon_185_env* __env_self = (__anon_185_env*)__env_self_void;
  return (((__env_self->mu_kind) == 5LL) ? ({ __auto_type __let_tmp_out = mere_vec_int_new(__lang_current_region); __auto_type mu_out = __let_tmp_out; (void)(mere_vec_int_push(mu_out, (__env_self->mu_bk))); (void)(({ __auto_type __da0 = mu_out; __auto_type __da1 = mu_best_uq65; mu_push_all__direct(__da0, __da1); })); mu_out; }) : ({ __auto_type __let_tmp_cand = ({ __auto_type __da0 = (__env_self->mu_kind); __auto_type __da1 = (__env_self->mu_row_uq63); __auto_type __da2 = (__env_self->mu_prev); __auto_type __da3 = (__env_self->mu_bpp); mu_filter_row__direct(__da0, __da1, __da2, __da3); }); __auto_type mu_cand = __let_tmp_cand; __auto_type __let_tmp_c = ({ __auto_type __da0 = mu_cand; mu_row_cost__direct(__da0); }); __auto_type mu_c = __let_tmp_c; ((mu_c < (__env_self->mu_bc)) ? ({ __auto_type __ia0 = ((__env_self->mu_kind) + 1LL); __auto_type __ia1 = (__env_self->mu_kind); __auto_type __ia2 = mu_c; __auto_type __ia3 = mu_cand; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), (__env_self->mu_bpp), __ia0, __ia1, __ia2, __ia3); }) : ({ __auto_type __ia0 = ((__env_self->mu_kind) + 1LL); __auto_type __ia1 = (__env_self->mu_bk); __auto_type __ia2 = (__env_self->mu_bc); __auto_type __ia3 = mu_best_uq65; __lifted_go_uq64_115__direct((__env_self->mu_row_uq63), (__env_self->mu_prev), (__env_self->mu_bpp), __ia0, __ia1, __ia2, __ia3); })); }));
}
static int __anon_186_fn(void* __env_self_void, long long mu_seen_uq73) {
  __anon_186_env* __env_self = (__anon_186_env*)__env_self_void;
  return (((__env_self->mu_i) >= (__env_self->mu_n)) ? 0 : ({ __auto_type __let_tmp_c = mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)); __auto_type mu_c = __let_tmp_c; (((mu_c >= 48LL) && (mu_c <= 57LL)) ? ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_got); __auto_type __ia2 = ((((__env_self->mu_cur) * 10LL) + mu_c) - 48LL); __auto_type __ia3 = 1LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); }) : ((mu_seen_uq73 == 1LL) ? ({ (void)(mere_vec_int_push((__env_self->mu_acc), (__env_self->mu_cur))); (((__env_self->mu_got) == 2LL) ? ({ (void)(mere_vec_int_push((__env_self->mu_acc), ((__env_self->mu_i) + 1LL))); 0; }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = ((__env_self->mu_got) + 1LL); __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); })); }) : ({ __auto_type __ia0 = ((__env_self->mu_i) + 1LL); __auto_type __ia1 = (__env_self->mu_got); __auto_type __ia2 = 0LL; __auto_type __ia3 = 0LL; __lifted_fields_129__direct((__env_self->mu_n), (__env_self->mu_data), (__env_self->mu_acc), __ia0, __ia1, __ia2, __ia3); }))); }));
}
static closure_Vec_int_closure_Vec_int_unit __anon_187_fn(void* __env_self_void, mere_vec_int* mu_lsyms) {
  __anon_187_env* __env_self = (__anon_187_env*)__env_self_void;
  return ({ __anon_203_env* __env = (__anon_203_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_203_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_203_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); (closure_Vec_int_closure_Vec_int_unit){.env = __env, .fn = __anon_203_fn, .fn2 = __anon_203_fn2}; });
}
static closure_Vec_int_unit __anon_187_fn2(void* __env_self_void, mere_vec_int* mu_lsyms, mere_vec_int* mu_dcount) {
  __anon_187_env* __env_self = (__anon_187_env*)__env_self_void;
  return ({ __anon_204_env* __env = (__anon_204_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_204_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_204_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = mu_lsyms; __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_204_fn}; });
}
static closure_Vec_int_unit __anon_188_fn(void* __env_self_void, mere_vec_int* mu_dcount) {
  __anon_188_env* __env_self = (__anon_188_env*)__env_self_void;
  return ({ __anon_205_env* __env = (__anon_205_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_205_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_205_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = (__env_self->mu_lsyms); __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_205_fn}; });
}
static int __anon_188_fn2(void* __env_self_void, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms) {
  __anon_188_env* __env_self = (__anon_188_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, mu_dcount, mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static closure_Vec_int_unit __anon_189_fn(void* __env_self_void, mere_vec_int* mu_dcount) {
  __anon_189_env* __env_self = (__anon_189_env*)__env_self_void;
  return ({ __anon_206_env* __env = (__anon_206_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_206_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_206_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = (__env_self->mu_lsyms); __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_206_fn}; });
}
static int __anon_189_fn2(void* __env_self_void, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms) {
  __anon_189_env* __env_self = (__anon_189_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, mu_dcount, mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_190_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_190_env* __env_self = (__anon_190_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static closure_Vec_int_unit __anon_191_fn(void* __env_self_void, mere_vec_int* mu_dcount) {
  __anon_191_env* __env_self = (__anon_191_env*)__env_self_void;
  return ({ __anon_207_env* __env = (__anon_207_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_207_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_207_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = (__env_self->mu_lsyms); __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_207_fn}; });
}
static int __anon_191_fn2(void* __env_self_void, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms) {
  __anon_191_env* __env_self = (__anon_191_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, mu_dcount, mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_192_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_192_env* __env_self = (__anon_192_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_193_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_193_env* __env_self = (__anon_193_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static closure_int_bool __anon_194_fn(void* __env_self_void, long long mu_c) {
  __anon_194_env* __env_self = (__anon_194_env*)__env_self_void;
  return ({ __anon_208_env* __env = (__anon_208_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_208_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_208_env; __env->mu_data = (__env_self->mu_data); __env->mu_i = (__env_self->mu_i); __env->mu_a = (__env_self->mu_a); __env->mu_b = (__env_self->mu_b); __env->mu_c = mu_c; (closure_int_bool){.env = __env, .fn = __anon_208_fn}; });
}
static int __anon_194_fn2(void* __env_self_void, long long mu_c, long long mu_d) {
  __anon_194_env* __env_self = (__anon_194_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == mu_c)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static int __anon_195_fn(void* __env_self_void, long long mu_d) {
  __anon_195_env* __env_self = (__anon_195_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == (__env_self->mu_c))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static int __anon_196_fn(void* __env_self_void, long long mu_d) {
  __anon_196_env* __env_self = (__anon_196_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == (__env_self->mu_c))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static int __anon_197_fn(void* __env_self_void, long long mu_d) {
  __anon_197_env* __env_self = (__anon_197_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == (__env_self->mu_c))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static mere_bytebuf* __anon_198_fn(void* __env_self_void, long long mu_bpp) {
  __anon_198_env* __env_self = (__anon_198_env*)__env_self_void;
  return ({ __auto_type __let_tmp_stride = ((__env_self->mu_width) * mu_bpp); __auto_type mu_stride = __let_tmp_stride; __auto_type __let_tmp_out = mere_bytebuf_new(__lang_current_region, (mu_stride * (__env_self->mu_height))); __auto_type mu_out = __let_tmp_out; __lifted_row_40((__env_self->mu_height), mu_out, (__env_self->mu_start), mu_stride, (__env_self->mu_raw), mu_bpp, 0LL); });
}
static closure_Vec_int_unit __anon_199_fn(void* __env_self_void, long long mu_t3) {
  __anon_199_env* __env_self = (__anon_199_env*)__env_self_void;
  return ({ __anon_209_env* __env = (__anon_209_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_209_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_209_env; __env->mu_out = (__env_self->mu_out); __env->mu_t0 = (__env_self->mu_t0); __env->mu_t1 = (__env_self->mu_t1); __env->mu_t2 = (__env_self->mu_t2); __env->mu_t3 = mu_t3; (closure_Vec_int_unit){.env = __env, .fn = __anon_209_fn}; });
}
static int __anon_199_fn2(void* __env_self_void, long long mu_t3, mere_vec_int* mu_body) {
  __anon_199_env* __env_self = (__anon_199_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, mu_t3)); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static int __anon_200_fn(void* __env_self_void, mere_vec_int* mu_body) {
  __anon_200_env* __env_self = (__anon_200_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t3))); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static int __anon_201_fn(void* __env_self_void, mere_vec_int* mu_body) {
  __anon_201_env* __env_self = (__anon_201_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t3))); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static int __anon_202_fn(void* __env_self_void, mere_vec_int* mu_body) {
  __anon_202_env* __env_self = (__anon_202_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t3))); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static closure_Vec_int_unit __anon_203_fn(void* __env_self_void, mere_vec_int* mu_dcount) {
  __anon_203_env* __env_self = (__anon_203_env*)__env_self_void;
  return ({ __anon_210_env* __env = (__anon_210_env*)__lang_region_alloc(__lang_current_region, sizeof(__anon_210_env)); __env->__r = __lang_current_region; __env->__copy = __mcopy_env___anon_210_env; __env->mu_s = (__env_self->mu_s); __env->mu_data = (__env_self->mu_data); __env->mu_lcount = (__env_self->mu_lcount); __env->mu_lsyms = (__env_self->mu_lsyms); __env->mu_out = (__env_self->mu_out); __env->mu_dcount = mu_dcount; (closure_Vec_int_unit){.env = __env, .fn = __anon_210_fn}; });
}
static int __anon_203_fn2(void* __env_self_void, mere_vec_int* mu_dcount, mere_vec_int* mu_dsyms) {
  __anon_203_env* __env_self = (__anon_203_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, mu_dcount, mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_204_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_204_env* __env_self = (__anon_204_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_205_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_205_env* __env_self = (__anon_205_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_206_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_206_env* __env_self = (__anon_206_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_207_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_207_env* __env_self = (__anon_207_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static int __anon_208_fn(void* __env_self_void, long long mu_d) {
  __anon_208_env* __env_self = (__anon_208_env*)__env_self_void;
  return ((((mere_vec_int_get((__env_self->mu_data), (__env_self->mu_i)) == (__env_self->mu_a)) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 1LL)) == (__env_self->mu_b))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 2LL)) == (__env_self->mu_c))) && (mere_vec_int_get((__env_self->mu_data), ((__env_self->mu_i) + 3LL)) == mu_d));
}
static int __anon_209_fn(void* __env_self_void, mere_vec_int* mu_body) {
  __anon_209_env* __env_self = (__anon_209_env*)__env_self_void;
  return ({ (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mere_vec_int_len(mu_body); mu_bytes_push4be__direct(__da0, __da1); })); __auto_type __let_tmp_tagged = mere_vec_int_new(__lang_current_region); __auto_type mu_tagged = __let_tmp_tagged; (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t0))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t1))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t2))); (void)(mere_vec_int_push(mu_tagged, (__env_self->mu_t3))); (void)(({ __auto_type __da0 = mu_tagged; __auto_type __da1 = mu_body; mu_push_all__direct(__da0, __da1); })); (void)(({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = mu_tagged; mu_push_all__direct(__da0, __da1); })); ({ __auto_type __da0 = (__env_self->mu_out); __auto_type __da1 = ({ __auto_type __da0 = mu_tagged; mu_crc32_vec__v2__direct(__da0); }); mu_bytes_push4be__direct(__da0, __da1); }); });
}
static int __anon_210_fn(void* __env_self_void, mere_vec_int* mu_dsyms) {
  __anon_210_env* __env_self = (__anon_210_env*)__env_self_void;
  return __lifted_loop_14((__env_self->mu_s), (__env_self->mu_data), (__env_self->mu_lcount), (__env_self->mu_lsyms), (__env_self->mu_out), mu_len_base, mu_len_extra, (__env_self->mu_dcount), mu_dsyms, mu_dist_base, mu_dist_extra, 0);
}
static closure_int_closure_closure_int_closure_int_int_int mu_list_fold_closure_fn(void* __env, list_int mu_xs) {
  (void)__env;
  return mu_list_fold(mu_xs);
}
static const closure_list_int_closure_int_closure_closure_int_closure_int_int_int mu_list_fold_as_value = {.env = NULL, .fn = mu_list_fold_closure_fn};
static closure_int_closure_list_int_list_int mu__range_down_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu__range_down(mu_a);
}
static const closure_int_closure_int_closure_list_int_list_int mu__range_down_as_value = {.env = NULL, .fn = mu__range_down_closure_fn};
static closure_int_list_int mu_range_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_range(mu_a);
}
static list_int mu_range_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_range__direct(__a0, __a1);
}
static const closure_int_closure_int_list_int mu_range_as_value = {.env = NULL, .fn = mu_range_closure_fn, .fn2 = mu_range_closure_fn2};
static long long mu_list_sum_closure_fn(void* __env, list_int mu_xs) {
  (void)__env;
  return mu_list_sum(mu_xs);
}
static const closure_list_int_int mu_list_sum_as_value = {.env = NULL, .fn = mu_list_sum_closure_fn};
static long long mu_list_product_closure_fn(void* __env, list_int mu_xs) {
  (void)__env;
  return mu_list_product(mu_xs);
}
static const closure_list_int_int mu_list_product_as_value = {.env = NULL, .fn = mu_list_product_closure_fn};
static closure_int_str mu__u8_nth_closure_fn(void* __env, list_str mu_cs) {
  (void)__env;
  return mu__u8_nth(mu_cs);
}
static const char* mu__u8_nth_closure_fn2(void* __env, list_str __a0, long long __a1) {
  (void)__env;
  return mu__u8_nth__direct(__a0, __a1);
}
static const closure_list_str_closure_int_str mu__u8_nth_as_value = {.env = NULL, .fn = mu__u8_nth_closure_fn, .fn2 = mu__u8_nth_closure_fn2};
static closure_int_str mu_utf8_at_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_utf8_at(mu_s);
}
static const char* mu_utf8_at_closure_fn2(void* __env, const char* __a0, long long __a1) {
  (void)__env;
  return mu_utf8_at__direct(__a0, __a1);
}
static const closure_str_closure_int_str mu_utf8_at_as_value = {.env = NULL, .fn = mu_utf8_at_closure_fn, .fn2 = mu_utf8_at_closure_fn2};
static int mu__cp_bad_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu__cp_bad(mu_n);
}
static const closure_int_bool mu__cp_bad_as_value = {.env = NULL, .fn = mu__cp_bad_closure_fn};
static const char* mu_str_of_codepoint_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_str_of_codepoint(mu_n);
}
static const closure_int_str mu_str_of_codepoint_as_value = {.env = NULL, .fn = mu_str_of_codepoint_closure_fn};
static long long mu__cp_span_closure_fn(void* __env, long long mu_b) {
  (void)__env;
  return mu__cp_span(mu_b);
}
static const closure_int_int mu__cp_span_as_value = {.env = NULL, .fn = mu__cp_span_closure_fn};
static closure_int_int mu__cp_cont_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu__cp_cont(mu_s);
}
static long long mu__cp_cont_closure_fn2(void* __env, const char* __a0, long long __a1) {
  (void)__env;
  return mu__cp_cont__direct(__a0, __a1);
}
static const closure_str_closure_int_int mu__cp_cont_as_value = {.env = NULL, .fn = mu__cp_cont_closure_fn, .fn2 = mu__cp_cont_closure_fn2};
static long long mu_codepoint_of_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_codepoint_of(mu_s);
}
static const closure_str_int mu_codepoint_of_as_value = {.env = NULL, .fn = mu_codepoint_of_closure_fn};
static closure_int_int mu_codepoint_or_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_codepoint_or(mu_s);
}
static long long mu_codepoint_or_closure_fn2(void* __env, const char* __a0, long long __a1) {
  (void)__env;
  return mu_codepoint_or__direct(__a0, __a1);
}
static const closure_str_closure_int_int mu_codepoint_or_as_value = {.env = NULL, .fn = mu_codepoint_or_closure_fn, .fn2 = mu_codepoint_or_closure_fn2};
static closure_int_int mu_codepoint_at_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_codepoint_at(mu_s);
}
static long long mu_codepoint_at_closure_fn2(void* __env, const char* __a0, long long __a1) {
  (void)__env;
  return mu_codepoint_at__direct(__a0, __a1);
}
static const closure_str_closure_int_int mu_codepoint_at_as_value = {.env = NULL, .fn = mu_codepoint_at_closure_fn, .fn2 = mu_codepoint_at_closure_fn2};
static closure_int_closure_int_closure_str_str mu__u8_slice_closure_fn(void* __env, list_str mu_cs) {
  (void)__env;
  return mu__u8_slice(mu_cs);
}
static const closure_list_str_closure_int_closure_int_closure_str_str mu__u8_slice_as_value = {.env = NULL, .fn = mu__u8_slice_closure_fn};
static closure_int_closure_int_str mu_utf8_sub_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_utf8_sub(mu_s);
}
static const closure_str_closure_int_closure_int_str mu_utf8_sub_as_value = {.env = NULL, .fn = mu_utf8_sub_closure_fn};
static closure_str_str mu__u8_rev_join_closure_fn(void* __env, list_str mu_cs) {
  (void)__env;
  return mu__u8_rev_join(mu_cs);
}
static const char* mu__u8_rev_join_closure_fn2(void* __env, list_str __a0, const char* __a1) {
  (void)__env;
  return mu__u8_rev_join__direct(__a0, __a1);
}
static const closure_list_str_closure_str_str mu__u8_rev_join_as_value = {.env = NULL, .fn = mu__u8_rev_join_closure_fn, .fn2 = mu__u8_rev_join_closure_fn2};
static const char* mu_utf8_rev_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_utf8_rev(mu_s);
}
static const closure_str_str mu_utf8_rev_as_value = {.env = NULL, .fn = mu_utf8_rev_closure_fn};
static long long mu__eaw_width_closure_fn(void* __env, long long mu_cp) {
  (void)__env;
  return mu__eaw_width(mu_cp);
}
static const closure_int_int mu__eaw_width_as_value = {.env = NULL, .fn = mu__eaw_width_closure_fn};
static closure_int_closure_int_closure_int_int mu__u8w_go_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu__u8w_go(mu_s);
}
static const closure_str_closure_int_closure_int_closure_int_int mu__u8w_go_as_value = {.env = NULL, .fn = mu__u8w_go_closure_fn};
static long long mu_utf8_width_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_utf8_width(mu_s);
}
static const closure_str_int mu_utf8_width_as_value = {.env = NULL, .fn = mu_utf8_width_closure_fn};
static closure_int_str mu_pad_right_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_pad_right(mu_s);
}
static const char* mu_pad_right_closure_fn2(void* __env, const char* __a0, long long __a1) {
  (void)__env;
  return mu_pad_right__direct(__a0, __a1);
}
static const closure_str_closure_int_str mu_pad_right_as_value = {.env = NULL, .fn = mu_pad_right_closure_fn, .fn2 = mu_pad_right_closure_fn2};
static closure_int_str mu_pad_left_closure_fn(void* __env, const char* mu_s) {
  (void)__env;
  return mu_pad_left(mu_s);
}
static const char* mu_pad_left_closure_fn2(void* __env, const char* __a0, long long __a1) {
  (void)__env;
  return mu_pad_left__direct(__a0, __a1);
}
static const closure_str_closure_int_str mu_pad_left_as_value = {.env = NULL, .fn = mu_pad_left_closure_fn, .fn2 = mu_pad_left_closure_fn2};
static long long mu_sign_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_sign(mu_n);
}
static const closure_int_int mu_sign_as_value = {.env = NULL, .fn = mu_sign_closure_fn};
static long long mu_incr_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_incr(mu_n);
}
static const closure_int_int mu_incr_as_value = {.env = NULL, .fn = mu_incr_closure_fn};
static long long mu_decr_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_decr(mu_n);
}
static const closure_int_int mu_decr_as_value = {.env = NULL, .fn = mu_decr_closure_fn};
static long long mu_square_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_square(mu_n);
}
static const closure_int_int mu_square_as_value = {.env = NULL, .fn = mu_square_closure_fn};
static long long mu_cube_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_cube(mu_n);
}
static const closure_int_int mu_cube_as_value = {.env = NULL, .fn = mu_cube_closure_fn};
static closure_int_int mu_sum_range_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_sum_range(mu_a);
}
static long long mu_sum_range_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_sum_range__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_sum_range_as_value = {.env = NULL, .fn = mu_sum_range_closure_fn, .fn2 = mu_sum_range_closure_fn2};
static closure_int_closure_int_int mu__pow_go_closure_fn(void* __env, long long mu_acc) {
  (void)__env;
  return mu__pow_go(mu_acc);
}
static const closure_int_closure_int_closure_int_int mu__pow_go_as_value = {.env = NULL, .fn = mu__pow_go_closure_fn};
static closure_int_int mu_pow_closure_fn(void* __env, long long mu_b) {
  (void)__env;
  return mu_pow(mu_b);
}
static long long mu_pow_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_pow__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_pow_as_value = {.env = NULL, .fn = mu_pow_closure_fn, .fn2 = mu_pow_closure_fn2};
static closure_int_int mu_lcm_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_lcm(mu_a);
}
static long long mu_lcm_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_lcm__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_lcm_as_value = {.env = NULL, .fn = mu_lcm_closure_fn, .fn2 = mu_lcm_closure_fn2};
static closure_int_tuple_int_int mu_divmod_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_divmod(mu_a);
}
static tuple_int_int mu_divmod_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_divmod__direct(__a0, __a1);
}
static const closure_int_closure_int_tuple_int_int mu_divmod_as_value = {.env = NULL, .fn = mu_divmod_closure_fn, .fn2 = mu_divmod_closure_fn2};
static closure_str_unit mu_assert_closure_fn(void* __env, int mu_cond) {
  (void)__env;
  return mu_assert(mu_cond);
}
static int mu_assert_closure_fn2(void* __env, int __a0, const char* __a1) {
  (void)__env;
  return mu_assert__direct(__a0, __a1);
}
static const closure_bool_closure_str_unit mu_assert_as_value = {.env = NULL, .fn = mu_assert_closure_fn, .fn2 = mu_assert_closure_fn2};
static closure_int_int mu_crc_byte_closure_fn(void* __env, long long mu_crc) {
  (void)__env;
  return mu_crc_byte(mu_crc);
}
static long long mu_crc_byte_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_crc_byte__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_crc_byte_as_value = {.env = NULL, .fn = mu_crc_byte_closure_fn, .fn2 = mu_crc_byte_closure_fn2};
static long long mu_crc32_vec_closure_fn(void* __env, mere_vec_int* mu_bytes) {
  (void)__env;
  return mu_crc32_vec(mu_bytes);
}
static const closure_Vec_int_int mu_crc32_vec_as_value = {.env = NULL, .fn = mu_crc32_vec_closure_fn};
static mere_vec_int* mu_br_new__int__Vec_int_closure_fn(void* __env, long long mu_pos) {
  (void)__env;
  return mu_br_new__int__Vec_int(mu_pos);
}
static const closure_int_Vec_int mu_br_new__int__Vec_int_as_value = {.env = NULL, .fn = mu_br_new__int__Vec_int_closure_fn};
static closure_Vec_int_int mu_br_bit_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_br_bit(mu_s);
}
static long long mu_br_bit_closure_fn2(void* __env, mere_vec_int* __a0, mere_vec_int* __a1) {
  (void)__env;
  return mu_br_bit__direct(__a0, __a1);
}
static const closure_Vec_int_closure_Vec_int_int mu_br_bit_as_value = {.env = NULL, .fn = mu_br_bit_closure_fn, .fn2 = mu_br_bit_closure_fn2};
static closure_Vec_int_closure_int_int mu_br_bits_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_br_bits(mu_s);
}
static const closure_Vec_int_closure_Vec_int_closure_int_int mu_br_bits_as_value = {.env = NULL, .fn = mu_br_bits_closure_fn};
static int mu_br_align_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_br_align(mu_s);
}
static const closure_Vec_int_unit mu_br_align_as_value = {.env = NULL, .fn = mu_br_align_closure_fn};
static tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int_closure_fn(void* __env, mere_vec_int* mu_lengths) {
  (void)__env;
  return mu_huff_build__Vec_int__tuple_Vec_int_Vec_int(mu_lengths);
}
static const closure_Vec_int_tuple_Vec_int_Vec_int mu_huff_build__Vec_int__tuple_Vec_int_Vec_int_as_value = {.env = NULL, .fn = mu_huff_build__Vec_int__tuple_Vec_int_Vec_int_closure_fn};
static closure_Vec_int_closure_Vec_int_closure_Vec_int_int mu_huff_decode_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_huff_decode(mu_s);
}
static const closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_int mu_huff_decode_as_value = {.env = NULL, .fn = mu_huff_decode_closure_fn};
static mere_vec_int* mu_vec_of_closure_fn(void* __env, list_int mu_xs) {
  (void)__env;
  return mu_vec_of(mu_xs);
}
static const closure_list_int_Vec_int mu_vec_of_as_value = {.env = NULL, .fn = mu_vec_of_closure_fn};
static mere_vec_int* mu_fixed_lit_lengths_closure_fn(void* __env, int mu__u) {
  (void)__env;
  return mu_fixed_lit_lengths(mu__u);
}
static const closure_unit_Vec_int mu_fixed_lit_lengths_as_value = {.env = NULL, .fn = mu_fixed_lit_lengths_closure_fn};
static mere_vec_int* mu_fixed_dist_lengths_closure_fn(void* __env, int mu__u) {
  (void)__env;
  return mu_fixed_dist_lengths(mu__u);
}
static const closure_unit_Vec_int mu_fixed_dist_lengths_as_value = {.env = NULL, .fn = mu_fixed_dist_lengths_closure_fn};
static closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit mu_inflate_codes_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_inflate_codes(mu_s);
}
static const closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_closure_Vec_int_unit mu_inflate_codes_as_value = {.env = NULL, .fn = mu_inflate_codes_closure_fn};
static closure_Vec_int_tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int(mu_s);
}
static const closure_Vec_int_closure_Vec_int_tuple_Vec_int_Vec_int mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int_as_value = {.env = NULL, .fn = mu_read_dynamic__Vec_int__Vec_int__tuple_Vec_int_Vec_int_closure_fn};
static closure_int_Vec_int mu_inflate_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_inflate(mu_data);
}
static mere_vec_int* mu_inflate_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_inflate__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_Vec_int mu_inflate_as_value = {.env = NULL, .fn = mu_inflate_closure_fn, .fn2 = mu_inflate_closure_fn2};
static long long mu_gzip_body_start_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_gzip_body_start(mu_data);
}
static const closure_Vec_int_int mu_gzip_body_start_as_value = {.env = NULL, .fn = mu_gzip_body_start_closure_fn};
static int mu_is_gzip_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_gzip(mu_data);
}
static const closure_Vec_int_bool mu_is_gzip_as_value = {.env = NULL, .fn = mu_is_gzip_closure_fn};
static mere_vec_int* mu_gunzip_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_gunzip(mu_data);
}
static const closure_Vec_int_Vec_int mu_gunzip_as_value = {.env = NULL, .fn = mu_gunzip_closure_fn};
static closure_int_int mu_gzip_trailer_u32_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_gzip_trailer_u32(mu_data);
}
static long long mu_gzip_trailer_u32_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_gzip_trailer_u32__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_int mu_gzip_trailer_u32_as_value = {.env = NULL, .fn = mu_gzip_trailer_u32_closure_fn, .fn2 = mu_gzip_trailer_u32_closure_fn2};
static closure_Vec_int_bool mu_gunzip_ok_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_gunzip_ok(mu_data);
}
static int mu_gunzip_ok_closure_fn2(void* __env, mere_vec_int* __a0, mere_vec_int* __a1) {
  (void)__env;
  return mu_gunzip_ok__direct(__a0, __a1);
}
static const closure_Vec_int_closure_Vec_int_bool mu_gunzip_ok_as_value = {.env = NULL, .fn = mu_gunzip_ok_closure_fn, .fn2 = mu_gunzip_ok_closure_fn2};
static closure_int_Vec_int mu_new_vec_closure_fn(void* __env, long long mu_n) {
  (void)__env;
  return mu_new_vec(mu_n);
}
static const closure_int_closure_int_Vec_int mu_new_vec_as_value = {.env = NULL, .fn = mu_new_vec_closure_fn};
static int mu_is_png_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_png(mu_data);
}
static const closure_Vec_int_bool mu_is_png_as_value = {.env = NULL, .fn = mu_is_png_closure_fn};
static closure_int_int mu_u32be_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_u32be(mu_data);
}
static long long mu_u32be_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_u32be__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_int mu_u32be_as_value = {.env = NULL, .fn = mu_u32be_closure_fn, .fn2 = mu_u32be_closure_fn2};
static closure_int_closure_int_closure_int_closure_int_closure_int_bool mu_type_is_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_type_is(mu_data);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_closure_int_closure_int_bool mu_type_is_as_value = {.env = NULL, .fn = mu_type_is_closure_fn};
static closure_int_bool mu_is_ihdr_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_ihdr(mu_data);
}
static int mu_is_ihdr_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_ihdr__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_ihdr_as_value = {.env = NULL, .fn = mu_is_ihdr_closure_fn, .fn2 = mu_is_ihdr_closure_fn2};
static closure_int_bool mu_is_idat_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_idat(mu_data);
}
static int mu_is_idat_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_idat__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_idat_as_value = {.env = NULL, .fn = mu_is_idat_closure_fn, .fn2 = mu_is_idat_closure_fn2};
static closure_int_bool mu_is_iend_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_iend(mu_data);
}
static int mu_is_iend_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_iend__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_iend_as_value = {.env = NULL, .fn = mu_is_iend_closure_fn, .fn2 = mu_is_iend_closure_fn2};
static closure_int_bool mu_is_plte_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_plte(mu_data);
}
static int mu_is_plte_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_plte__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_plte_as_value = {.env = NULL, .fn = mu_is_plte_closure_fn, .fn2 = mu_is_plte_closure_fn2};
static closure_int_bool mu_is_text_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_text(mu_data);
}
static int mu_is_text_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_text__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_text_as_value = {.env = NULL, .fn = mu_is_text_closure_fn, .fn2 = mu_is_text_closure_fn2};
static closure_int_bool mu_is_gama_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_gama(mu_data);
}
static int mu_is_gama_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_gama__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_gama_as_value = {.env = NULL, .fn = mu_is_gama_closure_fn, .fn2 = mu_is_gama_closure_fn2};
static closure_int_bool mu_is_trns_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_trns(mu_data);
}
static int mu_is_trns_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_is_trns__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_is_trns_as_value = {.env = NULL, .fn = mu_is_trns_closure_fn, .fn2 = mu_is_trns_closure_fn2};
static mere_vec_int* mu_read_header__Vec_int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_read_header__Vec_int__Vec_int(mu_data);
}
static const closure_Vec_int_Vec_int mu_read_header__Vec_int__Vec_int_as_value = {.env = NULL, .fn = mu_read_header__Vec_int__Vec_int_closure_fn};
static long long mu_channels_closure_fn(void* __env, long long mu_colour) {
  (void)__env;
  return mu_channels(mu_colour);
}
static const closure_int_int mu_channels_as_value = {.env = NULL, .fn = mu_channels_closure_fn};
static closure_int_int mu_pixel_bytes_closure_fn(void* __env, long long mu_colour) {
  (void)__env;
  return mu_pixel_bytes(mu_colour);
}
static long long mu_pixel_bytes_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_pixel_bytes__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_pixel_bytes_as_value = {.env = NULL, .fn = mu_pixel_bytes_closure_fn, .fn2 = mu_pixel_bytes_closure_fn2};
static mere_vec_int* mu_palette_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_palette(mu_data);
}
static const closure_Vec_int_Vec_int mu_palette_as_value = {.env = NULL, .fn = mu_palette_closure_fn};
static mere_vec_int* mu_idat_bytes_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_idat_bytes(mu_data);
}
static const closure_Vec_int_Vec_int mu_idat_bytes_as_value = {.env = NULL, .fn = mu_idat_bytes_closure_fn};
static mere_vec_str* mu_text_chunks_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_text_chunks(mu_data);
}
static const closure_Vec_int_Vec_str mu_text_chunks_as_value = {.env = NULL, .fn = mu_text_chunks_closure_fn};
static long long mu_gamma_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_gamma(mu_data);
}
static const closure_Vec_int_int mu_gamma_as_value = {.env = NULL, .fn = mu_gamma_closure_fn};
static int mu_has_transparency_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_has_transparency(mu_data);
}
static const closure_Vec_int_bool mu_has_transparency_as_value = {.env = NULL, .fn = mu_has_transparency_closure_fn};
static closure_int_closure_int_int mu_paeth_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_paeth(mu_a);
}
static const closure_int_closure_int_closure_int_int mu_paeth_as_value = {.env = NULL, .fn = mu_paeth_closure_fn};
static closure_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf_closure_fn(void* __env, mere_vec_int* mu_raw) {
  (void)__env;
  return mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf(mu_raw);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf_as_value = {.env = NULL, .fn = mu_unfilter_from__Vec_int__int__int__int__int__ByteBuf_closure_fn};
static closure_int_closure_int_closure_int_ByteBuf mu_unfilter__Vec_int__int__int__int__ByteBuf_closure_fn(void* __env, mere_vec_int* mu_raw) {
  (void)__env;
  return mu_unfilter__Vec_int__int__int__int__ByteBuf(mu_raw);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_ByteBuf mu_unfilter__Vec_int__int__int__int__ByteBuf_as_value = {.env = NULL, .fn = mu_unfilter__Vec_int__int__int__int__ByteBuf_closure_fn};
static int mu_is_interlaced_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_is_interlaced(mu_data);
}
static const closure_Vec_int_bool mu_is_interlaced_as_value = {.env = NULL, .fn = mu_is_interlaced_closure_fn};
static long long mu_adam7_xstart_closure_fn(void* __env, long long mu_p_uq22) {
  (void)__env;
  return mu_adam7_xstart(mu_p_uq22);
}
static const closure_int_int mu_adam7_xstart_as_value = {.env = NULL, .fn = mu_adam7_xstart_closure_fn};
static long long mu_adam7_ystart_closure_fn(void* __env, long long mu_p_uq23) {
  (void)__env;
  return mu_adam7_ystart(mu_p_uq23);
}
static const closure_int_int mu_adam7_ystart_as_value = {.env = NULL, .fn = mu_adam7_ystart_closure_fn};
static long long mu_adam7_xstep_closure_fn(void* __env, long long mu_p_uq24) {
  (void)__env;
  return mu_adam7_xstep(mu_p_uq24);
}
static const closure_int_int mu_adam7_xstep_as_value = {.env = NULL, .fn = mu_adam7_xstep_closure_fn};
static long long mu_adam7_ystep_closure_fn(void* __env, long long mu_p_uq25) {
  (void)__env;
  return mu_adam7_ystep(mu_p_uq25);
}
static const closure_int_int mu_adam7_ystep_as_value = {.env = NULL, .fn = mu_adam7_ystep_closure_fn};
static closure_int_int mu_ceil_div_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_ceil_div(mu_a);
}
static long long mu_ceil_div_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_ceil_div__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_ceil_div_as_value = {.env = NULL, .fn = mu_ceil_div_closure_fn, .fn2 = mu_ceil_div_closure_fn2};
static closure_int_int mu_pass_width_closure_fn(void* __env, long long mu_p_uq26) {
  (void)__env;
  return mu_pass_width(mu_p_uq26);
}
static long long mu_pass_width_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_pass_width__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_pass_width_as_value = {.env = NULL, .fn = mu_pass_width_closure_fn, .fn2 = mu_pass_width_closure_fn2};
static closure_int_int mu_pass_height_closure_fn(void* __env, long long mu_p_uq27) {
  (void)__env;
  return mu_pass_height(mu_p_uq27);
}
static long long mu_pass_height_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_pass_height__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_pass_height_as_value = {.env = NULL, .fn = mu_pass_height_closure_fn, .fn2 = mu_pass_height_closure_fn2};
static closure_int_closure_int_closure_int_ByteBuf mu_deinterlace__Vec_int__int__int__int__ByteBuf_closure_fn(void* __env, mere_vec_int* mu_raw) {
  (void)__env;
  return mu_deinterlace__Vec_int__int__int__int__ByteBuf(mu_raw);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_ByteBuf mu_deinterlace__Vec_int__int__int__int__ByteBuf_as_value = {.env = NULL, .fn = mu_deinterlace__Vec_int__int__int__int__ByteBuf_closure_fn};
static mere_bytebuf* mu_decode__Vec_int__ByteBuf_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_decode__Vec_int__ByteBuf(mu_data);
}
static const closure_Vec_int_ByteBuf mu_decode__Vec_int__ByteBuf_as_value = {.env = NULL, .fn = mu_decode__Vec_int__ByteBuf_closure_fn};
static mere_vec_int* mu_rgb8_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_rgb8(mu_data);
}
static const closure_Vec_int_Vec_int mu_rgb8_as_value = {.env = NULL, .fn = mu_rgb8_closure_fn};
static mere_vec_int* mu_vec_of__v2_closure_fn(void* __env, list_int mu_xs) {
  (void)__env;
  return mu_vec_of__v2(mu_xs);
}
static const closure_list_int_Vec_int mu_vec_of__v2_as_value = {.env = NULL, .fn = mu_vec_of__v2_closure_fn};
static closure_int_int mu_crc_byte__v2_closure_fn(void* __env, long long mu_crc) {
  (void)__env;
  return mu_crc_byte__v2(mu_crc);
}
static long long mu_crc_byte__v2_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_crc_byte__v2__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_crc_byte__v2_as_value = {.env = NULL, .fn = mu_crc_byte__v2_closure_fn, .fn2 = mu_crc_byte__v2_closure_fn2};
static long long mu_crc32_vec__v2_closure_fn(void* __env, mere_vec_int* mu_bytes) {
  (void)__env;
  return mu_crc32_vec__v2(mu_bytes);
}
static const closure_Vec_int_int mu_crc32_vec__v2_as_value = {.env = NULL, .fn = mu_crc32_vec__v2_closure_fn};
static mere_vec_int* mu_bw_new__unit__Vec_int_closure_fn(void* __env, int mu__u) {
  (void)__env;
  return mu_bw_new__unit__Vec_int(mu__u);
}
static const closure_unit_Vec_int mu_bw_new__unit__Vec_int_as_value = {.env = NULL, .fn = mu_bw_new__unit__Vec_int_closure_fn};
static closure_Vec_int_closure_int_closure_int_unit mu_bw_bits_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_bw_bits(mu_s);
}
static const closure_Vec_int_closure_Vec_int_closure_int_closure_int_unit mu_bw_bits_as_value = {.env = NULL, .fn = mu_bw_bits_closure_fn};
static closure_Vec_int_unit mu_bw_flush_closure_fn(void* __env, mere_vec_int* mu_s) {
  (void)__env;
  return mu_bw_flush(mu_s);
}
static int mu_bw_flush_closure_fn2(void* __env, mere_vec_int* __a0, mere_vec_int* __a1) {
  (void)__env;
  return mu_bw_flush__direct(__a0, __a1);
}
static const closure_Vec_int_closure_Vec_int_unit mu_bw_flush_as_value = {.env = NULL, .fn = mu_bw_flush_closure_fn, .fn2 = mu_bw_flush_closure_fn2};
static closure_int_int mu_rev_bits_closure_fn(void* __env, long long mu_v) {
  (void)__env;
  return mu_rev_bits(mu_v);
}
static long long mu_rev_bits_closure_fn2(void* __env, long long __a0, long long __a1) {
  (void)__env;
  return mu_rev_bits__direct(__a0, __a1);
}
static const closure_int_closure_int_int mu_rev_bits_as_value = {.env = NULL, .fn = mu_rev_bits_closure_fn, .fn2 = mu_rev_bits_closure_fn2};
static tuple_int_int_int mu_len_encode_closure_fn(void* __env, long long mu_len) {
  (void)__env;
  return mu_len_encode(mu_len);
}
static const closure_int_tuple_int_int_int mu_len_encode_as_value = {.env = NULL, .fn = mu_len_encode_closure_fn};
static tuple_int_int_int mu_dist_encode_closure_fn(void* __env, long long mu_dist) {
  (void)__env;
  return mu_dist_encode(mu_dist);
}
static const closure_int_tuple_int_int_int mu_dist_encode_as_value = {.env = NULL, .fn = mu_dist_encode_closure_fn};
static closure_int_closure_int_tuple_int_int mu_find_match_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_find_match(mu_data);
}
static const closure_Vec_int_closure_int_closure_int_tuple_int_int mu_find_match_as_value = {.env = NULL, .fn = mu_find_match_closure_fn};
static closure_int_closure_int_Vec_int mu_build_lengths__Vec_int__int__int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_freq) {
  (void)__env;
  return mu_build_lengths__Vec_int__int__int__Vec_int(mu_freq);
}
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_build_lengths__Vec_int__int__int__Vec_int_as_value = {.env = NULL, .fn = mu_build_lengths__Vec_int__int__int__Vec_int_closure_fn};
static closure_int_closure_int_Vec_int mu_canon_codes__Vec_int__int__int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_lengths) {
  (void)__env;
  return mu_canon_codes__Vec_int__int__int__Vec_int(mu_lengths);
}
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_canon_codes__Vec_int__int__int__Vec_int_as_value = {.env = NULL, .fn = mu_canon_codes__Vec_int__int__int__Vec_int_closure_fn};
static closure_int_int mu_vmax_closure_fn(void* __env, mere_vec_int* mu_v) {
  (void)__env;
  return mu_vmax(mu_v);
}
static long long mu_vmax_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_vmax__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_int mu_vmax_as_value = {.env = NULL, .fn = mu_vmax_closure_fn, .fn2 = mu_vmax_closure_fn2};
static mere_vec_int* mu_tokenize__Vec_int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_tokenize__Vec_int__Vec_int(mu_data);
}
static const closure_Vec_int_Vec_int mu_tokenize__Vec_int__Vec_int_as_value = {.env = NULL, .fn = mu_tokenize__Vec_int__Vec_int_closure_fn};
static mere_vec_int* mu_deflate_dynamic__Vec_int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_deflate_dynamic__Vec_int__Vec_int(mu_data);
}
static const closure_Vec_int_Vec_int mu_deflate_dynamic__Vec_int__Vec_int_as_value = {.env = NULL, .fn = mu_deflate_dynamic__Vec_int__Vec_int_closure_fn};
static mere_vec_int* mu_deflate_stored__Vec_int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_deflate_stored__Vec_int__Vec_int(mu_data);
}
static const closure_Vec_int_Vec_int mu_deflate_stored__Vec_int__Vec_int_as_value = {.env = NULL, .fn = mu_deflate_stored__Vec_int__Vec_int_closure_fn};
static mere_vec_int* mu_deflate_body__Vec_int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_deflate_body__Vec_int__Vec_int(mu_data);
}
static const closure_Vec_int_Vec_int mu_deflate_body__Vec_int__Vec_int_as_value = {.env = NULL, .fn = mu_deflate_body__Vec_int__Vec_int_closure_fn};
static mere_vec_int* mu_deflate_gzip_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_deflate_gzip(mu_data);
}
static const closure_Vec_int_Vec_int mu_deflate_gzip_as_value = {.env = NULL, .fn = mu_deflate_gzip_closure_fn};
static closure_int_unit mu_bytes_push4be_closure_fn(void* __env, mere_vec_int* mu_out) {
  (void)__env;
  return mu_bytes_push4be(mu_out);
}
static int mu_bytes_push4be_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_bytes_push4be__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_unit mu_bytes_push4be_as_value = {.env = NULL, .fn = mu_bytes_push4be_closure_fn, .fn2 = mu_bytes_push4be_closure_fn2};
static closure_Vec_int_unit mu_push_all_closure_fn(void* __env, mere_vec_int* mu_out) {
  (void)__env;
  return mu_push_all(mu_out);
}
static int mu_push_all_closure_fn2(void* __env, mere_vec_int* __a0, mere_vec_int* __a1) {
  (void)__env;
  return mu_push_all__direct(__a0, __a1);
}
static const closure_Vec_int_closure_Vec_int_unit mu_push_all_as_value = {.env = NULL, .fn = mu_push_all_closure_fn, .fn2 = mu_push_all_closure_fn2};
static long long mu_adler32_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_adler32(mu_data);
}
static const closure_Vec_int_int mu_adler32_as_value = {.env = NULL, .fn = mu_adler32_closure_fn};
static closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit mu_chunk_closure_fn(void* __env, mere_vec_int* mu_out) {
  (void)__env;
  return mu_chunk(mu_out);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_closure_int_closure_Vec_int_unit mu_chunk_as_value = {.env = NULL, .fn = mu_chunk_closure_fn};
static closure_int_closure_int_int mu_paeth_e_closure_fn(void* __env, long long mu_a) {
  (void)__env;
  return mu_paeth_e(mu_a);
}
static const closure_int_closure_int_closure_int_int mu_paeth_e_as_value = {.env = NULL, .fn = mu_paeth_e_closure_fn};
static closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_filter_row_closure_fn(void* __env, long long mu_kind) {
  (void)__env;
  return mu_filter_row(mu_kind);
}
static const closure_int_closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_filter_row_as_value = {.env = NULL, .fn = mu_filter_row_closure_fn};
static long long mu_row_cost_closure_fn(void* __env, mere_vec_int* mu_row_uq60) {
  (void)__env;
  return mu_row_cost(mu_row_uq60);
}
static const closure_Vec_int_int mu_row_cost_as_value = {.env = NULL, .fn = mu_row_cost_closure_fn};
static closure_Vec_int_closure_int_Vec_int mu_best_filter__Vec_int__Vec_int__int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_row_uq63) {
  (void)__env;
  return mu_best_filter__Vec_int__Vec_int__int__Vec_int(mu_row_uq63);
}
static const closure_Vec_int_closure_Vec_int_closure_int_Vec_int mu_best_filter__Vec_int__Vec_int__int__Vec_int_as_value = {.env = NULL, .fn = mu_best_filter__Vec_int__Vec_int__int__Vec_int_closure_fn};
static closure_int_bool mu_all_grey_closure_fn(void* __env, mere_vec_int* mu_pixels) {
  (void)__env;
  return mu_all_grey(mu_pixels);
}
static int mu_all_grey_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_all_grey__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_bool mu_all_grey_as_value = {.env = NULL, .fn = mu_all_grey_closure_fn, .fn2 = mu_all_grey_closure_fn2};
static closure_int_Vec_int mu_collect_palette_closure_fn(void* __env, mere_vec_int* mu_pixels) {
  (void)__env;
  return mu_collect_palette(mu_pixels);
}
static mere_vec_int* mu_collect_palette_closure_fn2(void* __env, mere_vec_int* __a0, long long __a1) {
  (void)__env;
  return mu_collect_palette__direct(__a0, __a1);
}
static const closure_Vec_int_closure_int_Vec_int mu_collect_palette_as_value = {.env = NULL, .fn = mu_collect_palette_closure_fn, .fn2 = mu_collect_palette_closure_fn2};
static closure_int_closure_int_closure_int_int mu_palette_index_closure_fn(void* __env, mere_vec_int* mu_pal) {
  (void)__env;
  return mu_palette_index(mu_pal);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_int mu_palette_index_as_value = {.env = NULL, .fn = mu_palette_index_closure_fn};
static closure_int_closure_int_closure_int_Vec_int mu_encode_at_depth__Vec_int__int__int__int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_pixels) {
  (void)__env;
  return mu_encode_at_depth__Vec_int__int__int__int__Vec_int(mu_pixels);
}
static const closure_Vec_int_closure_int_closure_int_closure_int_Vec_int mu_encode_at_depth__Vec_int__int__int__int__Vec_int_as_value = {.env = NULL, .fn = mu_encode_at_depth__Vec_int__int__int__int__Vec_int_closure_fn};
static closure_int_closure_int_Vec_int mu_encode_rgb8__Vec_int__int__int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_pixels) {
  (void)__env;
  return mu_encode_rgb8__Vec_int__int__int__Vec_int(mu_pixels);
}
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_encode_rgb8__Vec_int__int__int__Vec_int_as_value = {.env = NULL, .fn = mu_encode_rgb8__Vec_int__int__int__Vec_int_closure_fn};
static closure_int_closure_int_Vec_int mu_encode16__Vec_int__int__int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_pixels) {
  (void)__env;
  return mu_encode16__Vec_int__int__int__Vec_int(mu_pixels);
}
static const closure_Vec_int_closure_int_closure_int_Vec_int mu_encode16__Vec_int__int__int__Vec_int_as_value = {.env = NULL, .fn = mu_encode16__Vec_int__int__int__Vec_int_closure_fn};
static const char* mu_colour_name_closure_fn(void* __env, long long mu_c) {
  (void)__env;
  return mu_colour_name(mu_c);
}
static const closure_int_str mu_colour_name_as_value = {.env = NULL, .fn = mu_colour_name_closure_fn};
static int mu_info_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_info(mu_data);
}
static const closure_Vec_int_unit mu_info_as_value = {.env = NULL, .fn = mu_info_closure_fn};
static mere_vec_int* mu_ppm_bytes_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_ppm_bytes(mu_data);
}
static const closure_Vec_int_Vec_int mu_ppm_bytes_as_value = {.env = NULL, .fn = mu_ppm_bytes_closure_fn};
static mere_vec_int* mu_read_ppm__Vec_int__Vec_int_closure_fn(void* __env, mere_vec_int* mu_data) {
  (void)__env;
  return mu_read_ppm__Vec_int__Vec_int(mu_data);
}
static const closure_Vec_int_Vec_int mu_read_ppm__Vec_int__Vec_int_as_value = {.env = NULL, .fn = mu_read_ppm__Vec_int__Vec_int_closure_fn};
static mere_vec_int* mu_load_closure_fn(void* __env, const char* mu_path) {
  (void)__env;
  return mu_load(mu_path);
}
static const closure_str_Vec_int mu_load_as_value = {.env = NULL, .fn = mu_load_closure_fn};
static closure_str_unit mu_encode_cmd_closure_fn(void* __env, const char* mu_path) {
  (void)__env;
  return mu_encode_cmd(mu_path);
}
static int mu_encode_cmd_closure_fn2(void* __env, const char* __a0, const char* __a1) {
  (void)__env;
  return mu_encode_cmd__direct(__a0, __a1);
}
static const closure_str_closure_str_unit mu_encode_cmd_as_value = {.env = NULL, .fn = mu_encode_cmd_closure_fn, .fn2 = mu_encode_cmd_closure_fn2};
static int mu___mere_user_main_closure_fn(void* __env, int mu__u) {
  (void)__env;
  return mu___mere_user_main(mu__u);
}
static const closure_unit_unit mu___mere_user_main_as_value = {.env = NULL, .fn = mu___mere_user_main_closure_fn};
static const char* mu_png_info_closure_fn(void* __env, mere_bytes* mu_data) {
  (void)__env;
  return mu_png_info(mu_data);
}
static const closure_bytes_str mu_png_info_as_value = {.env = NULL, .fn = mu_png_info_closure_fn};
static mere_bytes* mu_png_to_ppm_closure_fn(void* __env, mere_bytes* mu_data) {
  (void)__env;
  return mu_png_to_ppm(mu_data);
}
static const closure_bytes_bytes mu_png_to_ppm_as_value = {.env = NULL, .fn = mu_png_to_ppm_closure_fn};
static mere_bytes* mu_ppm_to_png_closure_fn(void* __env, mere_bytes* mu_data) {
  (void)__env;
  return mu_ppm_to_png(mu_data);
}
static const closure_bytes_bytes mu_ppm_to_png_as_value = {.env = NULL, .fn = mu_ppm_to_png_closure_fn};
__attribute__((noinline)) static const char* show_int(long long v) {
  char __b[24]; int __n = 0;
  unsigned long long __u = v < 0 ? (unsigned long long)(-(v + 1)) + 1ULL
                                 : (unsigned long long)v;
  do { __b[__n++] = (char)('0' + (__u % 10)); __u /= 10; } while (__u);
  if (v < 0) __b[__n++] = '-';
  char* __r = __lang_str_alloc(__lang_current_region, (size_t)__n);
  for (int __i = 0; __i < __n; __i++) __r[__i] = __b[__n - 1 - __i];
  return __r;
}

/* mere --lib manifest (libmpng)
 * exported:
 *   mere_libmpng_png_info  <- png_info
 *   mere_libmpng_png_to_ppm  <- png_to_ppm
 *   mere_libmpng_ppm_to_png  <- ppm_to_png
 * skipped (entry-file bindings not exported):
 *   (none)
 */

typedef struct { const unsigned char* ptr; long long len; } mere_buf;
typedef enum { MERE_OK = 0, MERE_FAIL = 1 } mere_status;

/* One-time runtime setup. Every wrapper calls this itself, so a host
   that never does is still correct; calling it early only moves the
   cost. Unlike a standalone program, a library takes NO process-global
   actions: no signal handler, no stdout buffering, no atexit -- those
   are the host's policy. The entry file's top-level bindings (and its
   trailing expression, if it has one) run once, inside this call. */
static void __mere_lib_init_body(void) {
  __lang_region_init(&__lang_default_region, 1 << 22);

  (void)(({ mu_mask32 = 4294967295LL; ({ mu_len_base = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 15LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 17LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 19LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 23LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 27LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 31LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 35LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 43LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 51LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 59LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 67LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 83LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 99LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 115LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 131LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 163LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 195LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 227LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 258LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__direct(__da0); }); ({ mu_len_extra = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__direct(__da0); }); ({ mu_dist_base = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 17LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 25LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 33LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 49LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 65LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 97LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 129LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 193LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 257LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 385LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 513LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 769LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1025LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1537LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2049LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3073LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4097LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6145LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8193LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12289LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 16385LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 24577LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__direct(__da0); }); ({ mu_dist_extra = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__direct(__da0); }); ({ mu_clen_order = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 16LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 17LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 18LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 14LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 15LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__direct(__da0); }); ({ mu_png_sig = ({ __auto_type __let_tmp_v = mere_vec_int_new(__lang_current_region); __auto_type mu_v = __let_tmp_v; ({ (void)(__lifted_go_uq12_134(mu_v, 0LL)); mu_v; }); }); ({ mu_mask32__v2 = 4294967295LL; ({ mu_g_len_base = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 15LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 17LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 19LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 23LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 27LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 31LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 35LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 43LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 51LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 59LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 67LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 83LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 99LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 115LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 131LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 163LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 195LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 227LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 258LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__v2__direct(__da0); }); ({ mu_g_len_extra = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__v2__direct(__da0); }); ({ mu_g_dist_base = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 17LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 25LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 33LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 49LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 65LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 97LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 129LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 193LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 257LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 385LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 513LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 769LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1025LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1537LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2049LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3073LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4097LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6145LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8193LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12289LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 16385LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 24577LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__v2__direct(__da0); }); ({ mu_g_dist_extra = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__v2__direct(__da0); }); ({ mu_cl_order = ({ __auto_type __da0 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 16LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 17LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 18LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 0LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 8LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 7LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 9LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 6LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 10LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 5LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 11LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 4LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 12LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 3LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 13LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 2LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 14LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 1LL, .f1 = ({ list_int_node* __p = (list_int_node*)__lang_region_alloc(__lang_current_region, sizeof(list_int_node)); __p->payload.Cons = ((tuple_int_list_int){.f0 = 15LL, .f1 = list_int__mk(0, &__mshared_list_int_t0)}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); })}); list_int__mk(1, __p); }); mu_vec_of__v2__direct(__da0); }); ({ mu_usage = (__sl_33.s); 0; }); }); }); }); }); }); }); }); }); }); }); }); }); }));
}
static pthread_mutex_t __mere_lib_lock = PTHREAD_MUTEX_INITIALIZER;
static int __mere_lib_live = 0;
void mere_lib_init(void) {
  pthread_mutex_lock(&__mere_lib_lock);
  if (!__mere_lib_live) { __mere_lib_init_body(); __mere_lib_live = 1; }
  pthread_mutex_unlock(&__mere_lib_lock);
}
void mere_lib_shutdown(void) {
  pthread_mutex_lock(&__mere_lib_lock);
  if (!__mere_lib_live) { pthread_mutex_unlock(&__mere_lib_lock); return; }
  __mere_lib_live = 0;
  __lang_region_stats_report();

  __lang_region_free(&__lang_default_region);
  pthread_mutex_unlock(&__mere_lib_lock);
}
/* Frees any pointer this library handed to the host. malloc and free
   stay in the same translation unit, so a host built with a different
   allocator is safe to combine with. */
void mere_lib_free(void* p) { free(p); }

/* Boundary marshalling. IN: str/bytes are copied out of the host's
   buffer into the region -- the host's memory is borrowed only for
   the duration of the call. OUT: str/bytes are malloc'd copies the
   HOST OWNS (free with mere_lib_free); a str copy carries one extra
   NUL past .len as a courtesy to C callers. Region pointers never
   cross the boundary in either direction. */
static const char* __mere_lib_str_in(mere_buf b) {
  size_t n = b.len > 0 ? (size_t)b.len : 0;
  char* s = __lang_str_alloc(__lang_current_region, n);
  if (b.ptr && n) memcpy(s, b.ptr, n);
  return s;
}
static void __mere_lib_str_out(const char* s, mere_buf* out) {
  size_t n = __lang_str_size(s);
  unsigned char* p = (unsigned char*)malloc(n + 1);
  if (!p) __lang_fail_impl("out of memory");
  memcpy(p, s, n); p[n] = 0;
  out->ptr = p; out->len = (long long)n;
}
static mere_bytes* __mere_lib_bytes_in(mere_buf b) {
  long long n = b.len > 0 ? b.len : 0;
  mere_bytes* x = __lang_bytes_alloc(n);
  if (b.ptr && n) memcpy(x->data, b.ptr, (size_t)n);
  return x;
}
static void __mere_lib_bytes_out(mere_bytes* b, mere_buf* out) {
  unsigned char* p = (unsigned char*)malloc((size_t)b->len + 1);
  if (!p) __lang_fail_impl("out of memory");
  memcpy(p, b->data, (size_t)b->len); p[b->len] = 0;
  out->ptr = p; out->len = b->len;
}

mere_status mere_libmpng_png_info(mere_buf __a0, mere_buf* out, mere_buf* err) {
  mere_lib_init();
  if (err) { err->ptr = NULL; err->len = 0; }
  jmp_buf __saved_jmp; int __saved_set = __lang_fail_jmpbuf_set;
  __lang_region* __saved_cur = __lang_current_region;
  int __saved_rsn = __lang_region_active_n;
  memcpy(__saved_jmp, __lang_fail_jmpbuf, sizeof(jmp_buf));
  __lang_fail_jmpbuf_set = 1;
  if (_setjmp(__lang_fail_jmpbuf) != 0) {
    __lang_region_unwind(__saved_rsn);
    __lang_current_region = __saved_cur;
    __lang_fail_jmpbuf_set = __saved_set;
    memcpy(__lang_fail_jmpbuf, __saved_jmp, sizeof(jmp_buf));
    if (err) {
      size_t __n = strlen(__lang_fail_msg);
      unsigned char* __p = (unsigned char*)malloc(__n + 1);
      if (__p) { memcpy(__p, __lang_fail_msg, __n + 1); err->ptr = __p; err->len = (long long)__n; }
    }
    return MERE_FAIL;
  }
  __lang_region* __call_region = __lang_region_block_acquire("lib call");
  __lang_current_region = __call_region;
  const char* __r = mu_png_info__direct(__mere_lib_bytes_in(__a0));
  if (out) __mere_lib_str_out(__r, out);
  __lang_current_region = __saved_cur;
  __lang_region_block_release(__call_region);
  __lang_fail_jmpbuf_set = __saved_set;
  memcpy(__lang_fail_jmpbuf, __saved_jmp, sizeof(jmp_buf));
  return MERE_OK;
}
mere_status mere_libmpng_png_to_ppm(mere_buf __a0, mere_buf* out, mere_buf* err) {
  mere_lib_init();
  if (err) { err->ptr = NULL; err->len = 0; }
  jmp_buf __saved_jmp; int __saved_set = __lang_fail_jmpbuf_set;
  __lang_region* __saved_cur = __lang_current_region;
  int __saved_rsn = __lang_region_active_n;
  memcpy(__saved_jmp, __lang_fail_jmpbuf, sizeof(jmp_buf));
  __lang_fail_jmpbuf_set = 1;
  if (_setjmp(__lang_fail_jmpbuf) != 0) {
    __lang_region_unwind(__saved_rsn);
    __lang_current_region = __saved_cur;
    __lang_fail_jmpbuf_set = __saved_set;
    memcpy(__lang_fail_jmpbuf, __saved_jmp, sizeof(jmp_buf));
    if (err) {
      size_t __n = strlen(__lang_fail_msg);
      unsigned char* __p = (unsigned char*)malloc(__n + 1);
      if (__p) { memcpy(__p, __lang_fail_msg, __n + 1); err->ptr = __p; err->len = (long long)__n; }
    }
    return MERE_FAIL;
  }
  __lang_region* __call_region = __lang_region_block_acquire("lib call");
  __lang_current_region = __call_region;
  mere_bytes* __r = mu_png_to_ppm__direct(__mere_lib_bytes_in(__a0));
  if (out) __mere_lib_bytes_out(__r, out);
  __lang_current_region = __saved_cur;
  __lang_region_block_release(__call_region);
  __lang_fail_jmpbuf_set = __saved_set;
  memcpy(__lang_fail_jmpbuf, __saved_jmp, sizeof(jmp_buf));
  return MERE_OK;
}
mere_status mere_libmpng_ppm_to_png(mere_buf __a0, mere_buf* out, mere_buf* err) {
  mere_lib_init();
  if (err) { err->ptr = NULL; err->len = 0; }
  jmp_buf __saved_jmp; int __saved_set = __lang_fail_jmpbuf_set;
  __lang_region* __saved_cur = __lang_current_region;
  int __saved_rsn = __lang_region_active_n;
  memcpy(__saved_jmp, __lang_fail_jmpbuf, sizeof(jmp_buf));
  __lang_fail_jmpbuf_set = 1;
  if (_setjmp(__lang_fail_jmpbuf) != 0) {
    __lang_region_unwind(__saved_rsn);
    __lang_current_region = __saved_cur;
    __lang_fail_jmpbuf_set = __saved_set;
    memcpy(__lang_fail_jmpbuf, __saved_jmp, sizeof(jmp_buf));
    if (err) {
      size_t __n = strlen(__lang_fail_msg);
      unsigned char* __p = (unsigned char*)malloc(__n + 1);
      if (__p) { memcpy(__p, __lang_fail_msg, __n + 1); err->ptr = __p; err->len = (long long)__n; }
    }
    return MERE_FAIL;
  }
  __lang_region* __call_region = __lang_region_block_acquire("lib call");
  __lang_current_region = __call_region;
  mere_bytes* __r = mu_ppm_to_png__direct(__mere_lib_bytes_in(__a0));
  if (out) __mere_lib_bytes_out(__r, out);
  __lang_current_region = __saved_cur;
  __lang_region_block_release(__call_region);
  __lang_fail_jmpbuf_set = __saved_set;
  memcpy(__lang_fail_jmpbuf, __saved_jmp, sizeof(jmp_buf));
  return MERE_OK;
}

