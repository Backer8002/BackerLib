#include "Json.h"
#include <BackerLibLogging.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct Utf8Char {
    BL_Byte firstChar;
    BL_Byte secondChar;
    BL_Byte thirdChar;
    BL_Byte forthChar;
    BL_Byte amountOfCharsUsed;
} Utf8Char;

static void internal_tokenStorageDestructor(void* element) {
    JsonToken* jsonElement = element;
    if (jsonElement->tokenType == JsonTokenString)
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

/**
 * @param codepoint Unicode code point in little endian. Unused chars should be 0
 * @return Utf-8 encoding where first char is the first in the sequence
 */
static Utf8Char internal_codePointToUTF8(BL_Byte codepoint[3]) {
    int neededChars = 4;
    if (codepoint[2] == 0) {
        neededChars = 3;
        if (codepoint[1] < 0x08) {
            neededChars = 2;
            if (codepoint[0] < 0x80)
                neededChars = 1;
        }
    }

    if (neededChars == 1)
        return (Utf8Char) {.firstChar = codepoint[0], .secondChar = 0, .thirdChar = 0, .forthChar = 0, .amountOfCharsUsed = 1};
    if (neededChars == 2) {
        return (Utf8Char) {
            .firstChar         = 0xc0 | ((codepoint[1] & 0x07) << 2) | (codepoint[0] & 0xc0) >> 6,
            .secondChar        = 0x80 | (codepoint[0] & 0x3f),
            .thirdChar         = 0,
            .forthChar         = 0,
            .amountOfCharsUsed = 2};
    }
    if (neededChars == 3) {
        return (Utf8Char) {
            .firstChar         = 0xe0 | (codepoint[1] & 0xf0) >> 4,
            .secondChar        = 0x80 | (codepoint[1] & 0x0f) << 2 | (codepoint[0] & 0xc0) >> 6,
            .thirdChar         = 0x80 | (codepoint[0] & 0x3f),
            .forthChar         = 0,
            .amountOfCharsUsed = 3};
    }
    return (Utf8Char) {
        .firstChar         = 0xf0 | (codepoint[2] & 0x1c) >> 2,
        .secondChar        = 0x80 | (codepoint[2] & 0x03) << 4 | (codepoint[1] & 0xf0) >> 4,
        .thirdChar         = 0x80 | (codepoint[1] & 0x0f) << 2 | (codepoint[0] & 0xc0) >> 6,
        .forthChar         = 0x80 | (codepoint[0] & 0x3f),
        .amountOfCharsUsed = 4};
}

static Utf8Char internal_handleUnicodeEscape(FILE* file) {
    int firstChar  = fgetc(file);
    int secondChar = fgetc(file);
    int thirdChar  = fgetc(file);
    int fourthChar = fgetc(file);
    if (feof(file))
        return (Utf8Char) {0};
    if (!isxdigit(firstChar) || !isxdigit(secondChar) || !isxdigit(thirdChar) || !isxdigit(fourthChar))
        return (Utf8Char) {0};

    BL_Byte codepoint[2];

    codepoint[0] = strtoul((char[3]) {(char) firstChar, (char) secondChar, '\0'}, NULL, 16);
    codepoint[1] = strtoul((char[3]) {(char) thirdChar, (char) fourthChar, '\0'}, NULL, 16);

    if ((codepoint[0] & 0xfc) == 0xd8) {
        BL_Byte codepointSecondPart[2];
        BL_Byte unicodePoint[3];
        if (fgetc(file) != '\\')
            return (Utf8Char) {0};
        if (fgetc(file) != 'u')
            return (Utf8Char) {0};
        int fifthChar   = fgetc(file);
        int sixthChar   = fgetc(file);
        int seventhChar = fgetc(file);
        int eighthChar  = fgetc(file);
        if (!isxdigit(fifthChar) || !isxdigit(sixthChar) || !isxdigit(seventhChar) || !isxdigit(eighthChar))
            return (Utf8Char) {0};
        codepointSecondPart[0] = strtoul((char[3]) {(char) fifthChar, (char) sixthChar, '\0'}, NULL, 16);
        codepointSecondPart[1] = strtoul((char[3]) {(char) seventhChar, (char) eighthChar, '\0'}, NULL, 16);

        if ((codepointSecondPart[0] & 0xfc) != 0xdc)
            return (Utf8Char) {0};

        unicodePoint[0] = codepointSecondPart[1];
        unicodePoint[1] = codepointSecondPart[0] & 0x03;
        unicodePoint[1] |= (codepoint[1] & 0x3f) << 2;
        unicodePoint[2] = (codepoint[1] & 0xc0) >> 6;
        unicodePoint[2] |= (codepoint[0] & 0x03) << 2;
        unicodePoint[2]++;
        return internal_codePointToUTF8(unicodePoint);
    }
    return internal_codePointToUTF8((BL_Byte[3]) {codepoint[1], codepoint[0], 0});
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
                Utf8Char charToInsert = internal_handleUnicodeEscape(file);
                if (charToInsert.amountOfCharsUsed) {
                    insertionError = bl_container_dynamic_insert(&string,
                                                                 bl_container_size(&string.container),
                                                                 charToInsert.amountOfCharsUsed,
                                                                 sizeof(BL_Byte),
                                                                 &charToInsert);
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

JsonTokenStore jsonTokenizeFile(FILE* file) {
    JsonTokenStore tokenStorage = {.dynamicContainer = bl_container_dynamic_create_stack(0, sizeof(JsonToken)), .maxDepth = 0};
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
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenComma}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ':') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenColon}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '{') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenCurlyBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            if (++depthCount > tokenStorage.maxDepth)
                tokenStorage.maxDepth = depthCount;
            continue;
        }
        if (currentChar == '[') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            if (++depthCount > tokenStorage.maxDepth)
                tokenStorage.maxDepth = depthCount;
            continue;
        }
        if (currentChar == ']') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseBracket}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
            depthCount--;
            continue;
        }
        if (currentChar == '}') {
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseCurlyBracket}) != BL_ContainerOPSuccessful)
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
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType      = JsonTokenString,
                                                     .additionalData = (JsonMemberValue) {.string = string}}))
                goto ErrorExit;
            continue;
        }

        if (isdigit(currentChar) || currentChar == '-') {
            double number = internal_parseNumber(currentChar == '-' ? INFINITY : (double) (currentChar - '0'), file);
            if (number == NAN)
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType      = JsonTokenNumber,
                                                     .additionalData = (JsonMemberValue) {.number = number}}))
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
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonMemberValue) {.boolean = false}}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 't') {
            if ('r' != fgetc(file))
                goto ErrorExit;
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('e' != fgetc(file))
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonMemberValue) {.boolean = true}}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 'n') {
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if (bl_container_dynamic_append(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenNull}) != BL_ContainerOPSuccessful)
                goto ErrorExit;
        } else
            goto ErrorExit;
    }


ErrorExit:
    bl_container_dynamic_destroy_with_elements(&tokenStorage.dynamicContainer, internal_tokenStorageDestructor);
    return tokenStorage;
}

JsonObject jsonReadFile(FILE* file) {
    typedef struct {
        bool              isArrayScope;
        BL_DynamicContainer* scope;
    } JsonStackEntry;
    JsonTokenStore tokens = jsonTokenizeFile(file);
    if (!bl_container_dynamic_is_valid(&tokens.dynamicContainer))
        return (JsonObject) {0};

    BL_Container jsonObjectStack = bl_container_create_stack(tokens.maxDepth, sizeof(JsonStackEntry));
    if (!bl_container_is_valid(&jsonObjectStack)) {
        internal_tokenStorageDestructor(&tokens);
        return (JsonObject) {0};
    }

    size_t           stackPointer        = 0;
    bool             expectingIdentifier = false, expectingValue = true, expectingColon = false;
    JsonObject       returnObject         = {0};

    JsonObjectMember currentWorkingMember = {0};
    JsonStackEntry   currentScope         = {0};

    for (JsonToken* currentToken = bl_container_dynamic_front(&tokens.dynamicContainer); currentToken < (JsonToken*) bl_container_dynamic_end(&tokens.dynamicContainer); currentToken++) {
        switch (currentToken->tokenType) {
        case JsonTokenOpenCurlyBracket:
            if (!expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            expectingValue           = false;
            expectingIdentifier      = true;
            JsonObject currentObject = bl_container_dynamic_create_stack(0, sizeof(JsonObjectMember));
            if (!bl_container_dynamic_is_valid(&currentObject))
                goto ErrorExit;
            if (!stackPointer) {
                returnObject = currentObject;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &returnObject};
            } else if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope,
                                           sizeof(JsonArrayMember),
                                           &(JsonArrayMember) {.valueType = JsonTypeObject,
                                                               .value     = (JsonMemberValue) {.object = currentObject}}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &((JsonArrayMember*) bl_container_dynamic_back(currentScope.scope))->value.object};
            } else {
                currentWorkingMember.value.object = currentObject;
                currentWorkingMember.valueType    = JsonTypeObject;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &((JsonObjectMember*) bl_container_dynamic_back(currentScope.scope))->value.object};
            }
            bl_container_set(&jsonObjectStack, stackPointer, sizeof currentScope, &currentScope);
            stackPointer++;
            break;
        case JsonTokenOpenBracket:
            if (!stackPointer || !expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            JsonArray newArray = bl_container_dynamic_create_stack(0, sizeof(JsonArrayMember));
            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope,
                                           sizeof(JsonArrayMember),
                                           &(JsonArrayMember) {.valueType = JsonTypeArray,
                                                               .value     = (JsonMemberValue) {.array = newArray}}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = true, .scope = &((JsonArrayMember*) bl_container_dynamic_back(currentScope.scope))->value.array};
            } else {
                currentWorkingMember.value.array = newArray;
                currentWorkingMember.valueType   = JsonTypeArray;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = true, .scope = &((JsonObjectMember*) bl_container_dynamic_back(currentScope.scope))->value.array};
            }
            bl_container_set(&jsonObjectStack, stackPointer, sizeof currentScope, &currentScope);
            stackPointer++;
            break;

        case JsonTokenColon:
            if (!expectingColon)
                goto ErrorExit;
            expectingColon = false;
            expectingValue = true;
            break;
        case JsonTokenComma:
            if (expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            if (currentScope.isArrayScope)
                expectingValue = true;
            else
                expectingIdentifier = true;
            break;
        case JsonTokenCloseCurlyBracket:
            if (expectingValue || expectingIdentifier || expectingColon || !stackPointer || currentScope.isArrayScope)
                goto ErrorExit;
            stackPointer--;
            bl_sort_heap((BL_Container*)currentScope.scope,bl_string_compare_acending);
            if (stackPointer)
                currentScope = *(JsonStackEntry*) bl_container_get(&jsonObjectStack, stackPointer - 1);
            break;
        case JsonTokenCloseBracket:
            if (expectingValue || expectingIdentifier || expectingColon || !stackPointer || !currentScope.isArrayScope)
                goto ErrorExit;
            stackPointer--;
            if (stackPointer)
                currentScope = *(JsonStackEntry*) bl_container_get(&jsonObjectStack, stackPointer - 1);
            break;
        case JsonTokenString:
            if (expectingColon || !stackPointer)
                goto ErrorExit;
            if (expectingIdentifier) {
                currentWorkingMember.identifier            = currentToken->additionalData.string;
                currentToken->additionalData.string.container.header = 0; // Must invalidate, otherwise double free might happen at error.
                expectingColon                             = true;
                expectingIdentifier                        = false;
                continue;
            }
            if (!expectingValue)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeString, .value = currentToken->additionalData}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeString;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            currentToken->additionalData.string.container.header = 0; // Same here, otherwise double free might occur.
            expectingValue                             = false;
            break;
        case JsonTokenBool:
            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeBoolean, .value = currentToken->additionalData}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeBoolean;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case JsonTokenNull:
            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeNull}) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.valueType = JsonTypeNull;
                if (bl_container_dynamic_append(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != BL_ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case JsonTokenNumber:

            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (bl_container_dynamic_append(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeNumber, .value = currentToken->additionalData}) != BL_ContainerOPSuccessful)
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
            bl_log_debug("How did we get this token? %i",currentToken->tokenType);
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
    jsonObjectDestroy(&returnObject);
    bl_container_destroy(&currentWorkingMember.identifier);
    return (JsonObject) {0};
}


void jsonReadFileThread(void* sharedState) {
    JsonReadFilePack* information = sharedState;
    information->future.future = jsonReadFile(information->args);
    information->future.isValid = true;
}

FutureJsonObject*  jsonReadFileAsync(BL_ThreadPool* threadPool, size_t priority, FILE* file) {
    return bl_threadpool_job_assign(threadPool,priority,jsonReadFileThread,asyncArgsFutureOffset(JsonReadFilePack),(void*)&file,sizeof(FILE*),asyncArgsOffset(JsonReadFilePack));
}
