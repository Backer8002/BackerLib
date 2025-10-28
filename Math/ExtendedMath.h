#ifndef ExtMath_h_
#define ExtMath_h_

#include <stdint.h>

#ifdef __cplusplus
namespace BackerLib {
extern "C" {

#else
#define noexcept
#endif // __cplusplus

extern uint64_t factorial(uint32_t number) noexcept;
extern uint64_t partialFactorial(uint32_t startOfFactorial,uint32_t number) noexcept;
extern uint64_t nChooseK(uint32_t n, uint32_t k) noexcept;

#ifdef __cplusplus
}
};
#endif // __cplusplus

#endif
