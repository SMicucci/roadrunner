#include "roadrunner.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM 6

// extern void runner_debug(void);

void log_road(road_id val, const char *text);

void *my_road(void *arg);

void *await_road(void *arg);

mutex_t mux = PTHREAD_MUTEX_INITIALIZER;
struct road_mux_arg {
        road_id id;
        mutex_t *mux;
};
void *mux_road(void *arg);

int main(void)
{
        /* test create */
        log_road(0, "main road start");
        road_id roads[NUM];
        for (int i = 0; i < NUM; i++) {
                roads[i] = road_create(my_road, &(roads[i]));
                if (!(i % 5)) {
                        log_road(0, "main road yield");
                        road_yield();
                }
        }
        log_road(0, "main road wait recursive road");
        /* test join co */
        int *rec = malloc(sizeof(int));
        *rec = 5;
        road_id wait = road_create(await_road, rec);
        road_join(wait);
        free(rec);
        log_road(0, "main road end");
        road_yield();
        /* test mutex await */
        road_id mux_roads[NUM];
        for (int i = 0; i < NUM; i++) {
                struct road_mux_arg *arg = malloc(sizeof(*arg));
                mux_roads[i] = road_create(mux_road, arg);
                arg->id = mux_roads[i];
                arg->mux = &mux;
        }
        for (int i = 0; i < NUM; i++) {
                road_join(mux_roads[i]);
        }
        return 0;
}

void *my_road(void *arg)
{
        road_id val = *(road_id *)arg;
        log_road(val, "road start");
        log_road(val, "road yield");
        road_yield();
        log_road(val, "road end");
        return NULL;
}

void *await_road(void *arg)
{
        int i = *(int *)arg;
        printf(" > recursive => [%d]\n", i);
        if (i-- == 0) {
                log_road((road_id)-1, "recursive road base case");
                return NULL;
        }
        int *new_arg = malloc(sizeof(int));
        *new_arg = i;
        road_id id = road_create(await_road, new_arg);
        log_road(id, "recursive road recurse");
        road_join(id);
        log_road(id, "recursive road return");
        return NULL;
}

void *mux_road(void *a)
{
        struct road_mux_arg *arg = a;
        log_road(arg->id, "enter mux logic");
        road_lock(arg->mux);
        log_road(arg->id, "enter mux critical section and yield");
        road_yield();
        road_unlock(arg->mux);
        log_road(arg->id, "exit mux critical section");
        free(arg);
        return NULL;
}

void log_road(road_id val, const char *text)
{
        // (void)val;
        // (void)text;
        if (val)
                printf("\x1b[34m > \x1b[0m[\x1b[33m%04ld\x1b[0m]: "
                       "\x1b[32m%s\x1b[0m\n",
                       val, text);
        else
                printf("\x1b[34m > \x1b[0m[\x1b[34m%04ld\x1b[0m]: "
                       "\x1b[32m%s\x1b[0m\n",
                       val, text);
}
