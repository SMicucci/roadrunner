#ifndef ROADRUNNER_H
#define ROADRUNNER_H

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

/* @brief create and join road */
void *road_await(road_fn fn, void *arg);

/* @brief leave execution to new road */
void road_yield(void);

#endif /* end of include guard: ROADRUNNER_H */
