cmake_minimum_required(VERSION 3.16)

project(InkVision
    VERSION 0.1
    LANGUAGES CXX
)

# ============================================================
# 基础配置
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ============================================================
# Qt
# ============================================================

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ============================================================
# OpenCV
# ============================================================

# OpenCV 已安装在本机固定路径（项目结构稳定）
set(OpenCV_DIR "D:/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)

# ============================================================
# 源文件（固定列表）
# ============================================================

set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# ============================================================
# 目标定义
# ============================================================

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(InkVision
        MANUAL_FINALIZATION
        ${SOURCES}
        DrawBoard.h DrawBoard.cpp

        DebugWidget.h DebugWidget.cpp DebugWidget.ui
        RealtimeWidget.h RealtimeWidget.cpp RealtimeWidget.ui
        MyMath.h
        MyMath.cpp

    )
else()
    if(ANDROID)
        add_library(InkVision SHARED ${SOURCES})
    else()
        add_executable(InkVision ${SOURCES})
    endif()
endif()

# ============================================================
# 链接与包含
# ============================================================

target_link_libraries(InkVision
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        ${OpenCV_LIBS}
)

target_include_directories(InkVision
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenCV_INCLUDE_DIRS}
)

# ============================================================
# 平台属性
# ============================================================

if(QT_VERSION VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.InkVision
    )
endif()

set_target_properties(InkVision PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING
        ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# ============================================================
# 安装规则
# ============================================================

include(GNUInstallDirs)

install(TARGETS InkVision
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ============================================================
# Qt6 Finalize
# ============================================================

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(InkVision)
endif()
