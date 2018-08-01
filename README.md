# FutuOpenDClient

C++对接富途证券Open API，使用C++进行量化交易。

## 编译说明：

1. 依赖：
    - 支持C++ 11的编译器
    - libuv 1.22
    - google protobuf 3.5.1

2. Windows：VS2013可以直接打开sln
3. Linux：可以用cmake:
    - mkdir build && cd build
    - cmake ..

## 参考：

1. [Futu Open API下载](https://www.futunn.com/download/index)
2. [Futu Open API使用说明](https://futunnopen.github.io/futuquant/setup/FutuOpenDGuide.html)
3. [Python API](https://github.com/FutunnOpen/futuquant)
4. [protobuf协议文档](https://futunnopen.github.io/futuquant/protocol/intro.html)
