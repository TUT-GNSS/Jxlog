#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "thread_queue.h"

namespace logger
{
namespace context
{

class ThreadPool{
public:

  explicit ThreadPool(uint32_t thread_count):is_available_(false),
                                             is_shutdown_(false),
                                             thread_count_(thread_count){}
  ThreadPool(const ThreadPool& other) = delete;
  ThreadPool& operator=(const ThreadPool& other) = delete;

  ~ThreadPool() { Stop(); }

  bool Start();

  void Stop();

  //运行无返回值任务
  template<typename T, typename... Args>
  void RunTask(T&& func, Args&&... args){
    if(is_shutdown_.load() || !is_available_.load()){
      return;
    }
    
    auto task = std::bind(std::forward<T>(func), std::forward<Args>(args)...);
    task_queue_.Push([task](){task();});
  }

  //运行有返回值任务
  template<typename T, typename... Args>
  auto RunRetTask(T&& func, Args&&... args) -> std::shared_ptr<std::future<std::result_of_t<T(Args...)>>> {
    if(is_shutdown_.load() || !is_available_.load()){
      return nullptr;
    }
    //返回值类型
    using return_type = std::result_of_t<T(Args...)>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<T>(func), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    task_queue_.Push([task](){ (*task)(); });
    
    return std::make_shared<std::future<return_type>>(std::move(res));
  }

private:
  void AddThread();

  using ThreadPtr = std::shared_ptr<std::thread>;
  using Task = std::function<void()>;

  //存储线程信息
  struct ThreadInfo
  {
    ThreadInfo() = default;
    ~ThreadInfo();
  
    ThreadPtr thread_ptr{nullptr};  
  };

  using ThreadInfoPtr = std::shared_ptr<ThreadInfo>;

  std::vector<ThreadInfoPtr> worker_threads_;    //工作线程
  logger::context::ThreadQueue<Task> task_queue_;//任务队列

  std::atomic<uint32_t> thread_count_;
  std::atomic<bool> is_shutdown_;   // 线程池是否已经关闭
  std::atomic<bool> is_available_;  // 是否有可用的工作线程

};


} // namespace context
} // namespace logger
