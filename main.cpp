/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#include "utts.h"
#include <stdio.h>
#include <unistd.h>

void task1(void* ptr)
{
    printf("Task 1\n");
    usleep(100000);
}

void task2(void* ptr)
{
    printf("Task 2\n");
    usleep(100000);
}

void task3(void* ptr)
{
    printf("Task 3\n");
    usleep(100000);
}

int main(int argc, char* argv[])
{
    uTTS tts;
    int i = 0;
    TaskHandle_t handle[3];

    handle[0] = tts.TaskCreate(&task1, 500, 0, NULL);
    handle[1] = tts.TaskCreate(&task2, 250, 0, NULL);
    //tts.CreateTask(&task3, 200, 0, NULL);

    while(1)
    {
        utime_t delay = tts.TaskDispatch();
        if (delay)
        {
            printf("idle time %ld\n", delay);
            usleep(delay * 1000);
        }

        i++;

        if (i % 8 == 0)
        {
            tts.TaskSuspend(handle[0], 1000);
        }

        if (i > 100)
        {
            i = 0;
        }

    }

    return 0;
}

