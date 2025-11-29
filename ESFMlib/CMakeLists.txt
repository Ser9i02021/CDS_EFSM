cmake_minimum_required(VERSION 3.16)
project(EFSMStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
if (QT_VERSION_MAJOR EQUAL 6)
  find_package(Qt6 REQUIRED COMPONENTS Core Widgets Qml)
else()
  find_package(Qt5 REQUIRED COMPONENTS Core Widgets Qml)
endif()

add_executable(EFSMStudio
    main.cpp
    MainWindow.h MainWindow.cpp
    StateItem.h StateItem.cpp
    VarModel.h VarModel.cpp
    InputModel.h InputModel.cpp
    OutputModel.h OutputModel.cpp
    TransitionEditorDialog.h TransitionEditorDialog.cpp
    TransitionItem.h TransitionItem.cpp
    DiagramScene.h DiagramScene.cpp
)

target_link_libraries(EFSMStudio PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml        # <- novo
)
