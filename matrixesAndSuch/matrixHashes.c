#include"pch.h"
#include<matrix.h>
#include<stdint.h>
#include<math.h>
#include<hashMap.h>

uint64_t baseXtoBase10Vector(matrix* vector,uint16_t index) {
    uint16_t power = vector->rows - 1;
    uint64_t result = 0;
    for (uint16_t iterator = 0; iterator < vector->rows; iterator++) {
        switch (vector->matrixVarType) {
        case 0:
            result += (uint64_t)(*(uint8_t*)matrixGetPointer(vector, (uint16_t)iterator, index)) * (uint64_t)pow(vector->modulus, power);
            break;
        case 1:
            result += (uint64_t)(*(uint16_t*)matrixGetPointer(vector, (uint16_t)iterator, index)) * (uint64_t)pow(vector->modulus, power);
            break;
        case 2:
            result += (uint64_t)(*(uint32_t*)matrixGetPointer(vector, (uint16_t)iterator, index)) * (uint64_t)pow(vector->modulus, power);
            break;
        default:
            return 0;
        }
        power--;
    }
    return result;
}

bool hashVectorsInMatrix(BitSet* set, matrix* vectors) {
    bool alreadyIncludes = false;
    for (size_t iterator = 0; iterator < vectors->columns; iterator++)
        alreadyIncludes |= setAdd(set, baseXtoBase10Vector(vectors, iterator));
    return alreadyIncludes;
}