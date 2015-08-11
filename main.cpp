/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#include "task_scheduler.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

void task1(void* ptr)
{
    printf("Task 1\n");
    uTsk::TaskDelayMicro(100000);
}

void task2(void* ptr)
{
    printf("Task 2\n");
    uTsk::TaskDelayMicro(100000);
}

void task3(void* ptr)
{
    printf("Task 3\n");
    uTsk::TaskDelayMicro(100000);
}

int main(int argc, char* argv[])
{

    assert(uTsk::TaskInitialize() == 0);
    uTsk::TaskHandle_t handle[2];

    handle[0] = uTsk::TaskCreate(&task1, 500, 0, NULL, "");
    handle[1] = uTsk::TaskCreate(&task2, 250, 0, NULL, "");

    assert(handle[0]);
    assert(handle[1]);

    assert(uTsk::TaskInit(handle[0]) == 0);
    assert(uTsk::TaskInit(handle[1]) == 0);

    int i = 0;
    while(++i < 20)
    {
        uTsk::utime_t delay = uTsk::TaskDispatch();
        if (delay)
        {
            printf("idle time %lu\n", delay);
            uTsk::TaskDelayMicro(delay * 1000);
        }
    }

    assert(uTsk::TaskDelete(handle[0]) == 0);
    assert(uTsk::TaskDelete(handle[1]) == 0);
    assert(uTsk::TaskUninitialize() == 0);

    return (0);
}

