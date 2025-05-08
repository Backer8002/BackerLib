#include"pch.h"
#include<matrix.h>
#include<vector.h>
#include<hashMap.h>
#include<arrayList.h>

ArrayList* vectorSpann(matrix* vectors, size_t* maxMemAlloc) {
    matrix* counter = matrixCreate(vectors->columns,1,vectors->modulus,maxMemAlloc);
    if(counter == NULL) return NULL;
    ArrayList* arrayList = arrayListCreateMatrix(1);
    if(arrayList == NULL) {matrixFree(counter,maxMemAlloc);return NULL;}
    matrixVectorCounterReset(counter);
    size_t increment = 0;
    do {
        matrix* linjarCombination = matrixMultiply(vectors,counter,maxMemAlloc);
        if(linjarCombination == NULL) {matrixFree(counter,maxMemAlloc);arrayListDestroy(arrayList);return NULL;}
        arrayList->set(arrayList,increment,linjarCombination);
        increment++;
        matrixFree(linjarCombination,maxMemAlloc);
    } while (matrixVectorCounterIncrement(counter)==0);
    matrixFree(counter,maxMemAlloc);
    return arrayList;
}

bool vectorCheckIfLinar(matrix* matrixOfVectors)
{
    return false;
}

bool vectorCheckIfBase(matrix* base, matrix* room)
{
    return false;
}

bool vectorCheckIfRoom(matrix* room)
{
    return false;
}
