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

static bool jsonCopyValueHasFailed(const JsonObjectMemberValue* value, JsonObjectMemberType valueType) {
    if ((valueType == JsonTypeString || valueType == JsonTypeArray || valueType == JsonTypeObject) && !isValidObject((DataTypeFlags*) value))
        return true;
    return false;
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
    if (begin == end)
        return NULL;

    if (stringEqual(containerGet((const Container*) jsonObject, begin), string))
        return containerGet((const Container*) jsonObject, begin);
    if (containerSize((Container*) jsonObject) > 1 && stringEqual(containerGet((const Container*) jsonObject, end), string))
        return containerGet((const Container*) jsonObject, end);
    return NULL;
}

ContainerError jsonObjectAdd(JsonObject* jsonObject, StringView* string, JsonObjectMemberType valueType, const JsonObjectMemberValue* value) {
    String stringToUse = containerConvertToDynamicStack(
        containerGetSubArray((const Container*) string,
                             0,
                             containerSize((const Container*) string) - 1,
                             false));
    if (!isValidObject((DataTypeFlags*) &stringToUse))
        return ContainerAllocFailure;
    JsonObjectMemberValue valueToUse = jsonCopyValue(value, valueType);
    if (jsonCopyValueHasFailed(&valueToUse, valueType)) {
        containerDestroy(&stringToUse);
        return ContainerAllocFailure;
    }

    if (containerDynamicAppend(jsonObject, sizeof(JsonObjectMember), &(JsonObjectMember) {.identifier = stringToUse, .value = valueToUse, .valueType = valueType}) != ContainerOPSuccessful) {
        containerDestroy(&stringToUse);
        switch (valueType) {
        case JsonTypeArray:
            jsonArrayDestroy(&valueToUse);
            break;
        case JsonTypeObject:
            jsonObjectDestroy(&valueToUse);
            break;
        case JsonTypeString:
            containerDestroy(&valueToUse);
            break;
        default:
            break;
        }
        return ContainerAllocFailure;
    }
    heapSort((Container*)&jsonObject,stringCompareAcending);
    return ContainerOPSuccessful;
}

ContainerError jsonArrayAdd(JsonArray* jsonArray, JsonObjectMemberType valueType, const JsonObjectMemberValue* value) {
    return jsonArrayAddAtIndex(jsonArray, containerSize((Container*) jsonArray), valueType, value);
}

ContainerError jsonArrayAddAtIndex(JsonArray* jsonArray, size_t index, JsonObjectMemberType valueType, const JsonObjectMemberValue* value) {
    JsonObjectMemberValue valueToUse = jsonCopyValue(value, valueType);
    if (jsonCopyValueHasFailed(&valueToUse, valueType))
        return ContainerAllocFailure;
    return containerDynamicInsert(jsonArray, index, 1, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = valueType, .value = valueToUse});
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
        if (jsonCopyValueHasFailed(&jsonArrayMember->value, jsonArrayMember->valueType))
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
            break;
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

        if (jsonCopyValueHasFailed(&jsonObjectMember->value, jsonObjectMember->valueType)) {
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
            break;
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
