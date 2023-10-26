cmake_minimum_required(VERSION 3.5)

add_subdirectory(submodule/pybind11)
project(mce VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ONNXRUNTIME_ROOT_DIR /home/u/Desktop/onnxruntime/onnxruntime/)



find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${ONNXRUNTIME_ROOT_DIR}/include)
link_directories(${ONNXRUNTIME_ROOT_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
qt_add_resources(res.qrc)
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        inference/SegmentationSingleton.cpp
        inference/SegmentationSingleton.h
        inference/PhaseDetectionSingleton.cpp
        inference/PhaseDetectionSingleton.h
        widget/SliderDialog.cpp
        widget/SliderDialog.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mce
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        res.qrc
            control/movmanager.h control/movmanager.cpp
            annotation/annotationcontainer.h annotation/annotationitem.h canvas/canvas.h common.h widget/customlistwidget.h mainwindow.h control/movmanager.h
            annotation/annotationcontainer.cpp annotation/annotationitem.cpp canvas/canvas.cpp common.cpp widget/customlistwidget.cpp main.cpp mainwindow.cpp control/movmanager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET mce APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(mce SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(mce
            ${PROJECT_SOURCES}
                control/movmanager.h
                control/movmanager.cpp
            res.qrc
                widget/customlistwidget.h widget/customlistwidget.cpp
                canvas/canvas.h canvas/canvas.cpp
                common.h
            common.cpp
            annotation/annotationmanager.h annotation/annotationmanager.cpp




        )
    endif()
endif()
target_link_libraries(mce PRIVATE Qt${QT_VERSION_MAJOR}::Widgets  ${OpenCV_LIBS}  onnxruntime onnxruntime_providers_cuda pybind11::embed )


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mce)
endif()
set_target_properties(mce PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS mce
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mce)
endif()
# Define the source and destination paths for the model file
set(SEG_MODEL_SOURCE_PATH "${CMAKE_SOURCE_DIR}/res/models/seg.onnx")
set(SEG_MODEL_DEST_PATH "${CMAKE_BINARY_DIR}/models/seg.onnx")

set(PHASE_MODEL_SOURCE_PATH "${CMAKE_SOURCE_DIR}/res/models/phase.onnx")
set(PHASE_MODEL_DEST_PATH "${CMAKE_BINARY_DIR}/models/phase.onnx")
# Copy the model file to the build directory
configure_file(
        ${SEG_MODEL_SOURCE_PATH}
        ${SEG_MODEL_DEST_PATH}
        COPYONLY
)
configure_file(
        ${PHASE_MODEL_SOURCE_PATH}
        ${PHASE_MODEL_DEST_PATH}
        COPYONLY
)
set(PY_SCRIPT_SOURCE_PATH "${CMAKE_SOURCE_DIR}/res/script/findpeak.py")
set(PY_SCRIPT_DEST_PATH "${CMAKE_BINARY_DIR}/script/.py")
configure_file(
       ${PY_SCRIPT_SOURCE_PATH}
       ${PY_SCRIPT_DEST_PATH}
        COPYONLY
)