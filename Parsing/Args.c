#include "Args.h"
#include <stddef.h>
#include <BackerLibLogging.h>
#include <BackerLibTypes.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static BL_Arg* internal_argslist = NULL;
static size_t internal_argslist_len = 0;

static bool internal_sort_argslist(const void* first, const void* second) {
    return bl_string_compare_decending(&((BL_Arg*)first)->param, &((BL_Arg*)second)->param) <= 0;
}

void bl_internal_register_argslist(BL_Arg *argslist,size_t len) {
    internal_argslist = argslist;
    internal_argslist_len = len;
    bl_sort_heap(&(BL_Container){.header = ObjectFlagIsValid | ObjectFlagIsContainer,.amountOfIndexes = len, .array = argslist, .byteSizeOfSingleElement = sizeof(BL_Arg)}, internal_sort_argslist);
}

BL_Arg* bl_args_get(const BL_StringView* name) {
    if (!internal_argslist)
        return NULL;

    size_t begin = 0;
    size_t end = internal_argslist_len;

    while (end - begin > 1) {
        size_t mid = (end + begin) / 2;
        if (bl_string_compare_acending(name, &internal_argslist[mid].param))
            end = mid;
        else
            begin = mid;
    }

    if (bl_string_equal(name, &internal_argslist[begin].param))
        return &internal_argslist[begin];
    else if (end != internal_argslist_len && bl_string_equal(name, &internal_argslist[end].param))
        return &internal_argslist[end];
    return NULL;
}


bool bl_args_parse(int argc, char* argv[]) {
    if (!internal_argslist)
        return false;

    bool expectingArg = false;
    BL_Arg* argDef = NULL;
    
    for (int i = 0; i < argc; i++) {
        if (expectingArg) {
            argDef->arg.str = argv[i];
            expectingArg = false;
            continue;
        }
        size_t len = strcspn(argv[i], "=\0");
        if (argv[i][0] == '-') {
            argDef = bl_args_get(&(BL_StringView){.byteSizeOfSingleElement = 1,.amountOfIndexes = len + 1,.header = ObjectFlagIsContainer | ObjectFlagIsValid, .array = (unsigned char*)argv[i]});

            if (!argDef)  {
                bl_log_debug_location("Unknown argument: %*.s",len,argv[i]);
                return false;
            }

            if (argv[i][len] != '\0' && argDef->type != BL_ArgBoolean) {
                char* arg = argv[i] + len + 1;
                switch (argDef->type) {
                    case BL_ArgBoolean: break;
                    case BL_ArgStr: argDef->arg.str = arg; break;
                    case BL_ArgFloatingPoint: {
                        char* endPtr = NULL;
                        argDef->arg.fp = strtod(arg,&endPtr);
                        if (arg == endPtr || *endPtr) {
                            bl_log_debug("Expected floating point number.");
                            return false;
                        }
                    } break;
                    case BL_ArgSInt: {
                        char* endPtr = NULL;
                        argDef->arg.sInt = strtoll(arg, &endPtr, 0);
                        if (arg == endPtr || *endPtr) {
                            bl_log_debug("Expected integer.");
                            return false;
                        }
                    } break;
                    case BL_ArgUInt: {
                        char* endPtr = NULL;
                        argDef->arg.uInt = strtoull(arg, &endPtr, 0);
                        if (arg == endPtr || *endPtr || *arg == '-') {
                            bl_log_debug("Expected unsigned integer.");
                            return false;
                        }
                    } break;
                }
            } else if (argDef->type == BL_ArgBoolean) {
                if (argv[i][len] != '\0') {
                    bl_log_debug_location("%*.s is a boolean flag and expects no arguments",len,argv[i]);
                    return false;
                }
                argDef->arg.boolean = true;
            }
            else if (argDef->type == BL_ArgStr){
                expectingArg = true;
            } else {
                bl_log_debug_location("Expected argument to parameter: %s",argv[i]);
                return false;
            }
        }
    }
    if (expectingArg) {
        bl_log_debug_location("Expected argument for parameter.");
        return false;
    }
    return true;
}

char* bl_args_next_arg(int argc,char* argv[],char* current) {
    size_t beginIndex = SIZE_MAX;
    if (!current)
        beginIndex = 0;
    else {
        for (int i = 0; i < argc; i++) {
            if (argv[i] == current) {
                beginIndex = i;
                break;
            }
        }
        if (beginIndex == SIZE_MAX)
            return NULL;
    }

    for (int i = beginIndex; i < argc; i++) {
        if (argv[i][0] != '-')
            return argv[i];
        size_t len = strcspn(argv[i], "=\0");
        BL_Arg* arg = bl_args_get(&(BL_StringView){.byteSizeOfSingleElement = 1,.amountOfIndexes = len + 1,.header = ObjectFlagIsContainer | ObjectFlagIsValid, .array = (unsigned char*)argv[i]});
        if (arg == NULL)
            continue;
        if (arg->type == BL_ArgStr && !argv[i][len])
            i++;
    }
    return NULL;
}

void bl_args_print_defs(void) {
    for (size_t i = 0; i < internal_argslist_len; i++) {
        printf("%s %s",internal_argslist[i].param.array,internal_argslist[i].defStr);
        switch (internal_argslist[i].type) {
            case BL_ArgBoolean: puts(""); break;
            case BL_ArgUInt: printf(" Defualt: %"PRIo64"\n",internal_argslist[i].defualt.uInt); break;
            case BL_ArgSInt: printf(" Defualt: %"PRIi64"\n",internal_argslist[i].defualt.sInt); break;
            case BL_ArgFloatingPoint: printf(" Defualt: %lf\n",internal_argslist[i].defualt.fp); break;
            case BL_ArgStr: printf(" Defualt: %s\n",internal_argslist[i].defualt.str ? internal_argslist[i].defualt.str : "[[NULL]]"); break;
        }
    }
}
