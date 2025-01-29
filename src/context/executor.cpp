#include "executor.h"

namespace logger
{
namespace context
{

Executor::ExecutorTimer::ExecutorTimer(){
  thread_pool_ = std::make_unique<logger::context::ThreadPool>(1);
  repeated_task_id_.store(0);
  running_.store(true);
}

Executor::ExecutorTimer::~ExecutorTimer(){
  this->Stop();
}

void Executor::ExecutorTimer::Stop(){
  running_.store(false);
  cond_cv_.notify_all();
  thread_pool_.reset();
}

bool Executor::ExecutorTimer::Start(){
  if(running_){
    return true;
  }
  running_.store(true);
  bool ret = thread_pool_->Start(); // 开启线程池
  thread_pool_->RunTask(&Executor::ExecutorTimer::Run_,this); // 绑定工作线程任务
  return ret;
}

//处理p_queue的任务
void Executor::ExecutorTimer::Run_(){
  while (running_.load())
  {
    std::unique_lock<std::mutex> lk(mtx_);
    cond_cv_.wait(lk,[this](){return p_queue_.empty();});
    InternalS s = p_queue_.top();
    // 当前时间与任务执行时间的差 
    auto diff = s.time_point_ - std::chrono::high_resolution_clock::now();

    if(std::chrono::duration_cast<std::chrono::microseconds>(diff).count() > 0){
      //阻塞等待剩余时间   
      cond_cv_.wait_for(lk, diff);
      continue;
    }else{
      p_queue_.pop();
      lk.unlock();
      s.task_();//执行队列任务
    }
  }
}

void Executor::ExecutorTimer::PostDelayTask(Task task, const std::chrono::microseconds& delay_time){
  auto time_point = std::chrono::high_resolution_clock::now() + delay_time;
  {
    std::lock_guard<std::mutex> lk(mtx_);
    p_queue_.emplace(std::move(task), time_point);
  } 
  cond_cv_.notify_all();
}

//放入repeated_id字典 调用PostRepeatedTask_()
RepeatedTaskId Executor::ExecutorTimer::PostRepeatedTask(Task task, const std::chrono::microseconds& delay_time, RepeatedTaskNum repeated_num){
  // 获取repeated_task_id
  RepeatedTaskId repeated_task_id = GetNextPepeatedTaskId();
  repeated_id_state_set_.insert(repeated_task_id);
  PostRepeatedTask_(std::move(task), delay_time, repeated_task_id, repeated_num);
  return repeated_task_id;
}

void Executor::ExecutorTimer::CancelRepeatedTask(RepeatedTaskId repeated_task_id){
  repeated_id_state_set_.erase(repeated_task_id);
}

// 封装一层 供PostRepeatedTask_ bind  封装后方便拓展
void Executor::ExecutorTimer::PostTask_(Task task, std::chrono::microseconds delay_time, RepeatedTaskId repeated_task_id,RepeatedTaskNum repeated_task_num){
  PostRepeatedTask_(std::move(task), delay_time, repeated_task_id, repeated_task_num);
}   

//调用task() 然后将重复次数-1 重新将自己入队
void Executor::ExecutorTimer::PostRepeatedTask_(Task task, const std::chrono::microseconds& delay_time, RepeatedTaskId repeated_task_id,RepeatedTaskNum repeated_task_num){

  if(repeated_id_state_set_.find(repeated_task_id) == repeated_id_state_set_.end() || repeated_task_num == 0){
    return;
  }
  // 执行重复任务 
  task();
  Task func = std::bind(&Executor::ExecutorTimer::PostTask_, this, std::move(task), delay_time, repeated_task_id, repeated_task_num - 1);

  auto time_point_= std::chrono::high_resolution_clock::now() + delay_time;
  {
    std::lock_guard<std::mutex> lk(mtx_);
    p_queue_.emplace(std::move(func),time_point_,repeated_task_id);
  }
  cond_cv_.notify_all();
}
TaskRunnerTag Executor::ExecutorContext::AddTaskRunner(const TaskRunnerTag& tag){
  std::lock_guard<std::mutex> lock(mtx_);
  TaskRunnerTag latest_tag = tag;
  //找到没使用的tag
  while (task_runner_map_.find(latest_tag) != task_runner_map_.end()) {
    latest_tag = GetNextRunnerTag();
  }

  task_runner_map_[latest_tag] = std::make_unique<TaskRunner>(1);
  task_runner_map_[latest_tag]->Start();

  return latest_tag;
}

Executor::ExecutorContext::TaskRunner* Executor::ExecutorContext::GetTaskRunner(const TaskRunnerTag& tag){
  std::lock_guard<std::mutex> lk(mtx_);
  if(task_runner_map_.find(tag) == task_runner_map_.end()){
    return nullptr;
  }else{
    return task_runner_map_[tag].get();
  }
}

TaskRunnerTag Executor::ExecutorContext::GetNextRunnerTag(){
  static uint64_t index = 0;
  ++index;
  return index;
}

Executor::Executor(){
  executor_context_ = std::make_unique<ExecutorContext>();
  executor_timer_ = std::make_unique<ExecutorTimer>();
}

Executor::~Executor(){
  //显示释放指针
  executor_context_.reset();
  executor_timer_.reset();
}

TaskRunnerTag Executor::AddTaskRunner(const TaskRunnerTag& tag){
  return executor_context_->AddTaskRunner(tag);
}

void Executor::PostTask(const TaskRunnerTag& runner_tag, Task task){
  ExecutorContext::TaskRunner* task_runner = executor_context_->GetTaskRunner(runner_tag);
  task_runner->RunTask(std::move(task));
}

} // namespace context
} // namespace logger
