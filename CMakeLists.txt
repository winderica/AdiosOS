cmake_minimum_required(VERSION 3.15)

project(AdiosOS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(copy 1.1/copy.c)
add_executable(concurrency 1.2/main.cpp 1.2/components/timeWidget.cpp 1.2/components/counterWidget.cpp 1.2/components/sumWidget.cpp)
add_library(copy_syscall OBJECT 2/copy.c)
add_library(copy_test OBJECT 2/test.c)
add_executable(lkm 3/lkm.c)
add_executable(lkm_test 3/test.c)
add_executable(itop 4/main.cpp 4/core/monitor.cpp 4/utils/utils.cpp 4/components/mainWindow.cpp)
add_executable(bfs 5/main.cpp 5/core/disk.cpp 5/core/fs.cpp 5/utils/utils.cpp)

target_link_libraries(concurrency Qt5::Widgets)
target_link_libraries(itop Qt5::Widgets)
