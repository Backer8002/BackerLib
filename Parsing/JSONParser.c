#include "JSONParser.h"
#include <BackerLibEvent.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>
#include <uchar.h>
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
    size_t decimalDepth    = 1;
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
            if (fseek(file, -1L,SEEK_CUR) == -1)
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
            hitE ? parsedExponent : parsedNum += (currentChar - L'0') / pow(10, decimalDepth);
            decimalDepth++;
        } else {
            hitE ? parsedExponent : parsedNum *= 10;
            hitE ? parsedExponent : parsedNum += currentChar - L'0';
        }
    }
}

static inline StringUTF8 internal_parseUTF8String(FILE* file) {
    StringUTF8 string = stringUTF8Create(&(char32_t){0},1);
    if (!isValidObject(&string.header))
        return string;
    char32_t currentChar = 0;
    int remainingCharsInSequence = 0;
    while (1) {
        char currentInCharInScope = fgetc(file);
        if (remainingCharsInSequence == 0);
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
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenComma}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L':') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenColon}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L'{') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenOpenCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L'[') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenOpenBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L']') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenCloseBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }
        if (currentChar == L'}') {
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenCloseCurlyBracket}) != ContainerOPSuccessful)
                goto ErrorExit;
            continue;
        }

        if (iswdigit(currentChar) || currentChar == L'-') {
            double number = internal_parseNumber(currentChar == L'-' ? INFINITY : currentChar - L'0', file);
            if (number == NAN)
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage,
                                       sizeof(JsonToken),
                                       &(JsonToken){.tokenType = JsonTokenNumber,
                                                    .additionalData = (JsonObjectMemberValue){.number = number}}))
                goto ErrorExit;
            continue;
        }

        if (currentChar == L'"') {
            StringUTF8 string =
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
                                       &(JsonToken){.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue){.boolean = false}}) != ContainerOPSuccessful)
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
                                       &(JsonToken){.tokenType = JsonTokenBool, .additionalData = (JsonObjectMemberValue){.boolean = true}}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else if (currentChar == L'n') {
            if (L'u' != fgetwc(file))
                goto ErrorExit;
            if (L'l' != fgetwc(file))
                goto ErrorExit;
            if (L'l' != fgetwc(file))
                goto ErrorExit;
            if (containerDynamicAppend(&tokenStorage, sizeof(JsonTokenType), &(JsonTokenType){JsonTokenNull}) != ContainerOPSuccessful)
                goto ErrorExit;
        } else { goto ErrorExit; }
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