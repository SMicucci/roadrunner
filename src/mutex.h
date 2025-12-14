#ifndef MUTEX_H
#define MUTEX_H

#if defined(ROAD_SDL3)
#include <SDL3/SDL.h>
typedef SDL_Mutex mutex_t;
#elif defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
typedef pthread_mutex_t mutex_t;
#elif defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION mutex_t;
#else
/* make your own definition */
#endif

/* @brief create a mutex */
mutex_t *mutex_init(void);
/* @brief free a mutex */
void mutex_destroy(mutex_t *mux);
/* @brief lock mutex, return non-zero on fail */
int mutex_lock(mutex_t *mux);
/* @brief unlock a mutex */
void mutex_unlock(mutex_t *mux);

#if defined(MUTEX_IMPLEMENTATION)

#if defined(ROAD_SDL3)

mutex_t *mutex_init(void) { return SDL_CreateMutex(); }
void mutex_destroy(mutex_t *mux) { SDL_DestroyMutex(mux); }
int mutex_lock(mutex_t *mux) { return SDL_TryLockMutex(mux) == 0 ? 1 : 0; }
void mutex_unlock(mutex_t *mux) { SDL_UnlockMutex(mux); }

#elif defined(__unix__) || defined(__APPLE__)

#include <stdlib.h>
mutex_t *mutex_init(void)
{
        mutex_t *mux = malloc(sizeof(*mux));
        if (!mux)
                return NULL;
        if (pthread_mutex_init(mux, NULL)) {
                free(mux);
                return NULL;
        }
        return mux;
}
void mutex_destroy(mutex_t *mux)
{
        pthread_mutex_destroy(mux);
        free(mux);
}
int mutex_lock(mutex_t *mux) { return pthread_mutex_trylock(mux); }
void mutex_unlock(mutex_t *mux) { pthread_mutex_unlock(mux); }

#elif defined(_WIN32)

#include <stdlib.h>
mutex_t *mutex_init(void)
{
        mutex_t *mux = malloc(sizeof(*mux));
        if (!mux)
                return NULL;
        InitializeCriticalSection(mux);
        return mux;
}
void mutex_destroy(mutex_t *mux)
{
        DeleteCriticalSection(mux);
        free(mux);
}
int mutex_lock(mutex_t *mux) { return TryEnterCriticalSection(mux) == 0; }
void mutex_unlock(mutex_t *mux) { LeaveCriticalSection(mux); }

#endif

#endif

#endif /* end of include guard: MUTEX_H */
