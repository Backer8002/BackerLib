#ifndef ExtMath_h_
#define ExtMath_h_

#include <stdint.h>

#ifdef __cplusplus
namespace BackerLib {
extern "C" {

#else
#define noexcept
#endif // __cplusplus

extern uint64_t bl_math_factorial(uint32_t number) noexcept;
extern uint64_t bl_math_factorial_partial(uint32_t startOfFactorial,uint32_t number) noexcept;
extern uint64_t bl_math_permutation(uint32_t n, uint32_t k) noexcept;

#ifdef __cplusplus
}
};
#endif // __cplusplus

#endif
