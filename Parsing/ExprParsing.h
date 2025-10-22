#ifndef BACKERLIB_EXPRPARSING_H
#define BACKERLIB_EXPRPARSING_H

#include <BackerLibTypes.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {

#endif //__cplusplus


    typedef DynamicContainer ExprTokenStore;

    typedef String ExprAtom;

    typedef enum ExprOperatorType {
        ExprOperatorUnary,
        ExprOperatorBinary
    } ExprOperatorType;

    typedef struct ExprOperatorDefine {
        char             operator;
        ExprOperatorType operatorType;
    } ExprTokenDefine;

#ifdef __cplusplus
}
};
#endif //__cplusplus
#endif // BACKERLIB_EXPRPARSING_H