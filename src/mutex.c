#define MUTEX_IMPLEMENTATION
#include "mutex.h"
#include "runner.h"
#include <assert.h>

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
