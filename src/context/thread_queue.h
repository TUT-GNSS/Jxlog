#pragma once

#include <atomic>
#include <iostream>
#include <queue>
#include <thread>
#include <vector>

namespace logger {
namespace context {
template <typename T>
class ThreadQueue {
 public:
  ThreadQueue() = default;

  void Push(const T& new_val) {
    std::lock_guard<std::mutex> lock(mtx_);
    data_queue_.push(new_val);
    queue_cv_.notify_all();
  }
  void Push(const T&& new_val) {
    std::lock_guard<std::mutex> lock(mtx_);
    data_queue_.emplace(std::move(new_val));
    queue_cv_.notify_all();
  }

  // 阻塞pop
  bool WaitPop(T& val) {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_cv_.wait(
        lock, [this]() { return !data_queue_.empty() || stop_wait_.load(); });
    if (stop_wait_) {
      return false;
    }
    val = std::move(data_queue_.front());
    data_queue_.pop();
    return true;
  }

  // 阻塞pop
  T WaitPop() {
    std::unique_lock<std::mutex> lock(mtx_);
    // 等待直到队列非空或者停止阻塞
    queue_cv_.wait(lock,
                   [this]() { return !data_queue_.empty() || stop_wait_; });
    if (stop_wait_) {
      return std::shared_ptr<T>();
    }
    T res = std::move(data_queue_.front());
    data_queue_.pop();
    return res;
  }

  // 非阻塞pop
  bool TryPop(T& val) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_queue_.empty()) {
      return false;
    }
    val = std::move(data_queue_.front());
    data_queue_.pop();
    return true;
  }

  // 非阻塞pop
  T TryPop() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_queue_.empty()) {
      return std::shared_ptr<T>();
    }
    T res = std::move(data_queue_.front());
    data_queue_.pop();
    return res;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_queue_.empty();
  }
  void StopWait() {
    if (stop_wait_.load()) {
      return;
    }
    stop_wait_.store(true);
    queue_cv_.notify_all();
  }

 private:
  mutable std::mutex mtx_;
  std::queue<T> data_queue_;
  std::condition_variable queue_cv_;
  std::atomic<bool> stop_wait_{false};
};
}  // namespace context
}  // namespace logger
