#include "Trie.h"

#include "UnorderedContainer.h"

#include <stdlib.h>
#include <string.h>
#include <wctype.h>

typedef struct TrieNode {
    uintptr_t data;
    size_t    childrenIndexes[];
} TrieNode;

static inline void internal_trieInit(Trie* trie, size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) {
    trie->unorderedContainer = unorderedContainerCreateStack(1, sizeof(TrieNode) + sizeof(size_t) * amountOfCharsToStore, false);
    if (!isValidObject((DataTypeFlags*) trie))
        return;
    trie->amountOfChars      = amountOfCharsToStore;
    trie->composeCharToIndex = composeCharToIndex;
    trie->unorderedContainer.bitset[0] |= 0x80;
    trie->container.amountOfIndexes++;
    memset(trie->container.array, 0, trie->container.byteSizeOfSingleElement);
}

Trie trieCreateStack(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) {
    Trie trie;
    internal_trieInit(&trie, amountOfCharsToStore, composeCharToIndex);
    return trie;
}

Trie* trieCreateHeap(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) {
    Trie* trie = malloc(sizeof *trie);
    if (!trie)
        return NULL;
    internal_trieInit(trie, amountOfCharsToStore, composeCharToIndex);
    if (!isValidObject((DataTypeFlags*) trie)) {
        free(trie);
        return NULL;
    }
    trie->header |= ObjectFlagIsOnHeap;
    return trie;
}

ContainerError trieInsert(Trie* trie, const wchar_t* sequence, uintptr_t data) {
    size_t currentNode = 0;
    size_t i           = 0;
    if (!data)
        return ContainerInvalidIndex;
    while (1) {
        TrieNode* currentTrieNode = unorderedContainerGet((UnorderedContainer*) trie, currentNode).element;
        wchar_t   currentChar     = sequence[i];
        if (!currentChar) {
            currentTrieNode->data = data;
            break;
        }
        currentNode = currentTrieNode->childrenIndexes[trie->composeCharToIndex(currentChar)];
        if (!currentNode) {
            TrieNode                    nodeToInsert = {.data = 0};
            UnorderedContainerPutResult result       = unorderedContainerPut((UnorderedContainer*) trie, sizeof nodeToInsert, &nodeToInsert);
            if (result.resultCode != ContainerOPSuccessful)
                return ContainerAllocFailure;
            memset(unorderedContainerGet((UnorderedContainer*) trie, result.locationOfElement).element, 0, trie->container.byteSizeOfSingleElement);
            currentTrieNode->childrenIndexes[trie->composeCharToIndex(currentChar)] = result.locationOfElement;
            currentNode                                                             = result.locationOfElement;
        }
        i++;
    }
    return ContainerOPSuccessful;
}

bool trieGetPrefix(const Trie* trie, const wchar_t* sequence) {
    size_t currentNode = 0;
    while (*sequence) {
        TrieNode* currentTrieNode = unorderedContainerGet((UnorderedContainer*) trie, currentNode).element;
        currentNode               = currentTrieNode->childrenIndexes[trie->composeCharToIndex(*sequence)];
        if (!currentNode)
            return false;
        sequence++;
    }
    return true;
}

uintptr_t trieGet(const Trie* trie, const wchar_t* sequence) {
    size_t currentNode = 0;
    size_t i           = 0;
    while (sequence[i]) {
        TrieNode* currentTrieNode = unorderedContainerGet((UnorderedContainer*) trie, currentNode).element;
        currentNode               = currentTrieNode->childrenIndexes[trie->composeCharToIndex(sequence[i])];
        if (!currentNode)
            return 0;
        i++;
    }
    return ((TrieNode*) unorderedContainerGet((UnorderedContainer*) trie, currentNode).element)->data;
}

static ContainerError trieRemoveHelper(const Trie* trie, const wchar_t* partOfSequence, TrieNode* currentNode) {
    if (*partOfSequence) {
        size_t nextNumberIndex = trie->composeCharToIndex(*partOfSequence);
        if (!nextNumberIndex || !currentNode->childrenIndexes[nextNumberIndex])
            return ContainerInvalidIndex;

        TrieNode*      nextNode = unorderedContainerGet((UnorderedContainer*) trie, currentNode->childrenIndexes[nextNumberIndex]).element;
        ContainerError status   = trieRemoveHelper(trie, partOfSequence + 1, nextNode);
        if (status == ContainerInvalidIndex)
            return ContainerInvalidIndex;
        if (status == ContainerOPUnsuccessful) {
            unorderedContainerRemove((UnorderedContainer*) trie, currentNode->childrenIndexes[nextNumberIndex], NULL);
            currentNode->childrenIndexes[nextNumberIndex] = 0;

            size_t index                                  = 0;
            for (size_t i = 0; i < trie->amountOfChars; i++)
                index |= currentNode->childrenIndexes[i];
            if (!index)
                return ContainerOPUnsuccessful;
        }

        return ContainerOPSuccessful;
    }
    currentNode->data = 0;
    size_t index      = 0;
    for (size_t i = 0; i < trie->amountOfChars; i++)
        index |= currentNode->childrenIndexes[i];
    if (!index)
        return ContainerOPUnsuccessful;
    return ContainerOPSuccessful;
}

bool trieRemove(const Trie* trie, const wchar_t* sequence) {
    if (trieRemoveHelper(trie, sequence, trie->container.array) == ContainerInvalidIndex)
        return false;
    return true;
}
