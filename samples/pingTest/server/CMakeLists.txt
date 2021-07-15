cmake_minimum_required(VERSION 3.4.1)

PROJECT (TESTUV)

MESSAGE(STATUS "This is HELLO_SOURCE_DIR " ${TESTUV_SOURCE_DIR})
MESSAGE(STATUS "This is HELLO_BINARY_DIR " ${TESTUV_BINARY_DIR})
# 输出打印资源目录，与HELLO_SOURCE_DIR 一样 
MESSAGE(STATUS "This is PROJECT_SOURCE_DIR " ${PROJECT_SOURCE_DIR})
# 输出打印 CMake 资源目录，与 PROJECT_SOURCE_DIR 一样 
MESSAGE(STATUS "This is CMAKE_SOURCE_DIR " ${CMAKE_SOURCE_DIR})

#SET(SRC_LIST testRight.cpp LinkUtils.cc)
#包含原程序,即把给定目录下的源程序复制给变量DIR_SRC
#aux_source_directory(DIR_SRC ./)

#生成程序
#add_executable(helloworld ${DIR_SRC})

# 把当前工程目录下的 src 目录的下的所有 .cpp 和 .c 文件赋值给 SRC_LIST
# AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src SRC_LIST)
FILE(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 打印 SRC_LIST 文件列表
MESSAGE(STATUS ${SRC_LIST})

# 指定头文件目录
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(/usr/include)
INCLUDE_DIRECTORIES(/usr/local/include)

#打印一下目录情况
get_property(dirs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(">>> include_dirs=${dirs}") 

# 指定输出 .so 动态库的目录位置
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 指定生成动态库
#ADD_LIBRARY(math SHARED ${SRC_LIST})
# 指定生成版本号，VERSION指代动态库版本，SOVERSION指代API版本
# SET_TARGET_PROPERTIES(math PROPERTIES VERSION 1.2 SOVERSION 1)


# 生成可执行文件 hello ，${SRC_LIST}是引用变量，也就是源文件 hello.cpp
ADD_EXECUTABLE(TestServer ${SRC_LIST})


#添加静态库或者动态库
#而假设我们程序用到了在/usr/lib下的一个静态库libmy.a，那就需要添加如下两个命令
#库所在位置
link_directories(/usr/local/lib)
#为hello添加共享库链接
TARGET_LINK_LIBRARIES(TestServer uv pthread)