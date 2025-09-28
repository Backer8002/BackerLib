#include "JsonParser.h"
#include <BackerLibEvent.h>
#include <BackerLibTypes.h>
#include <stdio.h>

static void internal_printIndentation(FILE* file, size_t indentation) {
    for (size_t i = 0; i < indentation; i++)
        fputc(' ', file);
}

static void internal_printJsonString(FILE* file, const String* string) {
    for (const Byte* currentChar = containerDynamicFront(string); currentChar < (const Byte*) containerDynamicBack(string); currentChar++) { // Back not end since we don't want to write null terminator,
        switch (*currentChar) {
        case '\n':
            fprintf(file, "\\n");
            break;
        case '\b':
            fprintf(file, "\\b");
            break;
        case '\t':
            fprintf(file, "\\t");
            break;
        case '\f':
            fprintf(file, "\\f");
            break;
        case '\\':
            fprintf(file, "\\\\");
            break;
        case '/':
            fprintf(file, "\\/");
            break;
        case '\r':
            fprintf(file, "\\r");
            break;
        case '\"':
            fprintf(file, "\\\"");
            break;
        default:
            fputc(*currentChar, file);
        }
    }
}

static void jsonWriteObject(FILE* file, const JsonObject* object, const JsonFormat* format, size_t indentation, bool isFirstInArrayScope);

static void jsonWriteArray(FILE* file, const JsonArray* array, const JsonFormat* format, size_t indentation, bool isFirstInArrayScope) {
    if (format->breakBeforeAngleBracket && !isFirstInArrayScope) {
        fputc('\n', file);
        internal_printIndentation(file, indentation);
    }
    fputc('[', file);
    if (format->breakAfterOpeningAngleBracket) {
        fputc('\n', file);
        internal_printIndentation(file, indentation + format->amountOfSpacesForIndentation);
    }

    for (JsonArrayMember* currentMember = containerDynamicFront(array); currentMember < (JsonArrayMember*) containerDynamicEnd(array); currentMember++) {
        switch (currentMember->valueType) {
        case JsonTypeArray:
            jsonWriteArray(file, &currentMember->value.array, format, indentation + format->amountOfSpacesForIndentation,currentMember == containerDynamicFront(array));
            break;
        case JsonTypeObject:
            jsonWriteObject(file, &currentMember->value.object, format, indentation + format->amountOfSpacesForIndentation, currentMember == containerDynamicFront(array));
            break;
        case JsonTypeNull:
            fprintf(file, "null");
            break;
        case JsonTypeBoolean:
            fprintf(file, "%s", currentMember->value.boolean ? "true" : "false");
            break;
        case JsonTypeNumber:
            fprintf(file, "%.17le", currentMember->value.number);
            break;
        case JsonTypeString:
            internal_printJsonString(file, &currentMember->value.string);
            break;
        default:
            LogError("Uninitialized JSON found.");
            break;
        }
        if (currentMember != containerDynamicBack(array)) {
            if (format->spaceBeforeComma)
                fputc(' ', file);
            fputc(',', file);
            if (format->spaceAfterComma)
                fputc(' ', file);
            if (format->breakAfterCommaInArray) {
                fputc('\n', file);
                internal_printIndentation(file, indentation + format->amountOfSpacesForIndentation);
            }
        }
    }
    if (format->breakBeforeAngleBracket) {
        fputc('\n', file);
        internal_printIndentation(file, indentation);
    }
    fputc(']', file);
}

static void jsonWriteObject(FILE* file, const JsonObject* object, const JsonFormat* format, size_t indentation, bool isFirstInArrayScope) {
    if (format->breakBeforeCurlyBracket && !isFirstInArrayScope) {
        fputc('\n', file);
        internal_printIndentation(file, indentation);
    }
    fputc('{', file);
    if (format->breakAfterOpeningCurlyBracket) {
        fputc('\n', file);
        internal_printIndentation(file, indentation + format->amountOfSpacesForIndentation);
    }

    for (JsonObjectMember* currentMember = containerDynamicFront(object); currentMember < (JsonObjectMember*) containerDynamicEnd(object); currentMember++) {
        internal_printJsonString(file, &currentMember->identifier);
        if (format->spaceBeforeColon)
            fputc(' ', file);
        fputc(':', file);
        if (format->spaceAfterColon)
            fputc(' ', file);

        switch (currentMember->valueType) {
        case JsonTypeArray:
            jsonWriteArray(file, &currentMember->value.array, format, indentation + format->amountOfSpacesForIndentation,false);
            break;
        case JsonTypeObject:
            jsonWriteObject(file, &currentMember->value.object, format, indentation + format->amountOfSpacesForIndentation,false);
            break;
        case JsonTypeNull:
            fprintf(file, "null");
            break;
        case JsonTypeBoolean:
            fprintf(file, "%s", currentMember->value.boolean ? "true" : "false");
            break;
        case JsonTypeNumber:
            fprintf(file, "%le", currentMember->value.number);
            break;
        case JsonTypeString:
            internal_printJsonString(file, &currentMember->value.string);
            break;
        default:
            LogError("Uninitialized JSON found. Stopped printing scope.");
            return;
        }
        if (currentMember != containerDynamicBack(object)) {
            if (format->spaceBeforeComma)
                fputc(' ', file);
            fputc(',', file);
            if (format->spaceAfterComma)
                fputc(' ', file);
            if (format->breakAfterCommaInObject) {
                fputc('\n', file);
                internal_printIndentation(file, indentation + format->amountOfSpacesForIndentation);
            }
        }
    }
    if (format->breakBeforeCurlyBracket) {
        fputc('\n', file);
        internal_printIndentation(file, indentation);
    }
    fputc('}', file);
}

void jsonWriteFile(FILE* file, const JsonObject* object, const JsonFormat* format) {
    jsonWriteObject(file, object, format, 0,true); // it is first in the global scope, no need for newline in the begining
    fputc('\n', file);
}
