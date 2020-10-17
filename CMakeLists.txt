cmake_minimum_required(VERSION 3.0.0)

# set project name
project("Final Commit" LANGUAGES CXX)

# define compiler flags
set(CMAKE_CXX_FLAGS                ${CMAKE_CXX_FLAGS} "-Wall -pthread -pedantic -fpermissive -std=c++11 -Wno-int-to-pointer-cast")
set(CMAKE_CXX_FLAGS_DEBUG          "-g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
add_compile_definitions(NDEBUG)

# GL preference
set(OpenGL_GL_PREFERENCE "GLVND")

# additional global includes
include_directories(common)
include_directories(lib/irrklang/include)

# traverse
add_subdirectory(common)
add_subdirectory(applications)
add_subdirectory(lib)
