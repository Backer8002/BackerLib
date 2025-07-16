#ifndef BackerString_h_
#define BackerString_h_

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS
#define BACKERSTRINGS __declspec(dllexport)
#else
#define BACKERSTRINGS __declspec(dllimport)
#endif
#else
#define BACKERSTRINGS
#endif

#include "arrayList.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    ArrayList arrayList;
} String;

    extern BACKERSTRINGS String                  stringCreate(const char* string, size_t length);
    extern inline BACKERSTRINGS char             stringGetChar(const String* string, size_t index);
    extern inline BACKERSTRINGS ArrayListError_t stringRemoveSlice(String* string, size_t firstIndex, size_t lastIndex);
    extern BACKERSTRINGS String                  stringGetSlice(const String* string, size_t firstIndex, size_t lastIndex);
    extern inline BACKERSTRINGS String           stringCopy(const String* string);
    extern inline BACKERSTRINGS String           stringReverse(const String* string);
    extern BACKERSTRINGS ArrayListError_t        stringAdd(String* destString, const char* string2, size_t length);
    extern BACKERSTRINGS ArrayListError_t        stringConcat(String* destString, const String* secondString);
    extern BACKERSTRINGS String                  stringStrip(const String* string, char charToStrip, bool enforceStripLimit, int64_t amountToStrip);
    extern BACKERSTRINGS ArrayList               stringSplit(const String* string, char charToSplitOn, bool enforceSplitLimit, int64_t amountOfCharsToSplitAt);

    /*Inserts other into the destString at firstIndex of destString

     Returns ArrayListAccessViolation if firstIndex is invalid
     Returns ArrayListCannotAllocateMemory if string cannot grow.
     Return ArrayListInvalidType if other was not a valid string
     */
    static inline BACKERSTRINGS ArrayListError_t stringInsertSubString(String* destString, const String* other, size_t firstIndex) {return arrayListElementInsert((ArrayList*)destString,firstIndex,other->arrayList.amountOfElements,other->arrayList.list,other->arrayList.header.dataArrayVarType);}
    /*Inserts other into the destString at firstIndex of destString

    Returns ArrayListAccessViolation if firstIndex is invalid
    Returns ArrayListCannotAllocateMemory if string cannot grow.
    */
    static inline BACKERSTRINGS ArrayListError_t stringInsertSubCString(String* destString, const char* other, size_t lenOfOther, size_t firstIndex) {return arrayListElementInsert((ArrayList*)destString,firstIndex,lenOfOther,other,ListUInt8);}
    static inline BACKERSTRINGS void stringDestroy(void* string) {arrayListDestroy(string); }

    /* Replaces chars at firstIndex to firstIndex + len(stringToReplaceWith) with stringToReplaceWith.

      Returns ArrayListAccessViolation if firstIndex was invalid
      Returns ArrayListCannotAllocateMemory if string cannot grow
     */
    static inline BACKERSTRINGS ArrayListError_t stringReplace(String* destString, const String* stringToReplaceWith,size_t firstIndex) {
        for (size_t i = 0; i < stringToReplaceWith->arrayList.amountOfElements; i++) {
            ArrayListError_t errorCode;
            if ((errorCode = arrayListElementSet((ArrayList*) destString, i + firstIndex, arrayListElementGet((ArrayList*) stringToReplaceWith, i), ListUInt8) != ArrayListOperationSuccess))
                return errorCode;
        }
        return ArrayListOperationSuccess;
    }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif