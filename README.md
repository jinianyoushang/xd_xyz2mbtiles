# xyz2mbtiles

#### 介绍
将xyz格式的栅格瓦片转换为单个.mbtiles文件，便于数据迁移。

使用c++17写的程序

#### TODO

目前支jpg格式文件，后续将加入支持其他格式

#### 软件架构
该项目依赖sqlite（ https://github.com/sqlite/sqlite.git ）、CppSQLite3U（ http://softvoile.com/development/CppSQLite3U/ ，有些适应性修改）等，目前仅能将.jpg图片存入.mbtiles文件。
             
#### 安装教程
##### 安装教程（Windows）
1.  git clone 本项目
2.  cd xyz2mbtiles
3.  mkdir build
4.  cd build
5.  cmake .. 
6.  make

##### 安装教程（Linux）
1.  git clone 本项目
2.  cd xyz2mbtiles
3.  mkdir build
4.  cd build
5.  apt-get install -y build-essential cmake
6.  cmake .. 
7.  make

#### 使用说明

##### 使用说明（Windows）
1.  复制xyz2mbtiles.exe到png格式的xyz文件所在目录。
2.  双击xyz2mbtiles.exe启动程序，耐心等待程序运行完成。
3.  程序运行过程中会输出一些提示信息，最后会显示总共输入多少个jpg文件。按下任意键退出运行窗口。

##### 使用说明（Linux）
1.  复制xyz2mbtiles到png格式的xyz文件所在目录。
2.  ./xyz2mbtiles启动程序，耐心等待程序运行完成。
3.  程序运行过程中会输出一些提示信息，最后会显示总共输入多少个jpg文件。

#### 参考项目

[xyz2mbtiles: 将xyz格式的栅格瓦片转换为单个.mbtiles文件，便于数据迁移。 (gitee.com)](https://gitee.com/zjp369/xyz2mbtiles)
