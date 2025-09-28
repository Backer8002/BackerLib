#include "Json.h"
#include <BackerLibEvent.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct Utf8Char {
    Byte firstChar;
    Byte secondChar;
    Byte thirdChar;
    Byte forthChar;
    Byte amountOfCharsUsed;
} Utf8Char;

static void internal_tokenStorageDestructor(void* element) {
    JsonToken* jsonElement = element;
    if (jsonElement->tokenType == JsonTokenString)
        containerDestroy(&jsonElement->additionalData);
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
static Utf8Char internal_codePointToUTF8(Byte codepoint[3]) {
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
    if (neededChars == 2)
        return (Utf8Char) {
            .firstChar         = 0xc0 | ((codepoint[1] & 0x07) << 2) | (codepoint[0] & 0xc0) >> 6,
            .secondChar        = 0x80 | (codepoint[0] & 0x3f),
            .thirdChar         = 0,
            .forthChar         = 0,
            .amountOfCharsUsed = 2};
    if (neededChars == 3)
        return (Utf8Char) {
            .firstChar         = 0xe0 | (codepoint[1] & 0xf0) >> 4,
            .secondChar        = 0x80 | (codepoint[1] & 0x0f) << 2 | (codepoint[0] & 0xc0) >> 6,
            .thirdChar         = 0x80 | (codepoint[0] & 0x3f),
            .forthChar         = 0,
            .amountOfCharsUsed = 3};
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

    Byte codepoint[2];

    codepoint[0] = strtoul((char[3]) {(char) firstChar, (char) secondChar, '\0'}, NULL, 16);
    codepoint[1] = strtoul((char[3]) {(char) thirdChar, (char) fourthChar, '\0'}, NULL, 16);

    if ((codepoint[0] & 0xfc) == 0xd8) {
        Byte codepointSecondPart[2];
        Byte unicodePoint[3];
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
    return internal_codePointToUTF8((Byte[3]) {codepoint[1], codepoint[0], 0});
}

static String internal_parseUTF8String(FILE* file) {
    String string = containerDynamicCreateStack(0, sizeof(char), false);
    if (!isValidObject(&string.header))
        return string;

    while (1) {
        Byte currentChar = (Byte) fgetc(file);
        if (feof(file)) {
            containerDestroy(&string);
            return string;
        }

        if (currentChar == '\"') {
            if (containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\0'}) != ContainerOPSuccessful)
                containerDestroy(&string);
            return string;
        }

        if (currentChar == '\\') {
            int escapedChar = fgetc(file);
            if (feof(file) || escapedChar > 127) {
                containerDestroy(&string);
                return string;
            }
            ContainerError insertionError = ContainerOPSuccessful;
            if (escapedChar == 'r')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\r'});
            else if (escapedChar == 'n')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\n'});
            else if (escapedChar == '\"')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\"'});
            else if (escapedChar == '/')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'/'});
            else if (escapedChar == 'b')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\b'});
            else if (escapedChar == 't')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\t'});
            else if (escapedChar == 'f')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\f'});
            else if (escapedChar == '\\')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte) {'\\'});
            else if (escapedChar == 'u') {
                Utf8Char charToInsert = internal_handleUnicodeEscape(file);
                if (charToInsert.amountOfCharsUsed)
                    insertionError = containerDynamicInsert(&string,
                                                            containerSize(&string.container),
                                                            charToInsert.amountOfCharsUsed,
                                                            sizeof(Byte),
                                                            &charToInsert);
                else
                    insertionError = ContainerOPUnsuccessful;
            } else {
                containerDestroy(&string);
                return string;
            }

            if (insertionError != ContainerOPSuccessful) {
                containerDestroy(&string);
                return string;
            }
            continue;
        }

        if (containerDynamicAppend(&string, sizeof(Byte), &(Byte) {currentChar}) != ContainerOPSuccessful) {
            containerDestroy(&string);
            return string;
        }
    }
}

JsonTokenStore jsonTokenizeFile(FILE* file) {
    JsonTokenStore tokenStorage = {.dynamicContainer = containerDynamicCreateStack(0, sizeof(JsonToken), false), .maxDepth = 0};
    if (!isValidObject(&tokenStorage.dynamicContainer.header))
        return tokenStorage;
    size_t depthCount = 0;

    while (1) {
        Byte currentChar = (Byte) fgetc(file);
        if (feof(file))
            goto ErrorExit;

        if (isspace(currentChar))
            continue;

        if (currentChar == ',') {
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenComma}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ':') {
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenColon}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '{') {
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            if (++depthCount > tokenStorage.maxDepth)
                tokenStorage.maxDepth = depthCount;
            continue;
        }
        if (currentChar == '[') {
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            if (++depthCount > tokenStorage.maxDepth)
                tokenStorage.maxDepth = depthCount;
            continue;
        }
        if (currentChar == ']') {
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            depthCount--;
            continue;
        }
        if (currentChar == '}') {
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            if (--depthCount == 0)
                return tokenStorage;
            continue;
        }

        if (currentChar == '\"') {
            String string = internal_parseUTF8String(file);
            if (!isValidObject(&string.header))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage.dynamicContainer,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType      = JsonTokenString,
                                                     .additionalData = (JsonObjectMemberValue) {.string = string}}))
                goto ErrorExit;
            continue;
        }

        if (isdigit(currentChar) || currentChar == '-') {
            double number = internal_parseNumber(currentChar == '-' ? INFINITY : (double) (currentChar - '0'), file);
            if (number == NAN)
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage.dynamicContainer,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType      = JsonTokenNumber,
                                                     .additionalData = (JsonObjectMemberValue) {.number = number}}))
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
            if (containerDynamicAppend(&tokenStorage.dynamicContainer,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue) {.boolean = false}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 't') {
            if ('r' != fgetc(file))
                goto ErrorExit;
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('e' != fgetc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage.dynamicContainer,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue) {.boolean = true}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 'n') {
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage.dynamicContainer, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenNull}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else
            goto ErrorExit;
    }


ErrorExit:
    containerDynamicDestroyWithElements(&tokenStorage.dynamicContainer, internal_tokenStorageDestructor);
    return tokenStorage;
}

JsonObject jsonReadFile(FILE* file) {
    typedef struct {
        bool              isArrayScope;
        DynamicContainer* scope;
    } JsonStackEntry;
    JsonTokenStore tokens = jsonTokenizeFile(file);
    if (!isValidObject(&tokens.dynamicContainer.header))
        return (JsonObject) {0};

    Container jsonObjectStack = containerCreateStack(tokens.maxDepth, sizeof(JsonStackEntry), false);
    if (!isValidObject(&jsonObjectStack.header)) {
        internal_tokenStorageDestructor(&tokens);
        return (JsonObject) {0};
    }

    size_t           stackPointer        = 0;
    bool             expectingIdentifier = false, expectingValue = true, expectingColon = false;
    JsonObject       returnObject         = {0};

    JsonObjectMember currentWorkingMember = {0};
    JsonStackEntry   currentScope         = {0};

    for (JsonToken* currentToken = containerDynamicFront(&tokens.dynamicContainer); currentToken < (JsonToken*) containerDynamicEnd(&tokens.dynamicContainer); currentToken++) {
        switch (currentToken->tokenType) {
        case JsonTokenOpenCurlyBracket:
            if (!expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            expectingValue           = false;
            expectingIdentifier      = true;
            JsonObject currentObject = containerDynamicCreateStack(0, sizeof(JsonObjectMember), false);
            if (!isValidObject((DataTypeFlags*) &currentObject))
                goto ErrorExit;
            if (!stackPointer) {
                returnObject = currentObject;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &returnObject};
            } else if (currentScope.isArrayScope) {
                if (containerDynamicAppend(currentScope.scope,
                                           sizeof(JsonArrayMember),
                                           &(JsonArrayMember) {.valueType = JsonTypeObject,
                                                               .value     = (JsonObjectMemberValue) {.object = currentObject}}) != ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &((JsonArrayMember*) containerDynamicBack(currentScope.scope))->value.object};
            } else {
                currentWorkingMember.value.object = currentObject;
                currentWorkingMember.valueType    = JsonTypeObject;
                if (containerDynamicAppend(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = false, .scope = &((JsonObjectMember*) containerDynamicBack(currentScope.scope))->value.object};
            }
            containerSet(&jsonObjectStack, stackPointer, sizeof currentScope, &currentScope);
            stackPointer++;
            break;
        case JsonTokenOpenBracket:
            if (!stackPointer || !expectingValue || expectingColon || expectingIdentifier)
                goto ErrorExit;
            JsonArray newArray = containerDynamicCreateStack(0, sizeof(JsonArrayMember), false);
            if (currentScope.isArrayScope) {
                if (containerDynamicAppend(currentScope.scope,
                                           sizeof(JsonArrayMember),
                                           &(JsonArrayMember) {.valueType = JsonTypeArray,
                                                               .value     = (JsonObjectMemberValue) {.array = newArray}}) != ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = true, .scope = &((JsonArrayMember*) containerDynamicBack(currentScope.scope))->value.array};
            } else {
                currentWorkingMember.value.array = newArray;
                currentWorkingMember.valueType   = JsonTypeArray;
                if (containerDynamicAppend(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != ContainerOPSuccessful)
                    goto ErrorExit;
                currentScope = (JsonStackEntry) {.isArrayScope = true, .scope = &((JsonObjectMember*) containerDynamicBack(currentScope.scope))->value.array};
            }
            containerSet(&jsonObjectStack, stackPointer, sizeof currentScope, &currentScope);
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
        case JsonTokenCloseBracket:
        case JsonTokenCloseCurlyBracket:
            if (expectingValue || expectingIdentifier || expectingColon || !stackPointer)
                goto ErrorExit;
            stackPointer--;
            if (stackPointer)
                currentScope = *(JsonStackEntry*) containerGet(&jsonObjectStack, stackPointer - 1);
            break;
        case JsonTokenString:
            if (expectingColon || !stackPointer)
                goto ErrorExit;
            if (expectingIdentifier) {
                currentWorkingMember.identifier            = currentToken->additionalData.string;
                currentToken->additionalData.string.header = 0; // Must invalidate, otherwise double free might happen at error.
                expectingColon                             = true;
                expectingIdentifier                        = false;
                continue;
            }
            if (!expectingValue)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (containerDynamicAppend(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeString, .value = currentToken->additionalData}) != ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeString;
                if (containerDynamicAppend(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != ContainerOPSuccessful)
                    goto ErrorExit;
            }
            currentToken->additionalData.string.header = 0; // Same here, otherwise double free might occur.
            expectingValue                             = false;
            break;
        case JsonTokenBool:
            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (containerDynamicAppend(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeBoolean, .value = currentToken->additionalData}) != ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeBoolean;
                if (containerDynamicAppend(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case JsonTokenNull:
            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (containerDynamicAppend(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeNull}) != ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.valueType = JsonTypeNull;
                if (containerDynamicAppend(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case JsonTokenNumber:

            if (expectingColon || expectingIdentifier || !expectingValue || !stackPointer)
                goto ErrorExit;

            if (currentScope.isArrayScope) {
                if (containerDynamicAppend(currentScope.scope, sizeof(JsonArrayMember), &(JsonArrayMember) {.valueType = JsonTypeNumber, .value = currentToken->additionalData}) != ContainerOPSuccessful)
                    goto ErrorExit;
            } else {
                currentWorkingMember.value     = currentToken->additionalData;
                currentWorkingMember.valueType = JsonTypeNumber;
                if (containerDynamicAppend(currentScope.scope, sizeof currentWorkingMember, &currentWorkingMember) != ContainerOPSuccessful)
                    goto ErrorExit;
            }
            expectingValue = false;
            break;
        case JsonTokenInvalid:
            LogError("This token shall never appear.");
            goto ErrorExit;
        default:
            LogError("How did we get this token?");
            goto ErrorExit;
        }
    }
    containerDestroy(&tokens);
    containerDestroy(&jsonObjectStack);
    return returnObject;

ErrorExit:
    eventCall(&JsonFileIllFormated);
    internal_tokenStorageDestructor(&tokens);
    containerDestroy(&jsonObjectStack);
    jsonObjectDestroy(&returnObject);
    containerDestroy(&currentWorkingMember.identifier);
    return (JsonObject) {0};
}

