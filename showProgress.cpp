//
// Created by 17632 on 24-2-25.
//

#include <string>
#include <iostream>
#include "showProgress.h"

// 函数用于显示进度条和额外的消息
void show_progress_bar(int progress, int total, const std::string& message) {
    int width = 50; // 进度条的宽度
    float progress_level = (float)progress / total;
    int pos = width * progress_level;

    std::cout << "[";
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress_level * 100.0) << " % - "<<progress<<"/"<<total<<" " << message << "\r";
    std::cout.flush();
}
