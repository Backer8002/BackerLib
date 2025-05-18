#include"pch.h"
#include<stdint.h>
#include<stdlib.h>
#include<stdbool.h>
#include<matrix.h>
#include<backerStrings.h>
#include<assert.h>

matrix* matrixCreate(uint16_t amountOfRow,uint16_t amountOfColumn,uint32_t modulus,uint64_t* maxMemAlloc) {

    uint8_t matrixVarType;
    uint64_t neededMemory;
    uint64_t sizeOfRow;
    uint64_t sizeOfRowPointers;

    if (modulus == 0) return NULL;
    else if (modulus == 1) {
        matrixVarType = 3;
        neededMemory = amountOfRow * amountOfColumn * sizeof(int32_t) + sizeof(int32_t*) * amountOfRow;
        sizeOfRowPointers = amountOfRow*sizeof(int32_t*);
        sizeOfRow = amountOfColumn*sizeof(int32_t);
    }
    else if (modulus <= 256) {
        matrixVarType = 0;
        neededMemory = amountOfRow * amountOfColumn * sizeof(uint8_t) + sizeof(uint8_t*) * amountOfRow;
        sizeOfRowPointers = amountOfRow*sizeof(uint8_t*);
        sizeOfRow = amountOfColumn*sizeof(uint8_t);
    }
    else if (modulus <=65536) {
        matrixVarType = 1;
        neededMemory = amountOfRow * amountOfColumn * sizeof(uint16_t) + sizeof(uint16_t*) * amountOfRow;
        sizeOfRowPointers = amountOfRow*sizeof(uint16_t*);
        sizeOfRow = amountOfColumn*sizeof(uint16_t);
    }
    else {
        matrixVarType = 2;
        neededMemory = amountOfRow * amountOfColumn * sizeof(uint32_t) + sizeof(uint32_t*) * amountOfRow;
        sizeOfRowPointers = amountOfRow*sizeof(uint32_t*);
        sizeOfRow = amountOfColumn*sizeof(uint32_t);
    }

    if(neededMemory > *maxMemAlloc) return NULL;
    matrix* newMatrix = malloc(sizeof(matrix));
    if(newMatrix == NULL) return NULL;
    newMatrix->rows = amountOfRow;
    newMatrix->columns = amountOfColumn;
    newMatrix->modulus = modulus;
    newMatrix->matrixVarType = matrixVarType;
    newMatrix->sizeOfMatrix = neededMemory;

    void** rows = (void**)malloc(sizeOfRowPointers);
    if (rows == NULL) {
        free(newMatrix);
        return NULL;
    }

    uint16_t iterator;
    for(iterator = 0; iterator < amountOfRow;iterator++) {
        *(rows+iterator) = malloc(sizeOfRow);
        if (*(rows+iterator) == NULL) goto deAllocPartialMatrix;
    }

    newMatrix->matrix = rows;
    *maxMemAlloc = *maxMemAlloc - neededMemory;
    return newMatrix;

    deAllocPartialMatrix:
    
    for(uint16_t newIterator = 0; newIterator < iterator; newIterator++) {
        free(*(rows+newIterator));
    }
    free(rows);
    free(newMatrix);
    return NULL;
}

void matrixFree(matrix* matrix, uint64_t* maxMemAlloc) {
    for (uint16_t iterator = 0; iterator<matrix->rows;iterator++) {
        free(*(matrix->matrix+iterator));
    }
    free(matrix->matrix);
    *maxMemAlloc = *maxMemAlloc + matrix->sizeOfMatrix;
    free(matrix);
    return;
}
void* matrixGetPointer(matrix* matrix,uint16_t rows, uint16_t columns) {
    assert(rows < matrix->rows || columns < matrix->columns);
    switch (matrix->matrixVarType)
    {
        case 0:
            return *((uint8_t**)matrix->matrix+rows) + columns;
            break;
        case 1:
            return *((uint16_t**)matrix->matrix+rows) + columns;
            break;
        case 2:
            return *((uint32_t**)matrix->matrix+rows) + columns;
            break;
        case 3:
            return *((int32_t**)matrix->matrix+rows) + columns;
            break;
        default:
            return NULL;
    }
}
String matrixToStringUint8(matrix* matrix) {
    switch (matrix->matrixVarType)
    {
    case 0:
        char itoaBuff[charsPerUInt8+2];
        int code = 0;
        String string = stringCreate("[",1);
        if (string == NULL) return NULL;
        for (uint16_t row = 0;row<matrix->rows;row++) {
            
            code |= stringAdd(string,"[",1);

            for (uint16_t column = 0;column<matrix->columns;column++) {
                int amountOfChars = _snprintf_s(itoaBuff,5,5,"%hhu",*(uint8_t*)matrixGetPointer(matrix,row,column));
                code |= stringAdd(string,itoaBuff,amountOfChars);
                if(column < matrix->columns-1) code |= stringAdd(string,",",1);
                if(code!=0) return NULL;
            }
            if (row < matrix->rows-1) code |= stringAdd(string,"],\n",3);
            else code |= stringAdd(string,"]]\n\0",4);
        }
        if (code < 0) {
            stringDestroy(string);
            return NULL;
        }
        return string;
        break;
    default:
        return NULL;
        break;
    }
    
}
int matrixCompareEquality(matrix* firstMatrix, matrix* otherMatrix) {
    if(!(firstMatrix->columns == otherMatrix->columns && firstMatrix->rows == otherMatrix->rows)) return -2;
    
    for (uint16_t rowIterator = 0;rowIterator <firstMatrix->rows;rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < firstMatrix->columns; columnIterator++)
        {
            switch (firstMatrix->matrixVarType)
            {
            case 0:
                if (*(uint8_t*)matrixGetPointer(firstMatrix,rowIterator,columnIterator) != *(uint8_t*)matrixGetPointer(otherMatrix,rowIterator,columnIterator)) return -3;
                break;
            case 1:
                if (*(uint16_t*)matrixGetPointer(firstMatrix,rowIterator,columnIterator) != *(uint16_t*)matrixGetPointer(otherMatrix,rowIterator,columnIterator)) return -3;
                break;
            case 2:
                if (*(uint32_t*)matrixGetPointer(firstMatrix,rowIterator,columnIterator) != *(uint32_t*)matrixGetPointer(otherMatrix,rowIterator,columnIterator)) return -3;
                break;
            case 3:
                if (*(int32_t*)matrixGetPointer(firstMatrix,rowIterator,columnIterator) != *(int32_t*)matrixGetPointer(otherMatrix,rowIterator,columnIterator)) return -3;
                break;
            default:
                return -1;
                break;
            }
        }
    }
    return 0;
}

matrix* matrixDeepCopy(matrix* copyMatrix,uint64_t* maxMemAlloc) {
    matrix* pasteMatrix = matrixCreate(copyMatrix->rows,copyMatrix->columns,copyMatrix->modulus,maxMemAlloc);
    if(pasteMatrix == NULL) return NULL;

    for (uint16_t rowIterator = 0; rowIterator < copyMatrix->rows; rowIterator++) {
        for (uint16_t columnIterator = 0; columnIterator < copyMatrix->columns; columnIterator++) {
            switch (copyMatrix->matrixVarType) {
                case 0:
                    *(uint8_t*)matrixGetPointer(pasteMatrix,rowIterator,columnIterator) = *(uint8_t*)matrixGetPointer(copyMatrix,rowIterator,columnIterator);
                    break;
                case 1:
                    *(uint16_t*)matrixGetPointer(pasteMatrix,rowIterator,columnIterator) = *(uint16_t*)matrixGetPointer(copyMatrix,rowIterator,columnIterator);
                    break;
                case 2:
                    *(uint32_t*)matrixGetPointer(pasteMatrix,rowIterator,columnIterator) = *(uint32_t*)matrixGetPointer(copyMatrix,rowIterator,columnIterator);
                    break;
                case 3:
                    *(int32_t*)matrixGetPointer(pasteMatrix,rowIterator,columnIterator) = *(int32_t*)matrixGetPointer(copyMatrix,rowIterator,columnIterator);
                    break;
                default:
                    return NULL;
                    break;
            }
        }
    }
    return pasteMatrix;
}

int matrixVectorCounterIncrement(matrix* counter) {
    for (uint16_t iteratorCounter = counter->rows-1; iteratorCounter < counter->rows;iteratorCounter--) {
        switch (counter->matrixVarType)
        {
        case 0:
            uint8_t* pointerUInt8 = matrixGetPointer(counter,iteratorCounter,0);
            if (*pointerUInt8 >= counter->modulus-1) *pointerUInt8 = 0;
            else {
                (*pointerUInt8)++;
                return 0;
                }
            break;
        case 1:
            uint16_t* pointerUInt16 = matrixGetPointer(counter,iteratorCounter,0);
            if (*pointerUInt16 >= counter->modulus-1) *pointerUInt16 = 0;
            else {
                (*pointerUInt16)++;
                return 0;
            }
            break;
        case 2:
            uint32_t* pointerUInt32 = matrixGetPointer(counter,iteratorCounter,0);
            if (*pointerUInt32 >= counter->modulus-1) *pointerUInt32 = 0;
            else {
                (*pointerUInt32)++;
                return 0;
            }
            break;
        }
        if(iteratorCounter == 0) {
            return -1;
        }
    }
    return 0;
}

int matrixVectorCounterReset(matrix* counter) {
    for(uint16_t iterator = 0; iterator < counter->rows;iterator++) {
        switch (counter->matrixVarType)
        {
        case 0:
            *((uint8_t*)matrixGetPointer(counter,iterator,0)) = 0;
            break;
        case 1:
            *((uint16_t*)matrixGetPointer(counter,iterator,0)) = 0;
            break;
        case 2:
            *((uint32_t*)matrixGetPointer(counter,iterator,0)) = 0;
            break;
        default:
            return -1;
            break;
        }
    }
    return 0;
}

bool matrixCompareEqualityZeroMatrix(matrix* matrix) {
    for (uint16_t rowIterator = 0; rowIterator < matrix->rows; rowIterator++)
        for (uint16_t columnIterator = 0; columnIterator < matrix->columns; columnIterator++)
            switch (matrix->matrixVarType)
            {
            case 0:
                if (*(uint8_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != 0) return false;
                break;
            case 1:
                if (*(uint16_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != 0) return false;
                break;
            case 2:
                if (*(uint32_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != 0) return false;
                break;
            case 3:
                if (*(int32_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != 0) return false;
                break;
            default:
                break;
            };
    return true;
}

bool matrixCompareEqualityUnitMatrix(matrix* matrix) {
    for (uint16_t rowIterator = 0; rowIterator < matrix->rows; rowIterator++)
        for (uint16_t columnIterator = 0; columnIterator < matrix->columns; columnIterator++)
            switch (matrix->matrixVarType)
            {
            case 0:
                if (*(uint8_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != (columnIterator == rowIterator) ? 1u : 0u) return false;
                break;
            case 1:
                if (*(uint16_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != (columnIterator == rowIterator) ? 1u : 0u) return false;
                break;
            case 2:
                if (*(uint32_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != (columnIterator == rowIterator) ? 1u : 0u) return false;
                break;
            case 3:
                if (*(int32_t*)matrixGetPointer(matrix, rowIterator, columnIterator) != (columnIterator == rowIterator) ? 1 : 0) return false;
                break;
            default:
                break;
            };
            
    return true;
}

bool isWithinModulus(matrix* matrix, uint32_t value) {
    return matrix->modulus > value;
}