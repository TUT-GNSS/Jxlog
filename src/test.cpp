#pragma once
#include <iostream>
#include <memory>

#include "logger.h"
#include "sinks/sink.h"
#include "sinks/console_sink.h"
#include "log_common.h"
#include "utils/sys_util.h"
#include "context/executor.h"

int main(){
    auto sinkPtr=std::make_shared<logger::sink::ConsoleSink>();
    logger::Logger log("test1",sinkPtr);
    log.Log(logger::LogLevel::kInfo,logger::SourceLocation("test",1,"test.111"),std::to_string(logger::utils::GetThreadID()));

    logger::context::Executor executor;
    executor.AddTaskRunner(1);
    executor.AddTaskRunner(2);
    for(int i=0;i<5;++i){
        executor.PostTask(1,[i](){std::cout<<"1111  :"<<i<<"\n";});
        executor.PostTask(2,[i](){std::cout<<"2222  :"<<i<<"\n";});
    }
    while(1){}

    return 0;
}
