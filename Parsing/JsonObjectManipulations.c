#include "Json.h"
#include <BackerLibTypes.h>

static JsonMemberValue jsonCopyValue(const JsonMemberValue* value, JsonMemberType valueType) {
    switch (valueType) {
    case JsonTypeString:
        return (JsonMemberValue) {.string = bl_string_create(bl_container_dynamic_front(&value->string),
            bl_string_length(bl_stringview_ptr_cast(&value->string)))};
        break;
    case JsonTypeArray:
        return (JsonMemberValue) {.array = jsonArrayCopy(&value->array)};
        break;
    case JsonTypeObject:
        return (JsonMemberValue) {.object = jsonObjectCopy(&value->object)};
        break;
    default:
        return *value;
    }
}

static bool jsonCopyValueHasFailed(const JsonMemberValue* value, JsonMemberType valueType) {
    if ((valueType == JsonTypeString || valueType == JsonTypeArray || valueType == JsonTypeObject) && !bl_container_dynamic_is_valid(&value->object))
        return true;
    return false;
}

JsonObject       jsonObjectCreate(void) { return bl_container_dynamic_create_stack(0, sizeof(JsonObjectMember)); }

JsonArray        jsonArrayCreate(void) { return bl_container_dynamic_create_stack(0, sizeof(JsonArrayMember)); }

JsonArrayMember* jsonArrayMemberGet(const JsonArray* jsonArray, size_t index) { return bl_container_get((BL_Container*) jsonArray, index); }

JsonObjectMember* jsonObjectMemberGet(const JsonObject* jsonObject, BL_StringView* string) {
    size_t begin = 0;
    size_t end   = bl_container_size((const BL_Container*) jsonObject);
    while (end - begin > 1) {
        size_t mid = (begin + end) / 2;
        if (bl_string_compare_acending(bl_container_get((const BL_Container*) jsonObject, mid), string))
            begin = mid;
        else
            end = mid;
    }
    if (begin == end)
        return NULL;

    if (bl_string_equal(bl_container_get((const BL_Container*) jsonObject, begin), string))
        return bl_container_get((const BL_Container*) jsonObject, begin);
    if (bl_container_size((BL_Container*) jsonObject) > 1 && bl_string_equal(bl_container_get((const BL_Container*) jsonObject, end), string))
        return bl_container_get((const BL_Container*) jsonObject, end);
    return NULL;
}

BL_ContainerError jsonObjectAdd(JsonObject* jsonObject, BL_StringView* string, JsonMemberType valueType, const JsonMemberValue* value) {
    BL_String stringToUse = bl_dynamic_container_cast_container(
        bl_container_get_subarray((const BL_Container*) string,
                             0,
                             bl_container_size((const BL_Container*) string) - 1,
                             false));
    if (!bl_container_dynamic_is_valid(&stringToUse))
        return BL_ContainerAllocFailure;
    JsonMemberValue valueToUse = jsonCopyValue(value, valueType);
    if (jsonCopyValueHasFailed(&valueToUse, valueType)) {
        bl_container_destroy(&stringToUse);
        return BL_ContainerAllocFailure;
    }

    if (bl_container_dynamic_append(jsonObject, sizeof(JsonObjectMember), &(JsonObjectMember) {.identifier = stringToUse, .value = valueToUse, .valueType = valueType}) != BL_ContainerOPSuccessful) {
        bl_container_destroy(&stringToUse);
        switch (valueType) {
        case JsonTypeArray:
            jsonArrayDestroy(&valueToUse);
            break;
        case JsonTypeObject:
            jsonObjectDestroy(&valueToUse);
            break;
        case JsonTypeString:
            bl_container_destroy(&valueToUse);
            break;
        default:
            break;
        }
        return BL_ContainerAllocFailure;
    }
    bl_sort_heap((BL_Container*)jsonObject,bl_string_compare_acending);
    return BL_ContainerOPSuccessful;
}

BL_ContainerError jsonArrayAdd(JsonArray* jsonArray, JsonMemberType valueType, const JsonMemberValue* value) {
    return jsonArrayAddAtIndex(jsonArray, bl_container_size((BL_Container*) jsonArray), valueType, value);
}

BL_ContainerError jsonArrayAddAtIndex(JsonArray* jsonArray, size_t index, JsonMemberType valueType, const JsonMemberValue* value) {
    JsonMemberValue valueToUse = jsonCopyValue(value, valueType);
    if (jsonCopyValueHasFailed(&valueToUse, valueType))
        return BL_ContainerAllocFailure;
    return bl_container_dynamic_insert(jsonArray, index, 1, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = valueType, .value = valueToUse});
}

void jsonObjectRemove(JsonObject* jsonObject, JsonObjectMember* member) {

    bl_container_destroy(&member->identifier);

    switch (member->valueType) {
    case JsonTypeArray:
        jsonArrayDestroy(&member->value);
        break;
    case JsonTypeObject:
        jsonObjectDestroy(&member->value);
        break;
    case JsonTypeString:
        bl_container_destroy(&member->value);
        break;
    default:
        break;
    }
    const size_t indexInBaseObject = bl_container_index_from_reference((BL_Container*)jsonObject, member);
    bl_container_dynamic_remove(jsonObject, indexInBaseObject, indexInBaseObject);
}

bool jsonArrayRemove(JsonArray* jsonArray, size_t index) {
    if (index >= bl_container_size((BL_Container*) jsonArray))
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
        bl_container_destroy(&jsonArrayMember->value);
        break;
    default:
        break;
    }

    bl_container_dynamic_remove(jsonArray, index, index);
    return true;
}

JsonArray jsonArrayCopy(const JsonArray* jsonArray) {
    JsonArray newArray = bl_dynamic_container_cast_container(
        bl_container_get_subarray((const BL_Container*) jsonArray,
                             0,
                             bl_container_size((const BL_Container*) jsonArray) - 1,
                             false));
    if (!bl_container_dynamic_is_valid(&newArray))
        return newArray;
    JsonArrayMember* jsonArrayMember = NULL;
    for (jsonArrayMember = bl_container_dynamic_front(&newArray); jsonArrayMember < (JsonArrayMember*)bl_container_dynamic_end(&newArray); jsonArrayMember++) {
        jsonArrayMember->value = jsonCopyValue(&jsonArrayMember->value, jsonArrayMember->valueType);
        if (jsonCopyValueHasFailed(&jsonArrayMember->value, jsonArrayMember->valueType))
            goto jsonArrayCopyErrorExit;
    }
    return newArray;

jsonArrayCopyErrorExit:
    for (JsonArrayMember* member = bl_container_dynamic_front(&newArray); member < jsonArrayMember; member++) {
        switch (member->valueType) {
        case JsonTypeString:
            bl_container_destroy(&member->value);
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
    bl_container_destroy(&newArray);
    return newArray;
}

JsonObject jsonObjectCopy(const JsonObject* jsonObject) {
    JsonObject newObject = bl_dynamic_container_cast_container(
        bl_container_get_subarray((const BL_Container*) jsonObject,
                             0,
                             bl_container_size((const BL_Container*) jsonObject) - 1,
                             false));
    if (!bl_container_dynamic_is_valid(&newObject))
        return newObject;
    JsonObjectMember* jsonObjectMember = NULL;
    for (jsonObjectMember = bl_container_dynamic_front(&newObject); jsonObjectMember < (JsonObjectMember*)bl_container_dynamic_end(&newObject); jsonObjectMember++) {
        jsonObjectMember->identifier = bl_string_create(bl_container_dynamic_front(&jsonObjectMember->identifier),
            bl_string_length(bl_stringview_ptr_cast(&jsonObjectMember->identifier)));
        if (!bl_container_dynamic_is_valid(&jsonObjectMember->identifier))
            goto jsonObjectCopyErrorExit;

        jsonObjectMember->value = jsonCopyValue(&jsonObjectMember->value, jsonObjectMember->valueType);

        if (jsonCopyValueHasFailed(&jsonObjectMember->value, jsonObjectMember->valueType)) {
            bl_container_destroy(&jsonObjectMember->identifier);
            goto jsonObjectCopyErrorExit;
        }
    }

    return newObject;

jsonObjectCopyErrorExit:
    for (JsonObjectMember* member = bl_container_dynamic_front(&newObject); member < jsonObjectMember; member++) {
        bl_container_destroy(&member->identifier);
        switch (member->valueType) {
        case JsonTypeString:
            bl_container_destroy(&member->value);
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
    bl_container_destroy(&newObject);
    return newObject;
}

void jsonArrayDestroy(void* jsonArray) {
    if (!bl_container_dynamic_is_valid(jsonArray))
        return;
    for (JsonArrayMember* currentMember = bl_container_dynamic_front(jsonArray); currentMember < (JsonArrayMember*) bl_container_dynamic_end(jsonArray); currentMember++) {
        if (currentMember->valueType == JsonTypeString)
            bl_container_destroy(&currentMember->value.string);
        else if (currentMember->valueType == JsonTypeArray)
            jsonArrayDestroy(&currentMember->value.array);
        else if (currentMember->valueType == JsonTypeObject)
            jsonObjectDestroy(&currentMember->value.object);
    }
    bl_container_destroy(jsonArray);
}

void jsonObjectDestroy(void* jsonObject) {
    if (!bl_container_dynamic_is_valid(jsonObject))
        return;
    for (JsonObjectMember* currentMember = bl_container_dynamic_front(jsonObject); currentMember < (JsonObjectMember*) bl_container_dynamic_end(jsonObject); currentMember++) {
        bl_container_destroy(&currentMember->identifier);
        if (currentMember->valueType == JsonTypeString)
            bl_container_destroy(&currentMember->value.string);
        else if (currentMember->valueType == JsonTypeArray)
            jsonArrayDestroy(&currentMember->value.array);
        else if (currentMember->valueType == JsonTypeObject)
            jsonObjectDestroy(&currentMember->value.object);
    }
    bl_container_destroy(jsonObject);
}
