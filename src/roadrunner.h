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

/**
 *
 * MUTEX SUPPORT
 *
 * */
#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
typedef pthread_mutex_t mutex_t;
#elif defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION mutex_t;
#else
/* make your own definition */
#endif

/* @brief lock mutex or leave execution until ready to lock */
void road_lock(mutex_t *mux);

/* @brief unlock the given mutex */
void road_unlock(mutex_t *mux);

#endif /* end of include guard: ROADRUNNER_H */
