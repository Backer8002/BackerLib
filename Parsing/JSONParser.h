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
} JsonObjectMemberType;

typedef union JsonObjectMemberValue {
    bool       boolean;
    double     number;
    String string;
    JsonObject object;
    JsonArray  array;
} JsonObjectMemberValue;

typedef struct JsonObjectMember {
    String           identifier;
    JsonObjectMemberValue value;
    JsonObjectMemberType  valueType;
} JsonObjectMember;

typedef struct JsonArrayMember {
    JsonObjectMemberType  valueType;
    JsonObjectMemberValue value;
} JsonArrayMember;

typedef struct JsonFormat {
    size_t amountOfSpacesForIndentation;
    bool   indentWithTabs;
    bool   breakBeforeCurlyBracket;
    bool   breakAfterCurlyBracket;
    bool   breakBeforeAngleBracket;
    bool   breakAfterAngleBracket;
    bool   breakBeforeComma;
    bool   breakAfterComma;
    bool   spaceBeforeComma;
    bool   spaceAfterComma;
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
    JsonObjectMemberValue additionalData;
} JsonToken;

static const Event JsonFileIllFormated = {
    .id = (StringView) stringViewInitConstExpr("JsonFileIllFormated"),
    .groupIds = &ErrorLogLevel,
    .amountOfGroups = 1};

typedef Future(JsonObject) FutureJsonObject;

extern DynamicContainer jsonTokenizeFile(FILE* file);

extern JsonObject jsonReadFile(FILE* file);

extern FutureJsonObject jsonReadFileAsync(ThreadPool* threadPool, FILE* file);

extern void jsonWriteFile(FILE* file, JsonObject* object, JsonFormat format);

extern FutureVoid jsonWriteFileAsync(ThreadPool* threadPool, FILE* file, JsonObject* object, JsonFormat format);

extern void jsonWriteConsoleStyle(FILE* file, JsonObjectMember* object);

extern void jsonObjectDestroy(void* jsonObject);

#ifdef __cplusplus
}
};
#endif
#endif // JSONPARSER_H