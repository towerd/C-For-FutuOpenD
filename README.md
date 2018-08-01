# C++ For FutuOpenD

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
	- make
	
## 功能：

1. 基本流程已经运行正常，添加新功能时参考NetCenter里的做法就好：
    - socket连接成功后，要先发InitConnect协议，成功返回后就可以做事情了
	- 拉取数据参照Req_GetGlobalState
	- 接收实时数据推送前要先订阅（参考Req_Subscribe），并注册接收推送（参考Req_RegPush）
	- 接收推送数据参考QuoteHandler::OnRsp_Qot_UpdateTicker
	- 需要每隔一定时间向FutuOpenD发送心跳包，参考StartKeepAliveTimer

2. 当前代码对应FutuOpenD的1.02版本
3. 运行成功界面：

  ![image](https://github.com/towerd/C-For-FutuOpenD/doc/static/run.png)

## 参考：

1. [Futu Open API下载](https://www.futunn.com/download/index)
2. [Futu Open API使用说明](https://futunnopen.github.io/futuquant/setup/FutuOpenDGuide.html)
3. [Python API](https://github.com/FutunnOpen/futuquant)
4. [protobuf协议文档](https://futunnopen.github.io/futuquant/protocol/intro.html)
