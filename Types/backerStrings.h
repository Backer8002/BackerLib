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

#ifdef __cplusplus
}
#endif // __cplusplus

#endif