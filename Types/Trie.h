#ifndef TRIE_H
#define TRIE_H

#include "TypesMain.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#endif

    typedef union Trie {
        DataTypeFlags header;
        Container     container;
        struct {
            UnorderedContainer unorderedContainer;
            size_t             amountOfChars;
            size_t (*composeCharToIndex)(wchar_t chr);
        };
    } Trie;

    /**
     * @brief Creates a Trie on the stack. Use isValidObject to check validity
     * @param amountOfCharsToStore Amount of diffrent chars that are going to be handled
     * @param composeCharToIndex Callback to convert char to index in array. 0 shall be returned if char is out of range
     */
    extern Trie           trieCreateStack(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr));
    /**
     * @brief Creates a Trie on the heap.
     * @param amountOfCharsToStore Amount of diffrent chars that are going to be handled
     * @param composeCharToIndex Callback to convert char to index in array. 0 shall be returned if char is out of range
     * @return NULL if allocation failed.
     */
    extern Trie*          trieCreateHeap(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr));
    /**
     * @brief Will insert sequence into Trie.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to insert
     * @param data Data to store at node
     * @return ContainerInvalidIndex if data is 0.
     * @return ContainerAllocFailure if allocation could not happen.
     */
    extern ContainerError trieInsert(Trie* trie, const wchar_t* sequence, uintptr_t data);
    /**
     * @brief Looks if there is any sequence that begins in CString.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to look up
     * @return true if prefix exist.
     */
    extern bool           trieGetPrefix(const Trie* trie, const wchar_t* sequence);
    /**
     * @brief Looks for sequence in Trie.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to look up
     * @return Data stored at node. 0 if sequence did not exist.
     */
    extern uintptr_t      trieGet(const Trie* trie, const wchar_t* sequence);
    /**
     * @brief Removes a sequence from the Trie.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to remove
     * @return false if CString did not exist
     */
    extern bool           trieRemove(const Trie* trie, const wchar_t* sequence);
    /**
     * @brief Destroys Trie if applicable.
     * @param trie Pointer to Trie
     */
    extern void           trieDestroy(void* trie);

#ifdef __cplusplus
    }
};
#endif
#endif // TRIE_H
