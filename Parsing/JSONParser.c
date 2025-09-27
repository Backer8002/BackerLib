#include "JSONParser.h"
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
    size_t decimalDepth    = 1;
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
        return (Utf8Char){.firstChar = codepoint[0], .secondChar = 0, .thirdChar = 0, .forthChar = 0, .amountOfCharsUsed = 1};
    if (neededChars == 2)
        return (Utf8Char){
            .firstChar = 0xc0 | ((codepoint[1] & 0x07) << 2) | (codepoint[0] & 0xc0) >> 6,
            .secondChar = 0x80 | (codepoint[0] & 0x3f),
            .thirdChar = 0,
            .forthChar = 0,
            .amountOfCharsUsed = 2};
    if (neededChars == 3)
        return (Utf8Char){
            .firstChar = 0xe0 | (codepoint[1] & 0xf0) >> 4,
            .secondChar = 0x80 | (codepoint[1] & 0x0f) << 2 | (codepoint[0] & 0xc0) >> 6,
            .thirdChar = 0x80 | (codepoint[0] & 0x3f),
            .forthChar = 0,
            .amountOfCharsUsed = 3
        };
    return (Utf8Char){
        .firstChar = 0xf0 | (codepoint[2] & 0x1c) >> 2,
        .secondChar = 0x80 | (codepoint[2] & 0x03) << 4 | (codepoint[1] & 0xf0) >> 4,
        .thirdChar = 0x80 | (codepoint[1] & 0x0f) << 2 | (codepoint[0] & 0xc0) >> 6,
        .forthChar = 0x80 | (codepoint[0] & 0x3f),
        .amountOfCharsUsed = 4
    };
}

static Utf8Char internal_handleUnicodeEscape(FILE* file) {
    int firstChar  = fgetc(file);
    int secondChar = fgetc(file);
    int thirdChar  = fgetc(file);
    int fourthChar = fgetc(file);
    if (feof(file))
        return (Utf8Char){0};
    if (!isxdigit(firstChar) || !isxdigit(secondChar) || !isxdigit(thirdChar) || !isxdigit(fourthChar))
        return (Utf8Char){0};

    Byte codepoint[2];

    codepoint[0] = strtoul((char[3]){(char) firstChar, (char) secondChar, '\0'}, NULL, 16);
    codepoint[1] = strtoul((char[3]){(char) thirdChar, (char) fourthChar, '\0'}, NULL, 16);

    Utf8Char utf8Char = {0};

    if ((codepoint[0] & 0xfc) == 0xd8) {
        Byte codepointSecondPart[2];
        Byte unicodepoint[3];
        if (fgetc(file) != '\\')
            return (Utf8Char){0};
        if (fgetc(file) != 'u')
            return (Utf8Char){0};
        int fifthChar   = fgetc(file);
        int sixthChar   = fgetc(file);
        int seventhChar = fgetc(file);
        int eigthChar   = fgetc(file);
        if (!isxdigit(fifthChar) || !isxdigit(sixthChar) || !isxdigit(seventhChar) || !isxdigit(eigthChar))
            return (Utf8Char){0};
        codepointSecondPart[0] = strtoul((char[3]){(char) fifthChar, (char) sixthChar, '\0'}, NULL, 16);
        codepointSecondPart[1] = strtoul((char[3]){(char) seventhChar, (char) eigthChar, '\0'}, NULL, 16);

        if ((codepointSecondPart[0] & 0xfc) != 0xdc)
            return (Utf8Char){0};

        unicodepoint[0] = codepointSecondPart[1];
        unicodepoint[1] = codepointSecondPart[0] & 0x03;
        unicodepoint[1] |= (codepoint[1] & 0x3f) << 2;
        unicodepoint[2] = (codepoint[1] & 0xc0) >> 6;
        unicodepoint[2] |= (codepoint[0] & 0x03) << 2;
        unicodepoint[2]++;
        return internal_codePointToUTF8(unicodepoint);
    }
    return internal_codePointToUTF8((Byte[3]){codepoint[1], codepoint[0], 0});
}

static String internal_parseUTF8String(FILE* file) {
    String string = containerDynamicCreateStack(0, sizeof(char), false);
    if (!isValidObject(&string.header))
        return string;

    while (1) {
        Byte currentChar = (Byte)fgetc(file);
        if (feof(file)) {
            containerDestroy(&string);
            return string;
        }

        if (currentChar == '\"') {
            if (containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\0'}) != ContainerOPSuccessful)
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
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\r'});
            else if (escapedChar == 'n')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\n'});
            else if (escapedChar == '\"')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\"'});
            else if (escapedChar == '/')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'/'});
            else if (escapedChar == 'b')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\b'});
            else if (escapedChar == 't')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\t'});
            else if (escapedChar == 'f')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\f'});
            else if (escapedChar == '\\')
                insertionError = containerDynamicAppend(&string, sizeof(Byte), &(Byte){'\\'});
            else if (escapedChar == 'u') {
                Utf8Char charToInsert = internal_handleUnicodeEscape(file);
                if (charToInsert.amountOfCharsUsed)
                    insertionError        = containerDynamicInsert(&string,
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

        if (containerDynamicAppend(&string, sizeof(Byte), &(Byte){currentChar}) != ContainerOPSuccessful) {
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
        Byte currentChar = (Byte) fgetc(file);
        if (feof(file))
            goto ErrorExit;

        if (isspace(currentChar))
            continue;

        if (currentChar == ',') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenComma}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ':') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenColon}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '{') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenOpenCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            depthCount++;
            continue;
        }
        if (currentChar == '[') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenOpenBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == ']') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenCloseBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == '}') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenCloseCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            if (--depthCount == 0)
                return tokenStorage;
            continue;
        }

        if (currentChar == '\"') {
            String string = internal_parseUTF8String(file);
            if (!isValidObject(&string.header))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken){.tokenType = JsonTokenString,
                                                    .additionalData = (JsonObjectMemberValue){.string = string}}))
                goto ErrorExit;
            continue;
        }

        if (isdigit(currentChar) || currentChar == '-') {
            double number = internal_parseNumber(currentChar == '-' ? INFINITY : (double) (currentChar - '0'), file);
            if (number == NAN)
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken){.tokenType = JsonTokenNumber,
                                                    .additionalData = (JsonObjectMemberValue){.number = number}}))
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
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken){.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue){.boolean = false}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 't') {
            if ('r' != fgetc(file))
                goto ErrorExit;
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('e' != fgetc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken){.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue){.boolean = true}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == 'n') {
            if ('u' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if ('l' != fgetc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenNull}) != ContainerOPSuccessful)
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
    }                JsonStackEntry;
    DynamicContainer tokens = jsonTokenizeFile(file);
    return (JsonObject){0};
}