#include "Regex.h"
#include <BackerLibLogging.h>
#include <BackerLibTextprocessing.h>
#include <BackerLibTypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>

static void internal_regex_ast_destroy(void* regex);

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
    bool                   rangesIsInverse;
    struct RegexASTNode*   firstNode;
    union {
        struct RegexASTOp   secondOp;
        BL_DynamicContainer ranges;
    };
};

static BL_DynamicContainer internal_make_regex_ast(BL_UnicodeView expr) {

    BL_DynamicContainer result = bl_container_dynamic_create_stack(bl_unicodestr_length(expr), sizeof(struct RegexASTNode));
    if (!bl_container_dynamic_is_valid(&result))
        return result;
    BL_DynamicContainer stack   = bl_container_dynamic_create_stack(0, sizeof(struct RegexASTNode));
    struct RegexASTNode current = {.operation = RegexOpConcat, .firstNode = NULL};

    for (const BL_Unicodepoint* character = bl_unicodestr_front(expr); character; character = bl_unicodestr_next(expr, character)) {
        if (*character == U'(') {
            BL_ContainerError errorCode = bl_container_dynamic_append(&stack, sizeof current, &current);
            if (errorCode != BL_ContainerOPSuccessful) {
                bl_log_debug_location("%s", "Unable to push to stack");
                goto Cleanup;
            }
            current = (struct RegexASTNode) {.operation = RegexOpConcat, .firstNode = NULL};
            continue;
        }
        if (*character == U')') {
            if (bl_container_dynamic_is_empty(&stack)) {
                bl_log_debug_location("%s", "Unexpected ')' which closes nonexistant scope.");
                goto Cleanup;
            }
            current          = *(struct RegexASTNode*) bl_container_dynamic_back(&stack);
            current.secondOp = (struct RegexASTOp) {.isAtom = false, .node = bl_container_dynamic_back(&result)};
            bl_container_dynamic_pop(&stack);

        } else if (*character == U'|') {
            if (current.operation != RegexOpUnion && current.firstNode) {
                current.operation = RegexOpUnion;
                continue;
            }
            current.secondOp = (struct RegexASTOp) {.isAtom = true, .codepoint = *character};
        } else if (*character == U'[') {
            BL_DynamicContainer ranges    = bl_container_dynamic_create_stack(0, sizeof(struct CharacterRange));
            struct RegexASTNode rangeNode = {.operation = RegexOpUnionRange};

            character                     = bl_unicodestr_next(expr, character);
            struct CharacterRange currentRange;
            if (character && *character == '^') {
                rangeNode.rangesIsInverse = true;
                character                 = bl_unicodestr_next(expr, character);
            }
            if (!character) {
                bl_log_debug("%s", "Unclosed character range. Expected ']'");
                goto Cleanup;
            }
            currentRange.begin = *character;
            bool seenRangeMark = false;
            for (character = bl_unicodestr_next(expr, character); character && *character != ']'; character = bl_unicodestr_next(expr, character)) {
                if (seenRangeMark || *character != U'-') {
                    if (seenRangeMark)
                        currentRange.end = *character;
                    else
                        currentRange.end = currentRange.begin;
                    if (bl_container_dynamic_append(&ranges, sizeof currentRange, &currentRange) != BL_ContainerOPSuccessful)
                        goto Cleanup;
                    if (seenRangeMark)
                        currentRange.begin = BL_UNICODEPOINT_MAX;
                    else
                        currentRange.begin = *character;
                    seenRangeMark = false;
                    continue;
                }
                if (currentRange.begin == BL_UNICODEPOINT_MAX)
                    currentRange.begin = *character;
                else
                    seenRangeMark = true;
            }

            if (!character) {
                bl_log_debug("%s", "Unclosed character range. Expected ']'");
                goto Cleanup;
            }

            if (currentRange.begin != BL_UNICODEPOINT_MAX) {
                if (seenRangeMark) {
                    if (bl_container_dynamic_append(&ranges, sizeof(struct CharacterRange), &(struct CharacterRange) {.begin = currentRange.begin, .end = currentRange.end}) != BL_ContainerOPSuccessful)
                        goto Cleanup;
                    currentRange.begin = U'-';
                }
                currentRange.end = currentRange.begin;
                if (bl_container_dynamic_append(&ranges, sizeof currentRange, &currentRange) != BL_ContainerOPSuccessful)
                    goto Cleanup;
            }
            rangeNode.ranges = ranges;
            if (bl_container_dynamic_append(&result, sizeof rangeNode, &rangeNode) != BL_ContainerOPSuccessful)
                goto Cleanup;
            current.secondOp = (struct RegexASTOp) {.isAtom = false, .node = bl_container_dynamic_back(&result)};
        } else
            current.secondOp = (struct RegexASTOp) {.isAtom = true, .codepoint = *character};

        if (current.operation == RegexOpUnion) {
            struct RegexASTNode newCurrent = {.operation = RegexOpConcat, .firstNode = current.firstNode->firstNode, .secondOp = {.isAtom = false, .node = current.firstNode}};
            if (current.secondOp.isAtom) {
                if (bl_container_dynamic_append(&result, sizeof(struct RegexASTNode), &(struct RegexASTNode) {.operation = RegexOpConcat, .secondOp = current.secondOp}) != BL_ContainerOPSuccessful)
                    goto Cleanup;
                current.secondOp = (struct RegexASTOp) {.isAtom = false, .node = bl_container_dynamic_back(&result)};
            }
            current.firstNode->operation = RegexOpUnion;
            current.firstNode->firstNode = current.secondOp.node;
            current                      = newCurrent;
        }

        BL_Unicodepoint* optionalModifier1 = bl_unicodestr_next(expr, character);
        if (optionalModifier1 && (*optionalModifier1 == U'?' || *optionalModifier1 == U'+' || *optionalModifier1 == U'*')) {
            character = optionalModifier1;

            if (*optionalModifier1 == U'?') {
                struct RegexASTNode optional = {.operation = RegexOpOptional, .secondOp = current.secondOp};
                if (bl_container_dynamic_append(&result, sizeof optional, &optional) != BL_ContainerOPSuccessful)
                    goto Cleanup;

                current.secondOp = (struct RegexASTOp) {.isAtom = false, .node = bl_container_dynamic_back(&result)};
            } else {
                if (*optionalModifier1 == U'+') {
                    if (bl_container_dynamic_append(&result, sizeof current, &current) != BL_ContainerOPSuccessful)
                        goto Cleanup;
                    current.firstNode = bl_container_dynamic_back(&result);
                }

                BL_Unicodepoint* optionalModifier2 = bl_unicodestr_next(expr, character);
                if (optionalModifier2 && *optionalModifier2 == U'?')
                    character = optionalModifier2;

                struct RegexASTNode repeatingNode = {.operation = *character == U'?' ? RegexOpRepeat : RegexOpRepeatGreedy, .secondOp = current.secondOp};
                if (bl_container_dynamic_append(&result, sizeof repeatingNode, &repeatingNode) != BL_ContainerOPSuccessful)
                    goto Cleanup;
                current.secondOp = (struct RegexASTOp) {.isAtom = false, .node = bl_container_dynamic_back(&result)};
            }
        }
        if (bl_container_dynamic_append(&result, sizeof current, &current) != BL_ContainerOPSuccessful)
            goto Cleanup;
        current = (struct RegexASTNode) {.operation = RegexOpConcat, .firstNode = bl_container_dynamic_back(&result)};
    }
    bl_container_dynamic_destroy(&stack);
    return result;

Cleanup:
    bl_container_dynamic_destroy(&stack);
    bl_container_dynamic_destroy(&result);
    return result;
}

static void internal_regex_ast_print(const struct RegexASTNode* astNode, FILE* file) {
    if (astNode->firstNode)
        internal_regex_ast_print(astNode->firstNode, file);

    if (astNode->operation == RegexOpUnionRange) {
        fputc('[', file);
        if (astNode->rangesIsInverse)
            fputc('^', file);

        for (struct CharacterRange* range = bl_container_dynamic_front(&astNode->ranges); range; range = bl_container_dynamic_next(&astNode->ranges, range)) {
            BL_Textprocessing_UTFCodepoint utfCodepoint = bl_textprocessing_from_unicodepoint(range->begin, BL_Textprocessing_Encoding_UTF8);
            for (size_t i = 0; i < utfCodepoint.bytesUsed; i++)
                fputc(utfCodepoint.bytes[i], file);
            if (range->begin != range->end) {
                fputc('-', file);
                BL_Textprocessing_UTFCodepoint endRange = bl_textprocessing_from_unicodepoint(range->end, BL_Textprocessing_Encoding_UTF8);
                for (size_t i = 0; i < endRange.bytesUsed; i++)
                    fputc(endRange.bytes[i], file);
            }
        }

        fputc(']', file);
        fputc(' ', file);
    } else if (astNode->secondOp.isAtom) {
        BL_Textprocessing_UTFCodepoint utfCodepoint = bl_textprocessing_from_unicodepoint(astNode->secondOp.codepoint, BL_Textprocessing_Encoding_UTF8);
        for (size_t i = 0; i < utfCodepoint.bytesUsed; i++)
            fputc(utfCodepoint.bytes[i], file);
        fputc(' ', file);
    } else
        internal_regex_ast_print(astNode->secondOp.node, file);

    const char* nameOfOp;
    switch (astNode->operation) {
    case RegexOpConcat:
        nameOfOp = "Concat";
        break;
    case RegexOpUnion:
        nameOfOp = "Union";
        break;
    case RegexOpRepeat:
        nameOfOp = "Repeat(non greedy)";
        break;
    case RegexOpRepeatGreedy:
        nameOfOp = "Repeat";
        break;
    case RegexOpOptional:
        nameOfOp = "Optional";
        break;
    case RegexOpUnionRange:
        nameOfOp = "Range of characters";
        break;
    }
    fprintf(file, "%s ", nameOfOp);
}

BL_Regex bl_regex_create(BL_UnicodeView expr) {
    BL_DynamicContainer ast = internal_make_regex_ast(expr);
    if (!bl_container_dynamic_is_valid(&ast))
        return (BL_Regex){0};


    BL_Regex regex = {
        .ast       = ast,
        .bitset    = calloc((bl_container_dynamic_size(&ast) + 63) / 64, sizeof(uint64_t)),
        .positions = calloc(bl_container_dynamic_size(&ast), sizeof(size_t))};

    if (!regex.bitset || !regex.positions)
        goto ErrorCase;
    
    typedef BL_MAKE_PAIR_TYPE(bool,struct RegexASTNode*) StackVar;

    BL_DynamicContainer stack = bl_container_dynamic_create_stack(1,sizeof(StackVar)),stack2 = bl_container_dynamic_create_stack(0,sizeof(void*)), newAst = bl_container_dynamic_create_stack(bl_container_dynamic_size(&ast),sizeof(struct RegexASTNode));
    if (!bl_container_dynamic_is_valid(&stack) || !bl_container_dynamic_is_valid(&stack2) || !bl_container_dynamic_is_valid(&newAst))
        goto ErrorCase2;

    bl_container_dynamic_append(&stack,sizeof(StackVar),&BL_MAKE_PAIR(StackVar,false,(struct RegexASTNode*)bl_container_dynamic_back(&ast)));

    while(!bl_container_dynamic_is_empty(&stack)) {
        StackVar currentNode = *(StackVar*)bl_container_dynamic_back(&stack);
        bl_container_dynamic_pop(&stack);

        if(!currentNode.second)
            continue;

        if (currentNode.first) {
            if (currentNode.second->firstNode) {
                if (currentNode.second->operation == RegexOpUnionRange || currentNode.second->secondOp.isAtom) {
                    struct RegexASTNode* node = *(struct RegexASTNode**)bl_container_dynamic_back(&stack2);
                    bl_container_dynamic_pop(&stack2);
                    currentNode.second->firstNode = node;
                } else {
                    struct RegexASTNode* secondNode = *(struct RegexASTNode**)bl_container_dynamic_back(&stack2);
                    bl_container_dynamic_pop(&stack2);
                    struct RegexASTNode* firstNode = *(struct RegexASTNode**)bl_container_dynamic_back(&stack2);
                    bl_container_dynamic_pop(&stack2);
                    currentNode.second->firstNode = firstNode;
                    currentNode.second->secondOp.node = secondNode;
                }
            }
            bl_container_dynamic_append(&newAst,sizeof *currentNode.second,currentNode.second);
            struct RegexASTNode* newNodePlace = bl_container_dynamic_back(&newAst);
            if (bl_container_dynamic_append(&stack2,sizeof newNodePlace,&newNodePlace) != BL_ContainerOPSuccessful)
                goto ErrorCase2;
            continue;
        }
        currentNode.first = true;
        if (bl_container_dynamic_append(&stack,sizeof currentNode,&currentNode) != BL_ContainerOPSuccessful)
            goto ErrorCase2;
        if (currentNode.second->operation != RegexOpUnionRange && !currentNode.second->secondOp.isAtom) {
            if (bl_container_dynamic_append(&stack, sizeof(StackVar),&BL_MAKE_PAIR(StackVar,false,currentNode.second->secondOp.node)) != BL_ContainerOPSuccessful)
                goto ErrorCase2;        
        }
        if (bl_container_dynamic_append(&stack,sizeof(StackVar),&BL_MAKE_PAIR(StackVar,false,currentNode.second->firstNode)) != BL_ContainerOPSuccessful)
            goto ErrorCase2;
    }

    bl_container_dynamic_destroy(&stack);
    bl_container_dynamic_destroy(&stack2);
    bl_container_dynamic_destroy(&ast);

    regex.ast = newAst;

    internal_regex_ast_print(bl_container_dynamic_back(&newAst),stdout);
    
    return regex;

ErrorCase2:
    bl_container_dynamic_destroy(&stack);
    bl_container_dynamic_destroy(&stack2);
    bl_container_dynamic_destroy(&newAst);
ErrorCase:
    bl_regex_destroy(&regex);
    return regex;
}

static void internal_regex_ast_destroy(void* regex) {
    struct RegexASTNode* node = regex;
    if (node->operation == RegexOpUnionRange)
        bl_container_dynamic_destroy(&node->ranges);
}

BL_UnicodeView bl_regex_match(const BL_Regex* regex, BL_UnicodeView str) {
    size_t position = 0;
    size_t currentTextPosition = 0;

    // Walk ast in post order. Save currentPosition. If failed go back. If at first and it is still invalid then exit.
    // If found, walk back and try to expand. If having problems to match, try to expand previous multicharacter ones.
}

void bl_regex_destroy(void* regex) {
    bl_container_dynamic_destroy_with_elements(&((BL_Regex*) regex)->ast, internal_regex_ast_destroy);
    free(((BL_Regex*)regex)->bitset);
    free(((BL_Regex*)regex)->positions);
}
