include(CheckFunctionExists)

#find_package(RapidJSON REQUIRED)

# boost checks
find_package(Boost 1.58.0 COMPONENTS system program_options filesystem context thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions("-DHAVE_BOOST1_58")

# libpthreads
find_package(PThread REQUIRED)
include_directories(${LIBPTHREAD_INCLUDE_DIRS})
if (LIBPTHREAD_STATIC)
  add_definitions("-DPTW32_STATIC_LIB")
endif()
link_directories(${LIBPTHREAD_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES "${LIBPTHREAD_LIBRARIES}")
