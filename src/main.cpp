#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <tuple>
#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include "CppSQLite3U.h"
#include "showProgress.h"


using namespace std;
namespace fs = std::filesystem;

// 定义一个文件集合，包含坐标信息和文件名
typedef vector<std::string> Files;
vector<string> supportFiletype{".tif", ".png", ".jpg", ".pbf", ".webp", ".zlib", ".gzip"};
string filetype;//输入的文件类型



//文件路径是否支持
bool isSupportFileType(const filesystem::path &filePath) {
    // 获取文件的扩展名（以小写形式，以便进行不区分大小写的比较）
    std::string extension = filePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    // 检查文件扩展名是否在上面定义的图片扩展名列表中
    return std::find(supportFiletype.begin(), supportFiletype.end(), extension) != supportFiletype.end();
}


//解析路径
//返回 z x y
std::tuple<int, int, int>
ParsePath(const std::string &path, const std::string &pattern = R"({z}\\{x}\\{y}\\tile.jpg)") {
    // 转换模式到正则表达式
    std::string regexPattern = std::regex_replace(pattern, std::regex("\\{z\\}"), "(\\d+)");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\{x\\}"), "(\\d+)");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\{y\\}"), "(\\d+)");

    regexPattern = "\\.*" + regexPattern;
//    cout<<regexPattern<<endl;
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
            // 从文件路径中解析X, Y, Z坐标
//            printf("z:%d x:%d y:%d p:%s\n",z,x,y,path.c_str());
            return std::make_tuple(z, x, y);
        }
    }
    // 如果没有找到匹配或匹配格式错误
    throw std::runtime_error("Invalid path or pattern.");
}


// 处理给定路径下的文件,添加到任务列表files
void Process(const fs::path &path, Files &files) {
    for (const auto &entry: fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            // 如果是目录，则递归处理
            Process(entry.path(), files);
        } else {
            const auto &p = entry.path();
            //p是文件的完整路径
            isSupportFileType(p);
            if (isSupportFileType(p)) {
                files.push_back(p.string());
                if (files.size() % 10000 == 0) {
                    cout << "counting number:" << files.size() << endl;
                }
                //获得文件类型
                if (filetype.empty()) {
                    filetype = p.extension().string();
                    // 移除扩展名中的点
                    if (filetype.front() == '.') {
                        filetype.erase(0, 1); // 从索引0开始删除1个字符
                    }
                }
            }
        }
    }
}


// 将瓦片数据插入数据库
void InsertTilesToDB(CppSQLite3DB &db, const Files &files) {
    CppSQLite3Statement stmt = db.compileStatement("INSERT INTO tiles VALUES(?,?,?,?)");
    db.execDML("BEGIN TRANSACTION");

    int nCount = 0;
    for (const auto &file: files) {
        //解析路径
        int x = -1, y = -1, z = -1;
        try {
            auto [z_t, x_t, y_t] = ParsePath(file);//这里可以指定匹配类型
            x = x_t;
            y = y_t;
            z = z_t;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        ////在XYZ系统中，Y坐标是从地图的顶部向下计数的，而在TMS中，Y坐标是从底部向上计数的。
        //所以要转换z坐标
        int maxY = static_cast<int>(std::pow(2, z)) - 1;
        //                替换{-y}（翻转Y坐标）
        y = maxY - y;

        std::ifstream input(file, std::ios::binary);
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

        if (!buffer.empty()) {
            stmt.reset();
            stmt.bind(1, z); // 绑定Z坐标
            stmt.bind(2, x); // 绑定X坐标
            stmt.bind(3, y); // 绑定Y坐标
            stmt.bind(4, buffer.data(), (int) buffer.size()); // 绑定瓦片数据
            stmt.execDML();

            if (++nCount % 10000 == 0) {
                // 每插入10000条数据，提交一次事务
                db.execDML("END TRANSACTION");
                db.execDML("BEGIN TRANSACTION");
            }

            if (nCount % 100 == 0) {
                show_progress_bar(nCount, files.size(), " processing...");
            }

        }
    }

    //结束事务
    db.execDML("END TRANSACTION");
}


// 将可执行程序放到xyz目录可以完成转换
// 支持的格式如下
// C:\Users\17632\Desktop\xyz2mbtiles\xyz2mbtiles-master\china1-6\6\56\22\tile.png   R"({z}\\{x}\\{y}\\tile.png)"
// 10/534/772/tile.png    "{z}/{x}/{y}/tile.png"
// adsad/10/534/772.png   "{z}/{x}/{y}.png"
int main() {
    try {
        fs::path currentPath = fs::current_path(); // 获取当前路径
        Files files;
        files.reserve(100000);
        cout << "counting files" << endl;
        Process(currentPath, files); // 处理当前路径下的文件
        if (files.empty()) {
            cout << "no file to Process" << endl;
            return 0;
        }
        cout << "file numbers:" << files.size() << " filetype:" << filetype << endl;

        // 指定数据库文件名
        fs::path dbPath = currentPath / (currentPath.filename().string() + ".mbtiles");
        if (fs::exists(dbPath)) {
            cout << "tiles.mbtiles exist" << endl;
            fs::remove(dbPath); // 如果数据库文件已存在，则删除
        }

        CppSQLite3DB db;
        db.open(dbPath.string().c_str()); // 打开数据库
        // 创建metadata和tiles表
        db.execDML("CREATE TABLE metadata (name TEXT, value TEXT)");
        //插入元数据
        /*profile	{"profile":"spherical-mercator"}
            format	jpg
            bounds	-180,-85.0511,180,85.0511
         * */
        db.execDML("INSERT INTO metadata (name, value) VALUES ('version', '1.2')");
        std::string sql = "INSERT INTO metadata (name, value) VALUES ('format', '" + filetype + "')";
        db.execDML(sql.c_str());
        db.execDML("INSERT INTO metadata (name, value) VALUES ('bounds', '-180,-85.0511,180,85.0511')");
//        db.execDML(R"(INSERT INTO metadata (name, value) VALUES ('profile', '{"profile":"spherical-mercator"})");

        db.execDML(
                "CREATE TABLE tiles (zoom_level INTEGER NOT NULL, tile_column INTEGER NOT NULL, tile_row INTEGER NOT NULL, tile_data BLOB NOT NULL, UNIQUE (zoom_level, tile_column, tile_row))");

        InsertTilesToDB(db, files); // 将瓦片数据插入数据库
        std::cout.flush();
        std::cout << "Completed inserting tiles.\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
