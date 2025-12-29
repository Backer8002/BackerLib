#include "ExprParsing.h"

#include <BackerLibLogging.h>
#include <BackerLibTypes.h>
#include <ctype.h>
#include <stdio.h>

static BL_ExprOperation*     internal_expr_parse(const BL_DynamicContainer* tokens, BL_ExprParsingToken** currentTokenGlobal, BL_DynamicContainer* tree, size_t currentBindingPower, bool hasSeenBinaryOp, const BL_ExprOperatorDefine* operators, size_t amountOfOperators);
static BL_ExprOperatorDefine internal_get_operator(const BL_ExprOperatorDefine* operationDefine, size_t amountOfOperators, int operator);

BL_DynamicContainer          bl_expr_tokenize(BL_StringView* expresion,const BL_ExprOperatorDefine* operators, size_t amountOfOperators) {
    unsigned char splitablesList[256] = {' ', '\n', '\t', '\v', '\b', '\f', '\0'};
    bool          operatorChars[256]  = {0};

    for (size_t i = 0; i < amountOfOperators; i++) {
        splitablesList[i + 7]                    = operators[i].operatorID;
        operatorChars[operators[i].operatorID] = true;
    }

    BL_DynamicContainer atoms = bl_string_split_multi(expresion, splitablesList, amountOfOperators + 7, false, 0);
    if (!bl_container_dynamic_is_valid(&atoms))
        return (BL_DynamicContainer) {0};

    BL_DynamicContainer tokens      = bl_container_dynamic_create_stack(0, sizeof(BL_ExprParsingToken));

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
                if (bl_container_dynamic_append(&tokens, sizeof(BL_ExprParsingToken), &(BL_ExprParsingToken) {.isAtom = true, .atom = *(BL_String*) bl_container_get((BL_Container*) &atoms, currentAtom)}) != BL_ContainerOPSuccessful)
                    goto errorPath;
                currentAtom++;
            }
            isInAtom = false;
            continue;
        }

        foundToken = true;

        if (i != 0 && isInAtom) {
            if (bl_container_dynamic_append(&tokens, sizeof(BL_ExprParsingToken), &(BL_ExprParsingToken) {.isAtom = true, .atom = *(BL_String*) bl_container_get((BL_Container*) &atoms, currentAtom)}) != BL_ContainerOPSuccessful)
                goto errorPath;
            currentAtom++;
            isInAtom = false;
        }

        if (bl_container_dynamic_append(&tokens, sizeof(BL_ExprParsingToken),&(BL_ExprParsingToken){.isAtom = false, .operatorID = currentChar}) != BL_ContainerOPSuccessful)
            goto errorPath;
    }

    for (; currentAtom < sizeOfAtoms; currentAtom++) {
        if (bl_container_dynamic_append(&tokens, sizeof(BL_ExprParsingToken), &(BL_ExprParsingToken) {.isAtom = true, .atom = *(BL_String*) bl_container_get((BL_Container*) &atoms, currentAtom)}) != BL_ContainerOPSuccessful)
            goto errorPath;
    }

    bl_container_destroy(&atoms);
    return tokens;

errorPath:

    bl_container_destroy(&tokens);
    bl_container_dynamic_destroy_with_elements(&atoms, bl_container_destroy);
    return tokens;
}

BL_DynamicContainer bl_expr_parse(const BL_DynamicContainer* tokens, const BL_ExprOperatorDefine* operators, size_t amountOfOperators) {
    BL_DynamicContainer tree = bl_container_dynamic_create_stack(bl_container_size((BL_Container*) tokens) + 1, sizeof(BL_ExprOperation));


    if (!bl_container_dynamic_is_valid(&tree))
        return tree;

    if (bl_container_size((BL_Container*) tokens) == 1) {
        BL_ExprParsingToken* token = bl_container_dynamic_front(tokens);
        if (!token->isAtom) {
            bl_log_debug("A sole operator is not permitted");
            bl_container_destroy(&tree);
            return tree;
        }

        BL_ExprOperationOperand operand = {.isAtom = true, .atom = token->atom};
        bl_container_dynamic_append(&tree, sizeof(BL_ExprOperation), &(BL_ExprOperation) {.operatorID = '\0', .isBinaryOperation = false, .unaryOperation = {.unaryOperand = operand, .unaryOperatorWasOnRight = false}});
        return tree;
    }

    BL_ExprOperation* result = internal_expr_parse(tokens, &(BL_ExprParsingToken*) {bl_container_dynamic_front(tokens)}, &tree, 0, true, operators, amountOfOperators);
    if (!result) {
        bl_container_destroy(&tree);
        return tree;
    }
    bl_container_dynamic_append(&tree, sizeof(BL_ExprOperation), &(BL_ExprOperation) {.operatorID = '\0', .isBinaryOperation = false, .unaryOperation = {.unaryOperand = (BL_ExprOperationOperand) {.isAtom = false, .operation = result}, .unaryOperatorWasOnRight = false}});
    return tree;
}

BL_ExprOperation* internal_expr_parse(const BL_DynamicContainer* tokens, BL_ExprParsingToken** currentTokenGlobal,
                                   BL_DynamicContainer* tree,
                                   size_t currentBindingPower, bool hasSeenBinaryOp,
                                   const BL_ExprOperatorDefine* operators, size_t amountOfOperators) {
    bool           prevTokenWasAtom  = false;

    BL_ExprOperation* previousOperation = NULL;

    if (*currentTokenGlobal >= (BL_ExprParsingToken*) bl_container_dynamic_end(tokens))
        return NULL;

    for (BL_ExprParsingToken* currentTokenLocal = *currentTokenGlobal; currentTokenLocal; currentTokenLocal = bl_container_dynamic_next(tokens, currentTokenLocal)) {
        if (currentTokenLocal->isAtom && prevTokenWasAtom) {
            bl_log_debug("An atom cannot follow an atom without at binary operator in between.");
            *currentTokenGlobal = (BL_ExprParsingToken*) bl_container_dynamic_back(tokens);
            return NULL;
        }

        if (currentTokenLocal->isAtom) {
            if (!hasSeenBinaryOp) {
                bl_log_debug("A binary operator must exist between atoms");
                *currentTokenGlobal = (BL_ExprParsingToken*) bl_container_dynamic_back(tokens);
                return NULL;
            }
            prevTokenWasAtom = true;
            hasSeenBinaryOp  = false;
            continue;
        }
        BL_ExprOperatorDefine currentOperator = internal_get_operator(operators, amountOfOperators, currentTokenLocal->operatorID);
        if (!currentOperator.isUnaryOperator && hasSeenBinaryOp) {
            bl_log_debug("Two binary operators cannot exist between two atoms");
            *currentTokenGlobal = (BL_ExprParsingToken*) bl_container_dynamic_back(tokens);
            return NULL;
        }



        if ((currentOperator.isUnaryOperator && currentOperator.isBinaryOperator && hasSeenBinaryOp) || (currentOperator.isUnaryOperator && !currentOperator.isBinaryOperator)) {
            if (currentOperator.leftUnaryBinding && currentOperator.leftUnaryBinding <= currentBindingPower) {
                *currentTokenGlobal = currentTokenLocal - 1 - (prevTokenWasAtom ? 1 : 0);
                return previousOperation;
            }

            if (!hasSeenBinaryOp && currentOperator.leftUnaryBinding) {
                if (!previousOperation && !prevTokenWasAtom) { // TODO: Fix so that this will not fail if it is the first operation in all cases
                    bl_log_debug("A left binding unary operator must bind to an atom, but no such has been provided before it.");
                    *currentTokenGlobal = bl_container_dynamic_back(tokens);
                    return NULL;
                }
                BL_ExprOperationOperand operand   = !previousOperation
                                                   ? (BL_ExprOperationOperand) {.isAtom = true, .atom = (currentTokenLocal - 1)->atom}
                                                   : (BL_ExprOperationOperand) {.isAtom = false, .operation = previousOperation};
                BL_ExprOperation        operation = {
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
                bl_log_debug("A left binding operator with no right unary binding cannot be used as a right binding unary operator.");
                *currentTokenGlobal = bl_container_dynamic_back(tokens);
                return NULL;
            }

            currentTokenLocal++;
            BL_ExprOperation* operation = internal_expr_parse(tokens, &currentTokenLocal, tree, currentOperator.rightUnaryBinding, hasSeenBinaryOp, operators, amountOfOperators);
            if (operation) {
                bl_container_dynamic_append(tree,
                                       sizeof(BL_ExprOperation),
                                       &(BL_ExprOperation) {
                                           .operatorID      = currentOperator.operatorID,
                                           .isBinaryOperation = false,
                                           .unaryOperation    = {
                                                  .unaryOperand = (BL_ExprOperationOperand) {
                                                      .isAtom    = false,
                                                      .operation = operation},
                                                  .unaryOperatorWasOnRight = false}});
            } else {
                BL_ExprParsingToken* nextToken = bl_container_dynamic_next(tokens, currentTokenLocal);
                if (!nextToken || !nextToken->isAtom)
                    return NULL;
                bl_container_dynamic_append(tree,
                                       sizeof(BL_ExprOperation),
                                       &(BL_ExprOperation) {
                                           .operatorID      = currentOperator.operatorID,
                                           .isBinaryOperation = false,
                                           .unaryOperation    = {
                                                  .unaryOperand = (BL_ExprOperationOperand) {
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

        BL_ExprOperationOperand lhs, rhs;
        if (previousOperation)
            lhs = (BL_ExprOperationOperand) {.isAtom = false, .operation = previousOperation};
        else
            lhs = (BL_ExprOperationOperand) {.isAtom = true, .atom = (currentTokenLocal - 1)->atom};
        currentTokenLocal++;
        BL_ExprOperation* rhsOperation = internal_expr_parse(tokens, &currentTokenLocal, tree, currentOperator.rhsBinaryBinding, hasSeenBinaryOp, operators, amountOfOperators);
        if (!rhsOperation) {
            BL_ExprParsingToken* nextToken = bl_container_dynamic_next(tokens, currentTokenLocal);
            if (!nextToken || !nextToken->isAtom)
                return NULL;
            rhs = (BL_ExprOperationOperand) {.isAtom = true, .atom = nextToken->atom};
        } else
            rhs = (BL_ExprOperationOperand) {.isAtom = false, .operation = rhsOperation};

        BL_ExprOperation nextOperation = {.operatorID      = currentOperator.operatorID,
                                       .isBinaryOperation = true,
                                       .binaryOperands    = {.lhs = lhs,
                                                             .rhs = rhs}};
        bl_container_dynamic_append(tree, sizeof nextOperation, &nextOperation);
        previousOperation = bl_container_dynamic_back(tree);
        prevTokenWasAtom  = false;
    }
    *currentTokenGlobal = (BL_ExprParsingToken*) bl_container_dynamic_back(tokens) - 1;
    return previousOperation;
}



static BL_ExprOperatorDefine internal_get_operator(const BL_ExprOperatorDefine* operationDefine, size_t amountOfOperators, int operator) {
    size_t i = 0;
    while (i < amountOfOperators) {
        if (operationDefine[i].operatorID == operator)
            return operationDefine[i];
        i++;
    }
    return (BL_ExprOperatorDefine) {0};
}

static void internal_expr_print(FILE* file, const BL_ExprOperation* operation) {
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

void bl_expr_print(FILE* file, const BL_DynamicContainer* tree) {
    internal_expr_print(file, bl_container_dynamic_back(tree));
    fputc('\n', file);
}