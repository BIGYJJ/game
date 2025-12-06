cmake_minimum_required(VERSION 3.16)
project(PixelFarmRPG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置SFML路径（修改为你的实际路径）
set(SFML_DIR "D:/SFML-2.6.1/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# 源文件
set(SOURCES
    src/main.cpp
    src/Core/Game.cpp
    src/States/GameState.cpp
)

# 头文件（帮助IDE识别）
set(HEADERS
    src/Core/Game.h
    src/States/State.h
    src/States/GameState.h
    src/Entity/Player.h
    src/World/TileMap.h
    src/World/Camera.h
    src/Systems/TimeSystem.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接SFML库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows: 复制DLL
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

message(STATUS "========================================")
message(STATUS "  项目: ${PROJECT_NAME}")
message(STATUS "  C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "========================================")