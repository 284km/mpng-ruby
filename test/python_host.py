#!/usr/bin/env python3
"""The second host.

The Ruby extension is a polite client: it always passes valid buffers, never
calls shutdown early, never lies about a length. A boundary tested only by a
polite client keeps whatever bugs politeness cannot reach -- this harness
exists to be the other kind. It drove one real fix: calling an export after
mere_lib_shutdown was a use-after-free that answered correctly in every
polite test (mere v0.1.312 made the lifecycle re-armable).

usage: python3 test/python_host.py <libmpng.so> <test.png> <expected.ppm>
"""
import ctypes
import sys
import threading


class MereBuf(ctypes.Structure):
    _fields_ = [("ptr", ctypes.POINTER(ctypes.c_ubyte)), ("len", ctypes.c_longlong)]


def main() -> int:
    so_path, png_path, exp_path = sys.argv[1:4]
    lib = ctypes.CDLL(so_path)
    for name in ("mere_libmpng_png_info", "mere_libmpng_png_to_ppm",
                 "mere_libmpng_ppm_to_png"):
        fn = getattr(lib, name)
        fn.restype = ctypes.c_int
        fn.argtypes = [MereBuf, ctypes.POINTER(MereBuf), ctypes.POINTER(MereBuf)]
    lib.mere_lib_free.argtypes = [ctypes.c_void_p]
    lib.mere_lib_free.restype = None
    lib.mere_lib_shutdown.restype = None

    def buf(b: bytes) -> MereBuf:
        arr = (ctypes.c_ubyte * max(len(b), 1)).from_buffer_copy(b or b"\0")
        return MereBuf(ctypes.cast(arr, ctypes.POINTER(ctypes.c_ubyte)), len(b))

    def take(out: MereBuf) -> bytes:
        data = ctypes.string_at(out.ptr, out.len)
        lib.mere_lib_free(ctypes.cast(out.ptr, ctypes.c_void_p))
        return data

    png = open(png_path, "rb").read()
    expected = open(exp_path, "rb").read()

    # polite: the answer must match the computed expectation
    out, err = MereBuf(), MereBuf()
    assert lib.mere_libmpng_png_to_ppm(buf(png), ctypes.byref(out), ctypes.byref(err)) == 0
    assert take(out) == expected, "python host decode differs from expectation"

    # impolite: NULL out and NULL err are both allowed
    assert lib.mere_libmpng_png_to_ppm(buf(png), None, None) == 0

    # impolite: empty input with a NULL pointer fails, with words
    assert lib.mere_libmpng_png_info(MereBuf(None, 0), ctypes.byref(out), ctypes.byref(err)) == 1
    assert b"not a PNG" in take(err)

    # impolite: a negative length is an empty input, not a crash
    assert lib.mere_libmpng_png_info(MereBuf(None, -5), ctypes.byref(out), ctypes.byref(err)) == 1
    lib.mere_lib_free(ctypes.cast(err.ptr, ctypes.c_void_p))

    # abusive: shutdown early, then keep calling -- the library re-initializes
    lib.mere_lib_shutdown()
    assert lib.mere_libmpng_png_to_ppm(buf(png), ctypes.byref(out), ctypes.byref(err)) == 0
    assert take(out) == expected, "decode after shutdown differs"
    lib.mere_lib_shutdown()
    lib.mere_lib_shutdown()  # idempotent

    # concurrent: ctypes releases the GIL during the call, so these are real
    # threads. libmpng has module state (mgz's lookup tables are top-level
    # Vecs), and since mere v0.1.582 such a library refuses a call that
    # overlaps another thread's instead of letting them share that state.
    # Its contract: one thread at a time, or a lock around the calls.
    busy = b"called while another thread is inside this library"

    def run(threads_n: int, worker) -> list:
        bad: list = []
        threads = [threading.Thread(target=worker, args=(bad,)) for _ in range(threads_n)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()
        return bad

    # with a lock, as the refusal tells the host to: every call answers right
    lock = threading.Lock()

    def locked(bad: list) -> None:
        o, e = MereBuf(), MereBuf()
        for _ in range(200):
            with lock:
                rc = lib.mere_libmpng_png_to_ppm(buf(png), ctypes.byref(o), ctypes.byref(e))
                data = take(o) if rc == 0 else take(e)
            if rc != 0 or data != expected:
                bad.append(data[:120])
                return

    bad = run(8, locked)
    assert not bad, f"locked concurrent decode differs: {bad[0]!r}"

    # without a lock: a call either answers right or is refused, in words --
    # never a wrong answer
    refused = []

    def unlocked(bad: list) -> None:
        o, e = MereBuf(), MereBuf()
        for _ in range(200):
            if lib.mere_libmpng_png_to_ppm(buf(png), ctypes.byref(o), ctypes.byref(e)) != 0:
                msg = take(e)
                if busy not in msg:
                    bad.append(msg[:120])
                    return
                refused.append(1)
            elif take(o) != expected:
                bad.append(b"wrong answer")
                return

    bad = run(8, unlocked)
    assert not bad, f"unlocked concurrent decode: {bad[0]!r}"

    print(f"python host: ok (polite, impolite, lifecycle abuse, 8 threads locked; "
          f"unlocked refused {len(refused)} of 1600)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
