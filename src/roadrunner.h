#ifndef ROADRUNNER_H
#define ROADRUNNER_H

#include "mutex.h"
#include <stdint.h>

#define ROAD_ERROR ((void *)-1)

/* @brief _ */
typedef uint64_t road_id;

/* @brief road function */
typedef void *(*road_fn)(void *);

/* @brief create new road to run */
road_id road_create(road_fn fn, void *arg);

/* @brief join to a given road */
void *road_join(road_id id);

/* @brief non-blocking join, 0 on success */
int road_tryjoin(road_id id, void **retval);

/* @brief create and join road */
void *road_await(road_fn fn, void *arg);

/* @brief leave execution to new road */
void road_yield(void);

/* @brief lock mutex or leave execution until ready to lock */
void road_lock(mutex_t *mux);

/* @brief unlock the given mutex */
void road_unlock(mutex_t *mux);

#endif /* end of include guard: ROADRUNNER_H */
