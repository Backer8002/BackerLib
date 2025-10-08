#include "Json.h"
#include <BackerLibTypes.h>

static JsonObjectMemberValue jsonCopyValue(const JsonObjectMemberValue* value, JsonObjectMemberType valueType) {
    switch (valueType) {
    case JsonTypeString:
        return (JsonObjectMemberValue) {.string = stringCreate(containerDynamicFront(&value->string), stringLength(&value->string))};
        break;
    case JsonTypeArray:
        return (JsonObjectMemberValue) {.array = jsonArrayCopy(&value->array)};
        break;
    case JsonTypeObject:
        return (JsonObjectMemberValue) {.object = jsonObjectCopy(&value->object)};
        break;
    default:
        return *value;
    }
}

JsonObjectMember* jsonObjectMemberGetByIdentifier(const JsonObject* jsonObject, const String* string) {
    size_t begin = 0;
    size_t end   = containerSize((const Container*) jsonObject);
    while (end - begin > 1) {
        size_t mid = (begin + end) / 2;
        if (stringCompareAcending(containerGet((const Container*) jsonObject, mid), string))
            end = mid;
        else
            begin = mid;
    }
    if (stringEqual(containerGet((const Container*) jsonObject, begin), string))
        return containerGet((const Container*) jsonObject, begin);
    if (stringEqual(containerGet((const Container*) jsonObject, end), string))
        return containerGet((const Container*) jsonObject, end);
    return NULL;
}

ContainerError jsonObjectAdd(JsonObject* jsonObject, StringView* string, JsonObjectMemberType valueType, const JsonObjectMemberValue* value) {
}

ContainerError jsonArrayAdd(JsonArray* jsonArray, JsonObjectMemberType valueType, const JsonObjectMemberValue* value) {
    JsonObjectMemberValue valueToUse = jsonCopyValue(value, valueType);
    if ((valueType == JsonTypeString || valueType == JsonTypeArray || valueType == JsonTypeObject) && !isValidObject((DataTypeFlags*)&valueToUse))
        return ContainerAllocFailure;
    return containerDynamicAppend(jsonArray, sizeof(JsonArrayMember),&(JsonArrayMember){.valueType = valueType, .value = valueToUse});
}

ContainerError jsonArrayAddAtIndex(JsonArray* jsonArray, size_t index, JsonObjectMemberType valueType, const JsonObjectMemberType* value) {
}

bool jsonObjectRemove(JsonObject* jsonObject, size_t index) {
    if (index >= containerSize((Container*) jsonObject))
        return false;

    JsonObjectMember* jsonObjectMember = jsonObjectMemberGetByIndex(jsonObject, index);

    containerDestroy(&jsonObjectMember->identifier);

    switch (jsonObjectMember->valueType) {
    case JsonTypeArray:
        jsonArrayDestroy(&jsonObjectMember->value);
        break;
    case JsonTypeObject:
        jsonObjectDestroy(&jsonObjectMember->value);
        break;
    case JsonTypeString:
        containerDestroy(&jsonObjectMember->value);
        break;
    default:
        break;
    }

    containerDynamicRemove(jsonObject, index, index);
    return true;
}

bool jsonArrayRemove(JsonArray* jsonArray, size_t index) {
    if (index >= containerSize((Container*) jsonArray))
        return false;

    JsonArrayMember* jsonArrayMember = jsonArrayMemberGet(jsonArray, index);

    switch (jsonArrayMember->valueType) {
    case JsonTypeArray:
        jsonArrayDestroy(&jsonArrayMember->value);
        break;
    case JsonTypeObject:
        jsonObjectDestroy(&jsonArrayMember->value);
        break;
    case JsonTypeString:
        containerDestroy(&jsonArrayMember->value);
        break;
    default:
        break;
    }

    containerDynamicRemove(jsonArray, index, index);
    return true;
}


JsonArray jsonArrayCopy(const JsonArray* jsonArray) {
    JsonArray newArray = containerConvertToDynamicStack(
        containerGetSubArray((const Container*) jsonArray,
                             0,
                             containerSize((const Container*) jsonArray) - 1,
                             false));
    if (!isValidObject(&newArray.header))
        return newArray;
    JsonArrayMember* jsonArrayMember = NULL;
    for (jsonArrayMember = containerDynamicFront(&newArray); jsonArrayMember < containerDynamicEnd(&newArray); jsonArrayMember++) {
        jsonArrayMember->value = jsonCopyValue(&jsonArrayMember->value, jsonArrayMember->valueType);
        if ((jsonArrayMember->valueType == JsonTypeString || jsonArrayMember->valueType == JsonTypeArray || jsonArrayMember->valueType == JsonTypeObject)
                && !isValidObject((DataTypeFlags*) &jsonArrayMember->value))
            goto jsonArrayCopyErrorExit;
    }
    return newArray;

jsonArrayCopyErrorExit:
    for (JsonArrayMember* member = containerDynamicFront(&newArray); member < jsonArrayMember; member++) {
        switch (member->valueType) {
        case JsonTypeString:
            containerDestroy(&member->value);
            break;
        case JsonTypeArray:
            jsonArrayDestroy(&member->value);
            break;
        case JsonTypeObject:
            jsonObjectDestroy(&member->value);
            break;
        default:
        }
    }
    containerDestroy(&newArray);
    return newArray;
}

JsonObject jsonObjectCopy(const JsonObject* jsonObject) {
    JsonObject newObject = containerConvertToDynamicStack(
        containerGetSubArray((const Container*) jsonObject,
                             0,
                             containerSize((const Container*) jsonObject) - 1,
                             false));
    if (!isValidObject(&newObject.header))
        return newObject;
    JsonObjectMember* jsonObjectMember = NULL;
    for (jsonObjectMember = containerDynamicFront(&newObject); jsonObjectMember < containerDynamicEnd(&newObject); jsonObjectMember++) {
        jsonObjectMember->identifier = stringCreate(containerDynamicFront(&jsonObjectMember->identifier), stringLength(&jsonObjectMember->identifier));
        if (!isValidObject(&jsonObjectMember->identifier.header))
            goto jsonObjectCopyErrorExit;

        jsonObjectMember->value = jsonCopyValue(&jsonObjectMember->value, jsonObjectMember->valueType);

        if ((jsonObjectMember->valueType == JsonTypeString || jsonObjectMember->valueType == JsonTypeArray || jsonObjectMember->valueType == JsonTypeObject)
                && !isValidObject((DataTypeFlags*) &jsonObjectMember->value)) {
            containerDestroy(&jsonObjectMember->identifier);
            goto jsonObjectCopyErrorExit;
        }
    }

    return newObject;

jsonObjectCopyErrorExit:
    for (JsonObjectMember* member = containerDynamicFront(&newObject); member < jsonObjectMember; member++) {
        containerDestroy(&member->identifier);
        switch (member->valueType) {
        case JsonTypeString:
            containerDestroy(&member->value);
            break;
        case JsonTypeArray:
            jsonArrayDestroy(&member->value);
            break;
        case JsonTypeObject:
            jsonObjectDestroy(&member->value);
            break;
        default:
        }
    }
    containerDestroy(&newObject);
    return newObject;
}

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
