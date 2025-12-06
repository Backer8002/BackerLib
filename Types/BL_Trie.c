#include "BL_Trie.h"

#include "BL_UnorderedContainer.h"

#include <stdlib.h>
#include <string.h>
#include <wctype.h>

typedef struct TrieNode {
    uintptr_t data;
    size_t    childrenIndexes[];
} TrieNode;

static void internal_trie_init(BL_Trie* trie, size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) {
    trie->unorderedContainer = bl_unordered_container_create_stack(1, sizeof(TrieNode) + sizeof(size_t) * amountOfCharsToStore);
    if (!bl_unordered_container_is_valid(&trie->unorderedContainer))
        return;
    trie->amountOfChars      = amountOfCharsToStore;
    trie->composeCharToIndex = composeCharToIndex;
    trie->unorderedContainer.bitset[0] |= (uint64_t)INT64_MIN;
    bl_unordered_container_set(&trie->unorderedContainer,0,1,&(char){0});
    memset(bl_unordered_container_front(&trie->unorderedContainer),0,trie->unorderedContainer.byteSizeOfElement);
}

BL_Trie bl_trie_create_stack(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) {
    BL_Trie trie;
    internal_trie_init(&trie, amountOfCharsToStore, composeCharToIndex);
    return trie;
}

BL_Trie* bl_trie_create_heap(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) {
    BL_Trie* trie = malloc(sizeof *trie);
    if (!trie)
        return NULL;
    internal_trie_init(trie, amountOfCharsToStore, composeCharToIndex);
    if (!bl_trie_is_valid(trie)) {
        free(trie);
        return NULL;
    }
    trie->unorderedContainer.header |= ObjectFlagIsOnHeap;
    return trie;
}

BL_ContainerError bl_trie_insert(BL_Trie* trie, const wchar_t* sequence, uintptr_t data) {
    size_t currentNode = 0;
    size_t i           = 0;
    if (!data)
        return BL_ContainerInvalidIndex;
    while (1) {
        TrieNode* currentTrieNode = bl_unordered_container_get(&trie->unorderedContainer, currentNode);
        wchar_t   currentChar     = sequence[i];
        if (!currentChar) {
            currentTrieNode->data = data;
            break;
        }
        currentNode = currentTrieNode->childrenIndexes[trie->composeCharToIndex(currentChar)];
        if (!currentNode) {
            TrieNode                    nodeToInsert = {.data = 0};
            void* element       = bl_unordered_container_put(&trie->unorderedContainer, sizeof nodeToInsert, &nodeToInsert);
            if (!element)
                return BL_ContainerAllocFailure;
            memset(element, 0, trie->unorderedContainer.byteSizeOfElement);
            size_t index = bl_unordered_container_index_from_ref(&trie->unorderedContainer,element);
            currentTrieNode->childrenIndexes[trie->composeCharToIndex(currentChar)] = index;
            currentNode                                                             = index;
        }
        i++;
    }
    return BL_ContainerOPSuccessful;
}

bool bl_trie_get_prefix(const BL_Trie* trie, const wchar_t* sequence) {
    size_t currentNode = 0;
    while (*sequence) {
        TrieNode* currentTrieNode = bl_unordered_container_get(&trie->unorderedContainer, currentNode);
        currentNode               = currentTrieNode->childrenIndexes[trie->composeCharToIndex(*sequence)];
        if (!currentNode)
            return false;
        sequence++;
    }
    return true;
}

uintptr_t bl_trie_get(const BL_Trie* trie, const wchar_t* sequence) {
    size_t currentNode = 0;
    size_t i           = 0;
    while (sequence[i]) {
        TrieNode* currentTrieNode = bl_unordered_container_get(&trie->unorderedContainer, currentNode);
        currentNode               = currentTrieNode->childrenIndexes[trie->composeCharToIndex(sequence[i])];
        if (!currentNode)
            return 0;
        i++;
    }
    return ((TrieNode*) bl_unordered_container_get(&trie->unorderedContainer, currentNode))->data;
}

static BL_ContainerError trieRemoveHelper(BL_Trie* trie, const wchar_t* partOfSequence, TrieNode* currentNode) {
    if (*partOfSequence) {
        size_t nextNumberIndex = trie->composeCharToIndex(*partOfSequence);
        if (!nextNumberIndex || !currentNode->childrenIndexes[nextNumberIndex])
            return BL_ContainerInvalidIndex;

        TrieNode*      nextNode = bl_unordered_container_get(&trie->unorderedContainer, currentNode->childrenIndexes[nextNumberIndex]);
        BL_ContainerError status   = trieRemoveHelper(trie, partOfSequence + 1, nextNode);
        if (status == BL_ContainerInvalidIndex)
            return BL_ContainerInvalidIndex;
        if (status == BL_ContainerOPUnsuccessful) {
            bl_unordered_container_remove(&trie->unorderedContainer, currentNode->childrenIndexes[nextNumberIndex], NULL);
            currentNode->childrenIndexes[nextNumberIndex] = 0;

            size_t index                                  = 0;
            for (size_t i = 0; i < trie->amountOfChars; i++)
                index |= currentNode->childrenIndexes[i];
            if (!index)
                return BL_ContainerOPUnsuccessful;
        }

        return BL_ContainerOPSuccessful;
    }
    currentNode->data = 0;
    size_t index      = 0;
    for (size_t i = 0; i < trie->amountOfChars; i++)
        index |= currentNode->childrenIndexes[i];
    if (!index)
        return BL_ContainerOPUnsuccessful;
    return BL_ContainerOPSuccessful;
}

bool bl_trie_remove(BL_Trie* trie, const wchar_t* sequence) {
    if (trieRemoveHelper(trie, sequence, bl_unordered_container_front(&trie->unorderedContainer)) == BL_ContainerInvalidIndex)
        return false;
    return true;
}

bool bl_trie_is_valid(const BL_Trie* trie) {
    return bl_unordered_container_is_valid(&trie->unorderedContainer);
}

void bl_trie_destroy(void* obj) {
    bl_unordered_container_destroy(&((BL_Trie*)obj)->unorderedContainer);
}
