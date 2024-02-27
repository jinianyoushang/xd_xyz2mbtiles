# xd_xyz2mbtiles

#### 介绍
使用c++17写的，将xyz格式的栅格瓦片转换为单个.mbtiles文件，便于数据迁移的程序。

#### TODO

1. 虽然能完成功能，里面全局变量太多了，要重构代码

#### 已经完成功能

1. 支持配置文件，支持所有文件格式
2. 目前支jpg格式文件，后续将加入支持其他格式

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

##### 使用说明
1.  修改配置文件 config.ini 中输入目录 inputDir输出目录 outputDir  匹配模式pattern，并放到和可执行文件同级目录，运行即可。



#### 参考项目

[xyz2mbtiles: 将xyz格式的栅格瓦片转换为单个.mbtiles文件，便于数据迁移。 (gitee.com)](https://gitee.com/zjp369/xyz2mbtiles)
