#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "thread_pool.h"

namespace logger {
namespace context {

using Task = std::function<void(void)>;
using TaskRunnerTag = uint64_t;
using RepeatedTaskId = uint64_t;
using RepeatedTaskNum = uint64_t;

constexpr uint64_t krepeated_task_id_max = std::numeric_limits<uint64_t>::max();

class Executor {
  // 应用定时器
  class ExecutorTimer {
   public:
    // 任务信息
    struct InternalS {
      InternalS() = default;

      InternalS(Task task,
                std::chrono::time_point<std::chrono::high_resolution_clock> time_point,
                RepeatedTaskId id = krepeated_task_id_max)
          : task_(std::move(task)), time_point_(time_point), repeated_task_id_(id) {}

      Task task_;
      std::chrono::time_point<std::chrono::high_resolution_clock> time_point_;
      RepeatedTaskId repeated_task_id_;
      // 重载< 用于优先队列的比较 时间早的高优先级
      bool operator<(const InternalS& other) const { return this->time_point_ > other.time_point_; };
    };

    ExecutorTimer();
    ~ExecutorTimer();

    ExecutorTimer(const ExecutorTimer&) = delete;
    ExecutorTimer& operator=(const ExecutorTimer&) = delete;

    bool Start();
    void Stop();

    void PostDelayedTask(Task task, const std::chrono::microseconds& delay_time);

    RepeatedTaskId PostRepeatedTask(Task task,
                                    const std::chrono::microseconds& delay_time,
                                    RepeatedTaskNum repeated_num);

    void CancelRepeatedTask(RepeatedTaskId repeated_task_id);

   private:
    void Run_();
    // 封装一层 供PostRepeatedTask_ bind  封装后方便拓展
    void PostTask_(Task task,
                   std::chrono::microseconds delay_time,
                   RepeatedTaskId repeated_task_id,
                   RepeatedTaskNum repeated_task_num);

    void PostRepeatedTask_(Task task,
                           const std::chrono::microseconds& delay_time,
                           RepeatedTaskId repeated_task_id,
                           RepeatedTaskNum repeated_task_num);

    RepeatedTaskId GetNextPepeatedTaskId() { return repeated_task_id_++; }

    std::priority_queue<InternalS> p_queue_;
    std::mutex mtx_;
    std::condition_variable cond_cv_;
    std::atomic<bool> running_;
    std::unique_ptr<ThreadPool> thread_pool_;

    std::atomic<RepeatedTaskId> repeated_task_id_;
    std::unordered_set<RepeatedTaskId> repeated_id_state_set_;  // 存放RepeatedTaskId的字典
  };

  // 管理整个应用所有strand
  class ExecutorContext {
   public:
    ExecutorContext() = default;
    ~ExecutorContext() = default;

    ExecutorContext(const ExecutorContext& other) = delete;
    ExecutorContext& operator=(const ExecutorContext& other) = delete;

    TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);

   private:
    using TaskRunner = ThreadPool;
    using TaskRunnerPtr = std::unique_ptr<TaskRunner>;
    friend class Executor;

    TaskRunner* GetTaskRunner(const TaskRunnerTag& tag);

    TaskRunnerTag GetNextRunnerTag();

    std::unordered_map<TaskRunnerTag, TaskRunnerPtr> task_runner_map_;
    std::mutex mtx_;
  };

 public:
  Executor();
  ~Executor();

  Executor(const Executor&) = delete;
  Executor& operator=(const Executor&) = delete;

  TaskRunnerTag AddTaskRunner(const TaskRunnerTag& tag);

  void PostTask(const TaskRunnerTag& runner_tag, Task task);

  template <typename R, typename P>
  void PostDelayedTask(const TaskRunnerTag& runner_tag, Task task, const std::chrono::duration<R, P>& delta) {
    // 将对应线程池执行task任务 封装为一个func
    Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));

    executor_timer_->Start();
    executor_timer_->PostDelayedTask(std::move(func), std::chrono::duration_cast<std::chrono::microseconds>(delta));
  }

  template <typename R, typename P>
  RepeatedTaskId PostRepeatedTask(const TaskRunnerTag& runner_tag,
                                  Task task,
                                  const std::chrono::duration<R, P>& delta,
                                  uint64_t repeat_num) {
    Task func = std::bind(&Executor::PostTask, this, runner_tag, std::move(task));

    executor_timer_->Start();

    return executor_timer_->PostRepeatedTask(std::move(func),
                                             std::chrono::duration_cast<std::chrono::microseconds>(delta), repeat_num);
  }

  void CancelRepeatedTask(RepeatedTaskId task_id) { executor_timer_->CancelRepeatedTask(task_id); }

  template <typename T, typename... Args>
  auto PostTaskAndGetResult(const TaskRunnerTag& runner_tag, T&& func, Args&&... args)
      -> std::shared_ptr<std::future<std::result_of_t<T(Args...)>>> {
    ExecutorContext::TaskRunner* task_runner = executor_context_->GetTaskRunner(runner_tag);
    auto result = task_runner->RunRetTask(std::forward<T>(func), std::forward<Args>(args)...);
    return result;
  }

 private:
  std::unique_ptr<ExecutorContext> executor_context_;
  std::unique_ptr<ExecutorTimer> executor_timer_;
};

}  // namespace context
}  // namespace logger
