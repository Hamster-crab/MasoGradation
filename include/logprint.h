/*
** 一秒ごとに指定した文字を出力するコード
** 書き方
**
**
** #include "logprint.h"
** #include <thread>
** #include <chrono>
**
** int main() {
**  int count = 0;
**  while (true) {
**    logPrint("A - count: ", count);
**    count++;
*+    std::this_thread::sleep_for(std::chrono::seconds(1));
**  }
**  return 0;
** }
**
**
**/








#ifndef LOGPRINT_H
#define LOGPRINT_H

#include <iostream>

#if __cplusplus >= 202002L  // C++20 以降
    #include <format>
    template <typename... Args>
    void logPrint(const std::string& format_str, Args&&... args) {
        std::cout << std::format(format_str, std::forward<Args>(args)...) << std::endl;
    }
#else  // C++17 以前
    #include <sstream>
    template <typename... Args>
    void logPrint(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        std::cout << oss.str() << std::endl;
    }
#endif

#endif // LOGPRINT_H

