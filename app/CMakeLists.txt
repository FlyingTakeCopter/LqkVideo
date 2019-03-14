cmake_minimum_required(VERSION 3.4.1)

# 设置cpp文件路径
set(CPATH ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_library( native-lib
             SHARED
             ${CPATH}/native-lib.cpp )

# 添加头文件路径（相对于本文件路径）同路径下 include文件夹
include_directories(include)

# 设置ffmpeg库所在路径的变量
set(FF ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI})
# 添加一个标签库
add_library(avcodec SHARED IMPORTED)
add_library(avformat SHARED IMPORTED)
add_library(avutil SHARED IMPORTED)
add_library(swscale SHARED IMPORTED)
add_library(swresample SHARED IMPORTED)

# 设置参数
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FF}/libavcodec.so)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FF}/libavformat.so)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FF}/libavutil.so)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FF}/libswscale.so)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FF}/libswresample.so)

find_library( log-lib log )

target_link_libraries( # Specifies the target library.
                       native-lib
                       # GLESv2 EGL   #video
                       # OpenSLES     #audio
                       android      #android native
                       avcodec avformat avutil swscale swresample   #ffmpeg

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )