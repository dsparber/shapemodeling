cmake_minimum_required(VERSION 3.10)
project(assignment6) # executable for whole pipeline

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# libigl
option(LIBIGL_WITH_OPENGL            "Use OpenGL"   ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"     ON)
option(LIBIGL_WITH_OPENGL_GLFW_IMGUI "Use ImGui"    ON)

find_package(LIBIGL REQUIRED QUIET)

# Add your project files except for the main function file
FILE(GLOB SRCFILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h        
)

list(REMOVE_ITEM SRCFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

message(***************${SRCFILES}*********************)


# Set LIBRARY_LIST variable
SET(LIBRARY_LIST igl::core igl::opengl_glfw igl::opengl_glfw_imgui)

add_executable(assignment6 ${SRCFILES} src/main.cpp)
target_link_libraries(assignment6 ${LIBRARY_LIST})

# For each step in the pipeline, add executables for testing

# -----------------EXAMPLE---------------------#
project(pca)
add_executable(pca ${SRCFILES} test/pca_main.cpp)
target_link_libraries(pca ${LIBRARY_LIST})

project(landmark)
add_executable(landmark ${SRCFILES} test/landmark_main.cpp)
target_link_libraries(landmark ${LIBRARY_LIST})

project(warping)
add_executable(warping ${SRCFILES} test/warping_main.cpp)
target_link_libraries(warping ${LIBRARY_LIST})

project(generate)
add_executable(generate ${SRCFILES} test/generate_faces.cpp)
target_link_libraries(generate ${LIBRARY_LIST})

project(warp_meshes)
add_executable(warp_meshes ${SRCFILES} test/generate_warped_meshes.cpp)
target_link_libraries(warp_meshes ${LIBRARY_LIST})

project(train)
add_executable(train ${SRCFILES} test/train_main.cpp)
target_link_libraries(train ${LIBRARY_LIST})
#----------------------------------------------#
