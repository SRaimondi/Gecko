cmake_minimum_required(VERSION 3.16)
project(Gecko LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(HEADER_FILES)

set(SOURCE_FILES source/main.cpp)

set(EXECUTABLE_NAME Gecko)

add_executable(${EXECUTABLE_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE include)
target_include_directories(${EXECUTABLE_NAME}
        SYSTEM PRIVATE external/spdlog/include)

set(PROJECT_WARNINGS
        -Wall
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a
        # parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
        # non-virtual destructor. This helps catch hard to
        # track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual
        # function
        -Wpedantic # warn if non-standard C++ is used
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output
        # (ie printf)
        -Wmisleading-indentation # warn if indentation implies blocks where blocks
        # do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were
        # probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
        )

target_compile_options(${EXECUTABLE_NAME}
        PRIVATE ${PROJECT_WARNINGS})