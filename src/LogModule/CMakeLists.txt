CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#收集所有在指定路径下的源文件的文件名，将输出结果列表储存在指定的变量中
aux_source_directory(. SOURCE_LOGMODULE_FILES)     # SOURCE_LOGMODULE_FILES 是自定义的 是变量

#add_library(<name> ［STATIC | SHARED | MODULE］［EXCLUDE_FROM_ALL］ source1 ［source2 ...])
#构建库供他人模块使用
ADD_LIBRARY(LogModule ${SOURCE_LOGMODULE_FILES})

#用来显式的定义变量
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations -llog4cpp")

#将指定目录添加到编译器的头文件搜索路径下
INCLUDE_DIRECTORIES(../../third/include)

#将指定目录添加到需要链接的库文件目录下
LINK_DIRECTORIES(../../third/lib/iniparser)
LINK_DIRECTORIES(../../third/lib/log4cpp)

#该指令的作用是将目标文件与库文件进行链接
TARGET_LINK_LIBRARIES(LogModule iniparser)
TARGET_LINK_LIBRARIES(LogModule log4cpp)
#TARGET_LINK_LIBRARIES(common pthread)

TARGET_LINK_LIBRARIES(LogModule dl)