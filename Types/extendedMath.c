#include <extMath.h>
#include <stdint.h>

uint64_t factorial(uint32_t number) {
    uint64_t result = 1;
    for (uint64_t iterator = 1; iterator <= number; iterator++)
        result *= iterator;
    return result;
}

uint64_t partialFactorial(uint32_t startOfFactorial, uint32_t number) {
    uint64_t result = 1;
    if (startOfFactorial > number)
        return 0;
    for (uint64_t iterator = startOfFactorial; iterator <= number; iterator++)
        result *= iterator;
    return result;
}

uint64_t nChooseK(uint32_t n, uint32_t k) {
    if (n == k)
        return 1;
    if (k == 0)
        return 1;
    if (k == 1)
        return n;
    return partialFactorial(k + 1, n) / factorial(n - k);
}