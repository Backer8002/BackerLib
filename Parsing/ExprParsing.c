#include "ExprParsing.h"

#include <BackerLibEvent.h>
#include <BackerLibTypes.h>
#include <ctype.h>
#include <stdio.h>

static ExprOperation*     internal_expr_parse(const DynamicContainer* tokens, ExprParsingToken** currentTokenGlobal, DynamicContainer* tree, size_t currentBindingPower, ExprOperatorDefine* operators, size_t amountOfOperators);
static ExprOperatorDefine internal_get_operator(const ExprOperatorDefine* operationDefine, size_t amountOfOperators, unsigned char operator);

DynamicContainer          exprTokenize(StringView* expresion, ExprOperatorDefine* operators, size_t amountOfOperators) {
    unsigned char splitablesList[256] = {' ', '\n', '\t', '\v', '\b', '\f', '\0'};
    bool          operatorChars[256]  = {0};

    for (size_t i = 0; i < amountOfOperators; i++) {
        splitablesList[i + 7]                    = operators[i].operatorChar;
        operatorChars[operators[i].operatorChar] = true;
    }

    DynamicContainer atoms = stringSplitMulti(expresion, splitablesList, amountOfOperators + 7, false, 0);
    if (!isValidObject((DataTypeFlags*) &atoms))
        return (DynamicContainer) {0};

    DynamicContainer tokens      = containerDynamicCreateStack(0, sizeof(ExprParsingToken), false);

    size_t           currentAtom = 0, sizeOfAtoms = containerSize(&atoms.container);
    bool isInAtom = false,foundToken = false;
    for (size_t i = 0; i < stringLength(expresion); i++) {
        unsigned char currentChar = *stringGetChar(expresion, i);
        if (!operatorChars[currentChar] && !isspace(currentChar)) {
            isInAtom = true, foundToken = false;
            continue;
        }

        if (isspace(currentChar)) {
            if (!foundToken && currentAtom < sizeOfAtoms) {
                if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(String*) containerGet((Container*) &atoms, currentAtom)}) != ContainerOPSuccessful)
                    goto errorPath;
                currentAtom++;
            }
            isInAtom = false;
            continue;
        }

        foundToken = true;

        if (i != 0 && isInAtom) {
            if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(String*) containerGet((Container*) &atoms, currentAtom)}) != ContainerOPSuccessful)
                goto errorPath;
            currentAtom++;
            isInAtom = false;
        }

        if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken),&(ExprParsingToken){.isAtom = false, .operator = currentChar}) != ContainerOPSuccessful)
            goto errorPath;
    }

    for (; currentAtom < sizeOfAtoms;currentAtom++) {
        if (containerDynamicAppend(&tokens, sizeof(ExprParsingToken),&(ExprParsingToken){.isAtom = true, .atom = *(String*)containerGet((Container*)&atoms,currentAtom)}) != ContainerOPSuccessful)
            goto errorPath;
    }

    containerDestroy(&atoms);
    return tokens;

errorPath:

    containerDestroy(&tokens);
    containerDynamicDestroyWithElements(&atoms, containerDestroy);
    return tokens;
}

DynamicContainer exprParse(const DynamicContainer* tokens, ExprOperatorDefine* operators, size_t amountOfOperators) {
    DynamicContainer tree = containerDynamicCreateStack(containerSize((Container*) tokens), sizeof(ExprOperation), false);


    if (!isValidObject((DataTypeFlags*) &tree))
        return tree;

    if (containerSize((Container*) tokens) == 1) {
        ExprParsingToken* token = containerDynamicFront(tokens);
        if (!token->isAtom) {
            LogError("A sole operator is not permitted");
            containerDestroy(&tree);
            return tree;
        }

        ExprOperationOperand operand = {.isAtom = true, .atom = token->atom};
        containerDynamicAppend(&tree, sizeof(ExprOperation), &(ExprOperation) {.operatorChar = '\0', .isBinaryOperation = false, .unaryOperation = {.unaryOperand = operand, .unaryOperatorWasOnRight = false}});
        return tree;
    }

    ExprOperation* result = internal_expr_parse(tokens, &(ExprParsingToken*) {containerDynamicFront(tokens)}, &tree, 0, operators, amountOfOperators);
    if (!result) {
        containerDestroy(&tree);
        return tree;
    }
    containerDynamicAppend(&tree, sizeof(ExprOperation), &(ExprOperation) {.operatorChar = '\0', .isBinaryOperation = false, .unaryOperation = {.unaryOperand = (ExprOperationOperand) {.isAtom = false, .operation = result}, .unaryOperatorWasOnRight = false}});
    return tree;
}

ExprOperation* internal_expr_parse(const DynamicContainer* tokens, ExprParsingToken** currentTokenGlobal,
                                   DynamicContainer*   tree,
                                   size_t              currentBindingPower,
                                   ExprOperatorDefine* operators, size_t amountOfOperators) {
    bool           prevTokenWasAtom = false, hasSeenBinaryOp = true;

    ExprOperation* previousOperation = NULL;

    for (ExprParsingToken* currentTokenLocal = *currentTokenGlobal; currentTokenLocal; currentTokenLocal = containerDynamicNext(tokens, currentTokenLocal)) {
        if (currentTokenLocal->isAtom && prevTokenWasAtom) {
            LogError("An atom cannot follow an atom without at binary operator in between.");
            *currentTokenGlobal = (ExprParsingToken*) containerDynamicBack(tokens);
            return NULL;
        }

        if (currentTokenLocal->isAtom) {
            if (!hasSeenBinaryOp) {
                LogError("A binary operator must exist between atoms");
                *currentTokenGlobal = (ExprParsingToken*) containerDynamicBack(tokens);
                return NULL;
            }
            prevTokenWasAtom = true;
            hasSeenBinaryOp  = false;
            continue;
        }
        ExprOperatorDefine currentOperator = internal_get_operator(operators, amountOfOperators, currentTokenLocal->operator);
        if (!currentOperator.isUnaryOperator && hasSeenBinaryOp) {
            LogError("Two binary operator cannot exist between two atoms");
            *currentTokenGlobal = (ExprParsingToken*) containerDynamicBack(tokens);
            return NULL;
        }

        if ((currentOperator.isUnaryOperator && currentOperator.isBinaryOperator && hasSeenBinaryOp) || (currentOperator.isUnaryOperator && !currentOperator.isBinaryOperator)) {
            currentTokenLocal++;
            ExprOperation* operation = internal_expr_parse(tokens, &currentTokenLocal, tree, currentOperator.rightUnaryBinding, operators, amountOfOperators);
            if (operation) {
                containerDynamicAppend(tree,
                                       sizeof(ExprOperationOperand),
                                       &(ExprOperation) {
                                           .operatorChar      = currentOperator.operatorChar,
                                           .isBinaryOperation = false,
                                           .unaryOperation    = {
                                                  .unaryOperand = (ExprOperationOperand) {
                                                      .isAtom    = false,
                                                      .operation = operation},
                                                  .unaryOperatorWasOnRight = false}});
            } else {
                ExprParsingToken* nextToken = containerDynamicNext(tokens, currentTokenLocal);
                if (!nextToken || !nextToken->isAtom)
                    return NULL;
                containerDynamicAppend(tree,
                                       sizeof(ExprOperationOperand),
                                       &(ExprOperation) {
                                           .operatorChar      = currentOperator.operatorChar,
                                           .isBinaryOperation = false,
                                           .unaryOperation    = {
                                                  .unaryOperand = (ExprOperationOperand) {
                                                      .isAtom = true,
                                                      .atom   = nextToken->atom},
                                                  .unaryOperatorWasOnRight = false}});
            }

            previousOperation = containerDynamicBack(tree);
            continue;
        }

        if (currentOperator.lhsBinaryBinding <= currentBindingPower) {
            *currentTokenGlobal = currentTokenLocal - 2;
            return previousOperation;
        }

        prevTokenWasAtom = false;

        ExprOperationOperand lhs, rhs;
        if (previousOperation)
            lhs = (ExprOperationOperand) {.isAtom = false, .operation = previousOperation};
        else
            lhs = (ExprOperationOperand) {.isAtom = true, .atom = (currentTokenLocal - 1)->atom};
        currentTokenLocal++;
        ExprOperation* rhsOperation = internal_expr_parse(tokens, &currentTokenLocal, tree, currentOperator.rhsBinaryBinding, operators, amountOfOperators);
        if (!rhsOperation) {
            ExprParsingToken* nextToken = containerDynamicNext(tokens, currentTokenLocal);
            if (!nextToken || !nextToken->isAtom)
                return NULL;
            rhs = (ExprOperationOperand) {.isAtom = true, .atom = nextToken->atom};
        } else
            rhs = (ExprOperationOperand) {.isAtom = false, .operation = rhsOperation};

        ExprOperation nextOperation = {.operatorChar      = currentOperator.operatorChar,
                                       .isBinaryOperation = true,
                                       .binaryOperands    = {.lhs = lhs,
                                                             .rhs = rhs}};
        containerDynamicAppend(tree, sizeof nextOperation, &nextOperation);
        previousOperation = containerDynamicBack(tree);
        hasSeenBinaryOp = true;
    }
    *currentTokenGlobal = (ExprParsingToken*) containerDynamicBack(tokens) - 1;
    return previousOperation;
}



static ExprOperatorDefine internal_get_operator(const ExprOperatorDefine* operationDefine, size_t amountOfOperators, unsigned char operator) {
    size_t i = 0;
    while (i < amountOfOperators) {
        if (operationDefine[i].operatorChar == operator)
            return operationDefine[i];
        i++;
    }
    return (ExprOperatorDefine) {0};
}

static void internal_expr_print(FILE* file, const ExprOperation* operation) {
    if (operation->isBinaryOperation) {
        if (!operation->binaryOperands.lhs.isAtom)
            internal_expr_print(file, operation->binaryOperands.lhs.operation);
        else
            fputs(containerDynamicFront(&operation->binaryOperands.lhs.atom), file);
        fputc(' ', file);
        if (!operation->binaryOperands.rhs.isAtom)
            internal_expr_print(file, operation->binaryOperands.rhs.operation);
        else
            fputs(containerDynamicFront(&operation->binaryOperands.rhs.atom), file);
        fputc(' ', file);
        fputc(operation->operatorChar,file);
    } else {
        if (!operation->unaryOperation.unaryOperatorWasOnRight) {
            if (operation->operatorChar == '\0')
                fprintf(file, "Expr: ");
            else
                fputc(operation->operatorChar,file);
        }
        if (!operation->unaryOperation.unaryOperand.isAtom)
            internal_expr_print(file, operation->unaryOperation.unaryOperand.operation);
        else
            fputs(containerDynamicFront(&operation->unaryOperation.unaryOperand.atom), file);

        if (operation->unaryOperation.unaryOperatorWasOnRight)
            fputc(operation->operatorChar, file);
    }
}

void exprPrint(FILE* file, const DynamicContainer* tree) {
    internal_expr_print(file, containerDynamicBack(tree));
    fputc('\n', file);
}