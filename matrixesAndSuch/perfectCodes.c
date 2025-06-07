#include"pch.h"
#include<matrix.h>
#include<codes.h>
#include<stdint.h>
#include<extMath.h>
#include<math.h>
#include<hashMap.h>
#include<perfectCodes.h>
#include<stdlib.h>


uint32_t sizeOfBall(matrix* vector,uint32_t radius) {
    uint32_t result = 0;
    for (size_t sumIterator = 0; sumIterator <= radius; sumIterator++) {
        result += (uint32_t)nChooseK(vector->rows,sumIterator) * (uint32_t)pow(vector->modulus-1,sumIterator);
    }
    return result;
}

ball findBall(matrix* vector, uint32_t radius,uint64_t* maxMemAlloc) {
    ball Ball;
    Ball.amountOfElementsInBall = sizeOfBall(vector,radius);
    matrix** ballElements = malloc(sizeof(matrix*) * Ball.amountOfElementsInBall);
    if (ballElements == NULL) {
        Ball.amountOfElementsInBall = 0;
        return Ball;
    }
    Ball.elements = ballElements;
    matrix* counter = matrixCreate(vector->rows,1,vector->modulus,maxMemAlloc);
    if(counter == NULL) {
        Ball.amountOfElementsInBall = 0;
        free(ballElements);
        return Ball;
    }
    matrixVectorCounterReset(counter);
    for(uint32_t countOfElement = 0;countOfElement < Ball.amountOfElementsInBall;countOfElement++) {
        matrix* tempMatrix = matrixDeepCopy(counter,maxMemAlloc);
        if (tempMatrix == NULL) {
            matrixFree(counter,maxMemAlloc);
            for (size_t iterator = 0; iterator < countOfElement; iterator++) matrixFree(*(Ball.elements+iterator),maxMemAlloc);
            free(Ball.elements);
            Ball.amountOfElementsInBall = 0;
            return Ball;
        }
        *(Ball.elements + countOfElement) = tempMatrix;

        while (1)
        {
            if(computeHammingDistanceTwoVector(counter,0,vector,0) <= radius) break;
            int code = matrixVectorCounterIncrement(counter);
            if(code == -1) {
                Ball.amountOfElementsInBall = countOfElement+1;
                matrixFree(counter,maxMemAlloc);
                return Ball;
            }
        }
    }
    return Ball;
}