#include "qpid/dispatch/threading.h"

#include "qpid/dispatch/ctools.h"

#include <stdbool.h>
#include <windows.h>

// region sys_mutex
struct sys_mutex_t {
    CRITICAL_SECTION section;
};

sys_mutex_t *sys_mutex(void)
{
    sys_mutex_t *mutex = NEW(sys_mutex_t);
    InitializeCriticalSection(&mutex->section);
    return mutex;
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    DeleteCriticalSection(&mutex->section);
    free(mutex);
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    EnterCriticalSection(&mutex->section);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    LeaveCriticalSection(&mutex->section);
}
// endregion

// region sys_cond
struct sys_cond_t {
    CONDITION_VARIABLE cond;
};

sys_cond_t *sys_cond(void)
{
    sys_cond_t *cond = NEW(sys_cond_t);
    InitializeConditionVariable(&cond->cond);
    return cond;
}

void sys_cond_free(sys_cond_t *cond)
{
    free(cond);
}

void sys_cond_wait(sys_cond_t *cond, sys_mutex_t *held_mutex)
{
    SleepConditionVariableCS(&cond->cond, &held_mutex->section, INFINITE);
}

void sys_cond_signal(sys_cond_t *cond)
{
    WakeConditionVariable(&cond->cond);
}

void sys_cond_signal_all(sys_cond_t *cond)
{
    WakeAllConditionVariable(&cond->cond);
}
// endregion

// region sys_rwlock
/// Pthreads-like wrapper for windows SRWLock. Notice the symmetric unlock functions in the
/// Windows API, which require an extra bool flag to fit it into the pthreads API.
/// Cf. https://nachtimwald.com/2019/04/05/cross-platform-thread-wrapper/
struct sys_rwlock_t {
    SRWLOCK rwlock;
    bool    exclusive;
};

sys_rwlock_t *sys_rwlock(void)
{
    sys_rwlock_t *lock = NEW(sys_rwlock_t);
    InitializeSRWLock(&lock->rwlock);
    lock->exclusive = false;
    return lock;
}

void sys_rwlock_free(sys_rwlock_t *lock)
{
    free(lock);
}

void sys_rwlock_wrlock(sys_rwlock_t *lock)
{
    AcquireSRWLockExclusive(&lock->rwlock);
    lock->exclusive = true;
}

void sys_rwlock_rdlock(sys_rwlock_t *lock)
{
    AcquireSRWLockShared(&lock->rwlock);
}

void sys_rwlock_unlock(sys_rwlock_t *lock)
{
    if (lock->exclusive) {
        lock->exclusive = false;
        ReleaseSRWLockExclusive(&lock->rwlock);
    } else {
        ReleaseSRWLockShared(&lock->rwlock);
    }
}
// endregion

// region sys_thread
struct sys_thread_t {
    HANDLE thread;
    DWORD id;
    void *(*f)(void *);
    void *arg;
};

// thread function is forbidden to return void on 64bit Windows, have to wrap
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)#return-value
DWORD WINAPI sys_thread_function_wrapper(LPVOID lpParam)
{
    sys_thread_t *thread = (sys_thread_t *) lpParam;
    thread->f(thread->arg);
    return 0;
}

static sys_thread_t _main_thread_id;
static __thread sys_thread_t *_self = &_main_thread_id;

sys_thread_t *sys_thread(void *(*run_function)(void *), void *arg)
{
    sys_thread_t *thread = NEW(sys_thread_t);
    thread->f = run_function;
    thread->arg = arg;

    thread->thread = CreateThread(NULL,
                                  0,
                                  sys_thread_function_wrapper,
                                  thread,
                                  0,
                                  &thread->id);

    _self = thread;
    return thread;
}

void sys_thread_free(sys_thread_t *thread)
{
    CloseHandle(thread->thread);
}

void sys_thread_join(sys_thread_t *thread)
{
    WaitForSingleObject(thread->thread, INFINITE);
}

sys_thread_t *sys_thread_self(void)
{
    return _self;
}
// endregion
