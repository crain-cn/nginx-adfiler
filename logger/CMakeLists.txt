cmake_minimum_required(VERSION 2.8)

set(CXX_SOURCES)
auto_sources(files "*.cc" "RECURSE")
list(APPEND CXX_SOURCES ${files})

#project name
project(ngx_mapping)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#deps

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread -fpermissive -W -Wall -Wpointer-arith -Wno-unused ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASAN_ENABLED")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

message("--  CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})
#add dynamic library
set(target logger)
add_executable(${target} "${CXX_SOURCES}")

target_link_libraries(${target}  
  boost_thread
  boost_system 
  rt
  zmq
  #asan
)

install(TARGETS ${target} DESTINATION "/data/modules/ngx_mapping/sbin")
