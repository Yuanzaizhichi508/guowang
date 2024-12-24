cmake_minimum_required(VERSION 3.16)

project(finalControl VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_MFC_FLAG 2)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
# 添加预处理器定义
add_definitions(-D_AFXDLL)


# 设置运行时库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
file(GLOB SOURCES_FILES ${SRC_DIR}/*.cpp)
# add_definitions(-DDEBUG)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${SOURCES_FILES}
)
set(DLL_DIR ${CMAKE_SOURCE_DIR})


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(finalControl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        Fps.h Fps.cpp
        ControlThread.h ControlThread.cpp
        AcquisitionThread.h AcquisitionThread.cpp
        GlobalVariable.h GlobalVariable.cpp
        Common.h Common.cpp
        StdAfx.h
        ClickableLabel.h ClickableLabel.cpp
        ImageViewer.h ImageViewer.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET finalControl APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(finalControl SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(finalControl
            ${PROJECT_SOURCES}
        )
    endif()
endif()


include_directories(${INCLUDE_DIR}
    ${INCLUDE_DIR}/opencv2
    ${INCLUDE_DIR}/onnxruntime
    ${INCLUDE_DIR}/onnxDetector
    ${INCLUDE_DIR}/onnxDetector/yolo
    ${INCLUDE_DIR}/onnxDetector/paddle
)

link_directories(${LIB_DIR})

target_link_libraries(finalControl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${LIB_DIR}/opencv_world4100.lib
    ${LIB_DIR}/onnxruntime.lib
    ${LIB_DIR}/onnxruntime_providers_shared.lib
    ${LIB_DIR}/GxIAPI.lib
    ${LIB_DIR}/DxImageProc.lib
    ${LIB_DIR}/ModbusTcpAPI.lib
    ${LIB_DIR}/StandardModbusApi.lib
)
add_custom_command(TARGET finalControl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DLL_DIR} $<TARGET_FILE_DIR:finalControl>)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.finalControl)
endif()
set_target_properties(finalControl PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS finalControl
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(finalControl)
endif()
