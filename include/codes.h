#ifndef codes_h_
#define codes_h_

#ifdef _WINDOWS
#ifdef MATRIXESANDSUCH_EXPORTS 
#define CODES __declspec(dllexport)
#else
#define CODES __declspec(dllimport)
#endif
#else
#define CODES
#endif
#include <matrix.h>
#include<stdint.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	extern CODES uint32_t computeHammingDistanceTwoVector(matrix* firstVector, matrix* secondVector);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif