cmake_minimum_required(VERSION 3.16)

# 项目名称和版本，指定C++标准
project(BilibiliDownloader 
        VERSION 1.0.0 
        LANGUAGES CXX)

# 核心配置：适配Qt5.15.2
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 匹配Qt5.15及以上版本（兼容5.15.2）
find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)

# 仅启用MOC（元对象编译），移除UIC（UI文件编译）
set(CMAKE_AUTOMOC ON)
# 移除AUTOUIC：不再处理.ui文件
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 收集项目文件（仅保留源码和头文件，无UI文件）
set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

# 生成可执行文件（无UI文件）
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接Qt5库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
)

# Windows+MSVC专属配置
if (WIN32)
    # 强制使用UTF-8编码
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    # 保留控制台窗口（方便查看BBDown输出），纯GUI可改为ON
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE OFF
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
    
    # 自动复制BBDown.exe到输出目录（需将BBDown.exe放在项目根目录）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/BBDown.exe"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying BBDown.exe to output directory..."
    )
endif()

# 可选安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)