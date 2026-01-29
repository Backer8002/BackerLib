#include "BL_Map.h"
#include "BL_UnorderedContainer.h"
#include "TypesMain.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

struct Node {
    struct Node  *left,*right,*prev;
    signed char weight;
    alignas(max_align_t) unsigned char data[]; 
};

static void internal_map_init(BL_Map* map,size_t keySize,size_t elementSize,bool(*compEqual)(const void*,const void*),bool(*compLess)(const void*,const void*)) {
    map->compEqual = compEqual;
    map->compLess = compLess;
    size_t elementOffset = sizeof(struct Node) + keySize + (alignof(max_align_t) - keySize % alignof(max_align_t));
    size_t nodeSize = elementOffset + elementSize + (alignof(max_align_t) - elementSize % alignof(max_align_t));
    map->container = bl_unordered_container_create_stack(0,nodeSize);
    map->elementOffset = elementOffset;
    map->keySize = keySize;
    map->root = NULL;
}

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

static struct Node* internal_find_best_place(const BL_Map* map,const void* key) {
    struct Node* nextNode = map->root;
    while (true) {
        struct Node* currentNode = nextNode;
        if (map->compEqual(currentNode->data,key))
            return NULL;
        if (map->compLess(key,currentNode->data))
            nextNode = currentNode->left;
        else
            nextNode = currentNode->right;

        if (!nextNode)
            return currentNode;
    }
}

struct Node* internal_rotate_left(struct Node* node) {
    struct Node* rightNode = node->right;
    struct Node* rightLeftNode = rightNode->left;
    rightNode->prev = node->prev;
    node->right = rightLeftNode;
    rightNode->left = node;
    node->prev = rightNode;
    if (rightLeftNode)
        rightLeftNode->prev = rightNode;

    if (rightNode->weight == 0) {
        node->weight = 1;
        rightNode->weight = -1;
    } else {
        node->weight = 0;
        rightNode->weight = 0;
    }
    return rightNode;
}

struct Node* internal_rotate_right(struct Node* node) {
    struct Node* leftNode = node->left;
    struct Node* leftRightNode = leftNode->right;
    leftNode->prev = node->prev;
    node->left = leftRightNode;
    leftNode->right = node;
    node->prev = leftNode;
    if (leftRightNode)
        leftRightNode->prev = leftNode;

    if (leftNode->weight == 0) {
        node->weight = -1;
        leftNode->weight = 1;
    } else {
        node->weight = 0;
        leftNode->weight = 0;
    }
    return leftNode;
}

struct Node* internal_rotate_right_left(struct Node* node) {
    struct Node* rightNode = node->right;
    struct Node* rigthLeftNode = rightNode->left;

    if (rigthLeftNode->right)
        rigthLeftNode->right->prev = rightNode;
    rightNode->left = rigthLeftNode->right;
    rightNode->prev = rigthLeftNode;
    rigthLeftNode->prev = node;
    node->right = rigthLeftNode;
    rigthLeftNode->right = rightNode;


    rigthLeftNode = rigthLeftNode->left;
    rightNode = node->right;

    node->right = rigthLeftNode;
    if (rigthLeftNode)
        rigthLeftNode->prev = node;
    rightNode->prev = node->prev;
    node->prev = rightNode;
    rightNode->left = node;

    if (rightNode->weight == 0) {
        rightNode->left->weight = 0;
        rightNode->right->weight = 0;
    } else if (rightNode->weight > 0) {
        rightNode->left->weight = -1;
        rightNode->right->weight = 0;
    } else {
        rightNode->left->weight = 0;
        rightNode->right->weight = 1;
    }
    rightNode->weight = 0;
    return rightNode;
}

struct Node* internal_rotate_left_right(struct Node* node) {
    struct Node* leftNode = node->left;
    struct Node* leftRightNode = leftNode->right;

    leftNode->right = leftRightNode->left;
    if (leftRightNode->left)
        leftRightNode->left->prev = leftNode;
    leftNode->prev = leftRightNode;
    leftRightNode->left = leftNode;
    node->left = leftRightNode;
    leftRightNode->prev = node;

    leftRightNode = leftRightNode->right;
    leftNode = node->left;

    if(leftRightNode)
        leftRightNode->prev = node;
    node->left = leftRightNode;
    leftNode->prev = node->prev;
    node->prev = leftNode;
    leftNode->right = node;
    
    if (leftNode->weight == 0) {
        leftNode->left->weight = 0;
        leftNode->right->weight = 0;
    } else if (leftNode->weight > 0) {
        leftNode->left->weight = 0;
        leftNode->right->weight = 1;
    } else {
        leftNode->left->weight = -1;
        leftNode->right->weight = 0;
    }
    leftNode->weight = 0;
    return leftNode;
}

BL_ContainerError bl_map_insert(BL_Map* map,const void* key,size_t keySize,const void* element,size_t elementSize) {
    if (keySize != map->keySize || elementSize > map->container.byteSizeOfElement - map->elementOffset)
        return BL_ContainerInvalidSize;

    if (bl_map_is_empty(map)) {
        BL_ContainerError errorCode = bl_unordered_container_set(&map->container,0,sizeof(struct Node),&(struct Node){.left = NULL,.right = NULL,.prev = NULL});
        if(errorCode != BL_ContainerOPSuccessful)
            return errorCode;
        struct Node* elementInContainer = bl_unordered_container_get(&map->container,0);
        memcpy(elementInContainer->data,key,keySize);
        memcpy((BL_Byte*)elementInContainer + map->elementOffset,element,elementSize);
        map->root = elementInContainer;
        return BL_ContainerOPSuccessful;
    }

    struct Node* bestNode = internal_find_best_place(map,key);
    if (!bestNode)
        return BL_ContainerOPUnsuccessful;
    
    bool shouldBeOnLeft = false;

    if (map->compLess(key,bestNode->data))
        shouldBeOnLeft = true;
    struct Node* newNode = bl_unordered_container_put(&map->container,sizeof(struct Node),&(struct Node) {.left = NULL,.right = NULL,.prev = bestNode});
    if (!newNode)
        return BL_ContainerAllocFailure;
    memcpy(newNode->data,key,keySize);
    memcpy((BL_Byte*)newNode+map->elementOffset,element,elementSize);

    if (shouldBeOnLeft)
        bestNode->left = newNode;
    else
        bestNode->right = newNode;

    struct Node* prevNode = newNode;
    for (struct Node* node = bestNode; node; prevNode = node, node = node->prev) {
        struct Node* newNode = NULL;
        if (node->right == prevNode) {
            if (node->weight > 0) {
                if(prevNode->weight >= 0)
                    newNode = internal_rotate_left(node);
                else
                    newNode = internal_rotate_right_left(node);
            } else {
                if (node->weight < 0) {
                    node->weight = 0;
                    break;
                }
                node->weight = 1;
                continue;
            }
        } else {
            if (node->weight < 0) {
                if(prevNode->weight <= 0)
                    newNode = internal_rotate_right(node);
                else
                    newNode = internal_rotate_left_right(node);
            } else {
                if(node->weight > 0) {
                    node->weight = 0;
                    break;
                }
                node->weight = -1;
                continue;
            }
        }

        if (newNode->prev) {
            if (newNode->prev->left == node)
                newNode->prev->left = newNode;
            else
                newNode->prev->right = newNode;
        } else
            map->root = newNode;
        node = newNode;
    }
    return BL_ContainerOPSuccessful;
}

static struct Node* internal_get_node(const BL_Map* map,const void* key) {
    struct Node* current = map->root;
    while(current) {
        if (map->compEqual(current->data,key))
            return current;
        if (map->compLess(key,current->data))
            current = current->left;
        else
            current = current->right;
    }
    return NULL;
}

void* bl_map_get(const BL_Map* map,const void* key,size_t keySize) {
    if (keySize != map->keySize)
        return NULL;

    struct Node* node = internal_get_node(map, key);
    if (node)
        return (BL_Byte*)node + map->elementOffset;
    return NULL;
}

static struct Node* internal_get_inorder_successor(struct Node* nodeToStartAt) {
    struct Node *currentNode = nodeToStartAt;
    while (true) {
        if (!(currentNode->left && currentNode->right))
            return currentNode;
        currentNode = currentNode->left;
    }
}

BL_ContainerError bl_map_remove(BL_Map* map,const void* key,size_t keySize,void(*keyDestructor)(void*),void(*elementDestructor)(void*)) {
    if (keySize != map->keySize)
        return BL_ContainerInvalidSize;

    struct Node* nodeToRemove = internal_get_node(map, key);

    if (!nodeToRemove)
        return BL_ContainerOPUnsuccessful;

    if (keyDestructor)
        keyDestructor(nodeToRemove->data);
    if (elementDestructor)
        elementDestructor((BL_Byte*) nodeToRemove + map->elementOffset);

    struct Node* currentNode = nodeToRemove;

    if (nodeToRemove->left) {
        if (nodeToRemove->right) {
            struct Node* inorderSuccessor = internal_get_inorder_successor(nodeToRemove->right);
            if (!(inorderSuccessor->left || inorderSuccessor->right)) {
                
            }
        }
    } else if (nodeToRemove->right) {

    }

    for (struct Node* nextNode = currentNode->prev; nextNode; currentNode = nextNode,nextNode = nextNode->prev) {

    }    
    return BL_ContainerOPSuccessful;
}

void bl_map_destroy(BL_Map* map,void(*keyDestructor)(void*),void(*elementDestructor)(void*)) {
    if (!bl_map_is_valid(map))
        return;

    for (struct Node* node = bl_unordered_container_front(&map->container); node; node = bl_unordered_container_next(&map->container, node)) {
        if (keyDestructor)
            keyDestructor(node->data);
        if (elementDestructor)
            elementDestructor((BL_Byte*)node + map->elementOffset);
    }

    bl_unordered_container_destroy(&map->container);
}
