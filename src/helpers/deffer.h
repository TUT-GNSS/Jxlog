#pragma once

#include <functional>

namespace logger
{
namespace helpers
{
class ExecuteOnScopeExit{
public:
  ExecuteOnScopeExit() = default;
  ExecuteOnScopeExit(ExecuteOnScopeExit&&) = default;
  ExecuteOnScopeExit& operator=(ExecuteOnScopeExit&&) = default;

  ExecuteOnScopeExit(ExecuteOnScopeExit&) = delete; 
  ExecuteOnScopeExit& operator=(ExecuteOnScopeExit&) = delete;

  template<typename T, typename ...Args>
  ExecuteOnScopeExit(T&& func, Args&&... args){
    func_ = std::bind(std::forward<T>(func), std::forward<Args>(args)...); 
  }

  ~ExecuteOnScopeExit() noexcept{
    if(func_){
        func_();
    }
  }
private:
  std::function<void()> func_; 
};
    
} // namespace helpers
} // namespace logger

//将两个标识符 a 和 b 连接成一个新的标识符:例如_LOG_CONCAT(defer, 42)  结果是 defer42
#define _LOG_CONCAT(a, b) a##b
//使用当前行号创建唯一对象
//例如: _MAKE_DEFER_(42)就是 logger::helpers::ExecuteOnScopeExit deffer42 = [&]()
#define _MAKE_DEFER_(line) logger::helpers::ExecuteOnScopeExit _LOG_CONCAT(defer, line) = [&]()

#undef LOG_DEFER
#define LOG_DEFER _MAKE_DEFER_(__LINE__)

//使用 LOG_DEFER{ ... }   ...中为延迟释放的代码