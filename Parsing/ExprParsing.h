#ifndef BACKERLIB_EXPRPARSING_H
#define BACKERLIB_EXPRPARSING_H

#include <BackerLibTypes.h>
#include <stdio.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif //__cplusplus

    typedef BL_String ExprAtom; // Atom will be changed at a later date to a better suitable type.

    typedef struct ExprOperatorDefine {
        int    operatorID;
        bool   isBinaryOperator;
        bool   isUnaryOperator;
        size_t leftUnaryBinding, rightUnaryBinding, lhsBinaryBinding, rhsBinaryBinding;
    } ExprOperatorDefine;

    typedef struct ExprOperation ExprOperation;

    typedef struct ExprOperationOperand {
        bool isAtom;
        union {
            ExprAtom       atom;
            ExprOperation* operation;
        };
    } ExprOperationOperand;

    typedef struct ExprParsingToken {
        bool isAtom;
        union {
            ExprAtom atom;
            int      operatorID;
        };
    } ExprParsingToken;

    struct ExprOperation {
        int  operatorID;
        bool isBinaryOperation;
        union {
            struct {
                ExprOperationOperand unaryOperand;
                bool                 unaryOperatorWasOnRight;
            } unaryOperation;
            struct {
                ExprOperationOperand lhs;
                ExprOperationOperand rhs;
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
    extern BL_DynamicContainer exprTokenize(BL_StringView* expresion, const ExprOperatorDefine* operators, size_t amountOfOperators) noexcept;
    /**
     * @brief Parses tokens to a valid AST of an expression, if possible from provided operators.
     * @param tokens Pointer to valid DynamicContainer of tokens
     * @param operators Pointer to operators to use in tokenization
     * @param amountOfOperators Length of operators
     * @return Valid DynamicContainer if expression could be parsed.
     * @note Result cannot free internal memory allocations as the atoms internal array uses memory to that of the atoms in token. Instead, freeing the atoms in tokens should free them to aviod double frees.
     * @note A current bug were an expression like a + b! cannot be parsed if ! has higher prescience than +. The solution is to surround b in lower binding unary operators.
     * The function will always choose the first binary operator as the binary operator between functions.
     * Operator pairs like a * (b + c) assuming () has the lowest priority followed by + and * will parse to a (b c + *) because of operator precedence. Recommended for tokenizers to catch this.
     * Iterating the result inorder will traverse the AST in execution order. The last element should always have operatorID set to 0.
     */
    extern BL_DynamicContainer exprParse(const BL_DynamicContainer* tokens, const ExprOperatorDefine* operators, size_t amountOfOperators) noexcept;
    /**
     * @brief Prints a AST in reverse polish notation.
     * @param file Pointer to file to write to.
     * @param tree Valid pointer to result of exprParse
     */
    extern void             exprPrint(FILE* file, const BL_DynamicContainer* tree) noexcept;


#ifdef __cplusplus
    }
};
#endif //__cplusplus
#endif // BACKERLIB_EXPRPARSING_H