#include "Json.h"
#include <BackerLibTypes.h>







void jsonArrayDestroy(void* jsonArray) {
    if (!isValidObject((DataTypeFlags*) jsonArray))
        return;
    for (JsonArrayMember* currentMember = containerDynamicFront(jsonArray); currentMember < (JsonArrayMember*) containerDynamicEnd(jsonArray); currentMember++) {
        if (currentMember->valueType == JsonTypeString)
            containerDestroy(&currentMember->value.string);
        else if (currentMember->valueType == JsonTypeArray)
            jsonArrayDestroy(&currentMember->value.array);
        else if (currentMember->valueType == JsonTypeObject)
            jsonObjectDestroy(&currentMember->value.object);
    }
    containerDestroy(jsonArray);
}

void jsonObjectDestroy(void* jsonObject) {
    if (!isValidObject(jsonObject))
        return;
    for (JsonObjectMember* currentMember = containerDynamicFront(jsonObject); currentMember < (JsonObjectMember*) containerDynamicEnd(jsonObject); currentMember++) {
        containerDestroy(&currentMember->identifier);
        if (currentMember->valueType == JsonTypeString)
            containerDestroy(&currentMember->value.string);
        else if (currentMember->valueType == JsonTypeArray)
            jsonArrayDestroy(&currentMember->value.array);
        else if (currentMember->valueType == JsonTypeObject)
            jsonObjectDestroy(&currentMember->value.object);
    }
    containerDestroy(jsonObject);
}
