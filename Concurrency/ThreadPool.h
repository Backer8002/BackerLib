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
    BL_Heap               jobsQueue;
    BL_UnorderedContainer orders;
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

typedef Future(BL_Container) FutureContainer;

typedef Future(BL_DynamicContainer) FutureDynamicContainer;

typedef Future(BL_String) FutureString;

typedef Future(BL_UnorderedContainer) FutureUnorderedContainer;

typedef Future(BL_Hashmap) FutureHashMap;

typedef asyncArgsPackType(FutureString, FILE*) AsyncFileReadArg;

extern bool bl_threadpool_is_valid(const ThreadPool* threadPool) noexcept;

/**
 * @brief Initlizes ThreadPool. Use isValidObject to check validity.
 * @param threadPool Pointer to non-valid ThreadPool
 * @param amountOfThreads Amount of threads to use in ThreadPool
 * @param sizeOfLargestAsyncArgsPack The size of the largest asyncArgsPack this threadpool can accept.
 */
extern ConcurrencyError bl_threadpool_init(ThreadPool* threadPool, size_t amountOfThreads, size_t sizeOfLargestAsyncArgsPack) noexcept;

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
extern void* bl_threadpool_job_assign(ThreadPool* threadPool, size_t priority, void (*function)(void*), size_t futureOffset, void* args, size_t argsSize, size_t argsOffset) noexcept;

#define threadPoolJobAssignShort(threadPool,priority,function,argsPtr,asyncArgsPackType) bl_threadpool_job_assign(()threadPool),(priority),(function),asyncArgsFutureOffset(asyncArgsPackType),(argsPtr),sizeof(*(argsPtr)),asyncArgsOffset(asyncArgsPackType))

/**
 * @brief Ammends exit requests for all threads with the lowest priority. Awaits threads to exit.
 * @param threadPool Pointer to valid ThreadPool
 * @return ConcurrencyFailure if request could not be honored, due to issues with allocation or mutex locking.
 */
extern ConcurrencyError bl_threadpool_join(ThreadPool* threadPool) noexcept;

/**
 * @brief Threads quit after finishing what they currently work with. Awaits threads to exit.
 * @param threadPool Pointer to valid ThreadPool
 */
extern void bl_threadpool_exit(ThreadPool* threadPool) noexcept;

/**
 * @brief Reads file asyncronusly to String.
 * @param threadPool Pointer to valid ThreadPool
 * @param priority Priority to read
 * @param file Open file with read permission
 * @return Future to file contents. Invalid String if initialization of string failed.
 */
extern FutureString* bl_async_file_read(ThreadPool* threadPool, size_t priority, FILE* file) noexcept;

/**
 * @brief Destroys future
 * @param threadPool Pointer to valid ThreadPool
 * @param future Pointer to Future assigned from ThreadPool
 * @return false if future was not valid and thus could not be destroyed, else true.
 */
extern bool bl_future_destroy(ThreadPool* threadPool, void* future) noexcept;

/**
 * @brief Awaits future to be valid.
 * @param future Pointer to future
 */
extern void bl_future_await(void* future) noexcept;

/**
 * @brief Awaits future until future is valid or until timepoint.
 * @param future Pointer to future
 * @param timepoint Point in UTC time
 * @return true if function returned due to future is valid, else false
 */
extern bool bl_future_await_until(void* future, const struct timespec* timepoint) noexcept;

/**
 * @brief Await future to be valid or for a maximum of duration.
 * @param future Pointer to future
 * @param duration Duration to wait
 * @return true if future became valid in less than duration, else false.
 */
extern bool bl_future_await_for(void* future, const struct timespec* duration) noexcept;
#ifdef __cplusplus
}
};
#else
#undef noexcept
#endif
#endif // THREADPOOL_H
