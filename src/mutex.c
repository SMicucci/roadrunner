#include "mutex.h"
#include "runner.h"
#include <assert.h>
#include <stdlib.h>

void road_lock(mutex_t *mux)
{
        /* if avaible just continue */
        if (!mutex_lock(mux))
                return;
        road_t *trg;
        /* from ready to wait */
        road_list_pop_front(runner.ready, &trg);
        road_vec_push(runner.wait, trg);
        /* update road state */
        trg->state = ROAD_WAIT;
        trg->wait.tag = WAIT_MUX;
        trg->wait.val.mux = mux;
        /* return to scheduler */
        co_switch(runner_co);
}

void road_unlock(mutex_t *mux) { mutex_unlock(mux); }

#if defined(__linux__) || defined(__APPLE__)

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

#else
/* make your own definition */
#endif
