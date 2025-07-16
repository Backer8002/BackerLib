#pragma once

#include "../Types/arrayList.h"
#include "../Types/backerLibListTypes.h"
#include "../Types/backerStrings.h"
#include "../Types/hashMap.h"
#include "../Types/queue.h"


#define HashArrayNode void
#define HashArrayElement void

// #define arrayListElementInsert(arrayList, index, elements, amountOfElements)
// \
//  arrayListElementInsert(arrayList, index, element, amountOfElements, \
//                         valueTypeOf(*element))
// #define arrayListElementSet(arrayList, index, element) \
//  arrayListElementSet(arrayList, index, element, valueTypeOf(*element))

#define queueEnqueue(queue, element)                                           \
  queueEnqueue(queue, element, sizeof(*element))
#define queueDequeue(queue, elementBuff)                                       \
  queueDequeue(queue, elementBuff, sizeof(*elementBuff)
#define queueClearOut(queue,operation,destructor) queueClearOut(queue,operation,destructor,__LINE__,__FILE__)
#define queueDestroy(queue,destructor) queueDestroy(queue,destructor,__LINE__,__FILE__)