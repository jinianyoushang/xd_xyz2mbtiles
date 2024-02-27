#include <iostream>
#include <string>
#include <regex>
#include <tuple>

using namespace std;



//解析路径
//返回 z x y
std::tuple<int, int, int> ParsePath(const std::string& path, const std::string& pattern=R"({z}\\{x}\\{y}\\tile.png)") {
    // 转换模式到正则表达式
    std::string regexPattern = std::regex_replace(pattern, std::regex("\\{z\\}"), "(\\d+)");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\{x\\}"), "(\\d+)");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\{y\\}"), "(\\d+)");

    regexPattern=".*"+regexPattern;
    cout<<regexPattern<<endl;
    // 创建正则表达式
    std::regex regex(regexPattern);
    std::smatch matches;

    // 尝试匹配路径
    if (std::regex_search(path, matches, regex)) {
        // 检查是否有足够的匹配组
        if (matches.size() >= 4) {
            // 提取Z, X, Y值
            int z = std::stoi(matches[1]);
            int x = std::stoi(matches[2]);
            int y = std::stoi(matches[3]);
            return std::make_tuple(z, x, y);
        }
    }

    // 如果没有找到匹配或匹配格式错误
    throw std::runtime_error("Invalid path or pattern.");
}

int main() {
    try {
        {
            auto [z, x, y] = ParsePath("10/534/772/tile.png", "{z}/{x}/{y}/tile.png");
            std::cout << "Z: " << z << ", X: " << x << ", Y: " << y << std::endl;
        }
        {
            auto [z1, x1, y1] = ParsePath("adsad/10/534/772.png", "{z}/{x}/{y}.png");
            std::cout << "Z: " << z1 << ", X: " << x1 << ", Y: " << y1 << std::endl;
        }
        {
            auto [z2, x2, y2] = ParsePath(R"(C:\Users\17632\Desktop\xyz2mbtiles\xyz2mbtiles-master\china1-6\6\53\21\tile.png)", R"({z}\\{x}\\{y}\\tile.png)");
            std::cout << "Z: " << z2 << ", X: " << x2 << ", Y: " << y2 << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
