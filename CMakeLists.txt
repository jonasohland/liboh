cmake_minimum_required(VERSION 3.4)

project(liboh)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/liboh.cmake")

option(liboh_generate_examples "" OFF)
option(liboh_generate_docs "" OFF)

file(GLOB_RECURSE LIBOH_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

# -- generate a documentation target

if(liboh_generate_docs)

	find_package(Doxygen)
	if (DOXYGEN_FOUND)

		set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
		set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

		configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
		message(STATUS "Generating Doxygen build Task")

		add_custom_target( liboh_documentation ALL
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM )
	else (DOXYGEN_FOUND)
	message("Doxygen need to be installed to generate the doxygen documentation")
	endif (DOXYGEN_FOUND)

endif(liboh_generate_docs)


# -- find and link boost

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)	

find_package(Boost COMPONENTS 
					"system" 
					"date_time" 
					"regex" 
					REQUIRED)


generator_is_ide(IS_IDE)

# -- generate a target to edit in ides

if(${IS_IDE})

    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "o Headers" FILES ${LIBOH_HEADERS})

    add_library(liboh_headers STATIC ${LIBOH_HEADERS})

    set_target_properties(liboh_headers PROPERTIES LINKER_LANGUAGE CXX)

    target_include_directories(liboh_headers PUBLIC ${Boost_INCLUDE_DIRS})

    enable_cxx_17(liboh_headers)

endif()

# -- interface target to get the include dirs

add_library(liboh INTERFACE)

target_include_directories(liboh INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(liboh INTERFACE ${Boost_INCLUDE_DIRS})

if(liboh_generate_examples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
endif()