#ifndef vector_h_
#define vector_h_

#ifdef _WINDOWS
#ifdef MATRIXESANDSUCH_EXPORTS 
#define MATRIXES __declspec(dllexport)
#else
#define MATRIXES __declspec(dllimport)
#endif
#else
#define MATRIXES
#endif

#include<matrix.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#define VECTOR_COLUMN_INDEX 0

	extern MATRIXES ArrayList* vectorSpann(matrix* vectors, size_t* maxMemAlloc);
	extern MATRIXES int vectorCheckIfLinear(matrix* matrixOfVectors, size_t* maxMemAlloc);
	extern MATRIXES int vectorCheckIfBase(matrix* base, matrix* room, size_t* maxMemAlloc);
	extern MATRIXES matrix* createRoom(matrix* base, size_t* maxMemAlloc);
	extern MATRIXES int vectorCheckIfRoom(matrix* room, size_t* maxMemAlloc);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif