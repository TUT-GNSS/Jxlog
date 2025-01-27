#include "thread_pool.h"

namespace logger
{
namespace context
{
void ThreadPool::Stop(){
  if(is_available_.load()){
    is_shutdown_.store(true);
    task_cv_.notify_all();
    is_available_.store(false);
  }
  this->worker_threads_.clear();
}

bool ThreadPool::Start(){
  if(is_available_.load()){
    return false;
  }
  is_available_.store(true);
  uint32_t thread_count = thread_count_.load();
  for(uint32_t i = 0; i < thread_count; ++i){
    AddThread();
  }
  return true;
}

void ThreadPool::AddThread(){
  //创建ThreadInfo对象
  ThreadInfoPtr thread_info_ptr = std::make_shared<ThreadInfo>();

  auto func = [this](){
    while(!is_shutdown_){
      Task task;
      //从任务队列取出任务
      if(task_queue_.wait_pop(task, [this]()->bool {return is_shutdown_.load();})){
        task();
      }
    }
  };
  thread_info_ptr->thread_ptr = std::make_shared<std::thread>(func);
  worker_threads_.emplace_back(std::move(thread_info_ptr));
}

ThreadPool::ThreadInfo::~ThreadInfo() {
  if (thread_ptr && thread_ptr->joinable()) {
    thread_ptr->join();
  }
}

} // namespace context
} // namespace logger
