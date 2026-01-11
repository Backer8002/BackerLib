#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <BackerLibConcurrency.h>
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif

    typedef BL_DynamicContainer BL_JsonObject;
    typedef BL_DynamicContainer BL_JsonArray;

    typedef enum BL_JsonMemberType {
        JsonTypeInvalid,
        JsonTypeNull,
        JsonTypeBoolean,
        JsonTypeNumber,
        JsonTypeString,
        JsonTypeObject,
        JsonTypeArray
    } BL_JsonMemberType;

    typedef union BL_JsonMemberValue {
        bool       boolean;
        double     number;
        BL_String     string;
        BL_JsonObject object;
        BL_JsonArray  array;
    } BL_JsonMemberValue;

    typedef struct BL_JsonObjectMember {
        BL_String          identifier;
        BL_JsonMemberValue value;
        BL_JsonMemberType  valueType;
    } BL_JsonObjectMember;

    typedef struct BL_JsonArrayMember {
        BL_JsonMemberType  valueType;
        BL_JsonMemberValue value;
    } BL_JsonArrayMember;

    typedef struct BL_JsonFormat {
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
    } BL_JsonFormat;

    typedef enum BL_JsonTokenType {
        BL_JsonTokenInvalid = 0,
        BL_JsonTokenOpenCurlyBracket,
        BL_JsonTokenOpenBracket,
        BL_JsonTokenCloseBracket,
        BL_JsonTokenCloseCurlyBracket,
        BL_JsonTokenColon,
        BL_JsonTokenComma,
        BL_JsonTokenBool,
        BL_JsonTokenNumber,
        BL_JsonTokenString,
        BL_JsonTokenNull
    } BL_JsonTokenType;

    typedef struct BL_JsonToken {
        BL_JsonTokenType   tokenType;
        BL_JsonMemberValue additionalData;
    } BL_JsonToken;

    typedef struct BL_JsonTokenStore {
        BL_DynamicContainer dynamicContainer;
        size_t           maxDepth;
    } BL_JsonTokenStore;


    typedef BL_Future(BL_JsonObject) BL_FutureJsonObject;
    typedef BL_AsyncArgsType(BL_FutureJsonObject, FILE*) BL_JsonReadFilePack;

    typedef struct BL_JsonWriteFileArgs {
        FILE*            file;
        const BL_JsonObject object;
        const BL_JsonFormat format;
    } BL_JsonWriteFileArgs;

    typedef BL_AsyncArgsType(BL_FutureVoid, BL_JsonWriteFileArgs) BL_JsonWriteFilePack;

    /**
     * @brief Tokenizes a file until opening json object is closed.
     * @param file File to read from
     * @return Invalid object if any allocation has failed, else Container of BL_JsonToken with strings allocated.
     * @note An illformated file may read an arbitrary amount of chars from stream.
     */
    extern BL_JsonTokenStore    bl_json_tokenize_file(FILE* file) noexcept;

    /**
     * @brief Reads a json object from file until closing scope or undefined length if illformated.
     * @param file File to read from
     * @return BL_JsonObject if allocations have been successful and file was well formated, else invalid object.
     */
    extern BL_JsonObject        bl_json_read_file(FILE* file) noexcept;
    /**
     * @brief Runs bl_json_read_file asynchronously.
     * @param threadPool Pointer to valid ThreadPool
     * @param priority Priority to read with
     * @param file File to read from
     * @return NULL if job could not be amended, else pointer to future of object.
     */
    extern BL_FutureJsonObject* bl_json_read_file_threaded(BL_ThreadPool* threadPool, size_t priority, FILE* file) noexcept;
    /**
     * @brief Writes object to file.
     * @param file File to write to
     * @param object Object to write
     * @param format Formating to use
     * @note An incomplete object will result in illformed json.
     */
    extern void              bl_json_write_file(FILE* file, const BL_JsonObject* object, const BL_JsonFormat* format) noexcept;
    /**
     * @brief Writes BL_JsonObject to File asynchronusly.
     * @param threadPool Pointer to valid ThreadPool
     * @param priority Priority to write with
     * @param file File to write to
     * @param object Object to write
     * @param format Formating to use
     * @return BL_FutureVoid indicating completeness if successful at amending job to queue, else NULL.
     */
    extern BL_FutureVoid*      bl_json_write_file_threaded(BL_ThreadPool* threadPool, size_t priority, FILE* file, const BL_JsonObject* object, const BL_JsonFormat* format) noexcept;
    /**
     * @brief Writes a debug view of a BL_JsonMemberValue.
     * @param file File to write to
     * @param valueType Type of value
     * @param value Value to write
     * @note Output will be illformed JSON.
     */
    extern void              bl_json_write_tree_style(FILE* file, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) noexcept;
    /**
     * @brief Creates a BL_JsonObject.
     * @return Valid BL_JsonObject of size 0.
     */
    extern BL_JsonObject        bl_json_object_create(void) noexcept;
    /**
     * @brief Creates a BL_JsonArray.
     * @return Valid BL_JsonArray of size 0.
     */
    extern BL_JsonArray         bl_json_array_create(void) noexcept;
    /**
     * @brief Get a member from an BL_JsonArray.
     * @param jsonArray Pointer to valid BL_JsonArray
     * @param index Index in array
     * @return NULL if index is out of range, else pointer to array member.
     */
    extern BL_JsonArrayMember*  bl_json_array_get(const BL_JsonArray* jsonArray, size_t index) noexcept;
    /**
     * @brief Get member from object.
     * @param jsonObject Pointer to valid BL_JsonObject
     * @param identifier Key to search for
     * @return NULL if key does not exist in object, else pointer to member.
     */
    extern BL_JsonObjectMember* bl_json_object_get(const BL_JsonObject* jsonObject, BL_StringView* identifier) noexcept;
    /**
     * @brief Adds a new member to BL_JsonObject. Copying value.
     * @param jsonObject Pointer to valid BL_JsonObject
     * @param identifier Key for value
     * @param valueType Type of value
     * @param value Value to store
     * @return ContainerAllocFailure if allocation fails.
     */
    extern BL_ContainerError    bl_json_object_add(BL_JsonObject* jsonObject, BL_StringView* identifier, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) noexcept;
    /**
     * @brief Adds value to end of array. Copying value.
     * @param jsonArray Pointer to valid BL_JsonArray
     * @param valueType Type of value
     * @param value Value to store
     * @return ContainerAllocFailure if allocation fails.
     */
    extern BL_ContainerError    bl_json_array_add(BL_JsonArray* jsonArray, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) noexcept;
    /**
     * @brief Add value at index in array. Copying value.
     * @param jsonArray Pointer to valid BL_JsonArray
     * @param index Index to store at
     * @param valueType Type of value
     * @param value Value to store
     * @return ContainerAllocFailure if allocation fails. ContainerInvalidIndex if index is beyond the size of container.
     */
    extern BL_ContainerError    bl_json_array_add_at_index(BL_JsonArray* jsonArray, size_t index, BL_JsonMemberType valueType, const BL_JsonMemberValue* value) noexcept;
    /**
     * @brief Removes member from object.
     * @param jsonObject Pointer to valid BL_JsonObject
     * @param member Pointer to member in object
     * @note Function frees all associated and descending allocations of member in object. To save value you may set valueType to JsonTypeInvalid before calling this function.
     */
    extern void              bl_json_object_remove(BL_JsonObject* jsonObject, BL_JsonObjectMember* member) noexcept;
    /**
     * @brief Removes member from array.
     * @param jsonArray Pointer to valid BL_JsonArray
     * @param index Index in array to remove
     * @return false if index was out of range, else true.
     * @note Function frees all associated and descending allocations of member in array. To save value you may set valueType to JsonTypeInvalid before calling this function.
     */
    extern bool              bl_json_array_remove(BL_JsonArray* jsonArray, size_t index) noexcept;
    /**
     * @brief Deep-copies BL_JsonArray.
     * @param jsonArray Pointer to valid BL_JsonArray
     * @return Deepcopy of BL_JsonArray and descending structures. Invalid object if allocation fails.
     */
    extern BL_JsonArray         bl_json_array_copy(const BL_JsonArray* jsonArray) noexcept;
    /**
     * @brief Deep-copies BL_JsonObject.
     * @param jsonObject Pointer to valid BL_JsonObject
     * @return Deepcopy of BL_JsonObject and descending structures. Invalid object if allcation fails.
     */
    extern BL_JsonObject        bl_json_object_copy(const BL_JsonObject* jsonObject) noexcept;
    /**
     * @brief Destroys BL_JsonArray and descending structures.
     * @param jsonArray Pointer to valid BL_JsonArray
     */
    extern void              bl_json_array_destroy(void* jsonArray) noexcept;
    /**
     * @brief Destroys BL_JsonObject and descending structures.
     * @param jsonObject Pointer to valid BL_JsonObject
     */
    extern void              bl_json_object_destroy(void* jsonObject) noexcept;

#ifdef __cplusplus
}
#endif
#endif // JSONPARSER_H
