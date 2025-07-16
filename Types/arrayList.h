#ifndef ArrayList_h_
#define ArrayList_h_

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS
#define ARRAYLIST __declspec(dllexport)
#else
#define ARRAYLIST __declspec(dllimport)
#endif
#else
#define ARRAYLIST
#endif

#include "backerLibListTypes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>




#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    DataTypeHeader header;
    mtx_t          mutex;
    size_t         amountOfElements;
    size_t         totalAmountOfElements;
    size_t         elementSize;
    void*          list;
} ArrayList;

typedef enum ArrayListError {
    ArrayListOperationSuccess = 0,
    ArrayListCannotAllocMemory,
    ArrayListAccessViolation,
    ArrayListInvalidType
} ArrayListError_t;

extern ARRAYLIST ArrayListError_t arrayListSizeCheckAdd(ArrayList* arrayList);
extern ARRAYLIST ArrayListError_t arrayListSizeCheckRemove(ArrayList* arraylist);
extern ARRAYLIST void             arrayListElementsClear(ArrayList* arrayList);
extern ARRAYLIST void*            arrayListElementGet(const ArrayList* arrayList, size_t index);
extern ARRAYLIST ArrayListError_t arrayListElementPop(ArrayList* arrayList);
extern ARRAYLIST ArrayListError_t arrayListElementRemove(ArrayList* arrayList, size_t index, size_t lastIndex);
extern ARRAYLIST ArrayListError_t arrayListElementInsert(ArrayList* arrayList, size_t index, size_t amountOfElements, const void* elements, ListTypes_t elementType);
extern ARRAYLIST ArrayListError_t arrayListElementSet(ArrayList* arrayList, size_t index, void* element, ListTypes_t elementType);
extern ARRAYLIST ArrayList        arrayListCreateStack(size_t initialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers);
extern ARRAYLIST ArrayList*       arrayListCreate(size_t initialSize, size_t elementSize, ListTypes_t elementType, bool elementsArePointers);
extern ARRAYLIST void             arrayListDestroy(void* arraylist);
extern ARRAYLIST void             arrayListDestroyWithElements(ArrayList* arrayList, void(elementDestructor)(void* element));
extern ARRAYLIST ArrayList*       arrayListMoveStackToHeap(ArrayList arrayList, bool destroyInputOnFailiure);
extern ARRAYLIST ArrayList        arrayListMoveStack(ArrayList* arrayList);
extern ARRAYLIST ArrayList*       arrayListCopyStackToHeap(ArrayList* arrayList);
extern ARRAYLIST ArrayList        arrayListCopyStack(ArrayList* arrayList);

#ifdef __cplusplus
}
#endif


#endif