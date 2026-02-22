#ifndef BL_TYPES_MAP
#define BL_TYPES_MAP

#include<stdbool.h>
#include<stddef.h>
#include"BL_UnorderedContainer.h"

#ifdef __cplusplus
extern "C" {
#else
#define noexcept
#endif //extern c

typedef struct BL_Map {
    BL_UnorderedContainer container;
    size_t elementOffset,keyOffset;
    void* root;
    bool (*compEqual)(const void*,const void*);
    bool (*compLess)(const void*,const void*);
} BL_Map;

extern BL_Map bl_map_create_stack(size_t keyValueSize,size_t alignment,size_t elementOffset,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) noexcept;
extern BL_Map* bl_map_create_heap(size_t keyValueSize,size_t alignment,size_t elementOffset,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) noexcept;
/**
 * @brief Checks if map is valid
 * @param map Pointer to Map
 * @returns true if map is valid.
 */
extern bool bl_map_is_valid(const BL_Map* map) noexcept;
/**
 * @brief Is map empty?
 * @param map Pointer to valid Map
 * @returns true if Map contains no elements.
 */
extern bool bl_map_is_empty(const BL_Map* map) noexcept;
/**
 * @brief Size of map.
 * @param map Pointer to valid Map
 * @return Size of map.
 */
extern size_t bl_map_size(const BL_Map* map) noexcept;
/**
 * @brief Inserts an element into map.
 * @param map Pointer to valid Map
 * @param key Pointer to valid key for element
 * @param keySize Size of key to insert
 * @param element Pointer to valid element to insert
 * @param elementSize Size of element to insert
 * @return BL_ContainerOPUnsuccessful if key already exists in map.
 * @return BL_ContainerInvalidSize if size of key is not the key size of map
 */
extern BL_ContainerError bl_map_insert(BL_Map* map, const void* key,size_t keySize,const void* element,size_t elementSize) noexcept;
/**
 * @brief Gets element from map
 * @param map Pointer to valid Map
 * @param key Pointer to valid key
 * @param keySize Size of key
 * @returns NULL if key is not in map or if keySize is not that of map.
 */
extern void* bl_map_get(const BL_Map* map,const void* key, size_t keySize) noexcept;
/**
 * @brief Removes an element from the map.
 * @param map Pointer to valid Map
 * @param key Pointer to valid key
 * @param keySize Size of key
 * @param keyDestructor Optional callback to destroy key
 * @param elementDestructor Optional callback to destroy element
 * @return BL_ContainerOPUnsuccessful if key does not exist in map.
 * @return BL_ContainerInvaildSize if keySize is not the same as sizes of key in map.
 */
extern BL_ContainerError bl_map_remove(BL_Map* map,const void* key,size_t keySize,void(*keyDestructor)(void*),void(*elementDestructor)(void*)) noexcept;
/**
 * @brief Destroys a map. Provides optional destructors
 * @param map Pointer to Map
 * @param keyDestructor Optional callback to destroy key
 * @param elementDestructor Optional callback to destroy element
 */
extern void bl_map_destroy(BL_Map* map,void(*keyDestructor)(void*),void(*elementDestructor)(void*)) noexcept;
extern void* bl_map_front(const BL_Map* map) noexcept;
extern void* bl_map_back(const BL_Map* map) noexcept;
extern void* bl_map_next(const BL_Map* map,const void* element) noexcept;
extern void* bl_map_prev(const BL_Map* map, const void* element) noexcept;

#ifdef __cplusplus
}
#else
#undef noexcept
#endif //extern c end
#endif //Header Guard
