/*
 * Copyright (C) 2015 Anshul Routhu <anshul.m67@gmail.com>
 * All rights reserved.
 */

#ifndef UTSK_SCHEDULER_H_
#define UTSK_SCHEDULER_H_

#include <deque>
#include <limits.h>

namespace uTsk
{

typedef enum TaskEvent
{
    TASK_INIT = 0,
    TASK_DEINIT
}TaskEvent;

typedef long utime_t;
typedef void* TaskHandle_t;
#define MAX_TIME_VALUE LONG_MAX

typedef void (taskfunction)(void* appdata);

int TaskInitialize();
int TaskUninitialize();
TaskHandle_t TaskCreate(taskfunction* ptr, utime_t period, utime_t remaining_time, void* appdata, const char* name);
int TaskDelete(TaskHandle_t handle);
utime_t TaskDispatch();
int TaskSuspend(TaskHandle_t handle, utime_t delay = MAX_TIME_VALUE);
int TaskResume(TaskHandle_t handle);
void TaskDelayMilli(utime_t time);
void TaskDelayMicro(utime_t time);
int TaskInit(TaskHandle_t handle);
int TaskDeInit(TaskHandle_t handle);
const char* TaskGetName(TaskHandle_t handle);
int TaskGetID(TaskHandle_t handle);
int TaskJoin(TaskHandle_t handle);

}

#endif /* UTSK_SCHEDULER_H_ */
