#include "Json.h"
#include <BackerLibEvent.h>
#include <BackerLibTypes.h>
#include <stdio.h>

static void internal_printIndentation(FILE* file, size_t indentation) {
    for (size_t i = 0; i < indentation; i++)
        fputc(' ', file);
}

static void internal_printJsonString(FILE* file, const BL_String* string) {
    fputc('\"',file);
    for (const BL_Byte* currentChar = bl_container_dynamic_front(string); currentChar < (const BL_Byte*) bl_container_dynamic_end(string); currentChar++) {
        switch (*currentChar) {
        case '\0':
            break;
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
    fputc('\"',file);
}

static void internal_printTreePrepareNext(FILE* file, const size_t depth) {
    for (size_t i = 0; i < depth; i++)
        fprintf(file, "|   ");
    fprintf(file,"|\n");
    for (size_t i = 0; i < depth; i++)
        fprintf(file, "|   ");
    fprintf(file,"|- ");
}

static void jsonWriteObject(FILE* file, const JsonObject* object, const JsonFormat* format, size_t indentation, bool isFirstInArrayScope);

static void jsonWriteArray(FILE* file, const JsonArray* array, const JsonFormat* format, size_t indentation, bool isFirstInArrayScope) {
    if (format->breakBeforeAngleBracket && !isFirstInArrayScope) {
        fputc('\n', file);
        internal_printIndentation(file, indentation);
    }
    fputc('[', file);
    if (format->spaceWithinAngelBracket)
        fputc(' ',file);
    if (format->breakAfterOpeningAngleBracket) {
        fputc('\n', file);
        internal_printIndentation(file, indentation + format->amountOfSpacesForIndentation);
    }

    for (JsonArrayMember* currentMember = bl_container_dynamic_front(array); currentMember < (JsonArrayMember*) bl_container_dynamic_end(array); currentMember++) {
        switch (currentMember->valueType) {
        case JsonTypeArray:
            jsonWriteArray(file, &currentMember->value.array, format, indentation + format->amountOfSpacesForIndentation, currentMember == bl_container_dynamic_front(array));
            break;
        case JsonTypeObject:
            jsonWriteObject(file, &currentMember->value.object, format, indentation + format->amountOfSpacesForIndentation, currentMember == bl_container_dynamic_front(array));
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
        if (currentMember != bl_container_dynamic_back(array)) {
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
    if (format->spaceWithinAngelBracket)
        fputc(' ',file);
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

    for (JsonObjectMember* currentMember = bl_container_dynamic_front(object); currentMember < (JsonObjectMember*) bl_container_dynamic_end(object); currentMember++) {
        internal_printJsonString(file, &currentMember->identifier);
        if (format->spaceBeforeColon)
            fputc(' ', file);
        fputc(':', file);
        if (format->spaceAfterColon)
            fputc(' ', file);

        switch (currentMember->valueType) {
        case JsonTypeArray:
            jsonWriteArray(file, &currentMember->value.array, format, indentation + format->amountOfSpacesForIndentation, false);
            break;
        case JsonTypeObject:
            jsonWriteObject(file, &currentMember->value.object, format, indentation + format->amountOfSpacesForIndentation, false);
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
            LogError("Uninitialized JSON found.");
            break;
        }
        if (currentMember != bl_container_dynamic_back(object)) {
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
    jsonWriteObject(file, object, format, 0, true); // it is first in the global scope, no need for newline in the begining
    fputc('\n', file);
}



static void jsonWriteFileThread(void* sharedState) {
    JsonWriteFilePack* information = sharedState;
    jsonWriteFile(information->args.file,&information->args.object, &information->args.format);
    information->future = true;
}

FutureVoid* jsonWriteFileAsync(BL_ThreadPool* threadPool, size_t priority, FILE* file, const JsonObject* object, const JsonFormat* format) {
    return bl_threadpool_job_assign(threadPool,
        priority,
        jsonWriteFileThread,
        asyncArgsFutureOffset(JsonWriteFilePack),
        &(JsonWriteFileArgs){.file = file, .object = *object, .format = *format},sizeof(JsonWriteFileArgs),
        asyncArgsOffset(JsonWriteFilePack));
}


static void jsonWriteTree(FILE* file,JsonMemberType valueType,const JsonMemberValue* value,size_t depth) {
    switch (valueType) {
    case JsonTypeBoolean:
        fprintf(file, "[[Boolean]] %s\n",value->boolean ? "true" : "false");
        break;
    case JsonTypeNull:
        fprintf(file,"[[Null statement]] null\n");
        break;
    case JsonTypeNumber:
        fprintf(file,"[[Number]] %lf (decimal notation), %le (scientific notation)\n",value->number, value->number);
        break;
    case JsonTypeInvalid:
        fprintf(file,"[[invalid member]]\n");
        break;
    case JsonTypeString:
        fprintf(file,"[[String literal]] ");
        internal_printJsonString(file, &value->string);
        fputc('\n',file);
        break;
    case JsonTypeArray:
        fprintf(file, "[[Array]]\n");
        for (JsonArrayMember* currentMember = bl_container_dynamic_front(&value->array); currentMember < (JsonArrayMember*)bl_container_dynamic_end(&value->array); currentMember++) {
            internal_printTreePrepareNext(file, depth);
            jsonWriteTree(file, currentMember->valueType, &currentMember->value,depth + 1);
        }
        break;
    case JsonTypeObject:
        fprintf(file, "[[Object]]\n");
        for (JsonObjectMember* currentMember = bl_container_dynamic_front(&value->array); currentMember < (JsonObjectMember*)bl_container_dynamic_end(&value->array); currentMember++) {
            internal_printTreePrepareNext(file, depth);
            internal_printJsonString(file,&currentMember->identifier);
            fprintf(file, " -> ");
            jsonWriteTree(file, currentMember->valueType, &currentMember->value,depth + 1);
        }
        break;
        default:
        fprintf(file, "[[Invalid member]]\n");
    }
}

void jsonWriteTreeStyle(FILE* file,JsonMemberType valueType,const JsonMemberValue* value) {
    jsonWriteTree(file,valueType,value,0);
}
