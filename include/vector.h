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
#define VECTOR_COLUMN_INDEX 0

extern MATRIXES ArrayList* vectorSpann(matrix* vectors,size_t* maxMemAlloc);
extern MATRIXES bool vectorCheckIfLinar(matrix* matrixOfVectors);
extern MATRIXES bool vectorCheckIfBase(matrix* base, matrix* room);
extern MATRIXES bool vectorCheckIfRoom(matrix* room);
#endif