#include "JSONParser.h"
#include <BackerLibEvent.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

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

        if (currentChar == '}' || currentChar == ']' || currentChar == ',') {
            if (firstDigit == INFINITY)
                parsedNum *= -1.0f;

            if (exponentIsNegative)
                parsedExponent *= -1.0f;

            parsedNum = pow(parsedNum, parsedExponent);
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
                parsedExponent += (currentChar - '0') / pow(10, (double)decimalDepth);
            else
                parsedNum += (currentChar - '0') / pow(10, (double)decimalDepth);
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

static String internal_parseUTF8String(FILE* file) {
    String string = containerDynamicCreateStack(0, sizeof(char), false);
    if (!isValidObject(&string.header))
        return string;

    while (1) {
        int currentChar = fgetc(file);
        if (feof(file)) {
            containerDestroy(&string);
            return string;
        }

        if (currentChar == '\"') {
            if (containerDynamicAppend(&string,sizeof(Byte),&(Byte){'\0'}) != ContainerOPSuccessful)
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
                int firstChar = fgetc(file);
                int secondChar = fgetc(file);
                int thirdChar  = fgetc(file);
                int fourthChar = fgetc(file);
                if (feof(file)) {
                    containerDestroy(&string);
                    return string;
                }
                if (!isxdigit(firstChar) || !isxdigit(secondChar) || !isxdigit(thirdChar) || !isxdigit(fourthChar)) {
                    containerDestroy(&string);
                    return string;
                }

                Byte utf8Sequence[2];

                utf8Sequence[0] = strtoul((char[3]) {(char) firstChar, (char) secondChar, '\0'}, NULL, 16);
                utf8Sequence[1] = strtoul((char[3]) {(char) thirdChar, (char) fourthChar, '\0'}, NULL, 16);
                insertionError  = containerDynamicInsert(&string, containerSize(&string.container), 2, sizeof(Byte), &utf8Sequence);
            } else {
                containerDestroy(&string);
                return string;
            }

            if (insertionError != ContainerOPSuccessful) {
                containerDestroy(&string);
                return string;
            }
        }

        if (containerDynamicAppend(&string, sizeof(Byte), &(Byte) {currentChar}) != ContainerOPSuccessful) {
            containerDestroy(&string);
            return string;
        }
    }
}

DynamicContainer jsonTokenizeFile(FILE* file) {
    DynamicContainer tokenStorage = containerDynamicCreateStack(0, sizeof(JsonToken), false);
    if (!isValidObject(&tokenStorage.header))
        return tokenStorage;
    size_t depthCount = 0;

    while (1) {
        Byte currentChar = (Byte)fgetc(file);
        if (feof(file))
            goto ErrorExit;

        if (isspace(currentChar))
            continue;

        if (currentChar == ',') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenComma}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ':') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenColon}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '{') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            depthCount++;
            continue;
        }
        if (currentChar == '[') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ']') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '}') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            if (--depthCount == 0)
                return tokenStorage;
            continue;
        }

        if (isdigit(currentChar) || currentChar == '-') {
            double number = internal_parseNumber(currentChar == '-' ? INFINITY : (double)(currentChar - '0'), file);
            if (number == NAN)
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType      = JsonTokenNumber,
                                                     .additionalData = (JsonObjectMemberValue) {.number = number}}))
                goto ErrorExit;
            continue;
        }

        if (currentChar == '\"') {
            String string = internal_parseUTF8String(file);
            if (!isValidObject(&string.header))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                           sizeof(JsonToken),
                           &(JsonToken) {.tokenType      = JsonTokenNumber,
                                         .additionalData = (JsonObjectMemberValue) {.string = string}}))
                goto ErrorExit;
            continue;
        }

        if (currentChar == 'f') {
            if ('a' != fgetwc(file))
                goto ErrorExit;
            if ('l' != fgetwc(file))
                goto ErrorExit;
            if ('s' != fgetwc(file))
                goto ErrorExit;
            if ('e' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue) {.boolean = false}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 't') {
            if ('r' != fgetwc(file))
                goto ErrorExit;
            if ('u' != fgetwc(file))
                goto ErrorExit;
            if ('e' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue) {.boolean = true}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 'n') {
            if ('u' != fgetwc(file))
                goto ErrorExit;
            if ('l' != fgetwc(file))
                goto ErrorExit;
            if ('l' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenNull}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else
            goto ErrorExit;
    }


ErrorExit:
    containerDynamicDestroyWithElements(&tokenStorage, internal_tokenStorageDestructor);
    return tokenStorage;
}

JsonObject jsonReadFile(FILE* file) {
    typedef struct {
        bool              isArrayScope;
        DynamicContainer* scope;
    } JsonStackEntry;
    DynamicContainer tokens = jsonTokenizeFile(file);
    return (JsonObject){0};
}