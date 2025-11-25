#include "runner.h"
#include "libco.h"
#include "roadrunner.h"
#include <assert.h>
#include <stdint.h>

void runner_debug(void);

#define ARENA_IMPLEMENTATION
#define ARENA_REGION_COUNT (128)
#define ARENA_REGION_DEFAULT_CAPACITY (sizeof(road_t) * ARENA_REGION_COUNT)
#include "arena.h"

static Arena road_arena = {0};
static uint64_t arena_size = 0;
static road_vec *free_road;
static road_id next_id = 0;
static inline road_t *road_from_id(road_id id);

#define STACK_SIZE (1 << 16)
static __thread runner_t runner;
static __thread cothread_t runner_co;
static __thread cothread_t main_co;
static void schedule(void);

__attribute__((constructor)) static void init_runner()
{
        // prepare runner
        runner.ready = road_list_new();
        runner.wait = road_vec_new();
        runner.end = road_vec_new();
        // prepare arena
        arena_reset(&road_arena);
        free_road = road_vec_new();
        // create road main
        main_co = co_active();
        road_t *main = road_request();
        main->co = main_co;
        main->id = next_id++;
        road_list_push_back(runner.ready, main);
        // init scheduler
        runner_co = co_create(STACK_SIZE, schedule);
}

__attribute__((destructor)) static void destroy_runner()
{
        // remove main from road
        road_list_pop_front(runner.ready, NULL);
        // end all road first
        co_switch(runner_co);
        // release runner
        road_list_free(runner.ready);
        road_vec_free(runner.wait);
        road_vec_free(runner.end);
        // release arena
        arena_free(&road_arena);
        road_vec_free(free_road);
}

static void runner_handle_ready(void);
static void runner_handle_wait(void);
static void runner_handle_end(void);

static void schedule(void)
{
        while (1) {
                runner_handle_end();
                runner_handle_wait();
                runner_handle_ready();
                if (!runner.ready->nelem && !runner.wait->nelem)
                        co_switch(main_co);
        }
}

static void runner_handle_ready(void)
{
        /* loop untill something ready */
        if (runner.ready->nelem == 0)
                return;
        /* run first in queue */
        road_t *curr = runner.ready->head->val;
        co_switch(curr->co);
        /* here restart scheduler */
}

static void runner_handle_end(void)
{
        uint64_t i = 0;
        while (i < runner.end->nelem) {
                road_t *trg = runner.end->data[i];
                /* release resource */
                if (!trg->waitcount) {
                        /* free target */
                        road_vec_del(runner.end, i, NULL);
                        road_release(trg);
                } else {
                        /* check for next */
                        i++;
                }
        }
}

static void runner_waiting_co(road_t *waiting, uint64_t wait_pos);

static void runner_handle_wait(void)
{
        uint64_t i = 0;
        while (i < runner.wait->nelem) {
                road_t *trg = runner.wait->data[i];
                assert(trg->state == ROAD_WAIT && "wtf?");
                switch (trg->wait.tag) {
                        case WAIT_NONE:
                                continue;
                        case WAIT_CO:
                                runner_waiting_co(trg, i);
                }
                i++;
        }
}

static void runner_waiting_co(road_t *waiting, uint64_t wait_pos)
{
        if (!waiting || waiting->state != ROAD_WAIT ||
            waiting->wait.tag != WAIT_CO)
                return;
        road_t *trg = road_from_id(waiting->wait.val.id);
        if (!trg) {
                waiting->wait.val.res = ROAD_ERROR;
                waiting->state = ROAD_READY;
                road_vec_del(runner.wait, wait_pos, NULL);
                road_list_push_back(runner.ready, waiting);
        } else if (trg->state == ROAD_SUCCESS || trg->state == ROAD_FAIL) {
                trg->waitcount--;
                waiting->wait.val.res = trg->result;
                waiting->state = ROAD_READY;
                road_vec_del(runner.wait, wait_pos, NULL);
                road_list_push_back(runner.ready, waiting);
        }
}

static inline road_t *road_from_id(road_id id)
{
        Region *r = road_arena.begin;
        uint64_t i = 0;
        while (i < arena_size && r) {
                /* calculate entry and increment i */
                int entry = i++ % ARENA_REGION_COUNT;
                /* cast magic */
                road_t *trg = &((road_t *)r->data)[entry];
                /* match id */
                if (trg->id == id) {
                        return trg;
                }
                /* if end of arena go to next */
                if (!(i % ARENA_REGION_COUNT))
                        r = r->next;
        }
        return NULL;
}

road_t *road_request()
{
        road_t *new;
        if (free_road->nelem) {
                road_vec_pop(free_road, &new);
        } else {
                new = (road_t *)arena_alloc(&road_arena, sizeof(*new));
                arena_size++;
        }
        new->id = next_id++;
        return new;
}

void road_release(road_t *road)
{
        if (!road)
                return;
        co_delete(road->co);
        road->id = 0;
        road->result = road->arg = NULL;
        road_vec_push(free_road, road);
}

/**
 * ROAD IMPLEMENTATION
 * */

static __thread road_t *volatile trampoline_road;
static __thread cothread_t volatile trampoline_co;
static void trampoline(void)
{
        road_t *road = trampoline_road;
        co_switch(trampoline_co);
        road->result = road->fn(road->arg);
        road->state = road->result == ROAD_ERROR ? ROAD_FAIL : ROAD_SUCCESS;
        road_list_pop_front(runner.ready, NULL);
        road_vec_push(runner.end, road);
        co_switch(runner_co);
}

road_id road_create(road_fn fn, void *arg)
{
        road_t *new = road_request();
        new->state = ROAD_READY;
        new->fn = fn;
        new->arg = arg;
        trampoline_road = new;
        trampoline_co = co_active();
        new->co = co_create(STACK_SIZE, trampoline);
        co_switch(new->co);
        road_list_push_back(runner.ready, new);
        return new->id;
}

void *road_join(road_id id)
{
        road_t *curr; /* running road */
        road_list_pop_front(runner.ready, &curr);
        road_t *trg = road_from_id(id); /* waited road */
        if (!trg) {
                road_list_push_front(runner.ready, curr);
                return ROAD_ERROR;
        }
        trg->waitcount++; /* incremente waitcount */
        curr->state = ROAD_WAIT;
        curr->wait.tag = WAIT_CO;
        curr->wait.val.id = id;
        road_vec_push(runner.wait, curr);
        co_switch(runner_co);
        return curr->wait.val.res;
}

void *road_await(road_fn fn, void *arg)
{
        return road_join(road_create(fn, arg));
}

void road_yield(void)
{
        road_t *curr;
        road_list_pop_front(runner.ready, &curr);
        road_list_push_back(runner.ready, curr);
        printf("> yield\n");
        co_switch(runner_co);
}

#include <stdio.h>
#define D "\x1b[0m"
#define Y "\x1b[33m"
#define G "\x1b[32m"
#define B "\x1b[34m"
#define P "\x1b[35m"
#define LF "\n"
static inline const char *runner_debug_state(road_state_t a);
void runner_debug(void)
{
        printf("~~~" G "DEBUG" D "~~~" LF);
        printf(B "ready" D "(" P "%d" D "):" LF, runner.ready->nelem);
        road_list_node *iter = runner.ready->head;
        while (iter) {
                printf("  > [" Y "%ld" D "]" G " %s" D LF, iter->val->id,
                       runner_debug_state(iter->val->state));
                iter = iter->next;
        }
        printf(B "wait" D "(" P "%d" D "):" LF, runner.wait->nelem);
        for (uint64_t i = 0; i < runner.wait->nelem; i++) {
                printf("  > [" Y "%ld" D "]" G " %s" D LF,
                       runner.wait->data[i]->id,
                       runner_debug_state(runner.wait->data[i]->state));
        }
        printf(B "end" D "(" P "%d" D "):" LF, runner.end->nelem);
        for (uint64_t i = 0; i < runner.end->nelem; i++) {
                printf("  > [" G "%ld" D "]" G " %s" D LF,
                       runner.end->data[i]->id,
                       runner_debug_state(runner.end->data[i]->state));
        }
}

static char *state_tag[4] = {"READY", "WAIT", "SUCCESS", "FAIL"};
static inline const char *runner_debug_state(road_state_t a)
{
        return state_tag[(int)a];
}
