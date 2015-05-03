/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#ifndef UTTS_H_
#define UTTS_H_

#include <vector>

typedef long utime_t;
typedef void* TaskHandle_t;
#define MAX_TIME_VALUE ~(0)

class uTTS
{
    typedef void (taskfunction)(void* appdata);
    typedef struct TaskData
    {
        utime_t t_period;
        utime_t t_remaining;
        void* appdata;
        taskfunction* task;
        utime_t t_completion;
        utime_t suspend;
    }TestData;

public:
    uTTS();
    ~uTTS();

    TaskHandle_t TaskCreate(taskfunction* ptr, utime_t period, utime_t remaining_time, void* appdata);
    utime_t TaskDispatch();
    int TaskSuspend(TaskHandle_t handle, utime_t delay = MAX_TIME_VALUE);
    int TaskResume(TaskHandle_t handle);

private:
    static utime_t GetTime();
    std::vector<TaskData*> m_task_list;
    utime_t m_lastrun;
};



#endif /* UTTS_H_ */
