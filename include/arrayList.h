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
ARRAYLIST typedef enum {
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

ARRAYLIST typedef struct arraylist{
    size_t amountOfElements;
    size_t totalAmountOfElements;
    size_t elementSize;
    void* (*get)(struct arraylist*,size_t);
    void (*clear)(struct arraylist*);
    int (*add)(struct arraylist*,void*);
    int (*set)(struct arraylist*,size_t,void*);
    void* (*pop)(struct arraylist*);
    void (*remove)(struct arraylist*,size_t,size_t);
    void* list;
    ArrayListTypes listType;
} ArrayList;

extern ARRAYLIST int arrayListSizeCheckAdd(ArrayList* arrayList);
extern ARRAYLIST int arrayListSizeCheckRemove(ArrayList* arraylist);
extern ARRAYLIST void arrayListGenericClearElements(ArrayList* arrayList);
extern ARRAYLIST void* arrayListGenericGetElement(ArrayList* arrayList,size_t index);
extern ARRAYLIST int arrayListGenericAddElement(ArrayList* arrayList, void* element);
extern ARRAYLIST void* arrayListGenericPopElement(ArrayList* arrayList);
extern ARRAYLIST void arrayListGenericRemoveElement(ArrayList* arrayList,size_t index, size_t lastIndex);
extern ARRAYLIST void arrayListDestroy(ArrayList* arraylist);
#endif