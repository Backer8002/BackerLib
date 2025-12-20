#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <BackerLibConcurrency.h>
#include <BackerLibEvent.h>
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

    typedef BL_DynamicContainer JsonObject;
    typedef BL_DynamicContainer JsonArray;

    typedef enum JsonObjectMemberType {
        JsonTypeInvalid,
        JsonTypeNull,
        JsonTypeBoolean,
        JsonTypeNumber,
        JsonTypeString,
        JsonTypeObject,
        JsonTypeArray
    } JsonMemberType;

    typedef union JsonObjectMemberValue {
        bool       boolean;
        double     number;
        BL_String     string;
        JsonObject object;
        JsonArray  array;
    } JsonMemberValue;

    typedef struct JsonObjectMember {
        BL_String          identifier;
        JsonMemberValue value;
        JsonMemberType  valueType;
    } JsonObjectMember;

    typedef struct JsonArrayMember {
        JsonMemberType  valueType;
        JsonMemberValue value;
    } JsonArrayMember;

    typedef struct JsonFormat {
        size_t amountOfSpacesForIndentation;
        bool   breakBeforeCurlyBracket;
        bool   breakBeforeAngleBracket;
        bool   breakAfterOpeningCurlyBracket;
        bool   breakAfterOpeningAngleBracket;
        bool   breakAfterCommaInObject;
        bool   breakAfterCommaInArray;
        bool   spaceBeforeComma;
        bool   spaceAfterComma;
        bool   spaceAfterColon;
        bool   spaceBeforeColon;
        bool   spaceWithinAngelBracket;
    } JsonFormat;

    typedef enum JsonTokenType {
        JsonTokenInvalid = 0,
        JsonTokenOpenCurlyBracket,
        JsonTokenOpenBracket,
        JsonTokenCloseBracket,
        JsonTokenCloseCurlyBracket,
        JsonTokenColon,
        JsonTokenComma,
        JsonTokenBool,
        JsonTokenNumber,
        JsonTokenString,
        JsonTokenNull
    } JsonTokenType;

    typedef struct JsonToken {
        JsonTokenType   tokenType;
        JsonMemberValue additionalData;
    } JsonToken;

    typedef struct JsonTokenStore {
        BL_DynamicContainer dynamicContainer;
        size_t           maxDepth;
    } JsonTokenStore;

    static const Event JsonFileIllFormated = {
        .id             = (const BL_StringView) stringViewInitConstExpr("JsonFileIllFormated"),
        .groupIds       = &ErrorLogLevel,
        .amountOfGroups = 1};


    typedef Future(JsonObject) FutureJsonObject;
    typedef asyncArgsPackType(FutureJsonObject, FILE*) JsonReadFilePack;
    typedef struct JsonWriteFileArgs {
        FILE*            file;
        const JsonObject object;
        const JsonFormat format;
    } JsonWriteFileArgs;

    typedef asyncArgsPackType(FutureVoid, JsonWriteFileArgs) JsonWriteFilePack;
    /**
     * @brief Tokenizes a file until opening json object is closed.
     * @param file File to read from
     * @return Invalid object if any allocation has failed, else Container of JsonToken with strings allocated.
     * @note An illformated file may read an arbitrary amount of chars from stream.
     */
    extern JsonTokenStore    jsonTokenizeFile(FILE* file) noexcept;

    /**
     * @brief Reads a json object from file until closing scope or undefined length if illformated.
     * @param file File to read from
     * @return JsonObject if allocations have been successful and file was well formated, else invalid object.
     */
    extern JsonObject        jsonReadFile(FILE* file) noexcept;
    /**
     * @brief Runs jsonReadFile asynchronously.
     * @param threadPool Pointer to valid ThreadPool
     * @param priority Priority to read with
     * @param file File to read from
     * @return NULL if job could not be amended, else pointer to future of object.
     */
    extern FutureJsonObject* jsonReadFileAsync(BL_ThreadPool* threadPool, size_t priority, FILE* file) noexcept;
    /**
     * @brief Writes object to file.
     * @param file File to write to
     * @param object Object to write
     * @param format Formating to use
     * @note An incomplete object will result in illformed json.
     */
    extern void              jsonWriteFile(FILE* file, const JsonObject* object, const JsonFormat* format) noexcept;
    /**
     * @brief Writes JsonObject to File asynchronusly.
     * @param threadPool Pointer to valid ThreadPool
     * @param priority Priority to write with
     * @param file File to write to
     * @param object Object to write
     * @param format Formating to use
     * @return FutureVoid indicating completeness if successful at amending job to queue, else NULL.
     */
    extern FutureVoid*      jsonWriteFileAsync(BL_ThreadPool* threadPool, size_t priority, FILE* file, const JsonObject* object, const JsonFormat* format) noexcept;
    /**
     * @brief Writes a debug view of a JsonMemberValue.
     * @param file File to write to
     * @param valueType Type of value
     * @param value Value to write
     * @note Output will be illformed JSON.
     */
    extern void              jsonWriteTreeStyle(FILE* file, JsonMemberType valueType, const JsonMemberValue* value) noexcept;
    /**
     * @brief Creates a JsonObject.
     * @return Valid JsonObject of size 0.
     */
    extern JsonObject        jsonObjectCreate(void) noexcept;
    /**
     * @brief Creates a JsonArray.
     * @return Valid JsonArray of size 0.
     */
    extern JsonArray         jsonArrayCreate(void) noexcept;
    /**
     * @brief Get a member from an JsonArray.
     * @param jsonArray Pointer to valid JsonArray
     * @param index Index in array
     * @return NULL if index is out of range, else pointer to array member.
     */
    extern JsonArrayMember*  jsonArrayMemberGet(const JsonArray* jsonArray, size_t index) noexcept;
    /**
     * @brief Get member from object.
     * @param jsonObject Pointer to valid JsonObject
     * @param identifier Key to search for
     * @return NULL if key does not exist in object, else pointer to member.
     */
    extern JsonObjectMember* jsonObjectMemberGet(const JsonObject* jsonObject, BL_StringView* identifier) noexcept;
    /**
     * @brief Adds a new member to JsonObject. Copying value.
     * @param jsonObject Pointer to valid JsonObject
     * @param identifier Key for value
     * @param valueType Type of value
     * @param value Value to store
     * @return ContainerAllocFailure if allocation fails.
     */
    extern BL_ContainerError    jsonObjectAdd(JsonObject* jsonObject, BL_StringView* identifier, JsonMemberType valueType, const JsonMemberValue* value) noexcept;
    /**
     * @brief Adds value to end of array. Copying value.
     * @param jsonArray Pointer to valid JsonArray
     * @param valueType Type of value
     * @param value Value to store
     * @return ContainerAllocFailure if allocation fails.
     */
    extern BL_ContainerError    jsonArrayAdd(JsonArray* jsonArray, JsonMemberType valueType, const JsonMemberValue* value) noexcept;
    /**
     * @brief Add value at index in array. Copying value.
     * @param jsonArray Pointer to valid JsonArray
     * @param index Index to store at
     * @param valueType Type of value
     * @param value Value to store
     * @return ContainerAllocFailure if allocation fails. ContainerInvalidIndex if index is beyond the size of container.
     */
    extern BL_ContainerError    jsonArrayAddAtIndex(JsonArray* jsonArray, size_t index, JsonMemberType valueType, const JsonMemberValue* value) noexcept;
    /**
     * @brief Removes member from object.
     * @param jsonObject Pointer to valid JsonObject
     * @param member Pointer to member in object
     * @note Function frees all associated and descending allocations of member in object. To save value you may set valueType to JsonTypeInvalid before calling this function.
     */
    extern void              jsonObjectRemove(JsonObject* jsonObject, JsonObjectMember* member) noexcept;
    /**
     * @brief Removes member from array.
     * @param jsonArray Pointer to valid JsonArray
     * @param index Index in array to remove
     * @return false if index was out of range, else true.
     * @note Function frees all associated and descending allocations of member in array. To save value you may set valueType to JsonTypeInvalid before calling this function.
     */
    extern bool              jsonArrayRemove(JsonArray* jsonArray, size_t index) noexcept;
    /**
     * @brief Deep-copies JsonArray.
     * @param jsonArray Pointer to valid JsonArray
     * @return Deepcopy of JsonArray and descending structures. Invalid object if allocation fails.
     */
    extern JsonArray         jsonArrayCopy(const JsonArray* jsonArray) noexcept;
    /**
     * @brief Deep-copies JsonObject.
     * @param jsonObject Pointer to valid JsonObject
     * @return Deepcopy of JsonObject and descending structures. Invalid object if allcation fails.
     */
    extern JsonObject        jsonObjectCopy(const JsonObject* jsonObject) noexcept;
    /**
     * @brief Destroys JsonArray and descending structures.
     * @param jsonArray Pointer to valid JsonArray
     */
    extern void              jsonArrayDestroy(void* jsonArray) noexcept;
    /**
     * @brief Destroys JsonObject and descending structures.
     * @param jsonObject Pointer to valid JsonObject
     */
    extern void              jsonObjectDestroy(void* jsonObject) noexcept;

#ifdef __cplusplus
    }
};
#endif
#endif // JSONPARSER_H
