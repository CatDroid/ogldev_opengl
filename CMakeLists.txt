cmake_minimum_required(VERSION 2.8)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory (build is a good one)!" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

project(ogldev)

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

#参考LearnOpenGL 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# 查找是否成功可以通过<PackageName>_FOUND变量的值得到
# find_package指令有两种查找包的模式：一种是模块(Module)模式，一种是配置(Config)模式
# 默认情况下，首先使用模块(Module)模式，如果没有找到对应的模块(Module)，就会使用配置(Config)模式。
# 模块(Module)模式下，CMake会搜索一个名为Find<PackageName>.cmake。首先使用CMAKE_MODULE_PATH中的路径搜索，然后搜索CMake自带的Find模块
#
# CMake模块内置了?? ImageMagick的find方式 
# https://github.com/Kitware/CMake/blob/master/Modules/FindImageMagick.cmake
#
# Windows只要安装了 ImageMigaick就会找到对应的头文件和库 ImageMagick_INCLUDE_DIRS  ImageMagick_LIBRARIES
# https://imagemagick.org/script/download.php#windows ImageMagick-7.1.0-44-Q16-HDRI-x64-dll.exe
#
find_package(ImageMagick COMPONENTS Magick++)
message(STATUS "Found ImageMagick in ${ImageMagick_INCLUDE_DIRS}")

find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

#find_package(GLFW3 REQUIRED)
#message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# 必须在 Common 目录前 先设置 Include
include_directories(Include ${ImageMagick_INCLUDE_DIRS}  ${ASSIMP_INCLUDE_DIR})

# 在Common目录前 先设置上面的变量  Common是公共源文件(分别编译成库)
add_subdirectory(Common)


if(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
  
  # 有静态库 (LIB) 和动态库 (DLL) - 
  # 但请注意，
  # .LIB 文件可以是 静态库（ static libraries  包含目标文件）
  #              或 导入库（ import libraries  包含允许链接器链接到 DLL 的符号）
  
  # C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\lib.exe /list xxx.lib 
  # 如果打印.dll 就是导入库; 打印xxx.o 就是静态库
  
  # 
  
  # Windows直接放到Windows/Lib目录下 可以换成 freeglutd.lib (debug) 但是freelut*.lib两个都是导入库 需要设置dll路径
  #
  # Windows/Lib目录: freeglut.lib(导入库)  glfw3.lib(静态库) glfw3dll.lib(导入库) glew32.lib(导入库) assimp-vc142-mt.lib(导入库)
  #
  # Lib目录: assimp.lib(导入库)  glfx.lib(导入库)  CORE_RL_Magick++_.lib(导入库)  CORE_DB_Magick++_.lib(导入库)
  #          glfx_debug.lib(静态库) glfx_release.lib(静态库)
  #
  link_directories(${CMAKE_SOURCE_DIR}/Windows/Lib) 
  #link_directories(${CMAKE_SOURCE_DIR}/Lib) # assimp.lib  32bits的 不用这个目录下的
  set(LIBS opengl32 ${ASSIMP_LIBRARY} freeglut glew32)  
  
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  find_package(OpenGL REQUIRED)
  add_definitions(${OPENGL_DEFINITIONS})
  find_package(X11 REQUIRED)
  # note that the order is important for setting the libs
  # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
  set(LIBS ${GLFW3_LIBRARY} X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread freetype ${ASSIMP_LIBRARY})
  set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
  INCLUDE_DIRECTORIES(/System/Library/Frameworks)
  FIND_LIBRARY(COCOA_LIBRARY Cocoa)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
  FIND_LIBRARY(IOKit_LIBRARY IOKit)
  FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
  MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
  SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
  SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES})
  set(LIBS ${LIBS} ${APPLE_LIBS})
else()
  set(LIBS )
endif(WIN32)

 
set(CHAPTERS
    tutorial01
    tutorial02
    tutorial03
    tutorial04
    tutorial05
    tutorial06
    tutorial07
	tutorial08
	tutorial09
	tutorial10
	tutorial35
	tutorial36
	tutorial37
)


# Common/CMakeLists.txt 
set(LIBS ${LIBS} math_3d)
set(LIBS ${LIBS} pipeline)
set(LIBS ${LIBS} app)
set(LIBS ${LIBS} camera)
#set(LIBS ${LIBS} atb)
set(LIBS ${LIBS} util)
set(LIBS ${LIBS} technique)
set(LIBS ${LIBS} backend)
set(LIBS ${LIBS} basic_mesh)
set(LIBS ${LIBS} texture)
set(LIBS ${LIBS} STB_IMAGE)


 
configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

macro(makeLink src dest target)
  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dest}  DEPENDS  ${dest} COMMENT "mklink ${src} -> ${dest}")
endmacro()

function(create_project_from_sources chapter)
	file(GLOB SOURCE
            "${chapter}/*.h"
            "${chapter}/*.cpp"
            "${chapter}/shaders/*.vs"
            "${chapter}/shaders/*.fs"
            "${chapter}/shaders/*.gs"
            "${chapter}/shaders/*.cs"
    )
	set(NAME "${chapter}")
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} ${LIBS})
    if(MSVC)
		target_compile_options(${NAME} PRIVATE /std:c++17 /MP)
        target_link_options(${NAME} PUBLIC /ignore:4099)
    endif(MSVC)
    if(WIN32)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${chapter}")
        set_target_properties(${NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${chapter}/Debug")
    elseif(UNIX AND NOT APPLE)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${chapter}")
    elseif(APPLE)
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${chapter}")
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/${chapter}")
        set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/${chapter}")
    endif(WIN32)
    # copy shader files to build directory
    file(GLOB SHADERS
             "${chapter}/shaders/*.vs"
             "${chapter}/shaders/*.fs"
             "${chapter}/shaders/*.gs"
			 "${chapter}/shaders/*.cs"
    )
	# copy dlls 不拷贝dll 直接设置调试环境PATH, 在visualstudio.vcxproj.user.in中
	#file(GLOB DLLS "dlls/*.dll")
    foreach(SHADER ${SHADERS})
        if(WIN32)
            # configure_file(${SHADER} "test")
			add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${NAME}>/shaders/ )
            add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} $<TARGET_FILE_DIR:${NAME}>/shaders/)
			
			#get_filename_component(BASE_NAME   ${SHADER} NAME_WE)  # shader文件名字
			#get_filename_component(EXTERN_NAME ${SHADER} LAST_EXT)  # shader扩展名字
			# STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${SHADER}) 
			
			#add_custom_command(TARGET ${NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLLS} $<TARGET_FILE_DIR:${NAME}>)
        
		elseif(UNIX AND NOT APPLE)
            file(COPY ${SHADER} DESTINATION ${CMAKE_SOURCE_DIR}/bin/${chapter})
        elseif(APPLE)
            # create symbolic link for *.vs *.fs *.gs
            get_filename_component(SHADERNAME ${SHADER} NAME)
            makeLink(${SHADER} ${CMAKE_SOURCE_DIR}/bin/${chapter}/${SHADERNAME} ${NAME})
        endif(WIN32)
    endforeach(SHADER)
	
	if(WIN32)
		if ( SHADERS ) 
			# 由于源码不修改, 而修改shader的话 不会触发 post command的执行 所以增加 copy target
			# 直接拷贝目录
			add_custom_target(${NAME}_copy  ALL 
					COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${chapter}/shaders $<TARGET_FILE_DIR:${NAME}>/shaders/)
	
		endif() 
	endif(WIN32)
	 
	
	# 如果使用visual studio构建, 使用 对于多有工程生效的 配置文件
	# 这里会生成 visual studio 调试的环境变量 LocalDebuggerEnvironment>PATH=@CMAKE_SOURCE_DIR@/dlls;%PATH%
	# 根据visualstudio.vcxproj.user.in 文件生成  tutorial08.vcxproj.user !!! 
	# 修改了 PATH=@CMAKE_SOURCE_DIR@/dlls/ 改为 PATH=@CMAKE_SOURCE_DIR@/Windows/DLL 
    # if compiling for visual studio, also use configure file for each project (specifically to set up working directory)
    if(MSVC)
        configure_file(${CMAKE_SOURCE_DIR}/configuration/visualstudio.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.vcxproj.user @ONLY)
    endif(MSVC)
endfunction()
 
 
foreach(CHAPTER ${CHAPTERS})
	create_project_from_sources(${CHAPTER})
endforeach(CHAPTER)

include_directories(${CMAKE_SOURCE_DIR}/Include)