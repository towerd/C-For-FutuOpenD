# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (FutuOpenDClient)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fsigned-char")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(SRC_DIR ${CMAKE_SOURCE_DIR}/FutuOpenDClient)

include_directories("${ROOT_DIR}/3rd/Include")
link_directories("${ROOT_DIR}/3rd/Lib/Linux")

file(GLOB_RECURSE SRC "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.cc")

add_executable(${PROJECT_NAME}  ${SRC})
target_link_libraries(${PROJECT_NAME}  libf3cnn.a libprotobuf.a libprotobuf-lite.a libuv_a.a pthread ${CMAKE_DL_LIBS})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "$ENV{HOME}/futu/products/FutuOpenD2/${TARGET_OS}") 