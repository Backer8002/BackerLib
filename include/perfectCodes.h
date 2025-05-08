#ifndef perfectCodes_h_
#define perfectCodes_h_

#ifdef _WINDOWS
#ifdef MATRIXESANDSUCH_EXPORTS 
#define CODES __declspec(dllexport)
#else
#define CODES __declspec(dllimport)
#endif
#else
#define CODES
#endif
#include <stdint.h>
#include <matrix.h>
CODES typedef struct ball{
    uint32_t amountOfElementsInBall;
    matrix** elements;
} ball;

extern CODES uint32_t sizeOfBall(matrix* vector, uint32_t radius);
extern CODES ball findBall(matrix* vector, uint32_t radius, uint64_t* maxMemAlloc);
#endif