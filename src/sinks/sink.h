#pragma once

#include "../log_common.h"
#include "../log_msg.h"

namespace logger{

class LogSink{
public:
    LogSink() =default;
    ~LogSink() = default;
    void Log(const LogMsg& log_msg){
        std::cout<<"log"<<"\n";
    }
};
}