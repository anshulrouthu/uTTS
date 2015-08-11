/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#include "task_scheduler.h"
#include <algorithm>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

namespace uTsk
{

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
    int id;
}TaskData;

typedef std::deque<TaskData*> uTaskList;

/* TODO: The scope of the tasklist variable should be valid only within
 * TaskInitialize() and TaskUninitializ() methods
 * Implement the creation and deletion of tasklist in a better way
 */
struct SchedulerData
{
    uTaskList tasklist;
    utime_t lastrun;
};

SchedulerData* helper = NULL;

utime_t uGetTimeMilli()
{
    long mtime;
    timeval time;

    gettimeofday(&time, NULL);

    mtime = ((time.tv_sec) * 1000 + time.tv_usec / 1000);
    return (mtime);
}

int TaskInitialize()
{
    helper = new SchedulerData();
    helper->lastrun = uGetTimeMilli();
    return (0);
}

int TaskUninitialize()
{
    for (uTaskList::iterator it = helper->tasklist.begin(); it != helper->tasklist.end(); ++it)
    {
        delete (*it);
        (*it) = NULL;
    }
    helper->tasklist.clear();

    delete helper;

    return (0);
}

TaskHandle_t TaskCreate(taskfunction* ptr, utime_t period, utime_t remaining_time, void* appdata, const char* name)
{
    TaskData* task = new TaskData();
    task->task = ptr;
    task->t_period = period;
    task->t_remaining = remaining_time;
    task->appdata = appdata;
    task->t_completion = remaining_time;
    task->suspend = MAX_TIME_VALUE;
    task->name = name;
    /* TODO: Not sure if task id is really required
     * implement decrement of MaxTaskID value on task deletion
     */
    task->id = ++MaxTaskID;

    helper->tasklist.push_back(task);

#ifdef TASK_DEBUG
    printf("Task Created %p %ld %s\n", task, task->suspend, task->name);
#endif
    return (task);
}

int TaskDelete(TaskHandle_t handle)
{
    uTaskList::iterator it = std::find(helper->tasklist.begin(), helper->tasklist.end(), handle);

    if (it != helper->tasklist.end())
    {
        delete (*it);
        (*it) = NULL;
        helper->tasklist.erase(it);
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

    // disable interrupts at this point
    for (uTaskList::iterator it = helper->tasklist.begin(); it != helper->tasklist.end(); ++it)
    {
        if ((*it)->suspend && now >= (*it)->suspend)
        {
            uTsk::TaskResume((*it));
        }

        (*it)->t_remaining -= elapsed; // this calculation is being screwed up with unsigned long as utime_t

        if ((*it)->t_remaining <= 0)
        {
            (*it)->t_remaining += (*it)->t_period;
            utime_t start = uGetTimeMilli();

            // enable interrupts at this point
            // execute the task
            if (!(*it)->suspend)
            {
                currTask = (*it);
                (*it)->task((*it)->appdata);
                currTask = NULL;
            }

            // disable interrupts at this point
            tend = uGetTimeMilli();
            tremaining = start + (*it)->t_period;
            (*it)->t_completion = tremaining;
#ifdef TASK_DEBUG
            printf("Task %p Next execution now + %ld\n", (*it), (*it)->t_period);
#endif
        }
    }
    // enable interrupts at this point

    /*
     * find if any other tasks are completing soon and ready to be executed
     * calculate the idle_time based on next ready task
     * TODO: Find a better solution rather iterating through entire list again
     */
    for (uTaskList::iterator iter = helper->tasklist.begin(); iter != helper->tasklist.end(); ++iter)
    {
        if ((*iter)->t_completion >= tend)
        {
            tremaining = MIN(tremaining,(*iter)->t_completion);
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
    TaskData* taskdata = static_cast<TaskData*>(handle);
    taskdata->suspend = delay == MAX_TIME_VALUE ? MAX_TIME_VALUE : delay + uGetTimeMilli();

#ifdef TASK_DEBUG
    printf("Task Suspended %p %s\n", taskdata, taskdata->name);
#endif

    return (0);
}

int TaskResume(TaskHandle_t handle)
{
    TaskData* taskdata = static_cast<TaskData*>(handle);
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
    static_cast<TaskData*>(handle)->suspend = 0;
    return (0);
}

int TaskDeInit(TaskHandle_t handle)
{
    static_cast<TaskData*>(handle)->suspend = MAX_TIME_VALUE;
    return (0);
}

const char* TaskGetName(TaskHandle_t handle)
{
    return (static_cast<TaskData*>(handle)->name);
}

int TaskGetID(TaskHandle_t handle)
{
    return (static_cast<TaskData*>(handle)->id);
}

int TaskJoin(TaskHandle_t handle)
{
    TaskData* taskdata = static_cast<TaskData*>(handle);
    while(currTask == taskdata);

    return (0);
}

}
