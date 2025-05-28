#ifndef arrayList_h_
#define arrayList_h_

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS 
#define ARRAYLIST __declspec(dllexport)
#else
#define ARRAYLIST __declspec(dllimport)
#endif
#else
#define ARRAYLIST
#endif
#include<stddef.h>
#include<stdint.h>
#include<stdbool.h>
#include<threads.h>
#include<backerLibListsTypes.h>
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct arraylist {
        mtx_t mutex;
        bool elementsArePointers;
        bool structIsHeapAlloced;
        bool mutexExists;
        ListTypes_t listType;
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
    extern ARRAYLIST int arrayListElementInsert(ArrayList* arrayList, size_t index, void* element, size_t elementSize);
    extern ARRAYLIST int arrayListElementSetGeneric(ArrayList* arrayList, size_t index, void* element, size_t elementSize);
    extern ARRAYLIST ArrayList arrayListCreateStack(size_t intialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers);
    extern ARRAYLIST ArrayList* arrayListCreate(size_t intialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers);
    extern ARRAYLIST void arrayListDestroy(ArrayList* arraylist);
    extern ARRAYLIST void arrayListDestroyWithElements(ArrayList* arrayList, void(elementDestructor)(void* element));
    extern ARRAYLIST ArrayList* arrayListMoveStackToHeap(ArrayList arrayList, bool destroyInputOnFailiure);
    extern ARRAYLIST ArrayList arrayListMoveStack(ArrayList* arrayList);
    extern ARRAYLIST ArrayList* arrayListCopyStackToHeap(ArrayList* arrayList);
    extern ARRAYLIST ArrayList arrayListCopyStack(ArrayList* arrayList);
#ifdef __cplusplus
}
#endif
#endif