cmake_minimum_required(VERSION 3.16)
project(PersonalWorkloadSimEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_library(pwse_core
    src/Task.cpp
    src/Agent.cpp
    src/Scheduler.cpp
    src/SimulationEngine.cpp
    src/ConfigLoader.cpp
    src/Json.cpp
)
target_include_directories(pwse_core PUBLIC include)

add_executable(sim src/main.cpp)
target_link_libraries(sim PRIVATE pwse_core)

install(TARGETS sim RUNTIME DESTINATION bin)
