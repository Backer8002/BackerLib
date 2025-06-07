#include"pch.h"
#include<matrix.h>
#include<vector.h>
#include<codes.h>
#include<hashMap.h>
#include<stdint.h>

int matrixAdd(matrix* firstMatrix, matrix* secondMatrix) {
    if(firstMatrix->rows != secondMatrix->rows || firstMatrix->columns != secondMatrix->columns || firstMatrix->modulus != secondMatrix->modulus) return -2;

    for (uint16_t rowIterator = 0;rowIterator < firstMatrix->rows;rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < firstMatrix->columns; columnIterator++)
        {
            switch (firstMatrix->matrixVarType)
            {
            case 0:
                uint8_t* firstMatrixPointerUInt8 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerUInt8 = (*firstMatrixPointerUInt8+*(uint8_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator))%firstMatrix->modulus;
                break;
            case 1:
                uint16_t* firstMatrixPointerUInt16 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerUInt16 = (*firstMatrixPointerUInt16+*(uint16_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator))%firstMatrix->modulus;
                break;
            case 2:
                uint32_t* firstMatrixPointerUInt32 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerUInt32 = (*firstMatrixPointerUInt32+*(uint32_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator))%firstMatrix->modulus;
                break;
            case 3:
                int32_t* firstMatrixPointerInt32 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerInt32 = (*firstMatrixPointerInt32+*(int32_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator));
                break;
            default:
                return -3;
                break;
            }
        }
    }
    return 0;
}

int matrixSub(matrix* firstMatrix, matrix* secondMatrix) {
    if(firstMatrix->rows != secondMatrix->rows || firstMatrix->columns != secondMatrix->columns || firstMatrix->modulus != secondMatrix->modulus) return -2;

    for (uint16_t rowIterator = 0;rowIterator < firstMatrix->rows;rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < firstMatrix->columns; columnIterator++)
        {
            switch (firstMatrix->matrixVarType)
            {
            case 0:
                uint8_t* firstMatrixPointerUInt8 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerUInt8 = (*firstMatrixPointerUInt8-*(uint8_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator))%firstMatrix->modulus;
                break;
            case 1:
                uint16_t* firstMatrixPointerUInt16 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerUInt16 = (*firstMatrixPointerUInt16-*(uint16_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator))%firstMatrix->modulus;
                break;
            case 2:
                uint32_t* firstMatrixPointerUInt32 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerUInt32 = (*firstMatrixPointerUInt32-*(uint32_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator))%firstMatrix->modulus;
                break;
            case 3:
                int32_t* firstMatrixPointerInt32 = matrixGetPointer(firstMatrix,rowIterator,columnIterator);
                *firstMatrixPointerInt32 = (*firstMatrixPointerInt32-*(int32_t*)matrixGetPointer(secondMatrix,rowIterator,columnIterator));
                break;
            default:
                return -3;
                break;
            }
        }
    }
    return 0;
}

int matrixScalarMultiply(matrix* matrix, int64_t scalar) {
    for (uint16_t rowIterator = 0;rowIterator < matrix->rows;rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < matrix->columns; columnIterator++)
        {
            switch (matrix->matrixVarType)
            {
            case 0:
                uint8_t* matrixPointerUint8 = matrixGetPointer(matrix,rowIterator,columnIterator);
                *matrixPointerUint8 = (uint8_t)(*matrixPointerUint8*scalar)%matrix->modulus;
                break;
            case 1:
                uint16_t* matrixPointerUint16 = matrixGetPointer(matrix,rowIterator,columnIterator);
                *matrixPointerUint16 = (uint16_t)(*matrixPointerUint16*scalar)%matrix->modulus;
                break;
            case 2:
                uint32_t* matrixPointerUint32 = matrixGetPointer(matrix,rowIterator,columnIterator);
                *matrixPointerUint32 = (uint32_t)(*matrixPointerUint32*scalar)%matrix->modulus;
                break;
            case 3:
                int32_t* matrixPointerInt32 = matrixGetPointer(matrix,rowIterator,columnIterator);
                *matrixPointerInt32 = (int32_t)(*matrixPointerInt32*scalar);
                break;
            default:
                return -3;
                break;
            }
        }
    }
    return 0;   
}

int matrixInvertRows(matrix* matrix) {
    uint8_t tempValueUInt8;
    uint16_t tempValueUInt16;
    uint32_t tempValueUInt32;
    int32_t tempValueInt32;
    uint16_t maxColumnIterator;
    uint16_t minColumnIterator;
    for (uint16_t rowIterator = 0; rowIterator < matrix->rows; rowIterator++) {
        maxColumnIterator = matrix->columns-1;
        minColumnIterator = 0;
        while (minColumnIterator < maxColumnIterator)
        {
            switch (matrix->matrixVarType)
            {
            case 0:
                uint8_t* minPointerUInt8 = matrixGetPointer(matrix,rowIterator,minColumnIterator);
                uint8_t* maxPointerUInt8 = matrixGetPointer(matrix,rowIterator,maxColumnIterator);
                tempValueUInt8 = *minPointerUInt8;
                *minPointerUInt8 = *maxPointerUInt8;
                *maxPointerUInt8 = tempValueUInt8;
                break;
            case 1:
                uint16_t* minPointerUInt16 = matrixGetPointer(matrix,rowIterator,minColumnIterator);
                uint16_t* maxPointerUInt16 = matrixGetPointer(matrix,rowIterator,maxColumnIterator);
                tempValueUInt16 = *minPointerUInt16;
                *minPointerUInt16 = *maxPointerUInt16;
                *maxPointerUInt16 = tempValueUInt16;
                break;
            case 2:
                uint32_t* minPointerUInt32 = matrixGetPointer(matrix,rowIterator,minColumnIterator);
                uint32_t* maxPointerUInt32 = matrixGetPointer(matrix,rowIterator,maxColumnIterator);
                tempValueUInt32 = *minPointerUInt32;
                *minPointerUInt32 = *maxPointerUInt32;
                *maxPointerUInt32 = tempValueUInt32;
                break;
            case 3:
                int32_t* minPointerInt32 = matrixGetPointer(matrix,rowIterator,minColumnIterator);
                int32_t* maxPointerInt32 = matrixGetPointer(matrix,rowIterator,maxColumnIterator);
                tempValueInt32 = *minPointerInt32;
                *minPointerInt32 = *maxPointerInt32;
                *maxPointerInt32 = tempValueInt32;
                break;
            default:
                return -3;
                break;
            }
            maxColumnIterator--;
            minColumnIterator++;
        }
        
    }
    return 0;
}

int matrixInvertColumns(matrix* matrix) {
    uint8_t tempValueUInt8;
    uint16_t tempValueUInt16;
    uint32_t tempValueUInt32;
    int32_t tempValueInt32;
    uint16_t maxRowIterator;
    uint16_t minRowIterator;
    for (uint16_t columnIterator = 0; columnIterator < matrix->rows; columnIterator++) {
        maxRowIterator = matrix->columns-1;
        minRowIterator = 0;
        while (minRowIterator < maxRowIterator)
        {
            switch (matrix->matrixVarType)
            {
            case 0:
                uint8_t* minPointerUInt8 = matrixGetPointer(matrix,minRowIterator,columnIterator);
                uint8_t* maxPointerUInt8 = matrixGetPointer(matrix,maxRowIterator,columnIterator);
                tempValueUInt8 = *minPointerUInt8;
                *minPointerUInt8 = *maxPointerUInt8;
                *maxPointerUInt8 = tempValueUInt8;
                break;
            case 1:
                uint16_t* minPointerUInt16 = matrixGetPointer(matrix,minRowIterator,columnIterator);
                uint16_t* maxPointerUInt16 = matrixGetPointer(matrix,maxRowIterator,columnIterator);
                tempValueUInt16 = *minPointerUInt16;
                *minPointerUInt16 = *maxPointerUInt16;
                *maxPointerUInt16 = tempValueUInt16;
                break;
            case 2:
                uint32_t* minPointerUInt32 = matrixGetPointer(matrix,minRowIterator,columnIterator);
                uint32_t* maxPointerUInt32 = matrixGetPointer(matrix,maxRowIterator,columnIterator);
                tempValueUInt32 = *minPointerUInt32;
                *minPointerUInt32 = *maxPointerUInt32;
                *maxPointerUInt32 = tempValueUInt32;
                break;
            case 3:
                int32_t* minPointerInt32 = matrixGetPointer(matrix,minRowIterator,columnIterator);
                int32_t* maxPointerInt32 = matrixGetPointer(matrix,maxRowIterator,columnIterator);
                tempValueInt32 = *minPointerInt32;
                *minPointerInt32 = *maxPointerInt32;
                *maxPointerInt32 = tempValueInt32;
                break;
            default:
                return -3;
                break;
            }
            maxRowIterator--;
            minRowIterator++;
        }
        
    }
    return 0;
}

matrix* matrixMultiply(matrix* firstMatrix, matrix* secondMatrix, uint64_t* maxMemAlloc) {
    if (firstMatrix->columns != secondMatrix->rows || firstMatrix->modulus != secondMatrix->modulus) return NULL;

    matrix* resultMatrix = matrixCreate(firstMatrix->rows,secondMatrix->columns,firstMatrix->modulus,maxMemAlloc);
    if(resultMatrix == NULL) return NULL;

    size_t result;
    for (uint16_t firstMatrixRowIterator = 0; firstMatrixRowIterator < firstMatrix->rows; firstMatrixRowIterator++) {
        for (uint16_t secondMatrixColumnIterator = 0; secondMatrixColumnIterator < secondMatrix->columns; secondMatrixColumnIterator++) {
            result = 0;
            for (uint16_t secondMatrixRowIterator = 0; secondMatrixRowIterator < secondMatrix->rows; secondMatrixRowIterator++) {
                switch (firstMatrix->matrixVarType)
                {
                case 0:
                    result += *(uint8_t*)matrixGetPointer(firstMatrix,firstMatrixRowIterator,secondMatrixRowIterator)**(uint8_t*)matrixGetPointer(secondMatrix,secondMatrixRowIterator,secondMatrixColumnIterator);
                    break;
                case 1:
                    result += *(uint16_t*)matrixGetPointer(firstMatrix,firstMatrixRowIterator,secondMatrixRowIterator)**(uint16_t*)matrixGetPointer(secondMatrix,secondMatrixRowIterator,secondMatrixColumnIterator);
                    break;
                case 2:
                    result += *(uint32_t*)matrixGetPointer(firstMatrix,firstMatrixRowIterator,secondMatrixRowIterator)**(uint32_t*)matrixGetPointer(secondMatrix,secondMatrixRowIterator,secondMatrixColumnIterator);
                    break;
                case 3:
                    result += *(int32_t*)matrixGetPointer(firstMatrix,firstMatrixRowIterator,secondMatrixRowIterator)**(int32_t*)matrixGetPointer(secondMatrix,secondMatrixRowIterator,secondMatrixColumnIterator);
                    break;
                default:
                    matrixFree(resultMatrix,maxMemAlloc);
                    return NULL;
                    break;
                }
            }
            switch (firstMatrix->matrixVarType)
            {
            case 0:
                *(uint8_t*)matrixGetPointer(resultMatrix,firstMatrixRowIterator,secondMatrixColumnIterator) = (uint8_t)(result % firstMatrix->modulus);
                break;
            case 1:
                *(uint16_t*)matrixGetPointer(resultMatrix,firstMatrixRowIterator,secondMatrixColumnIterator) = (uint16_t)(result % firstMatrix->modulus);
                break;
            case 2:
                *(uint32_t*)matrixGetPointer(resultMatrix,firstMatrixRowIterator,secondMatrixColumnIterator) = (uint32_t)(result % firstMatrix->modulus);
                break;
            case 3:
                *(int32_t*)matrixGetPointer(resultMatrix,firstMatrixRowIterator,secondMatrixColumnIterator) = (int32_t)result;
                break;
            default:
                matrixFree(resultMatrix,maxMemAlloc);
                return NULL;
                break;
            }
        }
    }
    return resultMatrix;
}

matrix* matrixTranspose(matrix* matrixToTranspose,uint64_t* maxMemAlloc) {
    matrix* transposedMatrix = matrixCreate(matrixToTranspose->columns,matrixToTranspose->rows,matrixToTranspose->modulus,maxMemAlloc);
    if (transposedMatrix == NULL) return NULL;
    for (uint16_t rowIterator = 0; rowIterator < matrixToTranspose->rows; rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < matrixToTranspose->columns; columnIterator++) {
            switch (matrixToTranspose->matrixVarType)
            {
            case 0:
                *(uint8_t*)matrixGetPointer(transposedMatrix,columnIterator,rowIterator) = *(uint8_t*)matrixGetPointer(matrixToTranspose,rowIterator,columnIterator);
                break;
            case 1:
                *(uint16_t*)matrixGetPointer(transposedMatrix,columnIterator,rowIterator) = *(uint16_t*)matrixGetPointer(matrixToTranspose,rowIterator,columnIterator);
                break;
            case 2:
                *(uint32_t*)matrixGetPointer(transposedMatrix,columnIterator,rowIterator) = *(uint32_t*)matrixGetPointer(matrixToTranspose,rowIterator,columnIterator);
                break;
            case 3:
                *(int32_t*)matrixGetPointer(transposedMatrix,columnIterator,rowIterator) = *(int32_t*)matrixGetPointer(matrixToTranspose,rowIterator,columnIterator);
                break;
            
            default:
                matrixFree(transposedMatrix,maxMemAlloc);
                return NULL;
            }
        }
    }
    return transposedMatrix;
}

matrix* matrixFindInverse(matrix* matrixInput,size_t* maxMemAlloc) {
    if(matrixInput->matrixVarType == 3) return NULL;
    matrix* returnMatrix = matrixCreate(matrixInput->columns,matrixInput->rows,matrixInput->modulus,maxMemAlloc);
    if (returnMatrix == NULL) {
        return NULL;
    }
    matrix* counter = matrixCreate(matrixInput->columns,1,matrixInput->modulus,maxMemAlloc);
    if(counter == NULL) {
        matrixFree(returnMatrix,maxMemAlloc);
        return NULL;
    }
    matrix* vectorOfUnityMatrix = matrixCreate(matrixInput->rows,1,matrixInput->modulus,maxMemAlloc);
    if(vectorOfUnityMatrix == NULL) {
        matrixFree(counter,maxMemAlloc);
        matrixFree(returnMatrix,maxMemAlloc);
        return NULL;
    }


    for(uint16_t iteration = 0;iteration < matrixInput->columns;iteration++) {
        matrixVectorCounterReset(counter);
        for(uint16_t iterator = 0; iterator < counter->rows;iterator++) {
            switch (vectorOfUnityMatrix->matrixVarType)
            {
            case 0:
                uint8_t* pointerUInt8 = matrixGetPointer(vectorOfUnityMatrix,iterator,VECTOR_COLUMN_INDEX);
                *pointerUInt8 = (iteration==iterator);
                break;
            case 1:
                uint16_t* pointerUInt16 = matrixGetPointer(vectorOfUnityMatrix,iterator,VECTOR_COLUMN_INDEX);
                *pointerUInt16 = (iteration==iterator);
                break;
            case 2:
                uint32_t* pointerUInt32 = matrixGetPointer(vectorOfUnityMatrix,iterator,VECTOR_COLUMN_INDEX);
                *pointerUInt32 = (iteration==iterator);
                break;
            default:
                goto errorExit;
                break;
            }
        }
        while (1)
        {
            int code = matrixVectorCounterIncrement(counter);
            if (code == -1) goto errorExit;
            matrix* multiplyedMatrix = matrixMultiply(matrixInput,counter,maxMemAlloc);
            if (multiplyedMatrix == NULL) goto errorExit;
            if(computeHammingDistanceTwoVector(multiplyedMatrix,0,vectorOfUnityMatrix,0) == 0) {matrixFree(multiplyedMatrix,maxMemAlloc);break;}
            matrixFree(multiplyedMatrix,maxMemAlloc);
        }
        for(uint16_t rowIterator = 0; rowIterator < counter->rows;rowIterator++) {
            switch (counter->matrixVarType)
            {
            case 0:
                *((uint8_t*)matrixGetPointer(returnMatrix,rowIterator,iteration)) = *((uint8_t*)matrixGetPointer(counter,rowIterator,VECTOR_COLUMN_INDEX));
                break;
            case 1:
                *((uint16_t*)matrixGetPointer(returnMatrix,rowIterator,iteration)) = *((uint16_t*)matrixGetPointer(counter,rowIterator,VECTOR_COLUMN_INDEX));
                break;
            case 2:
                *((uint32_t*)matrixGetPointer(returnMatrix,rowIterator,iteration)) = *((uint32_t*)matrixGetPointer(counter,rowIterator,VECTOR_COLUMN_INDEX));
                break;
            default:
                break;
            }
        }
    }
    matrixFree(vectorOfUnityMatrix,maxMemAlloc);
    matrixFree(counter,maxMemAlloc);
    return returnMatrix;

    errorExit:
    matrixFree(vectorOfUnityMatrix,maxMemAlloc);
    matrixFree(counter,maxMemAlloc);
    matrixFree(returnMatrix,maxMemAlloc);
    return NULL;
}