cmake_minimum_required (VERSION 2.6)

project (hsm_wiki_samples)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") # Multiprocessor build
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX") # Max warning level and error on warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Za") # Disable language extensions
endif()

function(add_hsm_lib)
	message(STATUS "Adding hsm lib")
	file(GLOB HSM_SRC "../../src/*.*")
	file(GLOB HSM_INC "../../include/hsm/*.*")
	add_library(hsm ${HSM_SRC} ${HSM_INC})
endfunction(add_hsm_lib)

# Adds a project (exe) per source file found in source/${CHAPTER_NAME}
# Note: won't work if a sample spans multiple source files
function(add_chapter_samples CHAPTER_NAME)
	file(GLOB SAMPLES_SRC "source/${CHAPTER_NAME}/*.cpp")

	foreach(SRC_FILE ${SAMPLES_SRC})
		get_filename_component(SRC_NAME ${SRC_FILE} NAME_WE)
		set(PROJ_NAME "${CHAPTER_NAME}_${SRC_NAME}")
		message(STATUS "Adding project ${PROJ_NAME}")
		add_executable("${PROJ_NAME}" ${SRC_FILE})
		target_link_libraries("${PROJ_NAME}" hsm) # link against hsm lib
	endforeach(SRC_FILE)
endfunction(add_chapter_samples)

# hsm include dir
include_directories("../../include")

# add hsm library
add_hsm_lib()

# add sample exes
add_chapter_samples("ch2")
add_chapter_samples("ch3")
add_chapter_samples("ch4")
add_chapter_samples("ch5")
