#include"pch.h"
#include<matrix.h>
#include<stdlib.h>
#include<stdio.h>

int matrixFillWithRandomValues(matrix* matrixToFill) {
    for (uint16_t rowIterator = 0;rowIterator < matrixToFill->rows;rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < matrixToFill->columns; columnIterator++)
        {
            switch (matrixToFill->matrixVarType)
            {
            case 0:
                *(uint8_t*)matrixGetPointer(matrixToFill,rowIterator,columnIterator) = (uint8_t)(rowIterator*columnIterator*matrixToFill->sizeOfMatrix)%matrixToFill->modulus;
                break;
            case 1:
                *(uint16_t*)matrixGetPointer(matrixToFill,rowIterator,columnIterator) = (uint16_t)(rowIterator*columnIterator*matrixToFill->sizeOfMatrix)%matrixToFill->modulus;
                break;
            case 2:
                *(uint32_t*)matrixGetPointer(matrixToFill,rowIterator,columnIterator) = (uint32_t)(rowIterator*columnIterator*matrixToFill->sizeOfMatrix)%matrixToFill->modulus;
                break;
            case 3:
                *(int32_t*)matrixGetPointer(matrixToFill,rowIterator,columnIterator) = (int32_t)(rowIterator*columnIterator*matrixToFill->sizeOfMatrix);
                break;
            default:
                return -1;
                break;
            }
        }
    }
    return 0;
}