// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

// clang-format off
#include "platform/pch.h"
// clang-format on

#include "base/task_scheduler/task_scheduler.h"

namespace luka {

TaskScheduler::TaskScheduler() {
  enki::TaskSchedulerConfig task_scheduler_config{};
  task_scheduler_config.numTaskThreadsToCreate = kThreadCount - 1;
  task_scheduler_.Initialize(task_scheduler_config);
}

void TaskScheduler::Tick() {}

u32 TaskScheduler::GetThreadCount() const { return kThreadCount; }

void TaskScheduler::AddTaskSetToPipe(enki::ITaskSet* task_set) {
  task_scheduler_.AddTaskSetToPipe(task_set);
}

void TaskScheduler::WaitforTask(const enki::ICompletable* completable) {
  task_scheduler_.WaitforTask(completable);
}

}  // namespace luka
