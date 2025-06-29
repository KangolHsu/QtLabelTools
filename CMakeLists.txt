# CMake最低版本要求
cmake_minimum_required(VERSION 3.16)

# 项目名称
project(QtLabeler)

# --- Qt & C++ Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用Qt的MOC, UIC, RCC自动处理功能，这是现代Qt CMake项目的标准做法
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 确保在查找包时，CMake能找到我们的项目
# (这一行通常不是必需的，但有助于在某些环境中找到Qt)
set(CMAKE_PREFIX_PATH ${Qt6_DIR})

# 查找Qt6的依赖包，我们只需要Widgets，它会自动引入Core和Gui
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# --- Project Sources ---
# 定义一个变量来包含所有的源文件，方便管理
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    canvasview.cpp
    canvasview.h
    canvasscene.cpp
    canvasscene.h
    polygonitem.cpp
    polygonitem.h
    rectangleitem.cpp
    rectangleitem.h
)

# --- Build Target ---
# 定义可执行文件目标
add_executable(QtLabeler ${PROJECT_SOURCES})

# --- Include Directories (关键修复) ---
# 将当前源文件目录和当前二进制文件目录都添加到包含路径中。
# CMAKE_CURRENT_BINARY_DIR 是存放自动生成文件(如 ui_mainwindow.h)的地方。
# 这样做可以确保编译器和Qt的工具(uic)都能找到所有的头文件，解决文件找不到的问题。
target_include_directories(QtLabeler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# --- Link Libraries ---
# 将我们的目标链接到Qt6的Widgets库
target_link_libraries(QtLabeler PRIVATE Qt6::Widgets)
