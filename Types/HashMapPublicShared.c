#include "TypesMain.h"
#include "BackerStrings.h"
#include <stdarg.h>
#include <stdint.h>
#include "HashMap.h"

uint64_t bl_hashfunction_defualt(size_t amountOfVars, ...) {
    if (amountOfVars == 0)
        return 0;
    va_list argsToHash;
    va_start(argsToHash, amountOfVars);
    uint64_t hash = 0;
    for (size_t j = 0; j < amountOfVars; j++) {
        BL_HashingPair hashPair = va_arg(argsToHash, BL_HashingPair);
        hash ^= (*(uint64_t*)hashPair.second) ^ ~(hashPair.first << 10);
        for (size_t i = 0; i < hashPair.first; i++)
            hash ^= ((*((uint64_t*) hashPair.second + i) + hash) * (hash + (hash << 5)));
    }
    va_end(argsToHash);
    return hash;
}

uint64_t bl_hashfunction_defualt_single_var(const void* element, size_t size) {
    return bl_hashfunction_defualt(1, BL_MAKE_PAIR(BL_HashingPair,size,element));
}

uint64_t bl_hashfunction_string_view(const void* element) {
    const BL_StringView* sv = element;
    return bl_hashfunction_defualt_single_var(sv->array,sv->amountOfIndexes);
}