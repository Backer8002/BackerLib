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
    BL_DynamicContainer  result = bl_container_dynamic_create_stack(bl_unicodestr_length(expr), sizeof(struct RegexASTNode*));
    if (!bl_container_dynamic_is_valid(&result))
        return result;
    BL_DynamicContainer  stack  = bl_container_dynamic_create_stack(0, sizeof(struct RegexASTNode));

    bl_container_dynamic_append(&result, sizeof(struct RegexASTNode), &(struct RegexASTNode){.operation = RegexOpConcat,.firstNode = NULL});
    struct RegexASTNode* current   = bl_container_dynamic_back(&result);

    for (const BL_Unicodepoint* character = bl_unicodestr_front(expr); character; character = bl_unicodestr_next(expr, character)) {
        if (*character == U'(') {
            BL_ContainerError errorCode = bl_container_dynamic_append(&result, sizeof(struct RegexASTNode*), &current);
            if (errorCode != BL_ContainerOPSuccessful) {
                bl_log_debug_location("%s", "Unable to push to stack");
				goto Cleanup;
            }
            if (bl_container_dynamic_append(&result, sizeof(struct RegexASTNode), &(struct RegexASTNode){.firstNode = NULL,.operation = RegexOpConcat}) != BL_ContainerOPSuccessful)
                goto Cleanup;
            current = bl_container_dynamic_back(&result);
            continue;
        }
        if (*character == U')') {
            if (bl_container_dynamic_is_empty(&stack)) {
                bl_log_debug_location("%s", "Unexpected ')' which closes nonexistant scope.");
				goto Cleanup;
            }
			struct RegexASTNode* currentNode = *(struct RegexASTNode**)bl_container_dynamic_back(&stack);
            bl_container_dynamic_pop(&stack);
            currentNode->secondOp = (struct RegexASTOp ) {.isAtom = false,.node = current};
            if (bl_container_dynamic_append(&result, sizeof *currentNode, currentNode) != BL_ContainerOPSuccessful)
                goto Cleanup;
            current = bl_container_dynamic_back(&result);
            continue;
        }
        if (*character == U'|') {
            if (current->operation != RegexOpUnion) {
                current->operation = RegexOpUnion;
                continue;
            }
        } else if (*character == U'[') {
            #error Do character range stuff;
        } else if (*character == U'*') {
            if (!current->firstNode) {
                current->secondOp = (struct RegexASTOp){.isAtom = true,.codepoint = *character};
            } else {
                current->operation = RegexOpRepeatGreedy;
            }
        } else if (*character == U'?') {
            if (current->firstNode && current->firstNode->operation == ) // Confusing system. fix
        }
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
