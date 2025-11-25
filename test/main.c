#include "roadrunner.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM 6

// extern void runner_debug(void);

void log_road(road_id val, const char *text);
void *my_road(void *arg);
void *await_road(void *arg);

int main(void)
{
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
        int *rec = malloc(sizeof(int));
        *rec = 2;
        road_id wait = road_create(await_road, rec);
        road_join(wait);
        free(rec);
        log_road(0, "main road end");
        road_yield();
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
