#include "Json.h"
#include <BackerLibTypes.h>
#include <BackerLibLogging.h>
#include <stdio.h>

static BL_JsonMemberValue jsonCopyValue(const BL_JsonMemberValue* value, BL_JsonMemberType valueType) {
    switch (valueType) {
    case JsonTypeString:
        return (BL_JsonMemberValue) {.string = bl_string_create(bl_container_dynamic_front(&value->string),
            bl_string_length(bl_stringview_ptr_cast(&value->string)))};
        break;
    case JsonTypeArray:
        return (BL_JsonMemberValue) {.array = bl_json_array_copy(&value->array)};
        break;
    case JsonTypeObject:
        return (BL_JsonMemberValue) {.object = bl_json_object_copy(&value->object)};
        break;
    default:
        return *value;
    }
}

static bool jsonCopyValueHasFailed(const BL_JsonMemberValue* value, BL_JsonMemberType valueType) {
    if ((valueType == JsonTypeString || valueType == JsonTypeArray || valueType == JsonTypeObject) && !bl_container_dynamic_is_valid(&value->object))
        return true;
    return false;
}

BL_JsonObject       bl_json_object_create(void) { return bl_container_dynamic_create_stack(0, sizeof(BL_JsonObjectMember)); }

BL_JsonArray        bl_json_array_create(void) { return bl_container_dynamic_create_stack(0, sizeof(BL_JsonArrayMember)); }

BL_JsonArrayMember* bl_json_array_get(const BL_JsonArray* jsonArray, size_t index) { return bl_container_get((BL_Container*) jsonArray, index); }

BL_JsonObjectMember* bl_json_object_get(const BL_JsonObject* jsonObject, BL_StringView* string) {
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

BL_ContainerError bl_json_object_add(BL_JsonObject* jsonObject, BL_StringView* string, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) {
    bl_assert_debug(string->array, "Expected string");
    BL_String stringToUse = bl_dynamic_container_cast_container(bl_container_copy(&string->container));
    if (!bl_container_dynamic_is_valid(&stringToUse))
        return BL_ContainerAllocFailure;
    BL_JsonMemberValue valueToUse = jsonCopyValue(value, valueType);
    if (jsonCopyValueHasFailed(&valueToUse, valueType)) {
        bl_container_destroy(&stringToUse);
        return BL_ContainerAllocFailure;
    }

    if (bl_container_dynamic_append(jsonObject, sizeof(BL_JsonObjectMember), &(BL_JsonObjectMember) {.identifier = stringToUse, .value = valueToUse, .valueType = valueType}) != BL_ContainerOPSuccessful) {
        bl_container_destroy(&stringToUse);
        switch (valueType) {
        case JsonTypeArray:
            bl_json_array_destroy(&valueToUse);
            break;
        case JsonTypeObject:
            bl_json_object_destroy(&valueToUse);
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

BL_ContainerError bl_json_array_add(BL_JsonArray* jsonArray, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) {
    return bl_json_array_add_at_index(jsonArray, bl_container_size((BL_Container*) jsonArray), valueType, value);
}

BL_ContainerError bl_json_array_add_at_index(BL_JsonArray* jsonArray, size_t index, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) {
    BL_JsonMemberValue valueToUse = jsonCopyValue(value, valueType);
    if (jsonCopyValueHasFailed(&valueToUse, valueType))
        return BL_ContainerAllocFailure;
    return bl_container_dynamic_insert(jsonArray, index, 1, sizeof(BL_JsonArrayMember), &(BL_JsonArrayMember) {.valueType = valueType, .value = valueToUse});
}

void bl_json_object_remove(BL_JsonObject* jsonObject, BL_JsonObjectMember* member) {

    bl_container_destroy(&member->identifier);

    switch (member->valueType) {
    case JsonTypeArray:
        bl_json_array_destroy(&member->value);
        break;
    case JsonTypeObject:
        bl_json_object_destroy(&member->value);
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

bool bl_json_array_remove(BL_JsonArray* jsonArray, size_t index) {
    if (index >= bl_container_size((BL_Container*) jsonArray))
        return false;

    BL_JsonArrayMember* jsonArrayMember = bl_json_array_get(jsonArray, index);

    switch (jsonArrayMember->valueType) {
    case JsonTypeArray:
        bl_json_array_destroy(&jsonArrayMember->value);
        break;
    case JsonTypeObject:
        bl_json_object_destroy(&jsonArrayMember->value);
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

BL_JsonArray bl_json_array_copy(const BL_JsonArray* jsonArray) {
    BL_JsonArray newArray = bl_dynamic_container_cast_container(
        bl_container_get_subarray((const BL_Container*) jsonArray,
                             0,
                             bl_container_size((const BL_Container*) jsonArray) - 1,
                             false));
    if (!bl_container_dynamic_is_valid(&newArray))
        return newArray;
    BL_JsonArrayMember* jsonArrayMember = NULL;
    for (jsonArrayMember = bl_container_dynamic_front(&newArray); jsonArrayMember < (BL_JsonArrayMember*)bl_container_dynamic_end(&newArray); jsonArrayMember++) {
        jsonArrayMember->value = jsonCopyValue(&jsonArrayMember->value, jsonArrayMember->valueType);
        if (jsonCopyValueHasFailed(&jsonArrayMember->value, jsonArrayMember->valueType))
            goto jsonArrayCopyErrorExit;
    }
    return newArray;

jsonArrayCopyErrorExit:
    for (BL_JsonArrayMember* member = bl_container_dynamic_front(&newArray); member < jsonArrayMember; member++) {
        switch (member->valueType) {
        case JsonTypeString:
            bl_container_destroy(&member->value);
            break;
        case JsonTypeArray:
            bl_json_array_destroy(&member->value);
            break;
        case JsonTypeObject:
            bl_json_object_destroy(&member->value);
            break;
        default:
            break;
        }
    }
    bl_container_destroy(&newArray);
    return newArray;
}

BL_JsonObject bl_json_object_copy(const BL_JsonObject* jsonObject) {
    BL_JsonObject newObject = bl_dynamic_container_cast_container(
        bl_container_get_subarray((const BL_Container*) jsonObject,
                             0,
                             bl_container_size((const BL_Container*) jsonObject) - 1,
                             false));
    if (!bl_container_dynamic_is_valid(&newObject))
        return newObject;
    BL_JsonObjectMember* jsonObjectMember = NULL;
    for (jsonObjectMember = bl_container_dynamic_front(&newObject); jsonObjectMember < (BL_JsonObjectMember*)bl_container_dynamic_end(&newObject); jsonObjectMember++) {
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
    for (BL_JsonObjectMember* member = bl_container_dynamic_front(&newObject); member < jsonObjectMember; member++) {
        bl_container_destroy(&member->identifier);
        switch (member->valueType) {
        case JsonTypeString:
            bl_container_destroy(&member->value);
            break;
        case JsonTypeArray:
            bl_json_array_destroy(&member->value);
            break;
        case JsonTypeObject:
            bl_json_object_destroy(&member->value);
            break;
        default:
            break;
        }
    }
    bl_container_destroy(&newObject);
    return newObject;
}

void bl_json_array_destroy(void* jsonArray) {
    if (!bl_container_dynamic_is_valid(jsonArray))
        return;
    for (BL_JsonArrayMember* currentMember = bl_container_dynamic_front(jsonArray); currentMember < (BL_JsonArrayMember*) bl_container_dynamic_end(jsonArray); currentMember++) {
        if (currentMember->valueType == JsonTypeString)
            bl_container_destroy(&currentMember->value.string);
        else if (currentMember->valueType == JsonTypeArray)
            bl_json_array_destroy(&currentMember->value.array);
        else if (currentMember->valueType == JsonTypeObject)
            bl_json_object_destroy(&currentMember->value.object);
    }
    bl_container_destroy(jsonArray);
}

void bl_json_object_destroy(void* jsonObject) {
    if (!bl_container_dynamic_is_valid(jsonObject))
        return;
    for (BL_JsonObjectMember* currentMember = bl_container_dynamic_front(jsonObject); currentMember < (BL_JsonObjectMember*) bl_container_dynamic_end(jsonObject); currentMember++) {
        bl_container_destroy(&currentMember->identifier);
        if (currentMember->valueType == JsonTypeString)
            bl_container_destroy(&currentMember->value.string);
        else if (currentMember->valueType == JsonTypeArray)
            bl_json_array_destroy(&currentMember->value.array);
        else if (currentMember->valueType == JsonTypeObject)
            bl_json_object_destroy(&currentMember->value.object);
    }
    bl_container_destroy(jsonObject);
}
