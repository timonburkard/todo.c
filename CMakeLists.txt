cmake_minimum_required(VERSION 3.16)

project(todo LANGUAGES C)

add_executable(todo
    src/cli.c
    src/todo.c
    third_party/sqlite/sqlite3.c
)

target_include_directories(todo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite
)

set_target_properties(todo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
