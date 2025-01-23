#include <iostream>
#include <memory>

#include "logger.h"
#include "sinks/sink.h"

int main(){
    auto sinkPtr=std::make_shared<logger::LogSink>();
    logger::Logger log("test1",sinkPtr);
    return 0;
}