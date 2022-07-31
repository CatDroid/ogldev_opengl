cmake_minimum_required(VERSION 2.8)

project(Common)

# add_library 去掉 SHARED 都编译成静态的
add_library(math_3d OBJECT  math_3d.cpp)
add_library(pipeline OBJECT  pipeline.cpp)


add_library(STB_IMAGE OBJECT  "3rdparty/stb_image.cpp")

add_library(backend OBJECT  glut_backend.cpp ) # 暂时使用glut窗口 不用glfw ogldev_glfw_backend.cpp ogldev_backend.cpp
add_library(app OBJECT   ogldev_app.cpp) 
add_library(io_buffer OBJECT   io_buffer.cpp)
add_library(util OBJECT   ogldev_util.cpp)
add_library(basic_lighting OBJECT  ogldev_basic_lighting.cpp)
add_library(shadow_map_fbo OBJECT   ogldev_shadow_map_fbo.cpp)
#add_library(atb OBJECT  ogldev_atb.cpp) # 暂时不使用 atb

add_library(texture OBJECT   ogldev_texture.cpp random_texture.cpp cubemap_texture.cpp)
target_link_libraries(texture ${ImageMagick_LIBRARIES} STB_IMAGE)

add_library(camera  OBJECT  camera.cpp)
#target_link_libraries(camera atb)

add_library(basic_mesh OBJECT  ogldev_basic_mesh.cpp)
#target_link_libraries(basic_mesh texture)

add_library(skinned_mesh OBJECT  ogldev_skinned_mesh.cpp)
#target_link_libraries(skinned_mesh texture)

add_library(technique OBJECT  technique.cpp)
#target_link_libraries(technique util ${GLEW})