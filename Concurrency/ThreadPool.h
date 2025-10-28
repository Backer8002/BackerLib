#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <stdio.h>
#include <BackerLibTypes.h>
#include "ConcurrencyDefines.h"

#ifdef __cplusplus
namespace BackerLib {
    extern "C" {
#else
#define noexcept
#endif

/**
 * @brief Assembles future type of T.
 * @param T Type to make future of
 * @note Use in conjunction with typedef.
 */
#define Future(T) struct {bool isValid;T future;}

/**
 * @brief Assembles type compatible with threadpool.
 * @note Use in conjunction with typedef.
 */
#define asyncArgsPackType(FutureType,ArgsStructType) struct {struct{size_t priority;void(*operation)(void*);}; FutureType future; ArgsStructType args;}
/**
 * @brief Gets offset of ArgsStructType in a asyncArgsType.
 */
#define asyncArgsOffset(asyncArgsPackType) offsetof(asyncArgsPackType,args)
/**
* @brief Gets offset of Future type in a asyncArgsType.
*/
#define asyncArgsFutureOffset(asyncArgsPackType) offsetof(asyncArgsPackType,future)

typedef struct ThreadPool {
    Heap               jobsQueue;
    UnorderedContainer orders;
    Mutex              mutexForQueue;
    Thread*            threads;
    size_t             amountOfThreads;
    bool               mustExit;
} ThreadPool;

/**
 * @brief Special type of Future which does not contain a type.
 */
typedef bool FutureVoid;

typedef Future(char) FutureChar;

typedef Future(unsigned char) FutureUChar;

typedef Future(signed char) FutureSChar;

typedef Future(short) FutureShort;

typedef Future(unsigned short) FutureUShort;

typedef Future(int) FutureInt;

typedef Future(unsigned int) FutureUInt;

typedef Future(long) FutureLong;

typedef Future(unsigned long) FutureULong;

typedef Future(long long) FutureLongLong;

typedef Future(unsigned long long) FutureULongLong;

typedef Future(Container) FutureContainer;

typedef Future(DynamicContainer) FutureDynamicContainer;

typedef Future(String) FutureString;

typedef Future(UnorderedContainer) FutureUnorderedContainer;

typedef Future(HashMap) FutureHashMap;

typedef asyncArgsPackType(FutureString, FILE*) AsyncFileReadArg;

/**
 * @brief Initlizes ThreadPool. Use isValidObject to check validity.
 * @param threadPool Pointer to non-valid ThreadPool
 * @param amountOfThreads Amount of threads to use in ThreadPool
 * @param sizeOfLargestAsyncArgsPack The size of the largest asyncArgsPack this threadpool can accept.
 */
extern ConcurrencyError threadPoolInit(ThreadPool* threadPool, size_t amountOfThreads, size_t sizeOfLargestAsyncArgsPack) noexcept;

/**
 * @brief Assigns a job to the threadpool.
 * @param threadPool Pointer to valid ThreadPool
 * @param priority Execution priority of job. Higher is more urgent
 * @param function Callback function. Defined asyncArgsPackType is the parameter
 * @param futureOffset Offset of future in asyncArgsPackType
 * @param args Pointer to args for function
 * @param argsSize Size of args
 * @param argsOffset args offset in asyncArgsPackType
 * @return NULL if job could not be submitted, else pointer to future as defined by asyncArgsPackType.
 */
extern void* threadPoolJobAssign(ThreadPool* threadPool, size_t priority, void (*function)(void*), size_t futureOffset, void* args, size_t argsSize, size_t argsOffset) noexcept;

#define threadPoolJobAssignShort(threadPool,priority,function,argsPtr,asyncArgsPackType) threadPoolJobAssign(()threadPool),(priority),(function),asyncArgsFutureOffset(asyncArgsPackType),(argsPtr),sizeof(*(argsPtr)),asyncArgsOffset(asyncArgsPackType))

/**
 * @brief Ammends exit requests for all threads with the lowest priority. Awaits threads to exit.
 * @param threadPool Pointer to valid ThreadPool
 * @return ConcurrencyFailure if request could not be honored, due to issues with allocation or mutex locking.
 */
extern ConcurrencyError threadPoolJoin(ThreadPool* threadPool) noexcept;

/**
 * @brief Threads quit after finishing what they currently work with. Awaits threads to exit.
 * @param threadPool Pointer to valid ThreadPool
 */
extern void threadPoolExit(ThreadPool* threadPool) noexcept;

/**
 * @brief Reads file asyncronusly to String.
 * @param threadPool Pointer to valid ThreadPool
 * @param priority Priority to read
 * @param file Open file with read permission
 * @return Future to file contents. Invalid String if initialization of string failed.
 */
extern FutureString* asyncFileRead(ThreadPool* threadPool, size_t priority, FILE* file) noexcept;

/**
 * @brief Destroys future
 * @param threadPool Pointer to valid ThreadPool
 * @param future Pointer to Future assigned from ThreadPool
 * @return false if future was not valid and thus could not be destroyed, else true.
 */
extern bool futureDestroy(ThreadPool* threadPool, void* future) noexcept;

/**
 * @brief Awaits future to be valid.
 * @param future Pointer to future
 */
extern void futureAwait(void* future) noexcept;

/**
 * @brief Awaits future until future is valid or until timepoint.
 * @param future Pointer to future
 * @param timepoint Point in UTC time
 * @return true if function returned due to future is valid, else false
 */
extern bool futureAwaitUntil(void* future, const struct timespec* timepoint) noexcept;

/**
 * @brief Await future to be valid or for a maximum of duration.
 * @param future Pointer to future
 * @param duration Duration to wait
 * @return true if future became valid in less than duration, else false.
 */
extern bool futureAwaitFor(void* future, const struct timespec* duration) noexcept;
#ifdef __cplusplus
}
};
#endif
#endif // THREADPOOL_H
