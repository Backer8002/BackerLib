#ifndef BL_PARSING_ARGS
#define BL_PARSING_ARGS

#include <stdint.h>
#include <stddef.h>
#include <BackerLibTypes.h>

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

typedef enum BL_ArgType {
    BL_ArgBoolean,
    BL_ArgSInt,
    BL_ArgUInt,
    BL_ArgFloatingPoint,
    BL_ArgStr
} BL_ArgType;

typedef struct BL_Arg {
    const char* defStr;
    BL_StringView param;
    union {
        bool boolean;
        uint64_t uInt;
        int64_t sInt;
        double fp;
        const char* str;  
    } arg;
    union {
        uint64_t uInt;
        int64_t sInt;
        double fp;
        const char* str;  
    } defualt;
    BL_ArgType type;
} BL_Arg;

void bl_internal_register_argslist(BL_Arg* argslist,size_t len) noexcept;
BL_Arg* bl_args_get(const BL_StringView* name) noexcept;
bool bl_args_parse(int argc, char* argv[]) noexcept;
char* bl_args_next_arg(int argc,char* argv[],char* current) noexcept;
void bl_args_print_defs(void) noexcept;

#define BL_ARGSLIST_BEGIN BL_Arg bl_internal_argslist[] = {
#define BL_ARG_Flag(name,definition) {.defStr = definition,.param = bl_stringview_init_constexpr(name),.type = BL_ArgBoolean},
#define BL_ARG_SignedInterger(name,defualtArg,definition) {.defStr = definition,.param = bl_stringview_init_constexpr(name),.defualt = {.sInt = defualtArg},.arg = {.sInt = defualtArg},.type = BL_ArgSInt},
#define BL_ARG_UnsignedInterger(name,defualtArg,definition) {.defStr = definition,.param = bl_stringview_init_constexpr(name),.defualt = {.uInt = defualtArg},.arg = {.uInt = defualtArg},.type = BL_ArgUInt},
#define BL_ARG_FloatingPoint(name,defualtArg,definition) {.defStr = definition,.param = bl_stringview_init_constexpr(name),.defualt = {.fp = defualtArg},.arg= {.fp = defualtArg},.type = BL_ArgFloatingPoint},
#define BL_ARG_Str(name,defualtArg,definition) {.defStr = definition,.param = bl_stringview_init_constexpr(name),.defualt = {.str = defualtArg},.arg = {.str = defualtArg},.type = BL_ArgStr},
#define BL_ARGSLIST_END }; 
#define BL_ARGS_INIT() bl_internal_register_argslist(bl_internal_argslist,sizeof bl_internal_argslist / sizeof(BL_Arg))

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif
