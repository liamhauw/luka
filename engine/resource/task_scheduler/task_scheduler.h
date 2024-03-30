// SPDX license identifier: MIT.
// Copyright (C) 2023-present Liam Hauw.

#pragma once

// clang-format off
#include "platform/pch.h"
// clang-format on

#include <TaskScheduler.h>

namespace luka {

class TaskScheduler {
 public:
  TaskScheduler();

  void Tick();

  u32 GetThreadCount() const;
  void AddTaskSetToPipe(enki::ITaskSet* task_set);
  void WaitforTask(const enki::ICompletable* completable);

 private:
  const u32 kThreadCount{3};
  enki::TaskScheduler task_scheduler_;
};

}  // namespace luka
