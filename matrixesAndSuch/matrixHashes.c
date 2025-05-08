#include"pch.h"
#include<matrix.h>
#include<vector.h>

uint64_t baseXtoBase10Vector(void* vector) {
    matrix* vectorRefrenced = vector;
    uint16_t power = vectorRefrenced->rows - 1;
    uint64_t result = 0;
    for (size_t iterator = 0; iterator < vectorRefrenced->rows; iterator++) {
        switch (vectorRefrenced->matrixVarType) {
        case 0:
            result += (uint64_t)(*(uint8_t*)matrixGetPointer(vectorRefrenced, (uint16_t)iterator, VECTOR_COLUMN_INDEX)) * (uint64_t)pow(vectorRefrenced->modulus, power);
            break;
        case 1:
            result += (uint64_t)(*(uint16_t*)matrixGetPointer(vectorRefrenced, (uint16_t)iterator, VECTOR_COLUMN_INDEX)) * (uint64_t)pow(vectorRefrenced->modulus, power);
            break;
        case 2:
            result += (uint64_t)(*(uint32_t*)matrixGetPointer(vectorRefrenced, (uint16_t)iterator, VECTOR_COLUMN_INDEX)) * (uint64_t)pow(vectorRefrenced->modulus, power);
            break;
        default:
            return 0;
        }
        power--;
    }
    return result;
}