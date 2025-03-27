# Jxlog

## 1.install
项目通过vcpkg管理第三方库，请先安装vcpkg。

随后安装第三方库，命令如下：
```shell
vcpkg install gtest fmt zlib zstd openssl cryptopp protobuf
cd Jxlog
#linux mac系统
python3 ./script/build_linux.py
#windows 系统
python3 ./script/build_windows.py
```
## 2.example
示例放在```example```

## 3.项目集成
项目集成jxlog库，在项目CMakeLists.txt中加入：
```cmake
target_link_libraries(XXXXX logger)
```