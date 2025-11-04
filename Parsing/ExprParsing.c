#include "ExprParsing.h"

#include <BackerLibEvent.h>
#include <BackerLibTypes.h>
#include <ctype.h>
#include <stdio.h>

static ExprOperation*     internal_expr_parse(const BL_DynamicContainer* tokens, ExprParsingToken** currentTokenGlobal, BL_DynamicContainer* tree, size_t currentBindingPower, bool hasSeenBinaryOp, const ExprOperatorDefine* operators, size_t amountOfOperators);
static ExprOperatorDefine internal_get_operator(const ExprOperatorDefine* operationDefine, size_t amountOfOperators, int operator);

BL_DynamicContainer          exprTokenize(BL_StringView* expresion,const ExprOperatorDefine* operators, size_t amountOfOperators) {
    unsigned char splitablesList[256] = {' ', '\n', '\t', '\v', '\b', '\f', '\0'};
    bool          operatorChars[256]  = {0};

    for (size_t i = 0; i < amountOfOperators; i++) {
        splitablesList[i + 7]                    = operators[i].operatorID;
        operatorChars[operators[i].operatorID] = true;
    }

    BL_DynamicContainer atoms = bl_string_split_multi(expresion, splitablesList, amountOfOperators + 7, false, 0);
    if (!bl_container_dynamic_is_valid(&atoms))
        return (BL_DynamicContainer) {0};

    BL_DynamicContainer tokens      = bl_container_dynamic_create_stack(0, sizeof(ExprParsingToken));

    size_t           currentAtom = 0, sizeOfAtoms = bl_container_size(&atoms.container);
    bool             isInAtom = false, foundToken = false;
    for (size_t i = 0; i < bl_string_length(expresion); i++) {
        unsigned char currentChar = *bl_string_get_char(expresion, i);
        if (!operatorChars[currentChar] && !isspace(currentChar)) {
            isInAtom = true, foundToken = false;
            continue;
        }

        if (isspace(currentChar)) {
            if (!foundToken && currentAtom < sizeOfAtoms) {
                if (bl_container_dynamic_append(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(BL_String*) bl_container_get((BL_Container*) &atoms, currentAtom)}) != BL_ContainerOPSuccessful)
                    goto errorPath;
                currentAtom++;
            }
            isInAtom = false;
            continue;
        }

        foundToken = true;

        if (i != 0 && isInAtom) {
            if (bl_container_dynamic_append(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(BL_String*) bl_container_get((BL_Container*) &atoms, currentAtom)}) != BL_ContainerOPSuccessful)
                goto errorPath;
            currentAtom++;
            isInAtom = false;
        }

        if (bl_container_dynamic_append(&tokens, sizeof(ExprParsingToken),&(ExprParsingToken){.isAtom = false, .operatorID = currentChar}) != BL_ContainerOPSuccessful)
            goto errorPath;
    }

    for (; currentAtom < sizeOfAtoms; currentAtom++) {
        if (bl_container_dynamic_append(&tokens, sizeof(ExprParsingToken), &(ExprParsingToken) {.isAtom = true, .atom = *(BL_String*) bl_container_get((BL_Container*) &atoms, currentAtom)}) != BL_ContainerOPSuccessful)
            goto errorPath;
    }

    bl_container_destroy(&atoms);
    return tokens;

errorPath:

    bl_container_destroy(&tokens);
    bl_container_dynamic_destroy_with_elements(&atoms, bl_container_destroy);
    return tokens;
}

BL_DynamicContainer exprParse(const BL_DynamicContainer* tokens, const ExprOperatorDefine* operators, size_t amountOfOperators) {
    BL_DynamicContainer tree = bl_container_dynamic_create_stack(bl_container_size((BL_Container*) tokens) + 1, sizeof(ExprOperation));


    if (!bl_container_dynamic_is_valid(&tree))
        return tree;

    if (bl_container_size((BL_Container*) tokens) == 1) {
        ExprParsingToken* token = bl_container_dynamic_front(tokens);
        if (!token->isAtom) {
            LogError("A sole operator is not permitted");
            bl_container_destroy(&tree);
            return tree;
        }

        ExprOperationOperand operand = {.isAtom = true, .atom = token->atom};
        bl_container_dynamic_append(&tree, sizeof(ExprOperation), &(ExprOperation) {.operatorID = '\0', .isBinaryOperation = false, .unaryOperation = {.unaryOperand = operand, .unaryOperatorWasOnRight = false}});
        return tree;
    }

    ExprOperation* result = internal_expr_parse(tokens, &(ExprParsingToken*) {bl_container_dynamic_front(tokens)}, &tree, 0, true, operators, amountOfOperators);
    if (!result) {
        bl_container_destroy(&tree);
        return tree;
    }
    bl_container_dynamic_append(&tree, sizeof(ExprOperation), &(ExprOperation) {.operatorID = '\0', .isBinaryOperation = false, .unaryOperation = {.unaryOperand = (ExprOperationOperand) {.isAtom = false, .operation = result}, .unaryOperatorWasOnRight = false}});
    return tree;
}

ExprOperation* internal_expr_parse(const BL_DynamicContainer* tokens, ExprParsingToken** currentTokenGlobal,
                                   BL_DynamicContainer* tree,
                                   size_t currentBindingPower, bool hasSeenBinaryOp,
                                   const ExprOperatorDefine* operators, size_t amountOfOperators) {
    bool           prevTokenWasAtom  = false;

    ExprOperation* previousOperation = NULL;

    if (*currentTokenGlobal >= (ExprParsingToken*) bl_container_dynamic_end(tokens))
        return NULL;

    for (ExprParsingToken* currentTokenLocal = *currentTokenGlobal; currentTokenLocal; currentTokenLocal = bl_container_dynamic_next(tokens, currentTokenLocal)) {
        if (currentTokenLocal->isAtom && prevTokenWasAtom) {
            LogError("An atom cannot follow an atom without at binary operator in between.");
            *currentTokenGlobal = (ExprParsingToken*) bl_container_dynamic_back(tokens);
            return NULL;
        }

        if (currentTokenLocal->isAtom) {
            if (!hasSeenBinaryOp) {
                LogError("A binary operator must exist between atoms");
                *currentTokenGlobal = (ExprParsingToken*) bl_container_dynamic_back(tokens);
                return NULL;
            }
            prevTokenWasAtom = true;
            hasSeenBinaryOp  = false;
            continue;
        }
        ExprOperatorDefine currentOperator = internal_get_operator(operators, amountOfOperators, currentTokenLocal->operatorID);
        if (!currentOperator.isUnaryOperator && hasSeenBinaryOp) {
            LogError("Two binary operators cannot exist between two atoms");
            *currentTokenGlobal = (ExprParsingToken*) bl_container_dynamic_back(tokens);
            return NULL;
        }



        if ((currentOperator.isUnaryOperator && currentOperator.isBinaryOperator && hasSeenBinaryOp) || (currentOperator.isUnaryOperator && !currentOperator.isBinaryOperator)) {
            if (currentOperator.leftUnaryBinding && currentOperator.leftUnaryBinding <= currentBindingPower) {
                *currentTokenGlobal = currentTokenLocal - 1 - (prevTokenWasAtom ? 1 : 0);
                return previousOperation;
            }

            if (!hasSeenBinaryOp && currentOperator.leftUnaryBinding) {
                if (!previousOperation && !prevTokenWasAtom) { // TODO: Fix so that this will not fail if it is the first operation in all cases
                    LogError("A left binding unary operator must bind to an atom, but no such has been provided before it.");
                    *currentTokenGlobal = bl_container_dynamic_back(tokens);
                    return NULL;
                }
                ExprOperationOperand operand   = !previousOperation
                                                   ? (ExprOperationOperand) {.isAtom = true, .atom = (currentTokenLocal - 1)->atom}
                                                   : (ExprOperationOperand) {.isAtom = false, .operation = previousOperation};
                ExprOperation        operation = {
                           .operatorID      = currentOperator.operatorID,
                           .isBinaryOperation = false,
                           .unaryOperation    = {
                                  .unaryOperand            = operand,
                                  .unaryOperatorWasOnRight = true}};
                bl_container_dynamic_append(tree, sizeof operation, &operation);
                previousOperation = bl_container_dynamic_back(tree);
                prevTokenWasAtom  = false;
                continue;
            }

            if (!currentOperator.rightUnaryBinding) {
                LogError("A left binding operator with no right unary binding cannot be used as a right binding unary operator.");
                *currentTokenGlobal = bl_container_dynamic_back(tokens);
                return NULL;
            }

            currentTokenLocal++;
            ExprOperation* operation = internal_expr_parse(tokens, &currentTokenLocal, tree, currentOperator.rightUnaryBinding, hasSeenBinaryOp, operators, amountOfOperators);
            if (operation) {
                bl_container_dynamic_append(tree,
                                       sizeof(ExprOperation),
                                       &(ExprOperation) {
                                           .operatorID      = currentOperator.operatorID,
                                           .isBinaryOperation = false,
                                           .unaryOperation    = {
                                                  .unaryOperand = (ExprOperationOperand) {
                                                      .isAtom    = false,
                                                      .operation = operation},
                                                  .unaryOperatorWasOnRight = false}});
            } else {
                ExprParsingToken* nextToken = bl_container_dynamic_next(tokens, currentTokenLocal);
                if (!nextToken || !nextToken->isAtom)
                    return NULL;
                bl_container_dynamic_append(tree,
                                       sizeof(ExprOperation),
                                       &(ExprOperation) {
                                           .operatorID      = currentOperator.operatorID,
                                           .isBinaryOperation = false,
                                           .unaryOperation    = {
                                                  .unaryOperand = (ExprOperationOperand) {
                                                      .isAtom = true,
                                                      .atom   = nextToken->atom},
                                                  .unaryOperatorWasOnRight = false}});
            }

            previousOperation = bl_container_dynamic_back(tree);
            prevTokenWasAtom  = false;
            continue;
        }

        if (currentOperator.lhsBinaryBinding <= currentBindingPower) {
            *currentTokenGlobal = currentTokenLocal - 2;
            return previousOperation;
        }

        hasSeenBinaryOp = true;

        ExprOperationOperand lhs, rhs;
        if (previousOperation)
            lhs = (ExprOperationOperand) {.isAtom = false, .operation = previousOperation};
        else
            lhs = (ExprOperationOperand) {.isAtom = true, .atom = (currentTokenLocal - 1)->atom};
        currentTokenLocal++;
        ExprOperation* rhsOperation = internal_expr_parse(tokens, &currentTokenLocal, tree, currentOperator.rhsBinaryBinding, hasSeenBinaryOp, operators, amountOfOperators);
        if (!rhsOperation) {
            ExprParsingToken* nextToken = bl_container_dynamic_next(tokens, currentTokenLocal);
            if (!nextToken || !nextToken->isAtom)
                return NULL;
            rhs = (ExprOperationOperand) {.isAtom = true, .atom = nextToken->atom};
        } else
            rhs = (ExprOperationOperand) {.isAtom = false, .operation = rhsOperation};

        ExprOperation nextOperation = {.operatorID      = currentOperator.operatorID,
                                       .isBinaryOperation = true,
                                       .binaryOperands    = {.lhs = lhs,
                                                             .rhs = rhs}};
        bl_container_dynamic_append(tree, sizeof nextOperation, &nextOperation);
        previousOperation = bl_container_dynamic_back(tree);
        prevTokenWasAtom  = false;
    }
    *currentTokenGlobal = (ExprParsingToken*) bl_container_dynamic_back(tokens) - 1;
    return previousOperation;
}



static ExprOperatorDefine internal_get_operator(const ExprOperatorDefine* operationDefine, size_t amountOfOperators, int operator) {
    size_t i = 0;
    while (i < amountOfOperators) {
        if (operationDefine[i].operatorID == operator)
            return operationDefine[i];
        i++;
    }
    return (ExprOperatorDefine) {0};
}

static void internal_expr_print(FILE* file, const ExprOperation* operation) {
    fputc('(',file);
    if (operation->isBinaryOperation) {
        if (!operation->binaryOperands.lhs.isAtom)
            internal_expr_print(file, operation->binaryOperands.lhs.operation);
        else
            fputs(bl_container_dynamic_front(&operation->binaryOperands.lhs.atom), file);
        fputc(' ', file);
        if (!operation->binaryOperands.rhs.isAtom)
            internal_expr_print(file, operation->binaryOperands.rhs.operation);
        else
            fputs(bl_container_dynamic_front(&operation->binaryOperands.rhs.atom), file);
        fputc(' ', file);
        fputc(operation->operatorID, file);
    } else {
        if (!operation->unaryOperation.unaryOperatorWasOnRight) {
            if (operation->operatorID == '\0')
                fprintf(file, "Expr: ");
            else
                fputc(operation->operatorID, file);
        }
        if (!operation->unaryOperation.unaryOperand.isAtom)
            internal_expr_print(file, operation->unaryOperation.unaryOperand.operation);
        else
            fputs(bl_container_dynamic_front(&operation->unaryOperation.unaryOperand.atom), file);

        if (operation->unaryOperation.unaryOperatorWasOnRight)
            fputc(operation->operatorID, file);
    }
    fputc(')',file);
}

void exprPrint(FILE* file, const BL_DynamicContainer* tree) {
    internal_expr_print(file, bl_container_dynamic_back(tree));
    fputc('\n', file);
}