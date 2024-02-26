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

// ����һ���ļ����ϣ�����������Ϣ���ļ���
typedef vector<std::string> Files;
vector<string> supportFiletype{".tif", ".png", ".jpg", ".pbf", ".webp", ".zlib", ".gzip"};
string filetype;//������ļ�����



//�ļ�·���Ƿ�֧��
bool isSupportFileType(const filesystem::path &filePath) {
    // ��ȡ�ļ�����չ������Сд��ʽ���Ա���в����ִ�Сд�ıȽϣ�
    std::string extension = filePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    // ����ļ���չ���Ƿ������涨���ͼƬ��չ���б���
    return std::find(supportFiletype.begin(), supportFiletype.end(), extension) != supportFiletype.end();
}


//����·��
//���� z x y
std::tuple<int, int, int>
ParsePath(const std::string &path, const std::string &pattern = R"({z}\\{x}\\{y}\\tile.jpg)") {
    // ת��ģʽ��������ʽ
    std::string regexPattern = std::regex_replace(pattern, std::regex("\\{z\\}"), "(\\d+)");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\{x\\}"), "(\\d+)");
    regexPattern = std::regex_replace(regexPattern, std::regex("\\{y\\}"), "(\\d+)");

    regexPattern = "\\.*" + regexPattern;
//    cout<<regexPattern<<endl;
    // ����������ʽ
    std::regex regex(regexPattern);
    std::smatch matches;

    // ����ƥ��·��
    if (std::regex_search(path, matches, regex)) {
        // ����Ƿ����㹻��ƥ����
        if (matches.size() >= 4) {
            // ��ȡZ, X, Yֵ
            int z = std::stoi(matches[1]);
            int x = std::stoi(matches[2]);
            int y = std::stoi(matches[3]);
            // ���ļ�·���н���X, Y, Z����
//            printf("z:%d x:%d y:%d p:%s\n",z,x,y,path.c_str());
            return std::make_tuple(z, x, y);
        }
    }
    // ���û���ҵ�ƥ���ƥ���ʽ����
    throw std::runtime_error("Invalid path or pattern.");
}


// �������·���µ��ļ�,��ӵ������б�files
void Process(const fs::path &path, Files &files) {
    for (const auto &entry: fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            // �����Ŀ¼����ݹ鴦��
            Process(entry.path(), files);
        } else {
            const auto &p = entry.path();
            //p���ļ�������·��
            isSupportFileType(p);
            if (isSupportFileType(p)) {
                files.push_back(p.string());
                if (files.size() % 10000 == 0) {
                    cout << "counting number:" << files.size() << endl;
                }
                //����ļ�����
                if (filetype.empty()) {
                    filetype = p.extension().string();
                    // �Ƴ���չ���еĵ�
                    if (filetype.front() == '.') {
                        filetype.erase(0, 1); // ������0��ʼɾ��1���ַ�
                    }
                }
            }
        }
    }
}


// ����Ƭ���ݲ������ݿ�
void InsertTilesToDB(CppSQLite3DB &db, const Files &files) {
    CppSQLite3Statement stmt = db.compileStatement("INSERT INTO tiles VALUES(?,?,?,?)");
    db.execDML("BEGIN TRANSACTION");

    int nCount = 0;
    for (const auto &file: files) {
        //����·��
        int x = -1, y = -1, z = -1;
        try {
            auto [z_t, x_t, y_t] = ParsePath(file);//�������ָ��ƥ������
            x = x_t;
            y = y_t;
            z = z_t;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        ////��XYZϵͳ�У�Y�����Ǵӵ�ͼ�Ķ������¼����ģ�����TMS�У�Y�����Ǵӵײ����ϼ����ġ�
        //����Ҫת��z����
        int maxY = static_cast<int>(std::pow(2, z)) - 1;
        //                �滻{-y}����תY���꣩
        y = maxY - y;

        std::ifstream input(file, std::ios::binary);
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});

        if (!buffer.empty()) {
            stmt.reset();
            stmt.bind(1, z); // ��Z����
            stmt.bind(2, x); // ��X����
            stmt.bind(3, y); // ��Y����
            stmt.bind(4, buffer.data(), (int) buffer.size()); // ����Ƭ����
            stmt.execDML();

            if (++nCount % 10000 == 0) {
                // ÿ����10000�����ݣ��ύһ������
                db.execDML("END TRANSACTION");
                db.execDML("BEGIN TRANSACTION");
            }

            if (nCount % 100 == 0) {
                show_progress_bar(nCount, files.size(), " processing...");
            }

        }
    }

    //��������
    db.execDML("END TRANSACTION");
}


// ����ִ�г���ŵ�xyzĿ¼�������ת��
// ֧�ֵĸ�ʽ����
// C:\Users\17632\Desktop\xyz2mbtiles\xyz2mbtiles-master\china1-6\6\56\22\tile.png   R"({z}\\{x}\\{y}\\tile.png)"
// 10/534/772/tile.png    "{z}/{x}/{y}/tile.png"
// adsad/10/534/772.png   "{z}/{x}/{y}.png"
int main() {
    try {
        fs::path currentPath = fs::current_path(); // ��ȡ��ǰ·��
        Files files;
        files.reserve(100000);
        cout << "counting files" << endl;
        Process(currentPath, files); // ����ǰ·���µ��ļ�
        if (files.empty()) {
            cout << "no file to Process" << endl;
            return 0;
        }
        cout << "file numbers:" << files.size() << " filetype:" << filetype << endl;

        // ָ�����ݿ��ļ���
        fs::path dbPath = currentPath / (currentPath.filename().string() + ".mbtiles");
        if (fs::exists(dbPath)) {
            cout << "tiles.mbtiles exist" << endl;
            fs::remove(dbPath); // ������ݿ��ļ��Ѵ��ڣ���ɾ��
        }

        CppSQLite3DB db;
        db.open(dbPath.string().c_str()); // �����ݿ�
        // ����metadata��tiles��
        db.execDML("CREATE TABLE metadata (name TEXT, value TEXT)");
        //����Ԫ����
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

        InsertTilesToDB(db, files); // ����Ƭ���ݲ������ݿ�
        std::cout.flush();
        std::cout << "Completed inserting tiles.\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
