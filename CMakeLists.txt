cmake_minimum_required(VERSION 3.6)
project(Limetray)

set(CMAKE_C_STANDARD 99)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(PROJECT_SRC
    Limetray/src/limetray.c
)
set_source_files_properties(${CMAKE_SOURCE_DIR}/Limetray/Resource.rc LANGUAGE RC)
list(APPEND PROJECT_SRC ${CMAKE_SOURCE_DIR}/Limetray/Resource.rc)

if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
        set(RC_CFLAGS "-DMINGW -Ocoff")
endif(MINGW)

add_executable(Limetray
    ${PROJECT_SRC}
)

target_link_libraries(${PROJECT_NAME} -lgdi32 -luser32 -lshell32 -lcomctl32)