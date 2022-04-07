cmake_minimum_required(VERSION 3.13.4)

project(timer CXX)

INCLUDE_DIRECTORIES(src)
add_executable(${PROJECT_NAME} 
                src/main.cpp
                src/timer.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(timer PRIVATE Threads::Threads)