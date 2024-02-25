cmake_minimum_required(VERSION  3.10)
project (xyz2mbtiles)
set (CMAKE_CXX_STANDARD 17)
add_executable(xyz2mbtiles main.cpp sqlite3.c CppSQLite3U.cpp
        showProgress.cpp)
if (UNIX)
  target_link_libraries(xyz2mbtiles pthread dl)
endif()

#解析路径函数测试
add_executable(testxyz 01-testxyz.cpp)