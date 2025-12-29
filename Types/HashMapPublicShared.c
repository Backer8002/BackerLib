#include "TypesMain.h"
#include "BackerStrings.h"
#include <stdarg.h>
#include <stdint.h>

uint64_t bl_hashfunction_defualt(size_t amountOfVars, ...) {
    if (amountOfVars == 0)
        return 0;
    va_list argsToHash;
    va_start(argsToHash, amountOfVars);
    uint64_t hash = 0;
    for (size_t j = 0; j < amountOfVars; j++) {
        size_t amountOfBytes = va_arg(argsToHash, size_t);
        BL_Byte*  valuesToHash  = va_arg(argsToHash, BL_Byte*);
        hash ^= (uint64_t) valuesToHash[0] ^ ~(amountOfBytes << 10);
        for (size_t i = 0; i < amountOfBytes; i++)
            hash ^= (((uint64_t) valuesToHash[i] + hash) * (hash + (hash << 5)));
    }
    va_end(argsToHash);
    return hash;
}

uint64_t bl_hashfunction_defualt_single_var(const void* element, size_t size) {
    return bl_hashfunction_defualt(1, size, element);
}

uint64_t bl_hashfunction_string_view(const void* element) {
    const BL_StringView* sv = element;
    return bl_hashfunction_defualt_single_var(sv->array,sv->amountOfIndexes);
}