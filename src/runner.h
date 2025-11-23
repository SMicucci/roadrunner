#ifndef RUNNER_H
#define RUNNER_H

#include "cops.h"
#include "libco.h"
#include "roadrunner.h"
#include <stdint.h>

typedef enum road_state_t {
        ROAD_READY,
        ROAD_WAIT,
        ROAD_SUCCESS,
        ROAD_FAIL,
} road_state_t;

typedef enum road_wait_tag {
        WAIT_NONE,
        WAIT_CO,
} road_wait_tag;

typedef union road_wait_kind {
        void *res;
        road_id id;
} road_wait_kind;

typedef struct road_wait_t {
        road_wait_tag tag;
        road_wait_kind val;
} road_wait_t;

typedef struct road_t {
        road_id id;
        cothread_t co;
        road_state_t state;
        road_wait_t wait;
        road_fn fn;
        void *arg;
        void *result;
        uint64_t waitcount;
} road_t;

#undef X
#define X(name, T) __init_cops_vec(name, T)
X(road_vec, road_t *)
#undef X
#define X(name, T) __init_cops_list(name, T)
X(road_list, road_t *)
#undef X

typedef struct runner_t {
        road_list *ready;
        road_vec *wait;
        road_vec *end;
} runner_t;

/* @brief internal road new */
road_t *road_request();

/* @brief internal road delete */
void road_release(road_t *road);

#endif /* end of include guard: RUNNER_H */
