cmake_minimum_required(VERSION 3.16)
project(Gecko LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_VERBOSE_MAKEFILE ON)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# FMT
add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

# OpenGL
find_package(OpenGL REQUIRED)

# Project files
set(HEADER_FILES
        include/glutils/utils.hpp
        include/glutils/shader.hpp
        include/glutils/program.hpp
        include/camera/orbit_camera.hpp
        include/scalar_field/scalar_field.hpp)

set(SOURCE_FILES
        source/main.cpp
        source/glutils/utils.cpp
        source/glutils/shader.cpp
        source/glutils/program.cpp
        source/camera/orbit_camera.cpp)

set(GLAD_SOURCE
        source/glad/glad.c)

set(IMGUI_SOURCE
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/examples/imgui_impl_glfw.cpp
        external/imgui/examples/imgui_impl_opengl3.cpp)

set(EXECUTABLE_NAME Gecko)

add_executable(${EXECUTABLE_NAME}
        ${HEADER_FILES}
        ${GLAD_SOURCE}
        ${SOURCE_FILES}
        ${IMGUI_SOURCE})

target_include_directories(${EXECUTABLE_NAME} PRIVATE include)
target_include_directories(${EXECUTABLE_NAME}
        SYSTEM PRIVATE external/spdlog/include
        SYSTEM PRIVATE external/glm
        SYSTEM PRIVATE external/fmt/include
        SYSTEM PRIVATE external/imgui
        SYSTEM PRIVATE external/imgui/examples)

# Set warnings
set(MSVC_WARNINGS
        /W4 # Baseline reasonable warnings
        /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss
        # of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to
        # 'type2:field_bits', possible loss of data
        /w14263 # 'function': member function does not override any base class
        # virtual member function
        /w14265 # 'classname': class has virtual functions, but destructor is not
        # virtual instances of this class may not be destructed correctly
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable
        # declared in the for-loop is used outside the for-loop scope
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing
        # an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect; expected
        # operator with side-effect
        /w14549 # 'operator': operator before comma has no effect; did you intend
        # 'operator'?
        /w14555 # expression has no effect; expected expression with side- effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may
        # cause unexpected runtime behavior.
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined
        # conversion has been implicitly applied
        )

set(CLANG_WARNINGS
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
        )

set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks
        # do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were
        # probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
        )

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message("Building with MSVC warnings")
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Building with Clang warnings")
    # We disable some very pedantic warnings about C++98 compatibility
    set(PROJECT_WARNINGS ${CLANG_WARNINGS} -Wno-c++98-compat -Wno-c++98-compat-pedantic)
else ()
    message("Building with GCC warnings")
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
endif ()

target_compile_options(${EXECUTABLE_NAME}
        PRIVATE ${PROJECT_WARNINGS})

# Link OpenGL / GLFW libraries
target_link_libraries(${EXECUTABLE_NAME}
        PRIVATE glfw
        PRIVATE OpenGL::GL
        PRIVATE fmt::fmt-header-only)