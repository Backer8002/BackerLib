#include "Regex.h"
#include <BackerLibLogging.h>
#include <BackerLibTextprocessing.h>
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_AMOUNT_OF_NODES (SIZE_MAX - 2)
#define INVALID_END_NODE    (SIZE_MAX - 1)
#define VALID_END_NODE      (SIZE_MAX - 2)

struct CharacterRange {
    BL_Unicodepoint begin;
    BL_Unicodepoint end;
};

enum RegexASTOperation {
    RegexOpConcat,
    RegexOpUnion,
    RegexOpRepeat,
    RegexOpRepeatGreedy,
    RegexOpUnionRange,
    RegexOpOptional
};

struct RegexASTNode;

struct RegexASTOp {
    bool isAtom;
    union {
        struct RegexASTNode* node;
        BL_Unicodepoint      codepoint;
    };
};

struct RegexASTNode {
    enum RegexASTOperation operation;
    struct RegexASTNode* firstNode;
    union {
        struct RegexASTOp secondOp;
        BL_DynamicContainer ranges;
    };
};

static BL_DynamicContainer internal_make_regex_ast(BL_UnicodeView expr) {
    if (bl_unicodestr_length(expr) == 0) {
        bl_log_debug_location("%s","The empty word is not supported.");
        return (BL_DynamicContainer){0};
    }
    BL_DynamicContainer  result = bl_container_dynamic_create_stack(bl_unicodestr_length(expr), sizeof(struct RegexASTNode));
    if (!bl_container_dynamic_is_valid(&result))
        return result;
    BL_DynamicContainer  stack  = bl_container_dynamic_create_stack(0, sizeof(struct RegexASTNode));
    struct RegexASTNode* prev = NULL;
    struct RegexASTNode current = {.operation = RegexOpConcat,.firstNode = NULL};
    size_t amountOfStopablePoints = 0;

    for (const BL_Unicodepoint* character = bl_unicodestr_front(expr); character; character = bl_unicodestr_next(expr, character)) {
        if (*character == U'(') {
            BL_ContainerError errorCode = bl_container_dynamic_append(&result, sizeof current, &current);
            if (errorCode != BL_ContainerOPSuccessful) {
                bl_log_debug_location("%s", "Unable to push to stack");
	      			  goto Cleanup;
            }
            current = (struct RegexASTNode){.operation = RegexOpConcat, .firstNode = NULL};
            continue;
        }
        if (*character == U')') {
            if (bl_container_dynamic_is_empty(&stack)) {
                bl_log_debug_location("%s", "Unexpected ')' which closes nonexistant scope.");
				        goto Cleanup;
            }
			      current.secondOp = (struct RegexASTOp){.isAtom = false,.node = (struct RegexASTNode*)bl_container_dynamic_back(&stack)};
            bl_container_dynamic_pop(&stack);

        } else if (*character == U'|') {
            if (current.operation != RegexOpUnion && current.firstNode) {
                current.operation = RegexOpUnion;
                continue;
            }
            current.secondOp = (struct RegexASTOp){.isAtom = true, .codepoint = *character};
        } else if (*character == U'[') {
            #error Do character range stuff;
        } else
            current.secondOp = (struct RegexASTOp){.isAtom = true, .codepoint = *character};
        
        if (current.operation == RegexOpUnion) {
            
        }

        BL_Unicodepoint* optionalModifier1 = bl_unicodestr_next(expr,character);
        if (optionalModifier1 && (*optionalModifier1 == U'?' || *optionalModifier1 == U'+' || *optionalModifier1 == U'*')) {
            character = optionalModifier1;
            amountOfStopablePoints++;

            if (*optionalModifier1 == U'?') {
                struct RegexASTNode optional = {.operation = RegexOpOptional,.secondOp = current.secondOp};
                if (bl_container_dynamic_append(&result,sizeof optional,&optional) != BL_ContainerOPSuccessful)
                    goto Cleanup;

                current.secondOp = (struct RegexASTOp){.isAtom = false, .node = bl_container_dynamic_back(&result)};
            } else {
                if (*optionalModifier1 == '+') {
                    if (bl_container_dynamic_append(&result, sizeof current, &current) != BL_ContainerOPSuccessful)
                        goto Cleanup;
                    current.firstNode = bl_container_dynamic_back(&result);
                }

                BL_Unicodepoint* optionalModifier2 = bl_unicodestr_next(expr,character);
                if (optionalModifier2 && *optionalModifier2 == U'?')
                    character = optionalModifier2;

                struct RegexASTNode repeatingNode = {.operation = *character == U'?' ? RegexOpRepeat : RegexOpRepeatGreedy, .secondOp = current.secondOp};
                if (bl_container_dynamic_append(&result, sizeof repeatingNode, &repeatingNode) != BL_ContainerOPSuccessful)
                    goto Cleanup;
                current.secondOp = (struct RegexASTOp){.isAtom = false, .node = bl_container_dynamic_back(&result)};
            }
        }
        if (bl_container_dynamic_append(&result,sizeof current, &current) != BL_ContainerOPSuccessful)
            goto Cleanup;
        current = (struct RegexASTNode){.operation = RegexOpConcat,.firstNode = bl_container_dynamic_back(&result)};
    }

    return result;

Cleanup:
    bl_container_dynamic_destroy(&stack);
    bl_container_dynamic_destroy(&result);
    return result;
}


BL_Regex bl_regex_create(BL_UnicodeView expr) {
    BL_DynamicContainer tokens = internal_make_regex_ast(expr);
    if (!bl_container_dynamic_is_valid(&tokens))
        return (BL_Regex){0};


    BL_Regex regex = {.dfaNodes = bl_container_dynamic_create_stack(0, sizeof(struct DFANode))};

    return regex;
}
