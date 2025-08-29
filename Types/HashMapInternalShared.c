#include "TypesMain.h"
#include "HashMap_internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

bool internal_memCmpKey(size_t keySize, const void* key, const void* otherKey, bool isDataTypeFlagsQualified) {
    if (!otherKey || !key)
        return false;
    if (isDataTypeFlagsQualified) {
        if ((DataTypeFlags*) key != (DataTypeFlags*) otherKey)
            return false;
        if ((*(DataTypeFlags*) key & ObjectFlagIsContainer) && (*(DataTypeFlags*) key & ObjectFlagIsNotContinuous) == 0) {
            if (((Container*) key)->amountOfIndexes != ((Container*) otherKey)->amountOfIndexes || ((Container*) key)->byteSizeOfSingleElement != ((Container*) otherKey)->byteSizeOfSingleElement)
                return false;
            for (size_t i = 0; i < ((Container*) key)->amountOfIndexes; i++) {
                if (memcmp((Bytes) ((Container*) key)->array + ((Container*) key)->byteSizeOfSingleElement,
                           (Bytes) ((Container*) otherKey)->array + ((Container*) key)->byteSizeOfSingleElement,
                           ((Container*) key)->byteSizeOfSingleElement) != 0)
                    return false;
            }
            return true;
        }
    }
    if (memcmp(key, otherKey, keySize) != 0)
        return false;
    return true;
}