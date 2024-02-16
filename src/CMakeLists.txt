CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
PROJECT(shared_bike)

#将指定目录添加到编译器的头文件搜索路径下
INCLUDE_DIRECTORIES(../third/include)
INCLUDE_DIRECTORIES(./HeaderFiles)
INCLUDE_DIRECTORIES(./LogModule)
INCLUDE_DIRECTORIES(./ThreadPool)

#将指定目录添加到需要链接的库文件目录下
LINK_DIRECTORIES(../third/lib/iniparser)
LINK_DIRECTORIES(../third/lib/log4cpp)
LINK_DIRECTORIES(./SourceFiles)
LINK_DIRECTORIES(./LogModule)
LINK_DIRECTORIES(./ThreadPool)

#收集所有在指定路径下的源文件的文件名，将输出结果列表储存在指定的变量中
#内置变量： CMAKE_SOURCE_DIR 定义了顶级 CMakeLists.txt 所在的文件夹
#         PROJECT_SOURCE_DIR 定义了包涵最近的 PROJECT() 命令的 CMakeLists.txt 所在的文件夹
aux_source_directory(${PROJECT_SOURCE_DIR} SOURCE_FILES)

#使用给定的源文件，为工程引入一个可执行文件
ADD_EXECUTABLE(shared_bike ${SOURCE_FILES})

#用来显式的定义变量
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -pipe -std=c++11 -lrt -Wno-reorder -Wdeprecated-declarations -fpermissive -llog4cpp -lpthread -levent -lprotobuf -lmysqlclient")

#该指令的作用是将目标文件与库文件进行链接
TARGET_LINK_LIBRARIES(shared_bike iniparser)
TARGET_LINK_LIBRARIES(shared_bike log4cpp)
TARGET_LINK_LIBRARIES(shared_bike SourceFiles)  #依赖其他库的库一定要放到被依赖库的前面 SourceFiles 依赖 LogModule 和 ThreadPool
TARGET_LINK_LIBRARIES(shared_bike LogModule)
TARGET_LINK_LIBRARIES(shared_bike ThreadPool)
TARGET_LINK_LIBRARIES(shared_bike pthread)
TARGET_LINK_LIBRARIES(shared_bike event)
TARGET_LINK_LIBRARIES(shared_bike protobuf)
TARGET_LINK_LIBRARIES(shared_bike mysqlclient)

#增加子目录
ADD_SUBDIRECTORY(SourceFiles)
ADD_SUBDIRECTORY(LogModule)
ADD_SUBDIRECTORY(ThreadPool)

SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
INSTALL(TARGETS shared_bike DESTINATION bin)
