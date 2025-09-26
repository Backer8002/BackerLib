#include "JSONParser.h"
#include <BackerLibEvent.h>
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <uchar.h>
#include <wchar.h>
#include <wctype.h>

static inline void internal_tokenStorageDestructor(void* element) {
    JsonToken* jsonElement = element;
    if (jsonElement->tokenType == JsonTokenString)
        containerDestroy(&jsonElement->additionalData);
}

static inline double internal_parseNumber(const double firstDigit, FILE* file) {
    double parsedNum       = firstDigit == INFINITY ? 0 : firstDigit;
    double parsedExponent  = 0;
    bool   hitDecimalPoint = false, hitE = false, exponentIsNegative = false;
    size_t decimalDepth = 1;
    while (1) {
        wint_t currentChar = fgetwc(file);
        if (currentChar == WEOF)
            return NAN;

        if (currentChar == L'}' || currentChar == L']' || currentChar == L',') {
            if (firstDigit == INFINITY)
                parsedNum *= -1.0f;

            if (exponentIsNegative)
                parsedExponent *= -1.0f;

            parsedNum = pow(parsedNum, parsedExponent);
            if (fseek(file, -1L, SEEK_CUR) == -1)
                return NAN;
            return currentChar;
        }

        if (currentChar == L'e' || currentChar == L'E') {
            if (hitE)
                return NAN;
            hitE        = true;
            currentChar = fgetwc(file);

            if (!iswdigit(currentChar) && currentChar != L'-' && currentChar != L'+')
                return NAN;

            if (iswdigit(currentChar))
                parsedExponent = currentChar - L'0';
            else if (currentChar == L'-')
                exponentIsNegative = true;
            hitDecimalPoint = false;
            decimalDepth    = 1;
            continue;
        }

        if (currentChar == L'.') {
            if (hitDecimalPoint)
                return NAN;
            hitDecimalPoint = true;
            continue;
        }
        if (!iswdigit(currentChar))
            return NAN;

        if (hitDecimalPoint) {
            if (hitE)
                parsedExponent += (currentChar - L'0') / pow(10, (double)decimalDepth);
            else
                parsedNum += (currentChar - L'0') / pow(10, (double)decimalDepth);
            decimalDepth++;
        } else {
            if (hitE) {
                parsedExponent *= 10.0;
                parsedExponent += currentChar - L'0';
            } else {
                parsedNum *= 10.0;
                parsedNum += currentChar - L'0';
            }
        }
    }
}

static inline String internal_parseUTF8String(FILE* file) {
    String string = containerDynamicCreateStack(0, sizeof(char), false);
    if (!isValidObject(&string.header))
        return string;

    while (1) {
        int currentChar = fgetc(file);
        if (feof(file)) {
            containerDestroy(&string);
            return string;
        }

        if (currentChar == '\"')
            return string;

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
                int firstChar, secondChar, thirdChar, fourthChar;
                firstChar  = fgetc(file);
                secondChar = fgetc(file);
                thirdChar  = fgetc(file);
                fourthChar = fgetc(file);
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

DynamicContainer JsonTokenizeFile(FILE* file) {
    DynamicContainer tokenStorage = containerDynamicCreateStack(0, sizeof(JsonToken), false);
    if (!isValidObject(&tokenStorage.header))
        return tokenStorage;
    size_t depthCount = 0;

    while (1) {
        wint_t currentChar = fgetwc(file);
        if (currentChar == WEOF)
            goto ErrorExit;

        if (iswspace(currentChar))
            continue;

        if (currentChar == L',') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenComma}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L':') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenColon}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L'{') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L'[') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenOpenBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L']') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L'}') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenCloseCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }

        if (iswdigit(currentChar) || currentChar == L'-') {
            double number = internal_parseNumber(currentChar == L'-' ? INFINITY : currentChar - L'0', file);
            if (number == NAN)
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType      = JsonTokenNumber,
                                                     .additionalData = (JsonObjectMemberValue) {.number = number}}))
                goto ErrorExit;
            continue;
        }

        if (currentChar == L'"') {
            String string = internal_parseUTF8String(file);
        }

        if (currentChar == L'f') {
            if (L'a' != fgetwc(file))
                goto ErrorExit;
            if (L'l' != fgetwc(file))
                goto ErrorExit;
            if (L's' != fgetwc(file))
                goto ErrorExit;
            if (L'e' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue) {.boolean = false}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == L't') {
            if (L'r' != fgetwc(file))
                goto ErrorExit;
            if (L'u' != fgetwc(file))
                goto ErrorExit;
            if (L'e' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken) {.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue) {.boolean = true}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == L'n') {
            if (L'u' != fgetwc(file))
                goto ErrorExit;
            if (L'l' != fgetwc(file))
                goto ErrorExit;
            if (L'l' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType) {JsonTokenNull}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else {
            goto ErrorExit;
        }
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
}