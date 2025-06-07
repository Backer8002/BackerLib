#include"pch.h"
#include<matrix.h>
#include<arrayList.h>
#include<stdbool.h>
#include<stdint.h>

//Will destroy arrayList
matrix* matrixFromArrayList(ArrayList* arrayListOfMatrix, bool isSameSize,size_t* maxMemAlloc) {
	size_t sizeInColumns = 0;
	uint16_t amountOfRow = arrayListElementGetMatrix(arrayListOfMatrix, 0)->rows;
	if (!isSameSize) {
		for (uint16_t iterator = 0; iterator < arrayListOfMatrix->amountOfElements; iterator++)
		{
			matrix* oneMatrix = arrayListElementGetMatrix(arrayListOfMatrix, iterator);
			sizeInColumns += oneMatrix->columns;
			if (oneMatrix->rows != amountOfRow) return NULL;
		}
	}
	else
	{
		sizeInColumns = arrayListOfMatrix->amountOfElements * arrayListElementGetMatrix(arrayListOfMatrix, 0)->rows;
	}
	
	matrix* returnMatrix = matrixCreate(amountOfRow, sizeInColumns, arrayListElementGetMatrix(arrayListOfMatrix, 0)->modulus, maxMemAlloc);
	if (returnMatrix == NULL)
		return NULL;

	uint16_t columnIterator = 0;
	for (size_t matrixIterator = 0; matrixIterator < arrayListOfMatrix->amountOfElements; matrixIterator++) {
		matrix* currentMatrix = arrayListElementGetMatrix(arrayListOfMatrix,matrixIterator);
		for (uint16_t currentMatrixColumnIterator = 0; currentMatrixColumnIterator < currentMatrix->columns; currentMatrixColumnIterator++) {
			for (uint16_t currentMatrixRowIterator = 0; currentMatrixRowIterator < currentMatrix->rows; currentMatrixRowIterator++) {
				switch (returnMatrix->matrixVarType)
				{
				case 0: 
					*(uint8_t*)matrixGetPointer(returnMatrix, currentMatrixRowIterator, columnIterator) = *(uint8_t*)matrixGetPointer(currentMatrix, currentMatrixRowIterator, currentMatrixColumnIterator);
					break;
				case 1:
					*(uint16_t*)matrixGetPointer(returnMatrix, currentMatrixRowIterator, columnIterator) = *(uint16_t*)matrixGetPointer(currentMatrix, currentMatrixRowIterator, currentMatrixColumnIterator);
					break;
				case 2:
					*(uint32_t*)matrixGetPointer(returnMatrix, currentMatrixRowIterator, columnIterator) = *(uint32_t*)matrixGetPointer(currentMatrix, currentMatrixRowIterator, currentMatrixColumnIterator);
					break;
				case 3:
					*(int32_t*)matrixGetPointer(returnMatrix, currentMatrixRowIterator, columnIterator) = *(int32_t*)matrixGetPointer(currentMatrix, currentMatrixRowIterator, currentMatrixColumnIterator);
					break;
				default:
					break;
				}
			}
			columnIterator++;
		}
		matrixFree(currentMatrix, maxMemAlloc);
	}
	arrayListDestroy(arrayListOfMatrix);
	return returnMatrix;
}
#include<perfectCodes.h>
ArrayList* ballToMatrixArrayList(ball* Ball) {
	ArrayList* matrixList = arrayListCreateMatrix(Ball->amountOfElementsInBall);
	if (matrixList == NULL) return NULL;

	for (size_t iterator = 0; iterator < Ball->amountOfElementsInBall; iterator++) {
		arrayListElementSetMatrix(matrixList, iterator, *(Ball->elements + iterator));
	}
	return matrixList;
}

BitSet* ballToSet(ball* Ball) {
	BitSet* set = setCreateVector(*(Ball->elements));
	if (set == NULL) return NULL;

	for (size_t iterator = 0; iterator < Ball->amountOfElementsInBall; iterator++) {
		bitSetAdd(set, baseXtoBase10Vector(*(Ball->elements + iterator),0));
	}
	return set;
}
