cmake_minimum_required(VERSION 2.8)

project(Common)

# add_library 去掉 SHARED 都编译成静态的
add_library(math_3d STATIC math_3d.cpp)
add_library(pipeline STATIC pipeline.cpp)


add_library(backend STATIC ogldev_backend.cpp glut_backend.cpp ogldev_glfw_backend.cpp)
add_library(app STATIC ogldev_app.cpp)
add_library(io_buffer STATIC  io_buffer.cpp)
add_library(util  STATIC ogldev_util.cpp)
add_library(basic_lighting STATIC ogldev_basic_lighting.cpp)
add_library(shadow_map_fbo STATIC  ogldev_shadow_map_fbo.cpp)
add_library(atb STATIC ogldev_atb.cpp)

add_library(texture STATIC  ogldev_texture.cpp random_texture.cpp cubemap_texture.cpp)
target_link_libraries(texture atb ${ImageMagick_LIBRARIES})

add_library(camera STATIC  camera.cpp)
target_link_libraries(camera atb)

add_library(basic_mesh STATIC ogldev_basic_mesh.cpp)
target_link_libraries(basic_mesh texture)

add_library(skinned_mesh STATIC ogldev_skinned_mesh.cpp)
target_link_libraries(skinned_mesh texture)

add_library(technique STATIC technique.cpp)
target_link_libraries(technique util ${GLEW})