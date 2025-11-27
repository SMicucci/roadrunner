#ifndef MUTEX_H
#define MUTEX_H

#include "roadrunner.h"

/* @brief create a mutex */
mutex_t *mutex_init(void);
/* @brief free a mutex */
void mutex_destroy(mutex_t *mux);
/* @brief lock mutex, return non-zero on fail */
int mutex_lock(mutex_t *mux);
/* @brief unlock a mutex */
void mutex_unlock(mutex_t *mux);

#endif /* end of include guard: MUTEX_H */
