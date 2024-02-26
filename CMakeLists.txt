cmake_minimum_required(VERSION  3.10)
project (xyz2mbtiles)
set (CMAKE_CXX_STANDARD 17)


include_directories(third_party)

add_executable(xyz2mbtiles src/main.cpp src/sqlite3.c src/CppSQLite3U.cpp
        src/showProgress.cpp)
if (UNIX)
  target_link_libraries(xyz2mbtiles pthread dl)
endif()

#解析路径函数测试
add_executable(testxyz test/01-testxyz.cpp)