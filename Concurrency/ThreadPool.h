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
#define BL_Future(T) struct {volatile bool isValid;volatile T future;}

/**
 * @brief Assembles type compatible with threadpool.
 * @note Use in conjunction with typedef.
 */
#define BL_AsyncArgsType(FutureType,ArgsStructType) struct {struct{void(*operation)(void*); size_t futureOffset;}; FutureType future; ArgsStructType args;}
/**
 * @brief Gets offset of ArgsStructType in a asyncArgsType.
 */
#define bl_async_args_offset(asyncArgsPackType) offsetof(asyncArgsPackType,args)
/**
* @brief Gets offset of BL_Future type in a asyncArgsType.
*/
#define bl_async_args_future_offset(asyncArgsPackType) offsetof(asyncArgsPackType,future)

typedef struct BL_ThreadPool {
    BL_Heap               jobsQueue;
    BL_UnorderedContainer orders;
    BL_Mutex              mutexForQueue;
    BL_Thread*            threads;
    size_t             amountOfThreads;
    bool               mustExit;
} BL_ThreadPool;

/**
 * @brief Special type of BL_Future which does not contain a type.
 */
typedef bool BL_FutureVoid;

typedef BL_Future(char) BL_FutureChar;

typedef BL_Future(unsigned char) BL_FutureUChar;

typedef BL_Future(signed char) BL_FutureSChar;

typedef BL_Future(short) BL_FutureShort;

typedef BL_Future(unsigned short) BL_FutureUShort;

typedef BL_Future(int) BL_FutureInt;

typedef BL_Future(unsigned) BL_FutureUInt;

typedef BL_Future(long) BL_FutureLong;

typedef BL_Future(unsigned long) BL_FutureULong;

typedef BL_Future(long long) BL_FutureLongLong;

typedef BL_Future(unsigned long long) BL_FutureULongLong;

typedef BL_Future(BL_Container) BL_FutureContainer;

typedef BL_Future(BL_DynamicContainer) BL_FutureDynamicContainer;

typedef BL_Future(BL_String) BL_FutureString;

typedef BL_Future(BL_UnorderedContainer) BL_FutureUnorderedContainer;

typedef BL_Future(BL_Hashmap) BL_FutureHashMap;

typedef BL_AsyncArgsType(BL_FutureString, FILE*) BL_AsyncFileReadArg;

extern bool bl_threadpool_is_valid(const BL_ThreadPool* threadPool) noexcept;

/**
 * @brief Initlizes ThreadPool. Use isValidObject to check validity.
 * @param threadPool Pointer to non-valid ThreadPool
 * @param amountOfThreads Amount of threads to use in ThreadPool
 * @param sizeOfLargestAsyncArgsPack The size of the largest asyncArgsPack this threadpool can accept.
 */
extern BL_ConcurrencyError bl_threadpool_init(BL_ThreadPool* threadPool, size_t amountOfThreads, size_t sizeOfLargestAsyncArgsPack) noexcept;

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
extern void* bl_threadpool_job_assign(BL_ThreadPool* threadPool, size_t priority, void (*function)(void*), size_t futureOffset, const void* args, size_t argsSize, size_t argsOffset) noexcept;

#define bl_threadpool_job_assign_short(threadPool,priority,function,argsPtr,asyncArgsPackType) bl_threadpool_job_assign((threadPool),(priority),(function),bl_async_args_future_offset(asyncArgsPackType),(argsPtr),sizeof(*(argsPtr)),bl_async_args_offset(asyncArgsPackType))

/**
 * @brief Ammends exit requests for all threads with the lowest priority. Awaits threads to exit.
 * @param threadPool Pointer to valid ThreadPool
 * @return BL_ConcurrencyFailure if request could not be honored, due to issues with allocation or mutex locking.
 */
extern BL_ConcurrencyError bl_threadpool_join(BL_ThreadPool* threadPool) noexcept;

/**
 * @brief Threads quit after finishing what they currently work with. Awaits threads to exit.
 * @param threadPool Pointer to valid ThreadPool
 */
extern void bl_threadpool_exit(BL_ThreadPool* threadPool) noexcept;

/**
 * @brief Reads file asyncronusly to String.
 * @param threadPool Pointer to valid ThreadPool
 * @param priority Priority to read
 * @param file Open file with read permission
 * @return Future to file contents. Invalid String if initialization of string failed.
 */
extern BL_FutureString* bl_async_file_read(BL_ThreadPool* threadPool, size_t priority, FILE* file) noexcept;

/**
 * @brief Destroys future
 * @param threadPool Pointer to valid ThreadPool
 * @param future Pointer to BL_Future assigned from ThreadPool
 * @return false if future was not valid and thus could not be destroyed, else true.
 */
extern bool bl_future_destroy(BL_ThreadPool* threadPool, void* future) noexcept;

/**
 * @brief Awaits future to be valid.
 * @param future Pointer to future
 */
extern void bl_future_await(const void* future) noexcept;

/**
 * @brief Awaits future until future is valid or until timepoint.
 * @param future Pointer to future
 * @param timepoint Point in UTC time
 * @return true if function returned due to future is valid, else false
 */
extern bool bl_future_await_until(const void* future, const struct timespec* timepoint) noexcept;

/**
 * @brief Await future to be valid or for a maximum of duration.
 * @param future Pointer to future
 * @param duration Duration to wait
 * @return true if future became valid in less than duration, else false.
 */
extern bool bl_future_await_for(const void* future, const struct timespec* duration) noexcept;
#ifdef __cplusplus
}
};
#else
#undef noexcept
#endif
#endif // THREADPOOL_H
