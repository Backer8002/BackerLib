#include "ExtendedMath.h"
#include <stdint.h>

uint64_t bl_math_factorial(uint32_t number) {
    uint64_t result = 1;
    for (uint64_t iterator = 1; iterator <= number; iterator++)
        result *= iterator;
    return result;
}

uint64_t bl_math_factorial_partial(uint32_t startOfFactorial, uint32_t number) {
    uint64_t result = 1;
    if (startOfFactorial > number)
        return 0;
    for (uint64_t iterator = startOfFactorial ? startOfFactorial : 1; iterator <= number; iterator++)
        result *= iterator;
    return result;
}

uint64_t bl_math_permutation(uint32_t n, uint32_t k) {
    if (n == k)
        return 1;
    if (k == 0)
        return 1;
    if (k == 1)
        return n;
    return bl_math_factorial_partial(k + 1, n) / bl_math_factorial(n - k);
}
