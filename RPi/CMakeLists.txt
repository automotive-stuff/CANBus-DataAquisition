cmake_minimum_required(VERSION 3.15.2)

project(RPiCAN)

option(BUILD_TESTS "Turn on building of tests" OFF)

file(GLOB SRC_FILES
	"src/*.cpp"
	"src/*.c"
)

set(INC_FILES_DIR
	include/
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED)

if (BUILD_TESTS)
	set(BUILD_GMOCK ON)
	add_subdirectory(googletest)
	add_subdirectory(test)
endif()

include_directories(
	${INC_FILES_DIR}
	${CMAKE_SOURCE_DIR}/libgps/build
)

message(STATUS "${CMAKE_SOURCE_DIR}")
find_library(Gps_LIB libgps.a "${CMAKE_SOURCE_DIR}/libgps/build")

add_executable(RPiCAN
	${SRC_FILES}
)

target_link_libraries(RPiCAN ${Gps_LIB})

add_library(RPiCAN-static STATIC
	${SRC_FILES}
)
