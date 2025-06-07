#include "pch.h"
#include <arrayList.h>
#include <matrix.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
inline int arrayListElementSetMatrix(ArrayList* arrayList,size_t index, matrix* value) {
    return arrayListElementSet(arrayList, index, value);
}

inline matrix* arrayListElementGetMatrix(ArrayList* arrayList,size_t index) {
    return arrayListElementGet(arrayList, index);
}

inline ArrayList* arrayListCreateMatrix(size_t initialSize) {
    return arrayListCreate(initialSize,sizeof(matrix*),ListMatrix,true);
}

BitSet* setCreateVector(matrix* oneMatrixFromDimension) {
    BitSet* set = malloc(sizeof(BitSet));
    if (set == NULL) return NULL;
    uint64_t arraySize = ceil(pow(oneMatrixFromDimension->modulus, oneMatrixFromDimension->rows) / 64);
    uint64_t* setArray = calloc(arraySize, sizeof(uint64_t));
    if (setArray == NULL) {
        free(set);
        return NULL;
    }

    set->array                   = setArray;
    set->header.dataArrayVarType = ListMatrix;
    set->sizeOfArray             = arraySize;
    set->header.flags            = ObjectFlagIsOnHeap;
    return set;
}