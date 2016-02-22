cmake_minimum_required (VERSION 2.6)

project (hsm_unit_tests)

# Pass "-DHSM_DEBUG=ON" to CMake to enable debug features
# Note that this gets automatically defined in MSVC Debug configurations because it defines _DEBUG
option(HSM_DEBUG "Enable HSM debug features" OFF)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # Multiprocessor build
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX") # Max warning level and error on warnings
	if(MSVC_VERSION LESS 1900) # Starting from MSVC 14 (2015), STL needs language extensions enabled
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za") # Disable language extensions
	endif()
endif()

function(add_hsm_lib)
	message(STATUS "Adding hsm lib")
	# Add a header-only project (an INTERFACE library in CMake)
	add_library(hsm INTERFACE)
	target_include_directories(hsm INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>)
endfunction(add_hsm_lib)

function(add_catch_lib)
	message(STATUS "Adding catch lib")
	# Add a header-only project (an INTERFACE library in CMake)
	add_library(catch INTERFACE)
	target_include_directories(catch INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../catch>)
endfunction(add_catch_lib)

# Adds a project (exe) per source file found in source directory
function(add_tests)
	file(GLOB TEST_SRC "source/*.cpp")
	foreach(SRC_FILE ${TEST_SRC})
		get_filename_component(SRC_NAME ${SRC_FILE} NAME_WE)
		set(PROJ_NAME "${SRC_NAME}")
		message(STATUS "Adding project ${PROJ_NAME}")
		add_executable("${PROJ_NAME}" ${SRC_FILE})
		target_link_libraries("${PROJ_NAME}" hsm) # link against hsm lib
		target_link_libraries("${PROJ_NAME}" catch) # link against catch lib
		if(HSM_DEBUG)
			target_compile_definitions("${PROJ_NAME}" PRIVATE HSM_DEBUG=1)
		endif()			
	endforeach(SRC_FILE)
endfunction(add_tests)

add_hsm_lib()
add_catch_lib()
# add_tests()

set(PROJ_NAME "hsm_unit_tests")
file(GLOB TEST_SRC "source/*.*")
file(GLOB HSM_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../include/*.*")
file(GLOB CATCH_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../catch/*.*")
add_executable("${PROJ_NAME}" ${TEST_SRC} ${HSM_SRC} ${CATCH_SRC})
target_link_libraries("${PROJ_NAME}" hsm) # link against hsm lib
target_link_libraries("${PROJ_NAME}" catch) # link against catch lib
if(HSM_DEBUG)
	target_compile_definitions("${PROJ_NAME}" PRIVATE HSM_DEBUG=1)
endif()			
