# rat64_t
This is a small C++ library for a 64-bit rational number type.
It's a rather niche use case, but I think it will be helpful, and it's fun to implement the math.

When you have a word-size union which includes a rational number (e.g. mpq_class* from GMP),
it can be an advantage to also have a word-size rational number. This way
dynamic allocation can be avoided until necessary. Most operations must be checked
to determine if they overflow/underflow, in which case they will return true.

If I find myself using this class quite a bit, I will probably also create a rat32_t for 32-bit systems.
