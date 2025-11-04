cmake_minimum_required(VERSION 3.16)
project(echo_server_code VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}
)

# 公共源文件（被服务器和客户端共享）
set(COMMON_SOURCES
    src/common/packet_header.cpp
    src/common/data.cpp
    src/common/packet.cpp
)

# 网络层源文件
set(NET_SOURCES
    src/net/epoller.cpp
    src/net/tcp_epoller.cpp
    src/net/auto_flag_tcp_epoller.cpp
)

# 核心源文件
set(CORE_SOURCES
    src/core/center.cpp
    src/core/epoll_center.cpp
)

# 服务器源文件
set(SERVER_SOURCES
        src/server/server_main.cpp
        src/server/echo_server_epoller.cpp
        src/server/echo_server_center.cpp
)

# 客户端源文件
set(CLIENT_SOURCES
        src/client/client_main.cpp
)

# 平台特定的库
if(WIN32)
    set(PLATFORM_LIBS ws2_32)
else()
    set(PLATFORM_LIBS pthread)
endif()

# 创建服务器可执行文件
add_executable(echo_server
    ${COMMON_SOURCES}
    ${NET_SOURCES}
    ${CORE_SOURCES}
    ${SERVER_SOURCES}
)

target_link_libraries(echo_server ${PLATFORM_LIBS})

# 设置服务器输出目录
set_target_properties(echo_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 创建客户端可执行文件
add_executable(echo_client
    ${COMMON_SOURCES}
    ${NET_SOURCES}
    ${CORE_SOURCES}
    ${CLIENT_SOURCES}
)

target_link_libraries(echo_client ${PLATFORM_LIBS})

# 设置客户端输出目录
set_target_properties(echo_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 如果保留原有的 main.cpp，可以创建一个简单的可执行文件
# add_executable(echo_server_code main.cpp)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 安装规则（可选）
install(TARGETS echo_server echo_client
    RUNTIME DESTINATION bin
)
