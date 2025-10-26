#ifndef BACKERLIB_EXPRPARSING_H
#define BACKERLIB_EXPRPARSING_H

#include <BackerLibTypes.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {

#endif //__cplusplus

    typedef String ExprAtom;

    typedef struct ExprOperatorDefine {
        unsigned char operator;
        bool          isBinaryOperator;
        bool          isUnaryOperator;
        size_t        leftUnaryBinding, rightUnaryBinding, lhsBinaryBinding, rhsBinaryBinding;
    } ExprOperatorDefine;

    typedef struct ExprOperation ExprOperation;

    typedef struct ExprOperationOperand {
        bool isAtom;
        union {
            ExprAtom*       atom;
            ExprOperation* operation;
        };
    } ExprOperationOperand;

    typedef struct ExprParsingToken {
        bool isAtom;
        union {
            ExprAtom      atom;
            unsigned char operator;
        };
    } ExprParsingToken;

    struct ExprOperation {
        unsigned char operator;
        bool          isBinaryOperation;
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

    DynamicContainer exprTokenize(StringView* expresion, ExprOperatorDefine* operators, size_t amountOfOperators);
    DynamicContainer exprParse(const DynamicContainer* tokens, ExprOperatorDefine* operators, size_t amountOfOperators);


#ifdef __cplusplus
    }
};
#endif //__cplusplus
#endif // BACKERLIB_EXPRPARSING_H