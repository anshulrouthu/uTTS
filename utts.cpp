/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#include "utts.h"
#include <sys/time.h>
#include <assert.h>

#define MIN(a,b) a > b ? b : a

uTTS::uTTS():
    m_task_list(),
    m_lastrun(0)
{
    m_lastrun = GetTime();
}

uTTS::~uTTS()
{
    for (std::vector<TaskData*>::iterator it = m_task_list.begin(); it != m_task_list.end(); ++it)
    {
        delete (*it);
        (*it) = NULL;
    }
    m_task_list.clear();
}

utime_t uTTS::GetTime()
{
    long mtime;
    timeval time;

    gettimeofday(&time, NULL);

    mtime = ((time.tv_sec) * 1000 + time.tv_usec / 1000);
    return (mtime);
}

TaskHandle_t uTTS::TaskCreate(taskfunction* ptr, utime_t period, utime_t remaining_time, void* appdata)
{
    TaskData* task = new TaskData();
    task->task = ptr;
    task->t_period = period;
    task->t_remaining = remaining_time;
    task->appdata = appdata;
    task->t_completion = remaining_time;
    task->suspend = 0;

    m_task_list.push_back(task);

#ifdef TASK_DEBUG
    printf("Task Created %p\n", task);
#endif
    return (task);
}

utime_t uTTS::TaskDispatch()
{
    int idle_time = 0;

    utime_t now = GetTime();
    utime_t elapsed = now - m_lastrun;
    m_lastrun = now;
    utime_t tend = 0;
    utime_t tremaining = MAX_TIME_VALUE;

    // disable interrupts at this point
    for (std::vector<TaskData*>::iterator it = m_task_list.begin(); it != m_task_list.end(); ++it)
    {
        if ((*it)->suspend && now >= (*it)->suspend)
        {
            TaskResume((*it));
        }

        (*it)->t_remaining -= elapsed;

        if ((*it)->t_remaining <= 0)
        {
            (*it)->t_remaining += (*it)->t_period;
            utime_t start = GetTime();

            // enable interrupts at this point
            // execute the task
            if (!(*it)->suspend)
            {
                (*it)->task((*it)->appdata);
            }

            // disable interrupts at this point
            tend = GetTime();
            tremaining = start + (*it)->t_period;
            (*it)->t_completion = tremaining;
            printf("Task %p Next execution now + %ld\n", (*it), (*it)->t_period);
        }
    }
    // enable interrupts at this point

    /*
     * find if any other tasks are completing soon and ready to be executed
     * calculate the idle_time based on next ready task
     */
    for (std::vector<TaskData*>::iterator iter = m_task_list.begin(); iter != m_task_list.end(); ++iter)
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

int uTTS::TaskSuspend(TaskHandle_t handle, utime_t delay)
{
    TaskData* taskdata = static_cast<TaskData*>(handle);
    taskdata->suspend = delay + GetTime();

#ifdef TASK_DEBUG
    printf("Task Suspended %p\n", taskdata);
#endif

    return (0);
}

int uTTS::TaskResume(TaskHandle_t handle)
{
    TaskData* taskdata = static_cast<TaskData*>(handle);
    taskdata->suspend = 0;

#ifdef TASK_DEBUG
    printf("Task Resumed %p\n", taskdata);
#endif

    return (0);
}
