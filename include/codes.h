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
#include<stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef enum {
		MatrixCodeNormal = 0,
		MatrixCodeLinear = 1,
		MatrixCodeFormNormal = 2,
		MatrixCodeHamming = 3,
		MatrixCodeReed = 4
	} MatrixCodeTypes;

	typedef struct {
		matrix* Generator;
		matrix* ControlMatrix;
		uint32_t Seperation;
		MatrixCodeTypes typeOfCode;
	} MatrixCode;


	extern CODES uint32_t computeHammingDistanceTwoVector(matrix* firstVector, uint16_t columnIndexOfFirstVector, matrix* secondVector, uint16_t columnIndexOfSecondVector);
	extern CODES uint32_t codeComputeSeparation(matrix* codewords, bool isLinear);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif