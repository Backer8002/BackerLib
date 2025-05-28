#ifndef backerStrings_h_
#define backerStrings_h_

#ifdef DLL
#ifdef BASICFUNCTIONS_EXPORTS 
#define BACKERSTRINGS __declspec(dllexport)
#else
#define BACKERSTRINGS __declspec(dllimport)
#endif
#else
#define BACKERSTRINGS
#endif

#include<arrayList.h>
#include<stddef.h>
#include<stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	typedef ArrayList String;

	extern BACKERSTRINGS String stringCreate(char* string, size_t length);
	extern inline BACKERSTRINGS char stringGetChar(String* string, size_t index);
	extern inline BACKERSTRINGS void stringRemoveSlice(String* string, size_t firstIndex, size_t lastIndex);
	extern BACKERSTRINGS String stringGetSlice(String* string, size_t firstIndex, size_t lastIndex);
	extern inline BACKERSTRINGS String stringCopy(String* string);
	extern inline BACKERSTRINGS String stringReverse(String* string);
	extern BACKERSTRINGS int stringAdd(String* destString, char* string2, size_t length);
	extern BACKERSTRINGS int stringConcat(String* destString, String* secondString);
	extern BACKERSTRINGS String stringStrip(String* string, char charToStrip, bool enforceStripLimit, int64_t amountToStrip);
	extern BACKERSTRINGS ArrayList stringSplit(String* string, char charToSplitOn, bool enforceSplitLimit, int64_t amountOfCharsToSplitAt);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif