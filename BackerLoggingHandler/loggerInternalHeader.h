#pragma once
#pragma once
#include<queue.h>
#include<arrayList.h>
#include <hashMap.h>

struct loggingHandle {
    Queue     EventQueue;
    BitSet       EventLevels;
    ArrayList EventLevelNames;
    ArrayList EventLevelSubscribers;
    uint64_t  flags;
};