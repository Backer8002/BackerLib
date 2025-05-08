#include "pch.h"
#include <arrayList.h>
#include <matrix.h>
#include <stdlib.h>
#include <assert.h>
int setMatrixElement(ArrayList* arrayList,size_t index, void* value) {
    assert(index <= arrayList->amountOfElements);
    if(index == arrayList->amountOfElements) { 
        if (arrayListSizeCheckAdd(arrayList) == -1) return -1;
    }
    *((matrix**)arrayList->list+index) = value;
    if (index == arrayList->amountOfElements) arrayList->amountOfElements++;
    return 0;
}

void* getMatrixElement(ArrayList* arrayList,size_t index) {
    assert(index < arrayList->amountOfElements);
    return *((matrix**)arrayList->list+index);
}

ArrayList* arrayListCreateMatrix(size_t initialSize) {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    if(arrayList == NULL) return NULL;
    matrix** matrixList = calloc(initialSize,sizeof(matrix*));
    if(matrixList == NULL) {
        free(arrayList);
        return NULL;
    }
    
    arrayList->listType = Matrix;
    arrayList->add = arrayListGenericAddElement;
    arrayList->get = getMatrixElement;
    arrayList->pop = arrayListGenericPopElement;
    arrayList->set = setMatrixElement;
    arrayList->clear = arrayListGenericClearElements;
    arrayList->remove = arrayListGenericRemoveElement;
    arrayList->elementSize = sizeof(matrix*);
    arrayList->amountOfElements = initialSize;
    arrayList->totalAmountOfElements = initialSize;
    arrayList->list = matrixList;
    return arrayList;
}

Set* SetCreateVector(matrix* oneMatrixFromDimension) {
    Set* set = malloc(sizeof(Set));
    if (set == NULL) return NULL;
    uint64_t arraySize = ceil(pow(oneMatrixFromDimension->modulus, oneMatrixFromDimension->rows) / 64);
    uint64_t* setArray = calloc(arraySize, sizeof(uint64_t));
    if (setArray == NULL) {
        free(set);
        return NULL;
    }

    set->array = setArray;
    set->theHashOfTheSet = (uint64_t)pow(oneMatrixFromDimension->modulus, oneMatrixFromDimension->rows);
    set->typeOfHash = Vector;
    set->sizeOfArray = arraySize;
    set->hashFunction = baseXtoBase10Vector;
    set->equalityChecker = setCompare;
    return set;
}