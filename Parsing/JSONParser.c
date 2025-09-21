#include "JSONParser.h"
#include <BackerLibEvent.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>

static inline void internal_tokenStorageDestructor(void* element) {
    JsonToken* jsonElement = element;
    if (jsonElement->tokenType == JsonTokenString)
        containerDestroy(&jsonElement->additionalData);
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

        if (iswdigit(currentChar)) {
        }

        if (currentChar == L'"') {
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
}