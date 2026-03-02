cmake_minimum_required(VERSION 3.16)

# 项目基础配置
project(BilibiliDownloader 
        VERSION 1.0.0 
        LANGUAGES CXX)

# 核心开关（由CMakePresets传入，无需重复定义）
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找Qt5（适配VS2026的路径解析）
find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)

# 源文件列表
set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    mainwindow.h
)

# 生成可执行文件（GUI程序）
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
)

# Windows+VS2026专属配置
if (WIN32)
    # 强制GUI程序，无控制台弹窗
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        # 输出目录：适配Presets的build/debug/release
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    )
    
    # 自动复制BBDown.exe到输出目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/BBDown.exe"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying BBDown.exe to output directory..."
    )

    # Release模式自动部署Qt依赖（VS2026兼容windeployqt）
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${Qt5_DIR}/../../../bin/windeployqt.exe
            --release
            --no-compiler-runtime
            --no-angle
            --no-opengl-sw
            "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Deploying Qt dependencies for Release (VS2026)..."
        )
    endif()
endif()

# 安装规则（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)