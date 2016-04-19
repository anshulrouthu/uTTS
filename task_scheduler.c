/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#include "task_scheduler.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_TASKS 20
#define MIN(a,b) a > b ? b : a
static int MaxTaskID = 0;
static TaskHandle_t currTask = NULL;

typedef struct TaskData
{
    utime_t t_period;
    utime_t t_remaining;
    void* appdata;
    taskfunction* task;
    utime_t t_completion;
    utime_t suspend;
    const char* name;
    uint8_t id;
}TaskData;

/* TODO: The scope of the tasklist variable should be valid only within
 * TaskInitialize() and TaskUninitializ() methods
 * Implement the creation and deletion of tasklist in a better way
 */
typedef struct SchedulerData
{
    TaskData* tasklist[MAX_TASKS];
    utime_t lastrun;
} SchedulerData;

SchedulerData* helper = NULL;

utime_t uGetTimeMilli()
{
    long mtime;
    struct timeval time;

    gettimeofday(&time, NULL);

    mtime = ((time.tv_sec) * 1000 + time.tv_usec / 1000);
    return (mtime);
}

int TaskInitialize()
{
    helper = (SchedulerData*)malloc(sizeof(SchedulerData));
    helper->lastrun = uGetTimeMilli();
    return (0);
}

int TaskUninitialize()
{
    uint8_t i;

    for (i = 0; i < MAX_TASKS; ++i)
    {
        if (helper->tasklist[i])
        {
            TaskDeInit(helper->tasklist[i]);
        }
    }

    free(helper);

    return (0);
}

TaskHandle_t TaskCreate(taskfunction* ptr, utime_t period, utime_t remaining_time, void* appdata, const char* name)
{
    TaskData* task = (TaskData*)malloc(sizeof(TaskData));
    task->task = ptr;
    task->t_period = period;
    task->t_remaining = remaining_time;
    task->appdata = appdata;
    task->t_completion = remaining_time;
    task->suspend = MAX_TIME_VALUE;
    task->name = name;
    uint8_t i;

    /* TODO: Not sure if task id is really required
     * implement decrement of MaxTaskID value on task deletion
     */
    if (MaxTaskID == MAX_TASKS)
    {
        for (i = 0; i < MAX_TASKS; ++i)
        {
            if (!helper->tasklist[i])
            {
                task->id = i;
                helper->tasklist[task->id] = task;
                return (task);
            }
        }

        free(task);
        return (NULL);
    }

    task->id = MaxTaskID++;
    helper->tasklist[task->id] = task;

#ifdef TASK_DEBUG
    printf("Task Created %p %ld %s\n", task, task->suspend, task->name);
#endif
    return (task);
}

int TaskDelete(TaskHandle_t handle)
{
    uint8_t id = TaskGetID(handle);

    if (helper->tasklist)
    {
        free(helper->tasklist[id]);
        helper->tasklist[id] = NULL;
        return (0);
    }

    return (1);
}


utime_t TaskDispatch()
{
    int idle_time = 0;

    utime_t now = uGetTimeMilli();
    utime_t elapsed = now - helper->lastrun;
    helper->lastrun = now;
    utime_t tend = 0;
    utime_t tremaining = MAX_TIME_VALUE;
    uint8_t i;
    utime_t start;

    // disable interrupts at this point
    for (i = 0; i < MAX_TASKS; ++i)
    {
        TaskData* t = helper->tasklist[i];

        if (!t)
        {
            continue;
        }

        if (t->suspend && now >= t->suspend)
        {
            TaskResume(t);
        }

        t->t_remaining -= elapsed; // this calculation is being screwed up with unsigned long as utime_t

        if (t->t_remaining <= 0)
        {
            t->t_remaining += t->t_period;
            start = uGetTimeMilli();

            // enable interrupts at this point
            // execute the task
            if (!t->suspend)
            {
                currTask = t;
                t->task(t->appdata);
                currTask = NULL;
            }

            // disable interrupts at this point
            tend = uGetTimeMilli();
            tremaining = start + t->t_period;
            t->t_completion = tremaining;
#ifdef TASK_DEBUG
            printf("Task %p Next execution now + %ld\n", (*it), (*it)->t_period);
#endif
        }
    }
    // enable interrupts at this point

    /*
     * find if any other tasks are completing soon and ready to be executed
     * calculate the idle_time based on next ready task
     */
    for (i = 0; i < MAX_TASKS; ++i)
    {
        TaskData* t1 = helper->tasklist[i];

        if (!t1)
        {
            continue;
        }

        if (t1->t_completion >= tend)
        {
            tremaining = MIN(tremaining, t1->t_completion);
        }
    }

    if (tremaining > tend)
    {
        idle_time = tremaining - tend;
    }

    return (idle_time);
}

int TaskSuspend(TaskHandle_t handle, utime_t delay)
{
    TaskData* taskdata = (TaskData*)(handle);
    taskdata->suspend = delay == MAX_TIME_VALUE ? MAX_TIME_VALUE : delay + uGetTimeMilli();

#ifdef TASK_DEBUG
    printf("Task Suspended %p %s\n", taskdata, taskdata->name);
#endif

    return (0);
}

int TaskResume(TaskHandle_t handle)
{
    TaskData* taskdata =(TaskData*)(handle);
    taskdata->suspend = 0;

#ifdef TASK_DEBUG
    printf("Task Resumed %p %s\n", taskdata, taskdata->name);
#endif

    return (0);
}

void TaskDelayMilli(utime_t time)
{
    usleep(time*1000);
}

void TaskDelayMicro(utime_t time)
{
    usleep(time);
}

int TaskInit(TaskHandle_t handle)
{
    ((TaskData*)(handle))->suspend = 0;
    return (0);
}

int TaskDeInit(TaskHandle_t handle)
{
    ((TaskData*)(handle))->suspend = MAX_TIME_VALUE;
    return (0);
}

const char* TaskGetName(TaskHandle_t handle)
{
    return (((TaskData*)(handle))->name);
}

int TaskGetID(TaskHandle_t handle)
{
    return (((TaskData*)(handle))->id);
}

int TaskJoin(TaskHandle_t handle)
{
    TaskData* taskdata = (TaskData*)(handle);
    while(currTask == taskdata);

    return (0);
}

