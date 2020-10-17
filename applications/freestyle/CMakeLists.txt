# glob for source files
file(GLOB SOURCES "*.cpp")

# executable target
add_executable(freestyle ${SOURCES} jukebox.cpp jukebox.h battle_system/fighter.cpp battle_system/fighter.h battle_system/moves.cpp battle_system/moves.h battle_system/battlehandler.cpp battle_system/battlehandler.h battle_system/effects.cpp battle_system/effects.h player3d.cpp player3d.h world3d.cpp world3d.h rope.cpp rope.h files.cpp files.h checkpoints.cpp checkpoints.h soup.cpp soup.h curves/Bezier.cpp
        curves/Bezier.h
        curves/BSpline.cpp
        curves/BSpline.h
        curves/CatmullRom.cpp
        curves/CatmullRom.h
        curves/Curve.cpp
        curves/Curve.h
        curves/Vector.cpp
        curves/Vector.h)

# output in source
set_target_properties(freestyle PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# define base path for mesh/image lookups
add_definitions(-DEXECUTABLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# handle dependencies
# GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(freestyle ${GLEW_LIBRARIES})
# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(freestyle ${OPENGL_LIBRARIES})
# common libs
target_link_libraries(freestyle imgui cppgl)
# parser lib
target_link_libraries(freestyle expat)
# irrklang
include_directories(${CMAKE_SOURCE_DIR}/lib/irrklang/include/libirrklang)
link_directories(${CMAKE_SOURCE_DIR}/lib/irrklang/)
message(${CMAKE_SOURCE_DIR})
find_library(IrrKlang_LOC NAMES IrrKlang HINTS "${CMAKE_SOURCE_DIR}/lib/irrklang/bin/linux-gcc-64")
message(${IrrKlang_LOC})

add_library(irrklang SHARED IMPORTED)
target_link_libraries(freestyle ${IrrKlang_LOC})

set_target_properties(irrklang PROPERTIES
        IMPORTED LOCATION ${Irrklang_LOC}
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/irrklang/include/libirrklang")

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS})
message(${BULLET_LIBRARIES})
target_link_libraries(freestyle ${BULLET_LIBRARIES})
