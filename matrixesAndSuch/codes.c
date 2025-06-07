#include"pch.h"
#include<codes.h>
#include<matrix.h>
#include<vector.h>
#include<math.h>
#include<stdint.h>
#include<stddef.h>
#include<signal.h>

#define MATH_ERROR SIGSEGV

uint32_t computeHammingDistanceTwoVector(matrix* firstVector,uint16_t columnIndexOfFirstVector, matrix* secondVector,uint16_t columnIndexOfSecondVector) {
    uint32_t result = 0;
    if (firstVector->rows != secondVector->rows||firstVector->modulus!=secondVector->modulus) raise(MATH_ERROR);

    for (uint16_t vectorIterator = 0; vectorIterator < firstVector->rows; vectorIterator++) {
        switch (firstVector->matrixVarType) {
        case 0:
            result += (*(uint8_t*)matrixGetPointer(firstVector,vectorIterator,columnIndexOfFirstVector) != *(uint8_t*)matrixGetPointer(secondVector,vectorIterator,columnIndexOfSecondVector));
            break;
        case 1:
            result += (*(uint16_t*)matrixGetPointer(firstVector,vectorIterator,columnIndexOfFirstVector) != *(uint16_t*)matrixGetPointer(secondVector,vectorIterator,columnIndexOfSecondVector));
            break;
        case 2:
            result += (*(uint32_t*)matrixGetPointer(firstVector,vectorIterator,columnIndexOfFirstVector) != *(uint32_t*)matrixGetPointer(secondVector,vectorIterator,columnIndexOfSecondVector));
            break;
        case 3:
            result += (*(int32_t*)matrixGetPointer(firstVector,vectorIterator,columnIndexOfFirstVector) != *(int32_t*)matrixGetPointer(secondVector,vectorIterator,columnIndexOfSecondVector));
            break;
        default:
            break;
        }
    }
    return result;
}

uint32_t codeComputeSeparation(matrix* codewords, bool isLinear) {
    uint32_t seperation = UINT32_MAX;

    if (isLinear) {
        for (uint16_t columnIterator = 0; columnIterator < codewords->columns; columnIterator++) {
            uint32_t hammingComputeResult = 0;
            for (uint16_t vectorIterator = 0; vectorIterator < codewords->rows; vectorIterator++) {
                switch (codewords->matrixVarType) {
                case 0:
                    hammingComputeResult += *(uint8_t*) matrixGetPointer(codewords, vectorIterator, columnIterator) != 0;
                    break;
                case 1:
                    hammingComputeResult += *(uint16_t*) matrixGetPointer(codewords, vectorIterator, columnIterator) != 0;
                    break;
                case 2:
                    hammingComputeResult += *(uint32_t*) matrixGetPointer(codewords, vectorIterator, columnIterator) != 0;
                    break;
                case 3:
                    hammingComputeResult += *(int32_t*) matrixGetPointer(codewords, vectorIterator, columnIterator) != 0;
                default:
                    break;
                }
            }
            seperation = (seperation > hammingComputeResult) ? hammingComputeResult : seperation;
        }
    } else {
        for (uint16_t columnIterator = 0; columnIterator < codewords->columns; columnIterator++) {
            for (uint16_t secondColumnIterator = 0; secondColumnIterator < codewords->columns; secondColumnIterator++) {
                uint32_t hammingResult = computeHammingDistanceTwoVector(codewords, columnIterator, codewords, secondColumnIterator);
                seperation = (seperation > hammingResult) ? hammingResult : seperation;
            }
        }
    }
    return seperation;
}

MatrixCode codeMatrixCreate(matrix* generator,size_t* maxMemAlloc) {
    MatrixCode code = {generator,NULL,0,0};

    ArrayList* codewordsArray = arrayListCreateMatrix(pow(generator->modulus,generator->columns));
    if (codewordsArray == NULL)
        return code;
    matrix* counter = matrixCreate(generator->rows, 1, generator->modulus, maxMemAlloc);
    if (counter == NULL) {
        arrayListDestroy(codewordsArray);
        return code;
    }

    matrixVectorCounterReset(counter);
    do {
        matrix* multipliedMatrix = matrixMultiply(generator, counter, maxMemAlloc);
        if (multipliedMatrix == NULL) {
            for (size_t i = 0; i < codewordsArray->amountOfElements; i++)
                matrixFree(arrayListElementGet(codewordsArray, i),maxMemAlloc);
            arrayListDestroy(codewordsArray);
            matrixFree(counter, maxMemAlloc);
            return code;
        }
        arrayListElementSetMatrix(codewordsArray, codewordsArray->amountOfElements, multipliedMatrix);
    } while (matrixVectorCounterIncrement(counter));

    matrix* codewords = matrixFromArrayList(codewordsArray, true, maxMemAlloc);
    for (size_t i = 0; i < codewordsArray->amountOfElements; i++)
        matrixFree(arrayListElementGet(codewordsArray, i), maxMemAlloc);
    arrayListDestroy(codewordsArray);
    matrixFree(counter, maxMemAlloc);
    if (codewords == NULL)
        return code;

    if (vectorCheckIfLinear(codewords, maxMemAlloc)) {
        code.typeOfCode = MatrixCodeNormal;
        code.Seperation = codeComputeSeparation(codewords, false);
        matrixFree(codewords, maxMemAlloc);
        return code;
    }
    code.typeOfCode = MatrixCodeLinear;
    code.Seperation = codeComputeSeparation(codewords, true);
    
    if (!(generator->rows < generator->columns)) {
        matrixCompareEqualityUnitMatrix;
    }
}