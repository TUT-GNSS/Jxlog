#pragma once

#include <ratio>  // 引入ratio库，用于定义比例类型

namespace logger {
// 默认ratio为1
template <typename _Rep, typename _Capacity = std::ratio<1>>
class space;

/**
 * @brief 模板函数space_cast，用于将一种space类型转换为另一种space类型。
 *
 * 这个函数实现了不同比例单位之间的转换。例如，可以将kilobytes转换为bytes，或者将bytes转换为megabytes。
 * 转换过程基于C++标准库中的std::ratio，它提供了一种类型安全的方式来表示比例和进行比例运算。
 *
 * @tparam _ToSpace 目标space类型，即转换后的空间类型。
 * @tparam _Rep 源space的表示类型，即原始空间的数值类型。
 * @tparam _Capacity 源space的比例类型，即原始空间的比例单位。
 *
 * @param __d 源space对象，即要转换的空间对象。
 *
 * @return _ToSpace 返回转换后的空间对象。
 *
 * @note 这个函数可以在编译时进行计算，从而提高效率。
 */

template <typename _ToSpace, typename _Rep, typename _Capacity>
constexpr _ToSpace space_cast(const space<_Rep, _Capacity>& __d) {
  // 计算转换后的数值，使用ratio_divide来处理比例的转换
  auto __r = __d.count() * std::ratio_divide<_Capacity, typename _ToSpace::period>::num /
             std::ratio_divide<_Capacity, typename _ToSpace::period>::den;
  // 返回转换后的空间对象，使用_ToSpace类型来构造对象
  return _ToSpace(__r);
}

template <typename _Rep, typename _Capacity>
class space {
 public:
  using rep = _Rep;          // 使用rep类型来表示数值
  using period = _Capacity;  // 使用period类型来表示比例

  // 初始化_rep为默认值
  constexpr space() : _rep() {}

  // 从其他ratio<1>的rep类型初始化
  template <typename _Rep2>
  explicit constexpr space(const _Rep2& __r) : _rep(__r) {}

  // 构造函数，space类型初始化
  template <typename _Rep2, typename _Capacity2>
  constexpr space(const space<_Rep2, _Capacity2>& __d) : _rep(space_cast<space>(__d).count()) {}

  // 获取当前数值
  constexpr _Rep count() const { return _rep; }

  // 定义正运算符，返回当前对象
  constexpr space operator+() const { return *this; }

  // 定义负运算符，返回当前对象的负值
  constexpr space operator-() const { return space(-count()); }

  // 前置递增运算符
  space& operator++() {
    ++_rep;
    return *this;
  }

  // 后置递增运算符
  space operator++(int) {
    space __tmp(*this);
    ++_rep;
    return __tmp;
  }

  // 前置递减运算符
  space& operator--() {
    --_rep;
    return *this;
  }

  // 后置递减运算符
  space operator--(int) {
    space __tmp(*this);
    --_rep;
    return __tmp;
  }

  // 加法赋值运算符
  space& operator+=(const space& __d) {
    _rep += __d.count();
    return *this;
  }

  // 减法赋值运算符
  space& operator-=(const space& __d) {
    _rep -= __d.count();
    return *this;
  }

  // 乘法赋值运算符
  space& operator*=(const _Rep& __s) {
    _rep *= __s;
    return *this;
  }

  // 除法赋值运算符
  space& operator/=(const _Rep& __s) {
    _rep /= __s;
    return *this;
  }

  // 取模赋值运算符
  space& operator%=(const _Rep& __s) {
    _rep %= __s;
    return *this;
  }

 private:
  _Rep _rep;  // 存储数值的成员变量
};

// 定义加法运算符，用于space类型的加法
template <typename _Rep, typename _Capacity>
constexpr space<_Rep, _Capacity> operator+(const space<_Rep, _Capacity>& __x, const space<_Rep, _Capacity>& __y) {
  return space<_Rep, _Capacity>(__x.count() + __y.count());
}

// 定义减法运算符，用于space类型的减法
template <typename _Rep, typename _Capacity>
constexpr space<_Rep, _Capacity> operator-(const space<_Rep, _Capacity>& __x, const space<_Rep, _Capacity>& __y) {
  return space<_Rep, _Capacity>(__x.count() - __y.count());
}

// 定义常用的比例类型
using kilo = std::ratio<1024L, 1>;
using mega = std::ratio<1024L * 1024, 1>;
using giga = std::ratio<1024L * 1024 * 1024, 1>;
using tera = std::ratio<1024L * 1024 * 1024 * 1024, 1>;

// 定义常用的space类型
using bytes = space<size_t>;
using kilobytes = space<size_t, kilo>;
using megabytes = space<size_t, mega>;
using gigabytes = space<size_t, giga>;
using terabytes = space<size_t, tera>;

}  // namespace logger