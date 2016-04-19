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
    TaskDelayMicro(100000);
}

void task2(void* ptr)
{
    printf("Task 2\n");
    TaskDelayMicro(100000);
}

void task3(void* ptr)
{
    printf("Task 3\n");
    TaskDelayMicro(100000);
}

int main(int argc, char* argv[])
{

    assert(TaskInitialize() == 0);
    TaskHandle_t handle[2];

    handle[0] = TaskCreate(&task1, 500, 0, NULL, "");
    handle[1] = TaskCreate(&task2, 250, 0, NULL, "");

    assert(handle[0]);
    assert(handle[1]);

    assert(TaskInit(handle[0]) == 0);
    assert(TaskInit(handle[1]) == 0);

    int i = 0;
    while(++i < 20)
    {
        utime_t delay = TaskDispatch();
        if (delay)
        {
            printf("idle time %lu\n", delay);
            TaskDelayMicro(delay * 1000);
        }
    }

    assert(TaskDelete(handle[0]) == 0);
    assert(TaskDelete(handle[1]) == 0);
    assert(TaskUninitialize() == 0);

    return (0);
}

