#include "BL_Map.h"
#include "BL_UnorderedContainer.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

struct Node {
    
}

static void internal_map_init(BL_Map* map,size_t keySize,size_t elementSize,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) {
    map->compEqual = compEqual;
    map->compLess = compLess;
    size_t elementOffset = keySize + (alignof(max_align_t) - keySize % alignof(max_align_t));
    size_t nodeSize = elementOffset + elementSize + (alignof(max_align_t) - elementSize % alignof(max_align_t));
    map->container = bl_unordered_container_create_stack(0,nodeSize);
    map->elementOffset = elementOffset;
    map->keySize = keySize;
};

BL_Map bl_map_create_stack(size_t keySize,size_t elementSize,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) {
    BL_Map map;
    internal_map_init(&map,keySize,elementSize,compEqual,compLess);
    return map;
}

BL_Map* bl_map_create_heap(size_t keySize,size_t elementSize,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) {
    BL_Map* map = malloc(sizeof *map);
    if (!map)
        return NULL;
    
    internal_map_init(map,keySize,elementSize,compEqual,compLess);

    if (!bl_map_is_valid(map)) {
        free(map);
        return NULL;
    }

    map->container.header |= ObjectFlagIsOnHeap;
    return map; 
}

bool bl_map_is_valid(const BL_Map* map) {
    return bl_unordered_container_is_valid(&map->container);
}

bool bl_map_is_empty(const BL_Map* map) {
    return bl_unordered_container_is_empty(&map->container);
}
//Returns 0 if key already exists
static size_t internal_find_best_place(const BL_Map* map,const void* key) {
    size_t nextIndex = 1; //one indexed
    while (true) {
        void* currentKey = bl_unordered_container_get(&map->container,nextIndex-1);
        if(!currentKey)
            return nextIndex - 1;
        if (map->compEqual(currentKey,key))
            return 0;
        nextIndex*=2;
        if (!map->compLess(key,currentKey))
            nextIndex++;
    }
}

BL_ContainerError bl_map_insert(BL_Map* map,const void* key,size_t keySize,const void* element,size_t elementSize) {
    if (keySize != map->keySize || elementSize > map->container.byteSizeOfElement - map->elementOffset)
        return BL_ContainerInvalidSize;

    if (bl_map_is_empty(map)) {
        BL_ContainerError errorCode = bl_unordered_container_set(&map->container,0,keySize,key);
        if(errorCode != BL_ContainerOPSuccessful)
            return errorCode;
        void* elementInContainer = bl_unordered_container_get(&map->container,0);
        memcpy((BL_Byte*)elementInContainer + map->elementOffset,element,elementSize);
        return BL_ContainerOPSuccessful;
    }

    size_t bestIndex = internal_find_best_place(map,key);
    if (!bestIndex)
        return BL_ContainerOPUnsuccessful;

    if (bl_unordered_container_set(&map->container,bestIndex,keySize,key) != BL_ContainerOPSuccessful)
        return BL_ContainerAllocFailure;
    
    void* currentElement = bl_unordered_container_get(&map->container,bestIndex);
    memcpy((BL_Byte*)currentElement+map->elementOffset,element,elementSize);

    
}


void* bl_map_get(const BL_Map* map,const void* key,size_t keySize) {
    size_t currentIndex = 1; //one indexed

    while(true) {
        void* currentNode = bl_unordered_container_get(&map->container,currentIndex-1);
        if(!currentNode)
            return NULL;

        if(map->compEqual(currentNode,key))
            return (BL_Byte*)currentNode + map->elementOffset;
        
        currentIndex *=2;
        if (!map->compLess(key,currentNode))
            currentIndex++;
    }
}
