#include "roadrunner.h"
#include <stdio.h>

#define NUM 12

void log_road(road_id val, const char *text);
void *my_road(void *arg);

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

void log_road(road_id val, const char *text)
{
        if (val)
                printf("\x1b[34m > \x1b[0m[\x1b[33m%04ld\x1b[0m]: "
                       "\x1b[32m%s\x1b[0m\n",
                       val, text);
        else
                printf("\x1b[34m > \x1b[0m[\x1b[34m%04ld\x1b[0m]: "
                       "\x1b[32m%s\x1b[0m\n",
                       val, text);
}
