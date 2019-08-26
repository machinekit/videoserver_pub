project( videoserver_pub )
cmake_minimum_required( VERSION 2.8 )
set(CMAKE_CXX_STANDARD 14)

find_package( OpenCV REQUIRED )

message( STATUS "OpenCV library status:" )
message( STATUS "    version: ${OpenCV_VERSION}" )
message( STATUS "    libraries: ${OpenCV_LIBS}" )
message( STATUS "    include path: ${OpenCV_INCLUDE_DIRS}" )

## load in pkg-config support
find_package(PkgConfig)
## use pkg-config to get hints for 0mq locations
pkg_check_modules(PC_ZeroMQ QUIET zmq)

## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

find_package( Boost 1.40 COMPONENTS program_options log REQUIRED )
add_definitions(-DBOOST_LOG_DYN_LINK)

find_package (Threads)

add_subdirectory(proto)

include_directories( ${OpenCV_INCLUDE_DIRS} ${ZeroMQ_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIR})

add_executable( ${PROJECT_NAME} main.cpp)
target_link_libraries( ${PROJECT_NAME} proto ${OpenCV_LIBS} ${ZeroMQ_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin)
