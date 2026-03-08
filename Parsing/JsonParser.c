#include "Json.h"
#include <BackerLibLogging.h>
#include <BackerLibTextprocessing.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void internal_tokenStorageDestructor(void* element) {
    BL_JsonToken* jsonElement = element;
    if (jsonElement->tokenType == BL_JsonTokenString)
        bl_container_destroy(&jsonElement->additionalData);
}

static double internal_parseNumber(const double firstDigit, FILE* file) {
    double parsedNum       = firstDigit == INFINITY ? 0 : firstDigit;
    double parsedExponent  = 0;
    bool   hitDecimalPoint = false, hitE = false, exponentIsNegative = false;
    size_t decimalDepth = 1;
    while (1) {
        int currentChar = fgetc(file);
        if (feof(file))
            return NAN;

        if (currentChar == '}' || currentChar == ']' || currentChar == ',' || isspace(currentChar)) {
            if (firstDigit == INFINITY)
                parsedNum *= -1.0f;

            if (exponentIsNegative)
                parsedExponent *= -1.0f;
            if (hitE)
                parsedNum = parsedNum * pow(10, parsedExponent);
            if (fseek(file, -1L, SEEK_CUR) == -1)
                return NAN;
            return parsedNum;
        }

        if (currentChar == 'e' || currentChar == 'E') {
            if (hitE)
                return NAN;
            hitE        = true;
            currentChar = fgetc(file);

            if (!isdigit(currentChar) && currentChar != '-' && currentChar != '+')
                return NAN;

            if (isdigit(currentChar))
                parsedExponent = currentChar - '0';
            else if (currentChar == '-')
                exponentIsNegative = true;
            hitDecimalPoint = false;
            decimalDepth    = 1;
            continue;
        }

        if (currentChar == '.') {
            if (hitDecimalPoint)
                return NAN;
            hitDecimalPoint = true;
            continue;
        }
        if (!isdigit(currentChar))
            return NAN;

        if (hitDecimalPoint) {
            if (hitE)
                parsedExponent += (currentChar - '0') / pow(10, (double) decimalDepth);
            else
                parsedNum += (currentChar - '0') / pow(10, (double) decimalDepth);
            decimalDepth++;
        } else {
            if (hitE) {
                parsedExponent *= 10.0;
                parsedExponent += currentChar - '0';
            } else {
                parsedNum *= 10.0;
                parsedNum += currentChar - '0';
            }
        }
    }
}

static BL_Textprocessing_UTFCodepoint internal_handleUnicodeEscape(FILE* file) {
    int firstChar  = fgetc(file);
    int secondChar = fgetc(file);
    int thirdChar  = fgetc(file);
    int fourthChar = fgetc(file);
    if (feof(file))
        return (BL_Textprocessing_UTFCodepoint) {0};
    if (!isxdigit(firstChar) || !isxdigit(secondChar) || !isxdigit(thirdChar) || !isxdigit(fourthChar))
        return (BL_Textprocessing_UTFCodepoint) {0};

    BL_Byte codepoint[2];

    codepoint[0] = strtoul((char[3]) {(char) firstChar, (char) secondChar, '\0'}, NULL, 16);
    codepoint[1] = strtoul((char[3]) {(char) thirdChar, (char) fourthChar, '\0'}, NULL, 16);

    if ((codepoint[0] & 0xfc) == 0xd8) {
        BL_Byte codepointSecondPart[2];
        if (fgetc(file) != '\\')
            return (BL_Textprocessing_UTFCodepoint) {0};
        if (fgetc(file) != 'u')
            return (BL_Textprocessing_UTFCodepoint) {0};
        int fifthChar   = fgetc(file);
        int sixthChar   = fgetc(file);
        int seventhChar = fgetc(file);
        int eighthChar  = fgetc(file);
        if (!isxdigit(fifthChar) || !isxdigit(sixthChar) || !isxdigit(seventhChar) || !isxdigit(eighthChar))
            return (BL_Textprocessing_UTFCodepoint) {0};
        codepointSecondPart[0] = strtoul((char[3]) {(char) fifthChar, (char) sixthChar, '\0'}, NULL, 16);
        codepointSecondPart[1] = strtoul((char[3]) {(char) seventhChar, (char) eighthChar, '\0'}, NULL, 16);

        return bl_textprocessing_transcode_utfcodepoint(
            (BL_Textprocessing_UTFCodepoint) {
                .bytesUsed = 4,
                .bytes     = {codepoint[0], codepoint[1], codepointSecondPart[0], codepointSecondPart[1]}},
            BL_Textprocessing_Encoding_UTF16BE,
            BL_Textprocessing_Encoding_UTF8);
    }
    return bl_textprocessing_transcode_utfcodepoint((BL_Textprocessing_UTFCodepoint) {.bytesUsed = 2, .bytes = {codepoint[0], codepoint[1]}}, BL_Textprocessing_Encoding_UTF16BE, BL_Textprocessing_Encoding_UTF8);
}

static BL_String internal_parseUTF8String(FILE* file) {
    BL_String string = bl_container_dynamic_create_stack(0, sizeof(char));
    if (!bl_container_dynamic_is_valid(&string))
        return string;

    while (1) {
        BL_Byte currentChar = (BL_Byte) fgetc(file);
        if (feof(file)) {
            bl_container_destroy(&string);
            return string;
        }

        if (currentChar == '\"') {
            if (bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\0'}) != BL_ContainerOPSuccessful)
                bl_container_destroy(&string);
            return string;
        }

        if (currentChar == '\\') {
            int escapedChar = fgetc(file);
            if (feof(file)) {
                bl_container_destroy(&string);
                return string;
            }
            BL_ContainerError insertionError = BL_ContainerOPSuccessful;
            if (escapedChar == 'r')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\r'});
            else if (escapedChar == 'n')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\n'});
            else if (escapedChar == '\"')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\"'});
            else if (escapedChar == '/')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'/'});
            else if (escapedChar == 'b')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\b'});
            else if (escapedChar == 't')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\t'});
            else if (escapedChar == 'f')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\f'});
            else if (escapedChar == '\\')
                insertionError = bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {'\\'});
            else if (escapedChar == 'u') {
                BL_Textprocessing_UTFCodepoint charToInsert = internal_handleUnicodeEscape(file);
                if (charToInsert.bytesUsed) {
                    insertionError = bl_container_dynamic_insert(&string,
                                                                 bl_container_size(&string.container),
                                                                 charToInsert.bytesUsed,
                                                                 sizeof(BL_Byte),
                                                                 charToInsert.bytes);
                } else
                    insertionError = BL_ContainerOPUnsuccessful;
            } else {
                bl_container_destroy(&string);
                return string;
            }

            if (insertionError != BL_ContainerOPSuccessful) {
                bl_container_destroy(&string);
                return string;
            }
            continue;
        }

        if (bl_container_dynamic_append(&string, sizeof(BL_Byte), &(BL_Byte) {currentChar}) != BL_ContainerOPSuccessful) {
            bl_container_destroy(&string);
            return string;
        }
    }
}

BL_JsonTokenStore bl_json_tokenize_file(FILE* file) {
    BL_JsonTokenStore tokenStorage = {.dynamicContainer = bl_container_dynamic_create_stack(0, sizeof(BL_JsonToken)), .maxDepth = 0};
    if (!bl_container_dynamic_is_valid(&tokenStorage.dynamicContainer))
        return tokenStorage;
    size_t depthCount = 0;

    while (1) {
        BL_Byte currentChar = (BL_Byte) fgetc(file);
        if (feof(file))
            goto ErrorExit;

        if (isspace(currentChar))
            continue;

        if (currentChar == ',') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenComma}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ':') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenColon}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '{') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenOpenCurlyBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            if (++depthCount > tokenStorage.maxDepth)
                tokenStorage.maxDepth = depthCount;
            continue;
        }
        if (currentChar == '[') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenOpenBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            if (++depthCount > tokenStorage.maxDepth)
                tokenStorage.maxDepth = depthCount;
            continue;
        }
        if (currentChar == ']') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenCloseBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            depthCount--;
            continue;
        }
        if (currentChar == '}') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenCloseCurlyBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            if (--depthCount == 0)
                return tokenStorage;
            continue;
        }

        if (currentChar == '\"') {
            BL_String string = internal_parseUTF8String(file);
            if (!bl_container_dynamic_is_valid(&string))
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer,
                                            sizeof(BL_JsonToken),
                                            &(BL_JsonToken) {.tokenType      = BL_JsonTokenString,
                                                             .additionalData = (BL_JsonMemberValue) {.string = string}}))
                goto ErrorExit;
            continue;
        }

        if (isdigit(currentChar) || currentChar == '-') {
            double number = internal_parseNumber(currentChar == '-' ? INFINITY : (double) (currentChar - '0'), file);
            if (number == NAN)
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer,
                                            sizeof(BL_JsonToken),
                                            &(BL_JsonToken) {.tokenType      = BL_JsonTokenNumber,
                                                             .additionalData = (BL_JsonMemberValue) {.number = number}}))
                goto ErrorExit;
            continue;
        }


        if (currentChar == 'f') {
            if ('a' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if ('s' != fgetc(file))
                goto ErrorExit;
            if ('e' != fgetc(file))
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer,
                                            sizeof(BL_JsonToken),
                                            &(BL_JsonToken) {.tokenType = BL_JsonTokenBool, .additionalData = (BL_JsonMemberValue) {.boolean = false}}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 't') {
            if ('r' != fgetc(file))
                goto ErrorExit;
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('e' != fgetc(file))
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer,
                                            sizeof(BL_JsonToken),
                                            &(BL_JsonToken) {.tokenType = BL_JsonTokenBool, .additionalData = (BL_JsonMemberValue) {.boolean = true}}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 'n') {
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(BL_JsonTokenType), &(BL_JsonTokenType) {BL_JsonTokenNull}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        } else
            goto ErrorExit;
    }


ErrorExit:
    bl_container_dynamic_destroy_with_elements(&tokenStorage.dynamicContainer, internal_tokenStorageDestructor);
    return tokenStorage;
}

BL_JsonObject bl_json_read_file(FILE* file) {
    typedef struct {
        bool                 isArrayScope;
        BL_DynamicContainer* scope;
    } JsonStackEntry;
    BL_JsonTokenStore tokens = bl_json_tokenize_file(file);
    if (!bl_container_dynamic_is_valid(&tokens.dynamicContainer))
        return (BL_JsonObject) {0};

    BL_Container jsonObjectStack = bl_container_create_stack(tokens.maxDepth, sizeof(JsonStackEntry));
    if (!bl_container_is_valid(&jsonObjectStack)) {
        internal_tokenStorageDestructor(&tokens);
        return (BL_JsonObject) {0};
    }

    size_t              stackPointer        = 0;
    bool                expectingIdentifier = false, expectingValue = true, expectingColon = false;
    BL_JsonObject       returnObject         = {0};

    BL_JsonObjectMember currentWorkingMember = {0};
    JsonStackEntry      currentScope         = {0};

    for (BL_JsonToken* currentToken = bl_container_dynamic_front(&tokens.dynamicContainer); currentToken < (BL_JsonToken*) bl_container_dynamic_end(&tokens.dynamicContainer); currentToken++) {
        switch (currentToken->tokenType) {
        case BL_JsonTokenOpenCurlyBracket:
            if (!expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            expectingValue              = false;
            expectingIdentifier         = true;
            BL_JsonObject currentObject = bl_container_dynamic_create_stack(0, sizeof(BL_JsonObjectMember));
            if (!bl_container_dynamic_is_valid(&currentObject))
                goto ErrorExit;
            if (!stackPointer) {
                returnObject = currentObject;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &returnObject};
            } else if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope,
                                                sizeof(BL_JsonArrayMember),
                                                &(BL_JsonArrayMember) {.valueType = JsonTypeObject,
                                                                       .value     = (BL_JsonMemberValue) {.object = currentObject}}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &((BL_JsonArrayMember*) bl_container_dynamic_back(currentScope.scope))->value.object};
            } else {
                currentWorkingMember.value.object = currentObject;
                currentWorkingMember.valueType    = JsonTypeObject;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &((BL_JsonObjectMember*) bl_container_dynamic_back(currentScope.scope))->value.object};
            }
            bl_container_set(&jsonObjectStack, stackPointer, sizeof currentScope, &currentScope);
            stackPointer++;
            break;
        case BL_JsonTokenOpenBracket:
            if (!stackPointer || !expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            BL_JsonArray newArray = bl_container_dynamic_create_stack(0, sizeof(BL_JsonArrayMember));
            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope,
                                                sizeof(BL_JsonArrayMember),
                                                &(BL_JsonArrayMember) {.valueType = JsonTypeArray,
                                                                       .value     = (BL_JsonMemberValue) {.array = newArray}}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = true, .scope = &((BL_JsonArrayMember*) bl_container_dynamic_back(currentScope.scope))->value.array};
            } else {
                currentWorkingMember.value.array = newArray;
                currentWorkingMember.valueType   = JsonTypeArray;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = true, .scope = &((BL_JsonObjectMember*) bl_container_dynamic_back(currentScope.scope))->value.array};
            }
            bl_container_set(&jsonObjectStack, stackPointer, sizeof currentScope, &currentScope);
            stackPointer++;
            break;

        case BL_JsonTokenColon:
            if (!expectingColon)
                goto ErrorExit;
            expectingColon = false;
            expectingValue = true;
            break;
        case BL_JsonTokenComma:
            if (expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            if (currentScope.isArrayScope)
                expectingValue = true;
            else
                expectingIdentifier = true;
            break;
        case BL_JsonTokenCloseCurlyBracket:
            if (expectingValue || expectingIdentifier || expectingColon || !stackPointer || currentScope.isArrayScope)
                goto ErrorExit;
            stackPointer--;
            bl_sort_heap((BL_Container*) currentScope.scope, bl_string_compare_acending);
            if (stackPointer)
                currentScope = *(JsonStackEntry*) bl_container_get(&jsonObjectStack, stackPointer - 1);
            break;
        case BL_JsonTokenCloseBracket:
            if (expectingValue || expectingIdentifier || expectingColon || !stackPointer || !currentScope.isArrayScope)
                goto ErrorExit;
            stackPointer--;
            if (stackPointer)
                currentScope = *(JsonStackEntry*) bl_container_get(&jsonObjectStack, stackPointer - 1);
            break;
        case BL_JsonTokenString:
            if (expectingColon || !stackPointer)
                goto ErrorExit;
            if (expectingIdentifier) {
                currentWorkingMember.identifier                      = currentToken->additionalData.string;
                currentToken->additionalData.string.container.header = 0; // Must invalidate, otherwise double free might happen at error.
                expectingColon                                       = true;
                expectingIdentifier                                  = false;
                continue;
            }
            if (!expectingValue)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(BL_JsonArrayMember), &(BL_JsonArrayMember) {.valueType = JsonTypeString, .value = currentToken->additionalData}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeString;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            currentToken->additionalData.string.container.header = 0; // Same here, otherwise double free might occur.
            expectingValue                                       = false;
            break;
        case BL_JsonTokenBool:
            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(BL_JsonArrayMember), &(BL_JsonArrayMember) {.valueType = JsonTypeBoolean, .value = currentToken->additionalData}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeBoolean;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case BL_JsonTokenNull:
            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(BL_JsonArrayMember), &(BL_JsonArrayMember) {.valueType = JsonTypeNull}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.valueType = JsonTypeNull;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case BL_JsonTokenNumber:

            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(BL_JsonArrayMember), &(BL_JsonArrayMember) {.valueType = JsonTypeNumber, .value = currentToken->additionalData}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeNumber;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        default:
            bl_log_debug("How did we get this token? %i", currentToken->tokenType);
            goto ErrorExit;
        }
    }
    bl_container_destroy(&tokens);
    bl_container_destroy(&jsonObjectStack);
    return returnObject;

ErrorExit:
    bl_log_warn("Illformated JSON found while reading file.");
    internal_tokenStorageDestructor(&tokens);
    bl_container_destroy(&jsonObjectStack);
    bl_json_object_destroy(&returnObject);
    bl_container_destroy(&currentWorkingMember.identifier);
    return (BL_JsonObject) {0};
}


void jsonReadFileThread(void* sharedState) {
    BL_JsonReadFilePack* information = sharedState;
    information->future.future       = bl_json_read_file(information->args);
    information->future.isValid      = true;
}

BL_FutureJsonObject* bl_json_read_file_threaded(BL_ThreadPool* threadPool, size_t priority, FILE* file) {
    return bl_threadpool_job_assign(threadPool, priority, jsonReadFileThread, bl_async_args_future_offset(BL_JsonReadFilePack), (void*) &file, sizeof(FILE*), bl_async_args_offset(BL_JsonReadFilePack));
}
