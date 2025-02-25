#pragma once

#include <iostream>
#include <string>

// 时间类
class Timestamp
{
public:
    Timestamp();
    // explicit取消隐式转换,防止程序与我们预想的不一样
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;
};