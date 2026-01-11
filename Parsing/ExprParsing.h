#ifndef BACKERLIB_EXPRPARSING_H
#define BACKERLIB_EXPRPARSING_H

#include <BackerLibTypes.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif //__cplusplus

    typedef BL_String BL_ExprAtom; // Atom will be changed at a later date to a better suitable type.

    typedef struct BL_ExprOperatorDefine {
        int    operatorID;
        bool   isBinaryOperator;
        bool   isUnaryOperator;
        size_t leftUnaryBinding, rightUnaryBinding, lhsBinaryBinding, rhsBinaryBinding;
    } BL_ExprOperatorDefine;

    typedef struct BL_ExprOperation BL_ExprOperation;

    typedef struct BL_ExprOperationOperand {
        bool isAtom;
        union {
            BL_ExprAtom       atom;
            BL_ExprOperation* operation;
        };
    } BL_ExprOperationOperand;

    typedef struct BL_ExprParsingToken {
        bool isAtom;
        union {
            BL_ExprAtom atom;
            int      operatorID;
        };
    } BL_ExprParsingToken;

    struct BL_ExprOperation {
        int  operatorID;
        bool isBinaryOperation;
        union {
            struct {
                BL_ExprOperationOperand unaryOperand;
                bool                 unaryOperatorWasOnRight;
            } unaryOperation;
            struct {
                BL_ExprOperationOperand lhs;
                BL_ExprOperationOperand rhs;
            } binaryOperands;
        };
    };

    /**
     * @brief Basic tokenizing function.
     * @param expresion Pointer to string of expression to tokenize
     * @param operators Pointer to operators to use in tokenization
     * @param amountOfOperators Length of operators
     * @return Valid DynamicContainer if operation was successful.
     */
    extern BL_DynamicContainer bl_expr_tokenize(BL_StringView* expresion, const BL_ExprOperatorDefine* operators, size_t amountOfOperators) noexcept;
    /**
     * @brief Parses tokens to a valid AST of an expression, if possible from provided operators.
     * @param tokens Pointer to valid DynamicContainer of tokens
     * @param operators Pointer to operators to use in tokenization
     * @param amountOfOperators Length of operators
     * @return Valid DynamicContainer if expression could be parsed.
     * @note Result cannot free internal memory allocations as the atoms internal array uses memory to that of the atoms in token. Instead, freeing the atoms in tokens should free them to aviod double frees.
     * @note
     * The function will always choose the last binary operator possible as the binary operator between atoms.
     * Operator pairs like a * (b + c) assuming () has the lowest priority followed by + and * will parse to a (b c + *) because of operator precedence. Recommended for tokenizers to catch this.
     * Iterating the result inorder will traverse the AST in execution order. The last element always has operatorID set to 0.
     */
    extern BL_DynamicContainer bl_expr_parse(const BL_DynamicContainer* tokens, const BL_ExprOperatorDefine* operators, size_t amountOfOperators) noexcept;
    /**
     * @brief Prints a AST in reverse polish notation.
     * @param file Pointer to file to write to.
     * @param tree Valid pointer to result of exprParse
     */
    extern void             bl_expr_print(FILE* file, const BL_DynamicContainer* tree) noexcept;


#ifdef __cplusplus
}
#else
#undef noexcept
#endif //__cplusplus
#endif // BACKERLIB_EXPRPARSING_H