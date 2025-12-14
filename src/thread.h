#ifndef THREAD_H
#define THREAD_H

#if defined(ROAD_SDL3)
#include <SDL3/SDL.h>
typedef SDL_Thread thread_t;
#elif defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
typedef pthread_t thread_t;
#elif defined(_WIN32)
#include <windows.h>
typedef HANDLE thread_t;
#endif

thread_t *thread_create(void *fn(void *), void *arg);
void *thread_join(thread_t *thread);
int thread_try_join(thread_t *thread, void **retval);

#endif /* end of include guard: THREAD_H */
