#include"pch.h"
#include<matrix.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<ctype.h>
#include<math.h>
#include<string.h>
#include<backerStrings.h>

void matrixOutput(matrix* matrix,FILE* stream) {
    uint8_t charsPerInt;
    switch (matrix->matrixVarType)
    {
    case 0:
        charsPerInt = charsPerUInt8+2; 
        break;
    case 1:
        charsPerInt = charsPerUInt16+2; 
        break;
    case 2:
        charsPerInt = charsPerUInt32+2; 
        break;
    case 3: 
        charsPerInt = charsPerInt32+2; 
        break;
    default:
        return;
    }
    char* itoaBuff = malloc(charsPerInt);
    if(itoaBuff == NULL) return;
    int code = 0;
    int amountOfChars = 0;
    fwrite("[",1,1,stream);
    for(uint16_t row = 0; row < matrix->rows;row++) {
        String outputString = stringCreate("[",1);
        if (outputString.list == NULL) goto exit;

        switch (matrix->matrixVarType)
        {
        case 0:
            for (uint16_t column = 0;column < matrix->columns;column++) {
                amountOfChars = _snprintf_s(itoaBuff,charsPerInt,charsPerInt,"%hhu",*(uint8_t*)matrixGetPointer(matrix,row,column));
                code |= stringAdd(&outputString,itoaBuff,amountOfChars);
                if (column < matrix->columns-1) code|= stringAdd(&outputString,",",1);
            }
            break;
        case 1:
            for (uint16_t column = 0;column < matrix->columns;column++) {
                amountOfChars = _snprintf_s(itoaBuff,charsPerInt,charsPerInt,"%hu",*(uint16_t*)matrixGetPointer(matrix,row,column));
                code |= stringAdd(&outputString,itoaBuff,amountOfChars);
                if (column < matrix->columns-1) code|= stringAdd(&outputString,",",1);
            }
            break;
        case 2:
            for (uint16_t column = 0;column < matrix->columns;column++) {
                amountOfChars = _snprintf_s(itoaBuff,charsPerInt,charsPerInt,"%u",*(uint32_t*)matrixGetPointer(matrix,row,column));
                code |= stringAdd(&outputString,itoaBuff,amountOfChars);
                if (column < matrix->columns-1) code|= stringAdd(&outputString,",",1);
            }
            break;
        case 3:
            for (uint16_t column = 0;column < matrix->columns;column++) {
                amountOfChars = _snprintf_s(itoaBuff,charsPerInt,charsPerInt,"%d",*(int32_t*)matrixGetPointer(matrix,row,column));
                code |= stringAdd(&outputString,itoaBuff,amountOfChars);
                if (column < matrix->columns-1) code|= stringAdd(&outputString,",",1);
            }
            break;
        default:
            arrayListDestroy(&outputString);
            goto exit;
        }
        if(code == 0) fwrite(outputString.list,sizeof(char),outputString.amountOfElements,stream);
        if (row < matrix->rows-1) fwrite("],\n",sizeof(char),3,stream);
        arrayListDestroy(&outputString);
    }
    fwrite("]]\n",sizeof(char),3,stream);
    exit:
    free(itoaBuff);
    return;
}

matrix* textFileScanMatrix(FILE* stream, uint16_t amountOfRow, uint16_t amountOfColumn, uint32_t modulus, uint64_t* maxMemAlloc) {
    matrix* newMatrix = matrixCreate(amountOfRow,amountOfColumn,modulus,maxMemAlloc);
    if (newMatrix == NULL) {
        return NULL;
    }
    uint64_t maxLineSize = 0;

    char* numberBuff = NULL;
    switch (newMatrix->matrixVarType)
    {
    case 0:
        maxLineSize = (charsPerUInt8+1)*amountOfColumn + newLineAndNullPoint;
        numberBuff = (char*)malloc(charsPerUInt8+1);
        break;
    case 1:
        maxLineSize = (charsPerUInt16+1)*amountOfColumn + newLineAndNullPoint;
        numberBuff = (char*)malloc(charsPerUInt16+1);
        break;
    case 2:
        maxLineSize = (charsPerUInt32+1)*amountOfColumn + newLineAndNullPoint;
        numberBuff = (char*)malloc(charsPerUInt32+1);
        break;
    case 3:
        maxLineSize = (charsPerInt32+1)*amountOfColumn + newLineAndNullPoint;
        numberBuff = (char*)malloc(charsPerInt32+1);
        break;
    default:
        matrixFree(newMatrix,maxMemAlloc);
        return NULL;
    }
    if(numberBuff == NULL) {
        matrixFree(newMatrix, maxMemAlloc);
        return NULL;
    }
    char* lineBuffer = (char*)malloc(maxLineSize);
    if (lineBuffer == NULL) {
        matrixFree(newMatrix, maxMemAlloc);
        free(numberBuff);
        return NULL;
    }

    uint16_t currentColumn = 0;
    uint16_t elementIterator = 0;
    uint32_t lineIterator = 0;
    for (uint16_t rowIterator = 0; rowIterator<amountOfRow;rowIterator++) {
        currentColumn = 0;
        elementIterator = 0;
        char* fileReadCode = fgets(lineBuffer,(int)maxLineSize,stream);
        if (fileReadCode == NULL) {
            goto errorExit;
        }

        for (lineIterator = 0; lineIterator < maxLineSize; lineIterator++) {
            if (*(lineBuffer + lineIterator) == '\0') break;

            if (isdigit(*(lineBuffer + lineIterator)) != 0 || *(lineBuffer + lineIterator) == '-') {
                *(numberBuff + elementIterator) = *(lineBuffer + lineIterator);
                elementIterator++;
            } else {

                if (elementIterator == 0) goto errorExit;
                *(numberBuff + elementIterator) = '\0';
                int code = convertIntoElementFromString(newMatrix,rowIterator,currentColumn, numberBuff);
                if (code != 0) goto errorExit;

                currentColumn++;

                if (currentColumn >= newMatrix->columns) break;

                elementIterator = 0;
            }
        }
    }
    free(numberBuff);
    free(lineBuffer);
    return newMatrix;

    errorExit:
    free(numberBuff);
    free(lineBuffer);
    matrixFree(newMatrix,maxMemAlloc);
    return NULL;
}

int convertIntoElementFromString(matrix* matrix,uint16_t row, uint16_t column, char* string) {
    switch (matrix->matrixVarType)
    {
    case 0:
        uint8_t* elementPointerUInt8 = (uint8_t*)matrixGetPointer(matrix,row,column);
        if (elementPointerUInt8 == NULL) return -2;
        *elementPointerUInt8 = ((uint8_t)atoi(string))%matrix->modulus;
        break;
    case 1:
        uint16_t* elementPointerUInt16 = (uint16_t*)matrixGetPointer(matrix,row,column);
        if (elementPointerUInt16 == NULL) return -2;
        *elementPointerUInt16 = ((uint16_t)atoi(string))%matrix->modulus;
        break;
    case 2:
        uint32_t* elementPointerUInt32 = (uint32_t*)matrixGetPointer(matrix,row,column);
        if (elementPointerUInt32 == NULL) return -2;
        *elementPointerUInt32 = ((uint32_t)atoll(string))%matrix->modulus;
        break;
    case 3:
        int32_t* elementPointerInt32 = (int32_t*)matrixGetPointer(matrix,row,column);
        if (elementPointerInt32 == NULL) return -2;
        *elementPointerInt32 = atoi(string);
        break;
    default:
        return -1;
    }
    return 0;
}

matrix* matrixBasicFileRead(FILE* stream,uint64_t* maxMemAlloc) {
    size_t amountRead = 0;
    fseek(stream,0L,SEEK_END);
    size_t fileSize = ftell(stream);
    rewind(stream);

    char tempBuff[7];
    char compare[7];
    fread_s(tempBuff,sizeof(tempBuff),sizeof(char),6,stream);
    tempBuff[6] = '\0';
    _snprintf_s(compare,7,7,"MATRIX");
    if(strcmp(tempBuff,compare)!=0) {
        return NULL;
    }
    amountRead += 6;

    uint32_t modulus;
    fread_s(&modulus,sizeof(uint32_t),sizeof(uint32_t),1,stream);
    amountRead += sizeof(uint32_t);
    uint16_t rows;
    fread_s(&rows,sizeof(uint16_t),sizeof(uint16_t),1,stream);
    amountRead += sizeof(uint16_t);
    uint16_t columns;
    fread_s(&columns,sizeof(uint16_t),sizeof(uint16_t),1,stream);
    amountRead += sizeof(uint16_t);
    
    matrix* matrix = matrixCreate(rows,columns,modulus,maxMemAlloc);
    if (matrix == NULL) {
        return NULL;
    }
    for (rows = 0; rows < matrix->rows; rows++)
    {
        for (columns = 0; columns < matrix->columns; columns++)
        {
            if(amountRead > fileSize) {
                matrixFree(matrix,maxMemAlloc);
                return NULL;
            }
            switch (matrix->matrixVarType)
            {
            case 0:
                fread_s((uint8_t*)matrixGetPointer(matrix,rows,columns),sizeof(uint8_t),sizeof(uint8_t),1,stream);
                amountRead+=sizeof(uint8_t);
                break;
            case 1:
                fread_s((uint16_t*)matrixGetPointer(matrix,rows,columns),sizeof(uint16_t),sizeof(uint16_t),1,stream);
                amountRead+=sizeof(uint16_t);
                break;
            case 2:
                fread_s((uint32_t*)matrixGetPointer(matrix,rows,columns),sizeof(uint32_t),sizeof(uint32_t),1,stream);
                amountRead+=sizeof(uint32_t);
                break;
            case 3:
                fread_s((int32_t*)matrixGetPointer(matrix,rows,columns),sizeof(int32_t),sizeof(int32_t),1,stream);
                amountRead+=sizeof(int32_t);
                break;
            default:
                matrixFree(matrix,maxMemAlloc);
                return NULL;
                break;
            }
        }
    }
    return matrix;
}

int matrixBasicFileWrite(FILE* stream, matrix* matrix) {
    uint64_t writenBytes = 0;
    //Header Setup
    fwrite("MATRIX",1,6,stream); //Not writeing null of course
    writenBytes += 6;
    fwrite(&matrix->modulus,sizeof(matrix->modulus),1,stream);
    writenBytes += sizeof(matrix->modulus);
    fwrite(&matrix->rows,sizeof(matrix->rows),1,stream);
    writenBytes += sizeof(matrix->rows);
    fwrite(&matrix->columns,sizeof(matrix->columns),1,stream);
    writenBytes += sizeof(matrix->columns);
    fflush(stream);
    for (uint16_t rows = 0; rows < matrix->rows; rows++)
    {
        for (uint16_t columns = 0; columns < matrix->columns; columns++)
        {
            switch (matrix->matrixVarType)
            {
            case 0:
                fwrite(matrixGetPointer(matrix,rows,columns),sizeof(uint8_t),1,stream);
                writenBytes+=sizeof(uint8_t);
                break;
            case 1:
                fwrite(matrixGetPointer(matrix,rows,columns),sizeof(uint16_t),1,stream);
                writenBytes+=sizeof(uint16_t);
                break;
            case 2:
                fwrite(matrixGetPointer(matrix,rows,columns),sizeof(uint32_t),1,stream);
                writenBytes+=sizeof(uint32_t);
                break;
            case 3:
                fwrite(matrixGetPointer(matrix,rows,columns),sizeof(int32_t),1,stream);
                writenBytes+=sizeof(int32_t);
                break;
            default:
                return -1;
                break;
            }
        }
    }
    return 0;
}


// Will focus on these once I wish to. Not the main concern of the project
/* matrix* fileScanMatrix(String fileLocation,String fileName,uint64_t* maxMemAlloc) {
    char location[1000];
    snprintf(location,1000,"%s\\%s.matrix",fileLocation.content,fileName.content);
    FILE* file = fopen(location,"rb");
    if (file == NULL) {
        printf("File Failed To open");
        return NULL;
    }

    char tempBuff[7];
    char compare[7];
    fread_s(tempBuff,7,sizeof(char),6,file);
    tempBuff[6] = '\0';
    snprintf(compare,7,"MATRIX");
    if(strcmp(tempBuff,compare)!=0) {
        printf("Not a matrix");
        printf("\n%s\n%s\n",tempBuff,compare);
        fclose(file);
        return NULL;
    }

    uint32_t modulus;
    fread_s(&modulus,sizeof(uint32_t),sizeof(uint32_t),1,file);

    uint16_t rows;
    fread_s(&rows,sizeof(uint16_t),sizeof(uint16_t),1,file);

    uint16_t columns;
    fread_s(&columns,sizeof(uint16_t),sizeof(uint16_t),1,file);
    
    matrix* matrix = matrixCreate(rows,columns,modulus,maxMemAlloc);
    if (matrix == NULL) {
        printf("Faild to alloc matrix");
        fclose(file);
        return NULL;
    }
    uint8_t bitsPerElement = 32;
    if(matrix->modulus != 1) for(bitsPerElement = 1; (uint32_t)pow((double)2,(double)bitsPerElement)<matrix->modulus;bitsPerElement++);
    uint8_t bitStore = 0;
    uint8_t valueStore = 0;
    int type = 0;
    uint8_t valueStoreUInt8 = 0;
    uint16_t valueStoreUInt16 = 0;
    uint32_t valueStoreUInt32 = 0;
    int32_t valueStoreInt32 = 0;
    uint8_t eigthBitMask = 255;
    uint8_t bitsLeftOnStore = 8;
    uint8_t bitsLeftToRead = bitsPerElement;
    uint16_t currentRow = 0;
    uint16_t currentColumn = 0;
    uint32_t amountOfElements = matrix->columns*matrix->rows;
    uint64_t amountOfBytesToRead = (uint64_t)ceil(((double)(bitsPerElement*amountOfElements))/8);
    for (uint64_t byteIterator = 0; byteIterator < amountOfBytesToRead;byteIterator++) {
        if(currentRow >= matrix->rows) break;
        bitStore = fgetc(file);
        bitsLeftOnStore = 8;
        while (bitsLeftOnStore > 0)
        {
            if(currentRow >= matrix->rows) break;
            if (bitsLeftToRead <= bitsLeftOnStore) {
                valueStore = (bitStore >> (8-bitsLeftOnStore)) & (eigthBitMask>>(8-bitsLeftToRead));
                type = 0;
            } else if (bitsLeftToRead > bitsLeftOnStore) {
                valueStore = bitStore >> (8-bitsLeftOnStore);
                type = 1;
            }
            switch (matrix->matrixVarType)
            {
            case 0:
                valueStoreUInt8 |= ((uint8_t)valueStore)<<(bitsPerElement-bitsLeftToRead);
                break;
            case 1:
                valueStoreUInt16 |= ((uint16_t)valueStore)<<(bitsPerElement-bitsLeftToRead);
                break;
            case 2:
                valueStoreUInt32 |= ((uint32_t)valueStore)<<(bitsPerElement-bitsLeftToRead);
                break;
            case 3:
                valueStoreInt32 |= ((int32_t)valueStore)<<(bitsPerElement-bitsLeftToRead);
                break;
            default:
                break;
            }
            if (type == 0) {
                bitsLeftOnStore -= bitsLeftToRead;
                bitsLeftToRead = 0;
            } else if (type == 1)
            {
                bitsLeftToRead -= bitsLeftOnStore;
                bitsLeftOnStore = 0;
            }
            if (bitsLeftToRead <= 0) { 
                switch (matrix->matrixVarType)
                {
                case 0:
                    *(*((uint8_t**)matrix->matrix+currentRow)+currentColumn) = valueStoreUInt8;
                    valueStoreUInt8 = 0;
                    break;
                case 1:
                    *(*((uint16_t**)matrix->matrix+currentRow)+currentColumn) = valueStoreUInt16;
                    valueStoreUInt16 = 0;
                    break;
                case 2:
                    *(*((uint32_t**)matrix->matrix+currentRow)+currentColumn) = valueStoreUInt32;
                    valueStoreUInt32 = 0;
                    break;
                case 3:
                    *(*((int32_t**)matrix->matrix+currentRow)+currentColumn) = valueStoreInt32;
                    valueStoreInt32 = 0;
                    break;
                default:
                    fclose(file);
                    matrixFree(matrix,maxMemAlloc);
                    return NULL;
                    break;
                }
                bitsLeftToRead = bitsPerElement;
                currentColumn++;
                if (currentColumn >= matrix->columns) {
                    currentRow++;
                    currentColumn = 0;
                }
            }
        }
    }
    fclose(file);
    return matrix;
}

int matrixFileWrite(String fileLocation, String fileName, matrix* matrix) {
    char tempLocation[1000];
    char location[1000];
    snprintf(tempLocation,1000,"%s\\matrixTemp.bin",fileLocation.content);
    snprintf(location,1000,"%s\\%s.matrix",fileLocation.content,fileName.content);
    remove(tempLocation);
    FILE* matrixTemp = fopen(tempLocation,"wb");
    if (matrixTemp == NULL) return -2;
    uint64_t writenBytes = 0;
    //Header Setup
    fwrite("MATRIX",1,6,matrixTemp); //Not writeing null of course
    writenBytes += 6;
    fwrite(&matrix->modulus,sizeof(matrix->modulus),1,matrixTemp);
    writenBytes += sizeof(matrix->modulus);
    fwrite(&matrix->rows,sizeof(matrix->rows),1,matrixTemp);
    writenBytes += sizeof(matrix->rows);
    fwrite(&matrix->columns,sizeof(matrix->columns),1,matrixTemp);
    writenBytes += sizeof(matrix->columns);
    fflush(matrixTemp);

    uint8_t bitsPerElement = 32;
    if(matrix->modulus != 1) for(bitsPerElement = 1; (uint32_t)pow((double)2,(double)bitsPerElement)<matrix->modulus;bitsPerElement++);
    
    //Body
    uint8_t bitStore = 0;
    uint8_t matrixValue;
    uint8_t eigthBitMask = 255;
    uint8_t bitsLeftOnStore = 8;
    uint8_t bitsLeftToWrite = bitsPerElement;
    uint16_t currentRow = 0;
    uint16_t currentColumn = 0;
    uint32_t amountOfElements = matrix->columns*matrix->rows;
    uint64_t amountOfBytesToWrite = (uint64_t)ceil(((double)(bitsPerElement*amountOfElements))/8);
    for(uint64_t byteIterator=0; byteIterator<amountOfBytesToWrite;byteIterator++) {
        
        fflush(matrixTemp);

        while (bitsLeftOnStore > 0)
        {
            if (currentRow >= matrix->rows) {
                break;
            };
            switch (matrix->matrixVarType)
            {
            case 0:
                matrixValue = *(*((uint8_t**)matrix->matrix+currentRow)+currentColumn);
                break;
            case 1:
                uint16_t matrixValueUint16 = *(*((uint16_t**)matrix->matrix+currentRow)+currentColumn);
                matrixValueUint16 >>= (bitsPerElement - bitsLeftToWrite);
                matrixValue = (uint8_t)(matrixValueUint16&eigthBitMask);
                break;
            case 2:
                uint32_t matrixValueUint32 = *(*((uint32_t**)matrix->matrix+currentRow)+currentColumn);
                matrixValueUint32 >>= (bitsPerElement - bitsLeftToWrite);
                matrixValue = (uint8_t)(matrixValueUint32&eigthBitMask);
                break;
            case 3:
                int32_t matrixValueInt32 = *(*((int32_t**)matrix->matrix+currentRow)+currentColumn);
                matrixValueInt32 >>= (bitsPerElement - bitsLeftToWrite);
                matrixValue = (uint8_t)(matrixValueInt32&eigthBitMask);
                break;
            default:
                fclose(matrixTemp);
                return -1;
            }
            if(bitsLeftToWrite > bitsLeftOnStore) {
                matrixValue <<= (8-bitsLeftOnStore);
                bitsLeftToWrite -= bitsLeftOnStore;
                bitStore |= matrixValue;
                bitsLeftOnStore = 0;
            }
            else if (bitsLeftToWrite <= bitsLeftOnStore)
            {
                matrixValue <<= (8-bitsLeftOnStore);
                bitStore |= matrixValue;
                bitsLeftOnStore -=bitsLeftToWrite;
                bitsLeftToWrite = 0;
            }

            if(bitsLeftToWrite <= 0) {
                currentColumn++;
                bitsLeftToWrite = bitsPerElement;
            }
            if (currentColumn >= matrix->columns) {
                currentColumn = 0;
                currentRow++;
            }
        }
        
        fputc(bitStore,matrixTemp);
        writenBytes++;
        bitStore = 0;
        bitsLeftOnStore = 8;
    }
    fflush(matrixTemp);
    fclose(matrixTemp);
    remove(location);
    FILE* matrixFile = fopen(location,"wb");
    if (matrixFile == NULL) {
        return -4;
    }
    matrixTemp = fopen(tempLocation,"rb");
    if(matrixTemp == NULL) {
        fclose(matrixFile);
        return -4;
    }

    for(uint64_t iterator = 0; iterator < writenBytes ; iterator++) {
        uint8_t byte = 0;
        fread_s(&byte,1,1,1,matrixTemp);
        fputc(byte,matrixFile);
        fflush(matrixFile);
    }

    fclose(matrixTemp);
    fclose(matrixFile);

    return 0;
} */