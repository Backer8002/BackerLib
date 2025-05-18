#include"pch.h"
#include<hashMap.h>
#include<matrix.h>
#include<vector.h>
#include<arrayList.h>

ArrayList* vectorSpann(matrix* vectors, size_t* maxMemAlloc) {
    matrix* counter = matrixCreate(vectors->columns,1,vectors->modulus,maxMemAlloc);
    if(counter == NULL) return NULL;
    ArrayList* arrayList = arrayListCreateMatrix(1);
    if(arrayList == NULL) {
        matrixFree(counter,maxMemAlloc);
        return NULL;
    }
    matrixVectorCounterReset(counter);
    size_t increment = 0;
    do {
        matrix* linjarCombination = matrixMultiply(vectors,counter,maxMemAlloc);
        if(linjarCombination == NULL) {
            matrixFree(counter,maxMemAlloc);
            arrayListDestroy(arrayList);
            return NULL;
        }
        arrayListElementSetMatrix(arrayList,increment,linjarCombination);
        increment++;
        matrixFree(linjarCombination,maxMemAlloc);
    } while (matrixVectorCounterIncrement(counter)==0);
    matrixFree(counter,maxMemAlloc);
    return arrayList;
}

int vectorCheckIfLinear(matrix* matrixOfVectors,size_t* maxMemAlloc)
{   
    ArrayList* matrixArrayList = vectorSpann(matrixOfVectors, maxMemAlloc);
    if (matrixArrayList == NULL) return -1;
    for (size_t iterator = 1; iterator < matrixArrayList->amountOfElements; iterator++) {
        if (matrixCompareEqualityZeroMatrix(arrayListElementGetMatrix(matrixArrayList, iterator))) { 
            arrayListDestroy(matrixArrayList); 
            return 1; 
        }
    }
    for (size_t iterator = 0; iterator < matrixArrayList->amountOfElements; iterator++)
        matrixFree(arrayListElementGetMatrix(matrixArrayList, iterator),maxMemAlloc);
    arrayListDestroy(matrixArrayList);
    return 0;
}

int vectorCheckIfLinearWithSpann(ArrayList* spannOfVectors)
{
    for (size_t iterator = 1; iterator < spannOfVectors->amountOfElements; iterator++) {
        if (matrixCompareEqualityZeroMatrix(arrayListElementGetMatrix(spannOfVectors, iterator))) { 
            arrayListDestroy(spannOfVectors); 
            return 1; 
        }
    }
    return 0;
}

int vectorCheckIfBase(matrix* base, matrix* room,size_t* maxMemAlloc)
{
    ArrayList* spannOfBase = vectorSpann(base, maxMemAlloc);
    if (spannOfBase == NULL) return -1;

    if (vectorCheckIfLinearWithSpann(spannOfBase)) { 
        for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfBase, iterator),maxMemAlloc);
        arrayListDestroy(spannOfBase); 
        return -2; 
    }

    Set* set = setCreateVector(base);
    if (set == NULL) { 
        for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfBase, iterator),maxMemAlloc);
        arrayListDestroy(spannOfBase); 
        return -1; 
    }
    Set* secondSet = setCreateVector(arrayListElementGetMatrix(spannOfBase, 0));
    if (secondSet == NULL) {
        for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfBase, iterator),maxMemAlloc);
        arrayListDestroy(spannOfBase);
        setDestroy(set);
        return -1;
    }

    hashVectorsInMatrix(set, room);
    for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
    {
        setAdd(secondSet, baseXtoBase10Vector(arrayListElementGetMatrix(spannOfBase, iterator),0));
    }
    for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
        matrixFree(arrayListElementGetMatrix(spannOfBase, iterator),maxMemAlloc);
    arrayListDestroy(spannOfBase);
    setAnd(set, secondSet);
    setDestroy(secondSet);
    if (setIsEmpty(set)) {
        setDestroy(set);
        return 1;
    }
    setDestroy(set);
    return 0;
}

matrix* createRoom(matrix* base,size_t* maxMemAlloc) {
    ArrayList* spannOfBase = vectorSpann(base, maxMemAlloc);
    if (spannOfBase == NULL) return NULL;

    if (vectorCheckIfLinearWithSpann(spannOfBase)) {
        for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfBase, iterator), maxMemAlloc);
        arrayListDestroy(spannOfBase);
        return NULL;
    }
    matrix* returnMatrix = matrixFromArrayList(spannOfBase, true, maxMemAlloc);
    if (returnMatrix == NULL) {
        for (size_t iterator = 0; iterator < spannOfBase->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfBase, iterator), maxMemAlloc);
        arrayListDestroy(spannOfBase);
        return NULL;
    }
    if (!vectorCheckIfRoom(returnMatrix, maxMemAlloc)) {
        matrixFree(returnMatrix, maxMemAlloc);
        return NULL;
    }
    return returnMatrix;
}

int vectorCheckIfRoom(matrix* room,size_t* maxMemAlloc)
{
    ArrayList* spannOfRoom = vectorSpann(room, maxMemAlloc);
    if (spannOfRoom == NULL) return -1;

    Set* set = setCreateVector(room);
    if (set == NULL) {
        for (size_t iterator = 0; iterator < spannOfRoom->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfRoom, iterator),maxMemAlloc);
        arrayListDestroy(spannOfRoom);
        return -1;
    }
    Set* secondSet = setCreateVector(arrayListElementGetMatrix(spannOfRoom, 0));
    if (secondSet == NULL) {
        for (size_t iterator = 0; iterator < spannOfRoom->amountOfElements; iterator++)
            matrixFree(arrayListElementGetMatrix(spannOfRoom, iterator),maxMemAlloc);
        arrayListDestroy(spannOfRoom);
        setDestroy(set);
        return -1;
    }

    hashVectorsInMatrix(set, room);
    for (size_t iterator = 0; iterator < spannOfRoom->amountOfElements; iterator++)
        setAdd(secondSet, baseXtoBase10Vector(arrayListElementGetMatrix(spannOfRoom, iterator), 0));

    for (size_t iterator = 0; iterator < spannOfRoom->amountOfElements; iterator++)
        matrixFree(arrayListElementGetMatrix(spannOfRoom, iterator),maxMemAlloc);

    arrayListDestroy(spannOfRoom);
    setAnd(set, secondSet);
    setDestroy(secondSet);
    if (setIsEmpty(set)) {
        setDestroy(set);
        return 1;
    }
    setDestroy(set);
    return 0;
}