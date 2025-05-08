#ifndef extMath_h_
#define extMath_h_
#ifdef _WINDOWS
#ifdef BASICFUNCTIONS_EXPORTS 
#define EXTMATH __declspec(dllexport)
#else
#define EXTMATH __declspec(dllimport)
#endif
#else
#define EXTMATH
#endif
extern EXTMATH uint64_t factorial(uint32_t number);
extern EXTMATH uint64_t partialFactorial(uint32_t startOfFactorial, uint32_t number);
extern EXTMATH uint64_t nChooseK(uint32_t n,uint32_t k);
#endif