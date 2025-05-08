#ifndef matrix_h_
#define matrix_h_

#ifdef _WINDOWS
#ifdef MATRIXESANDSUCH_EXPORTS 
#define MATRIXES __declspec(dllexport)
#else
#define MATRIXES __declspec(dllimport)
#else
#define MATRIXES
#endif
#endif
    #include<stdbool.h>
    #include<stdint.h>
    #ifndef backerStrings_h_
    #include<backerStrings.h>
    #endif
    #include <stdio.h>
    MATRIXES typedef struct matrix
    {
        void** matrix;
        uint8_t matrixVarType;
        uint32_t modulus;
        uint64_t sizeOfMatrix;
        uint16_t rows;
        uint16_t columns;
    } matrix; 

    //matrixDef.c
    extern MATRIXES matrix* matrixCreate(uint16_t amountOfRows,uint16_t amountOfColumns,uint32_t modulus,uint64_t* maxMemAlloc);
    extern MATRIXES void matrixFree(matrix* matrix,uint64_t* maxMemAlloc);
    extern MATRIXES void* matrixGetPointer(matrix* matrix,uint16_t row, uint16_t column);
    extern MATRIXES String** matrixToStringUint8(matrix* matrix);
    extern MATRIXES int matrixCompareEquality(matrix* firstMatrix, matrix* otherMatrix);
    extern MATRIXES matrix* matrixDeepCopy(matrix* copyMatrix, uint64_t* maxMemAlloc);
    extern MATRIXES int matrixVectorCounterIncrement(matrix* counter);
    extern MATRIXES int matrixVectorCounterReset(matrix* counter);
    extern MATRIXES bool isWithinModulus(matrix* matrix, uint32_t value);

    //matrixMath.c
    extern MATRIXES int matrixAdd(matrix* firstMatrix, matrix* secondMatrix);
    extern MATRIXES int matrixSub(matrix* firstMatrix, matrix* secondMatrix);
    extern MATRIXES int matrixScalarMultiply(matrix* matrix, int64_t scalar);
    extern MATRIXES int matrixInvertRows(matrix* matrix);
    extern MATRIXES int matrixInvertColumns(matrix* matrix);
    extern MATRIXES matrix* matrixMultiply(matrix* firstMatrix, matrix* secondMatrix, uint64_t* maxMemAlloc);
    extern MATRIXES matrix* matrixTranspose(matrix* matrixToTranspose,uint64_t* maxMemAlloc);
    extern MATRIXES matrix* matrixFindInverse(matrix* matrixInput,size_t* maxMemAlloc);

    //matrixIO.c
    #define charsPerUInt8 3 // 3 for values
    #define charsPerUInt16 5 // 5 for values
    #define charsPerUInt32 10 // 10 for values
    #define charsPerInt32 11 // 1 for -, 10 for values
    #define newLineAndNullPoint 2

    extern MATRIXES void matrixOutput(matrix* matrix,FILE* stream);
    extern MATRIXES matrix* textFileScanMatrix(String** fileLocation,String** fileName, uint16_t amountOfRow, uint16_t amountOfColumn, uint32_t modulus, uint64_t* maxMemAlloc);
    extern MATRIXES int convertIntoElementFromString(matrix* matrix,uint16_t row, uint16_t column, char* string);
    extern MATRIXES matrix* matrixBasicFileRead(String** fileLocation,String** fileName,uint64_t* maxMemAlloc);
    extern MATRIXES int matrixBasicFileWrite(String** fileLocation, String** fileName, matrix* matrix);
    //matrix* fileScanMatrix(String fileLocation,String fileName,uint64_t* maxMemAlloc);
    //int matrixFileWrite(String fileLocation, String fileName, matrix* matrix);
    
    //matrixDebug.c
    extern MATRIXES int matrixFillWithRandomValues(matrix* matrixToFill);

    #include <arrayList.h>
    #include <hashMap.h>
    extern MATRIXES ArrayList* arrayListCreateMatrix(size_t initialSize);
    extern MATRIXES Set* SetCreateVector(matrix* oneMatrixFromDimension);

    extern MATRIXES uint64_t baseXtoBase10Vector(void* vector);
#endif