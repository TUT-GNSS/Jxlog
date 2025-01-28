#pragma once
#include <iostream>
#include <memory>

#include "logger.h"
#include "sinks/sink.h"
#include "sinks/console_sink.h"
#include "log_common.h"
#include "utils/sys_util.h"
#include "context/thread_pool.h"

// int main(){
//     auto sinkPtr=std::make_shared<logger::sink::ConsoleSink>();
//     logger::Logger log("test1",sinkPtr);
//     log.Log(logger::LogLevel::kInfo,logger::SourceLocation("test",1,"test.111"),std::to_string(logger::utils::GetThreadID()));
    
//     logger::context::ThreadPool thread_pool(1);
//     auto t = std::thread(thread_pool.Start());
//     thread_pool.RunTask([](){std::cout<<"ThreadPool"<<"\n";});
//     thread_pool.Stop();
//     t.join();
//     return 0;
// }

#include <thread>
#include <chrono>
#include <vector>

int main() {
    // 创建一个线程池，包含 4 个工作线程
    logger::context::ThreadPool thread_pool(4);

    // 启动线程池
    if (!thread_pool.Start()) {
        std::cerr << "Failed to start thread pool" << std::endl;
        return 1;
    }

    // 提交一些无返回值任务
    thread_pool.RunTask([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task 1 completed in thread " << std::this_thread::get_id() << std::endl;
    });

    thread_pool.RunTask([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Task 2 completed in thread " << std::this_thread::get_id() << std::endl;
    });

    // 提交一个有返回值任务
    auto future = thread_pool.RunRetTask([](int x, int y) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return x + y;
    }, 5, 3);

    // 等待有返回值任务完成并获取结果
    if (future) {
        std::cout << "Task 3 result: " << future->get() << std::endl;
    }

    // 提交更多任务
    thread_pool.RunTask([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Task 4 completed in thread " << std::this_thread::get_id() << std::endl;
    });

    thread_pool.RunTask([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Task 5 completed in thread " << std::this_thread::get_id() << std::endl;
    });

    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 停止线程池
    thread_pool.Stop();

    std::cout << "All tasks completed and thread pool stopped." << std::endl;

    return 0;
}