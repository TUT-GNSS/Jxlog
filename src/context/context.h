#pragma once

#include "context/executor.h"

namespace logger {
namespace context {

class ExecutorManager;

class Context {
 public:
  ~Context() = default;
  Executor* GetExecutor();

  static Context* GetInstance() {
    static Context* instance = new Context();
    return instance;
  }

  TaskRunnerTag NewTaskRunner(TaskRunnerTag tag);

  Context(const Context& other) = delete;
  Context& operator=(const Context& other) = delete;

 private:
  Context();

  std::unique_ptr<ExecutorManager> executor_manager_;
};
}  // namespace context
}  // namespace logger

#define CONTEXT logger::context::Context::GetInstance()

#define EXECUTOR CONTEXT->GetExecutor()

#define NEW_TASK_RUNNER(tag) CONTEXT->NewTaskRunner(tag)

#define POST_TASK(runner_tag, task) EXECUTOR->PostTask(runner_tag, task)

#define POST_DELAY_TASK(runner_tag, task, delay_time) \
  EXECUTOR->PostDelayedTask(runner_tag, task, delay_time)

#define POST_REPEATED_TASK(runner_tag, task, delay_time, repeat_num) \
  EXECUTOR->PostRepeatedTask(runner_tag, task, delay_time, repeat_num)

#define WAIT_TASK_IDLE(runner_tag) \
  EXECUTOR->PostTaskAndGetResult(runner_tag, []() {})->wait()