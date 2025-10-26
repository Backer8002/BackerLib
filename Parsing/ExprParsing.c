#include "ExprParsing.h"
#include <BackerLibTypes.h>


DynamicContainer      exprTokenize(StringView* expresion, ExprOperatorDefine* operators, size_t amountOfOperators) {
    unsigned char operatorsList[256] = {0};
    bool          operatorChars[256] = {0};

    for (size_t i = 0; i < amountOfOperators; i++) {
        operatorsList[i] = operators[i].operator;
        operatorChars[operators[i].operator] = true;
    }

    DynamicContainer atoms = stringSplitMulti(expresion, operatorsList, amountOfOperators, false, 0);
    if (!isValidObject((DataTypeFlags*) &atoms))
        return (DynamicContainer) {0};

    DynamicContainer tokens      = containerDynamicCreateStack(0, sizeof(ExprParsingToken), false);

    size_t           currentAtom = 0;
    for (size_t i = 0; i < stringLength(expresion); i++) {
        unsigned char currentChar = *stringGetChar(expresion, i);
        if (!operatorChars[currentChar])
            continue;

        if (i != 0 || !operatorChars[*stringGetChar(expresion, i - 1)]) {
            if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(String*) containerGet((Container*) &atoms, currentAtom)}) != ContainerOPSuccessful)
                goto errorPath;
            currentAtom++;
        }

        if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken),&(ExprParsingToken){.isAtom = false, .operator = currentChar}) != ContainerOPSuccessful)
            goto errorPath;
    }

    if (containerSize((Container*) &atoms) - 1 != currentAtom) {
        if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(String*) containerGet((Container*) &atoms, currentAtom)}) != ContainerOPSuccessful)
            goto errorPath;
    }

    containerDestroy(&atoms);
    return tokens;

errorPath:

    containerDestroy(&tokens);
    containerDynamicDestroyWithElements(&atoms, containerDestroy);
    return tokens;
}

