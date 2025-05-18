#ifndef arrayList_h_
#define arrayList_h_

#ifdef _WINDOWS
#ifdef BASICFUNCTIONS_EXPORTS 
#define ARRAYLIST __declspec(dllexport)
#else
#define ARRAYLIST __declspec(dllimport)
#endif
#else
#define ARRAYLIST
#endif
#include<stddef.h>
#include<stdarg.h>
#include<stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
    typedef enum {
        Integer,
        Long,
        LongLong,
        Unsigned,
        UnsignedLong,
        UnsignedLongLong,
        Short,
        UnsignedShort,
        Char,
        UnsignedChar,
        Matrix,
        STRING
    }ArrayListTypes;

    typedef struct arraylist {
        ArrayListTypes listType;
        size_t amountOfElements;
        size_t totalAmountOfElements;
        size_t elementSize;
        void* list;
    } ArrayList;

    extern ARRAYLIST int arrayListSizeCheckAdd(ArrayList* arrayList);
    extern ARRAYLIST int arrayListSizeCheckRemove(ArrayList* arraylist);
    extern ARRAYLIST void arrayListElementsClear(ArrayList* arrayList);
    extern ARRAYLIST void* arrayListElementGetGeneric(ArrayList* arrayList, size_t index);
    extern ARRAYLIST void* arrayListElementPopGeneric(ArrayList* arrayList);
    extern ARRAYLIST void arrayListElementRemove(ArrayList* arrayList, size_t index, size_t lastIndex);
    extern ARRAYLIST void arrayListDestroy(ArrayList* arraylist);
#ifdef __cplusplus
}
#endif
#endif