
# How to use the event subscriber system

## Function prootypes

If no additional args are supplied: 
```C 
void (*function)(EventCall) 
```

If additonal args are supplied:

```C
void (*function)(EventCall, size_t amountOfAdditionalArgs, ...)
```

## Flags

0x1 Function will accept additional args provided at registration.  
0x2 Function will be called in a new thread.

## Init functions

- eventInit(void)  
	>It will start the worker thread and create the event queue.  
	>Save the pointer returnd from it since it is the event handle.

- eventRegSubToId(EventHandle\*, const char\* ID, void\(*function)(EventCall))
	>Registers function to an ID that only takes the call as a paramiter.  
	>Returns EventOperationSuccsess if succsessful

- eventRegSubToIdWithAdditionalParams(EventHandle*, const char\* ID, void(*function)(EventCall,size\_t, va\_list additionalArgs), bool functionShouldRunOnSeperateThread, size\_t amountOfAdditionalParamiters, ...) 
	>Registers function to an ID that takes the call as a parameter and additionally args. Has option for the worker thread to create a thread for execution.  
	>Returns EventOperationSuccsess if succsessful

## Event Registers

- eventCall(const event\* const, thrd\_t currentThread)
- logCall(const event\* const, thrd\_t currentThread, uint32\_t line, const char\* file)
- free
- malloc
- realloc
- calloc