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
    size_t keySize,elementOffset;
    void* root;
    bool (*compEqual)(const void*,const void*);
    bool (*compLess)(const void*,const void*);
} BL_Map;

extern BL_Map bl_map_create_stack(size_t keySize,size_t elementSize,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) noexcept;
extern BL_Map* bl_map_create_heap(size_t keySize,size_t elementSize,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) noexcept;
extern bool bl_map_is_valid(const BL_Map* map) noexcept;
extern bool bl_map_is_empty(const BL_Map* map) noexcept;
extern BL_ContainerError bl_map_insert(BL_Map* map, const void* key,size_t keySize,const void* element,size_t elementSize) noexcept;
extern void* bl_map_get(const BL_Map* map,const void* key, size_t keySize) noexcept;
extern BL_ContainerError bl_map_remove(BL_Map* map,const void* key,size_t keySize,void(*keyDestructor)(void*),void(*elementDestructor)(void*)) noexcept;
extern void bl_map_destroy(BL_Map* map,void(*keyDestructor)(void*),void(*elementDestructor)(void*)) noexcept;


#ifdef __cplusplus
}
#else
#undef noexcept
#endif //extern c end
#endif //Header Guard
