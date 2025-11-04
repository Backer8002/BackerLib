#ifndef TRIE_H
#define TRIE_H

#include "TypesMain.h"
#include "BL_UnorderedContainer.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif
    typedef struct BL_Trie {
        BL_UnorderedContainer unorderedContainer;
        size_t                amountOfChars;
        size_t (*composeCharToIndex)(wchar_t chr);
    } BL_Trie;


    /**
     * @brief Creates a Trie on the stack. Use isValidObject to check validity
     * @param amountOfCharsToStore Amount of diffrent chars that are going to be handled
     * @param composeCharToIndex Callback to convert char to index in array. 0 shall be returned if char is out of range
     */
    extern BL_Trie           bl_trie_create_stack(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) noexcept;
    /**
     * @brief Creates a Trie on the heap.
     * @param amountOfCharsToStore Amount of diffrent chars that are going to be handled
     * @param composeCharToIndex Callback to convert char to index in array. 0 shall be returned if char is out of range
     * @return NULL if allocation failed.
     */
    extern BL_Trie*          bl_trie_create_heap(size_t amountOfCharsToStore, size_t (*composeCharToIndex)(wchar_t chr)) noexcept;
    /**
     * @brief Will insert sequence into Trie.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to insert
     * @param data Data to store at node
     * @return ContainerInvalidIndex if data is 0.
     * @return ContainerAllocFailure if allocation could not happen.
     */
    extern BL_ContainerError bl_trie_insert(BL_Trie* trie, const wchar_t* sequence, uintptr_t data) noexcept;
    /**
     * @brief Looks if there is any sequence that begins in CString.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to look up
     * @return true if prefix exist.
     */
    extern bool           bl_trie_get_prefix(const BL_Trie* trie, const wchar_t* sequence) noexcept;
    /**
     * @brief Looks for sequence in Trie.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to look up
     * @return Data stored at node. 0 if sequence did not exist.
     */
    extern uintptr_t      bl_trie_get(const BL_Trie* trie, const wchar_t* sequence) noexcept;
    /**
     * @brief Removes a sequence from the Trie.
     * @param trie Pointer to valid Trie
     * @param sequence Sequence to remove
     * @return false if CString did not exist
     */
    extern bool           bl_trie_remove(BL_Trie* trie, const wchar_t* sequence) noexcept;
    /**
     * @brief Destroys Trie if applicable.
     * @param trie Pointer to Trie
     */
    extern void           bl_trie_destroy(void* trie) noexcept;
    /**
     *
     * @param trie Pointer to Trie or NULL
     * @return true if trie is valid, else false
     */
    extern bool bl_trie_is_valid(const BL_Trie* trie) noexcept;

#ifdef __cplusplus
    }
};
#else
#undef noexcept
#endif
#endif // TRIE_H
