cmake_minimum_required(VERSION 3.16)

project(AdiosOS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5Charts)

add_executable(copy 1.1/copy.c)
add_executable(concurrency 1.2/main.cpp 1.2/components/timeWidget.cpp 1.2/components/counterWidget.cpp 1.2/components/sumWidget.cpp)
add_executable(itop 4/main.cpp 4/core/monitor.cpp 4/utils/utils.cpp 4/components/mainWindow.cpp 4/components/performanceTab.cpp 4/components/systemTab.cpp 4/components/processTab.cpp 4/components/aboutTab.cpp 4/components/moduleTab.cpp)
add_executable(bfs 5/main.cpp 5/core/disk.cpp 5/core/fs.cpp 5/utils/utils.cpp)

# The following items won't actually be built by CMake
add_library(copy_syscall OBJECT 2/copy.c)
add_library(copy_test OBJECT test/copy_test.c)
add_library(lkm OBJECT 3/lkm.c)
add_library(lkm_test OBJECT test/lkm_test.c)

target_link_libraries(concurrency Qt5::Widgets)
target_link_libraries(itop Qt5::Widgets Qt5::Charts)
