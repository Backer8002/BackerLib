#include"pch.h"
#include<codes.h>
#include<matrix.h>
#include<vector.h>
#include<stdint.h>
#include<stddef.h>
#include<signal.h>

#define MATH_ERROR SIGSEGV

uint32_t computeHammingDistanceTwoVector(matrix* firstVector, matrix* secondVector) {
    uint32_t result = 0;
    if (firstVector->rows != secondVector->rows||firstVector->modulus!=secondVector->modulus) raise(MATH_ERROR);

    for (uint16_t vectorIterator = 0; vectorIterator < firstVector->rows; vectorIterator++) {
        matrixOutput(firstVector,stdout);
        matrixOutput(secondVector,stdout);
        switch (firstVector->matrixVarType)
        {
        case 0:
            result += (*(uint8_t*)matrixGetPointer(firstVector,vectorIterator,VECTOR_COLUMN_INDEX) != *(uint8_t*)matrixGetPointer(secondVector,vectorIterator,VECTOR_COLUMN_INDEX));
            break;
        case 1:
            result += (*(uint16_t*)matrixGetPointer(firstVector,vectorIterator,VECTOR_COLUMN_INDEX) != *(uint16_t*)matrixGetPointer(secondVector,vectorIterator,VECTOR_COLUMN_INDEX));
            break;
        case 2:
            result += (*(uint32_t*)matrixGetPointer(firstVector,vectorIterator,VECTOR_COLUMN_INDEX) != *(uint32_t*)matrixGetPointer(secondVector,vectorIterator,VECTOR_COLUMN_INDEX));
            break;
        case 3:
            result += (*(int32_t*)matrixGetPointer(firstVector,vectorIterator,VECTOR_COLUMN_INDEX) != *(int32_t*)matrixGetPointer(secondVector,vectorIterator,VECTOR_COLUMN_INDEX));
            break;
        default:
            break;
        }
    }
    return result;
}

uint32_t codeComputeSeparation(matrix* codewords, bool isLinear) {
    return 0;
}