cmake_minimum_required(VERSION 3.12)
project(ultra VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
    main.cpp
    cpu.cpp
    memory.cpp
    cpu_instructions.cpp
    disassembler.cpp
    tests.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        dependencies/magic_get
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
)
