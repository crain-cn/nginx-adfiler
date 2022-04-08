cmake_minimum_required(VERSION 2.8)

set(CXX_SOURCES)
auto_sources(files "*.cc" "RECURSE")
list(APPEND CXX_SOURCES ${files})

#project name
project(ngx_mapping)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#deps
include_directories("${TP_DIR}")
include_directories("${TP_DIR}/nginx/objs")
include_directories("${TP_DIR}/nginx/src/core")
include_directories("${TP_DIR}/nginx/src/event")
include_directories("${TP_DIR}/nginx/src/os/unix")
include_directories("${TP_DIR}/nginx/src/http")
include_directories("${TP_DIR}/nginx/src/http/v2")
include_directories("${TP_DIR}/nginx/src/http/modules")

include_directories("${TP_DIR}/nginx/src/event/modules")
include_directories("${TP_DIR}/nginx/src/src/mail")

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread -fpermissive -W -Wall -Wpointer-arith -Wno-unused ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

message("--  CMAKE_CXX_FLAGS:" ${CMAKE_CXX_FLAGS})
#add dynamic library
set(target ngx_mapping)
add_library(${target} SHARED "${CXX_SOURCES}")

target_link_libraries(${target}  
  boost_thread
  boost_regex   
  boost_system 
  boost_filesystem 
  hiredis
  rt
)

install(TARGETS ${target} DESTINATION "/data/modules/ngx_mapping/modules")

install(DIRECTORY "${SCRIPTS_DIR}" DESTINATION "/data/modules/ngx_mapping")

install(DIRECTORY "${ETC_DIR}/conf/" DESTINATION "/data/modules/ngx_mapping/conf")

install(DIRECTORY "${ETC_DIR}/supervisor/" DESTINATION "/etc/supervisor")

install(DIRECTORY "${ETC_DIR}/html/" DESTINATION "/data/modules/ngx_mapping/html")

install(FILES "${ETC_DIR}/init.d/ngx_mapping"
   PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
   DESTINATION  "/etc/init.d"
)
