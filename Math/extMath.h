#ifndef extMath_h_
#define extMath_h_
#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS 
#define EXTMATH __declspec(dllexport)
#else
#define EXTMATH __declspec(dllimport)
#endif
#else
#define EXTMATH
#endif
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <stdint.h>
	extern EXTMATH uint64_t factorial(uint32_t number);
	extern EXTMATH uint64_t partialFactorial(uint32_t startOfFactorial, uint32_t number);
	extern EXTMATH uint64_t nChooseK(uint32_t n, uint32_t k);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif
