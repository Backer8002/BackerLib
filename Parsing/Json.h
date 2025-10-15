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


#endif

    typedef DynamicContainer JsonObject;
    typedef DynamicContainer JsonArray;

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
        String     string;
        JsonObject object;
        JsonArray  array;
    } JsonMemberValue;

    typedef struct JsonObjectMember {
        String                identifier;
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
        JsonTokenType         tokenType;
        JsonMemberValue additionalData;
    } JsonToken;

    typedef struct JsonTokenStore {
        DynamicContainer dynamicContainer;
        size_t           maxDepth;
    } JsonTokenStore;

    static const Event JsonFileIllFormated = {
        .id             = (StringView) stringViewInitConstExpr("JsonFileIllFormated"),
        .groupIds       = &ErrorLogLevel,
        .amountOfGroups = 1};

    typedef Future(JsonObject) FutureJsonObject;

    extern JsonTokenStore           jsonTokenizeFile(FILE* file);

    extern JsonObject               jsonReadFile(FILE* file);

    extern FutureJsonObject*         jsonReadFileAsync(ThreadPool* threadPool,size_t priority,FILE* file);

    extern void                     jsonWriteFile(FILE* file, const JsonObject* object, const JsonFormat* format);

    extern FutureVoid*               jsonWriteFileAsync(ThreadPool* threadPool,size_t priority, FILE* file,const JsonObject* object, const JsonFormat* format);

    extern void                     jsonWriteTreeStyle(FILE* file,JsonMemberType valueType,const JsonMemberValue* value);

    static inline JsonObject        jsonObjectCreate(void) { return containerDynamicCreateStack(0, sizeof(JsonObjectMember), false); }

    static inline JsonArray         jsonArrayCreate(void) { return containerDynamicCreateStack(0, sizeof(JsonArrayMember), false); }

    static inline JsonArrayMember*  jsonArrayMemberGet(const JsonArray* jsonArray, size_t index) { return containerGet((Container*) jsonArray, index); }

    extern JsonObjectMember*        jsonObjectMemberGet(const JsonObject* jsonObject, StringView* identifier);

    extern ContainerError           jsonObjectAdd(JsonObject* jsonObject, StringView* identifier, JsonMemberType valueType, const JsonMemberValue* value);

    extern ContainerError           jsonArrayAdd(JsonArray* jsonArray, JsonMemberType valueType, const JsonMemberValue* value);

    extern ContainerError           jsonArrayAddAtIndex(JsonArray* jsonArray, size_t index, JsonMemberType valueType, const JsonMemberValue* value);

    extern void                     jsonObjectRemove(JsonObject* jsonObject, JsonObjectMember* member);

    extern bool                     jsonArrayRemove(JsonArray* jsonArray, size_t index);

    extern JsonArray                jsonArrayCopy(const JsonArray* jsonArray);

    extern JsonObject               jsonObjectCopy(const JsonObject* jsonObject);

    extern void                     jsonArrayDestroy(void* jsonArray);

    extern void                     jsonObjectDestroy(void* jsonObject);

#ifdef __cplusplus
    }
};
#endif
#endif // JSONPARSER_H
