#pragma once
#include <iostream>
#include <memory>

#include "logger.h"
#include "sinks/sink.h"
#include "sinks/console_sink.h"
#include "log_common.h"
#include "utils/sys_util.h"
#include "context/context.h"

int main(){
    // auto sinkPtr=std::make_shared<logger::sink::ConsoleSink>();
    // logger::Logger log("test1",sinkPtr);
    // log.Log(logger::LogLevel::kInfo,logger::SourceLocation("test",1,"test.111"),std::to_string(logger::utils::GetThreadID()));

    // logger::context::Context* context = logger::context::Context::GetInstance();
    // context->GetExecutor()->AddTaskRunner(1);
    // for(int i=0;i<5;++i){
    //     context->GetExecutor()->PostTask(1,[i](){std::cout<<"1111  :"<<i<<"\n";});
    // }
    // while(1){}
    // 创建任务运行器

    logger::context::TaskRunnerTag runner_tag = NEW_TASK_RUNNER(1);

    // 提交一个普通任务
    POST_TASK(runner_tag, [] {
        std::cout << "Immediate task executed" << std::endl;
    });

    // 提交一个延迟任务
    POST_DELAY_TASK(runner_tag, [] {
        std::cout << "Delayed task executed" << std::endl;
    }, std::chrono::milliseconds(200));

    // 提交一个重复任务
    POST_REPEATED_TASK(runner_tag, [] {
        std::cout << "Repeated task executed" << std::endl;
    }, std::chrono::milliseconds(100), 3);

    // 确保所有任务都已完成
    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}
