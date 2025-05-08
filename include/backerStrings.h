#ifndef backerStrings_h_
#define backerStrings_h_

#ifdef _WINDOWS
#ifdef BASICFUNCTIONS_EXPORTS 
#define BACKERSTRINGS __declspec(dllexport)
#else
#define BACKERSTRINGS __declspec(dllimport)
#endif
#else
#define BACKERSTRINGS
#endif

#include<arrayList.h>
#include<stdbool.h>
#include<stddef.h>
#define stringNode false
#define stringRaw true
BACKERSTRINGS typedef struct string {
    union
    {
        ArrayList* node;
        char* data;
    };
    size_t length;
    bool type;
} String;

extern BACKERSTRINGS String ** stringCreate(char *string, size_t length);
extern BACKERSTRINGS String** stringCreateWithCharPointer(char* string, size_t length);
extern BACKERSTRINGS void stringDestroy(String** stringPtr);
extern BACKERSTRINGS char* stringCharGet(String** string,size_t index);
extern BACKERSTRINGS int stringAdd(String** destString, char* string2,size_t length);
extern BACKERSTRINGS int stringConcat(String** destString,String** secondString);
extern BACKERSTRINGS char* stringGetArray(String** string, size_t* arraySize);
#endif