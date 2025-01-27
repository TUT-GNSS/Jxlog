#pragma once

#include <atomic>
#include <iostream>
#include <queue>
#include <thread>
#include <vector>

namespace logger
{
namespace context
{
template <typename T>
class ThreadQueue {

public:
  ThreadQueue() = default; 
    
  void push(T new_val) {
    std::lock_guard<std::mutex> lock(mtx_);
    data_queue_.push(std::move(new_val));
    queue_cv_.notify_one();
  }

  //阻塞pop 
  void wait_pop(T& val) {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_cv_.wait(lock, [this]() { return !data_queue_.empty(); });
    val = std::move(data_queue_.front());
    data_queue_.pop();
  }
  //阻塞pop 
  void wait_pop(T& val) {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_cv_.wait(lock, [this]() { return !data_queue_.empty(); });
    val = std::move(data_queue_.front());
    data_queue_.pop();
  }

  //阻塞pop,提供additional_condition终止阻塞
  bool wait_pop(T& val, std::function<bool()> additional_condition) {
    std::unique_lock<std::mutex> lock(mtx_);
    queue_cv_.wait(lock, [this]() { return !data_queue_.empty() || additional_condition(); });
    if(additional_condition){
        return false;
    }
    val = std::move(data_queue_.front());
    data_queue_.pop();
    return true;
  }

  //非阻塞pop
  bool try_pop(T& val) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_queue_.empty()) {
        return false;
    }
    val = std::move(data_queue_.front());
    data_queue_.pop();
    return true;
  }

  //非阻塞pop
  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (data_queue_.empty()) {
        return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res =
        std::make_shared<T>(std::move(data_queue_.front()));
    data_queue_.pop();
    return res;
  }

  // 尝试从队列中弹出元素，带超时
  bool wait_pop_with_timeout(T& val, uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lock(mtx_);
    // 等待直到队列非空或超时
    if (queue_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]() { return !data_queue_.empty(); })) {
      val = std::move(data_queue_.front());
      data_queue_.pop();
      return true; // 成功获取元素
    } else {
      return false; // 超时，队列仍然为空
    }
  }

  // 尝试从队列中弹出元素，带超时
  std::shared_ptr<T> wait_pop_with_timeout(uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lock(mtx_);
    // 等待直到队列非空或超时
    if (queue_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]() { return !data_queue_.empty(); })) {
      std::shared_ptr<T> res =
        std::make_shared<T>(std::move(data_queue_.front()));
      data_queue_.pop();
      return res;
    } else {
      return std::shared_ptr<T>(); // 超时，队列仍然为空
    }
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return data_queue_.empty();
  }
private:
  mutable std::mutex mtx_;
  std::queue<T> data_queue_;
  std::condition_variable queue_cv_;
};     
} // namespace context
} // namespace logger

