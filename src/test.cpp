#pragma once
#include <iostream>
#include <memory>

#include "logger.h"
#include "sinks/sink.h"
#include "sinks/console_sink.h"
#include "log_common.h"
#include "utils/sys_util.h"

int main(){
    auto sinkPtr=std::make_shared<logger::sink::ConsoleSink>();
    logger::Logger log("test1",sinkPtr);
    log.Log(logger::LogLevel::kInfo,logger::SourceLocation("test",1,"test.111"),std::to_string(logger::utils::GetThreadID()));

    return 0;
}