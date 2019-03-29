macro(enable_cxx_17 input_target)

    message(STATUS "processing target: ${input_target}")

    set_target_properties(${input_target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS YES
    )

    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)

endmacro(enable_cxx_17)

macro(liboh_setup input_target)

    find_package(Threads)

    target_link_libraries(${input_target} PUBLIC Threads::Threads)


    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)	

    find_package(Boost COMPONENTS 
                        "system" 
                        "date_time" 
                        "regex" 
                        REQUIRED)
                        
    target_link_libraries(${input_target} PUBLIC ${Boost_LIBRARIES})

    if(WIN32)
        target_compile_options(${input_target} PUBLIC "/bigobj")
        target_compile_definitions(${input_target} PRIVATE _WIN32_WINNT=0x0A00)				
    endif()		

    target_link_libraries(${input_target} PUBLIC liboh)

    enable_cxx_17(${input_target})

endmacro(liboh_setup)

macro(version_tag input_target) 

    if(NOT ${VERSION_TAG})
        get_version_tag()
    endif()

    target_compile_definitions(${input_target} PRIVATE VERSION_TAG=${VERSION_TAG})

endmacro(version_tag)

function(generator_is_ide output_var)

    if ((${CMAKE_GENERATOR} MATCHES "Xcode") OR (${CMAKE_GENERATOR} MATCHES "Visual Studio 15 2017 Win64"))
        set(${output_var} TRUE PARENT_SCOPE)
    else()
        set(${output_var} FALSE PARENT_SCOPE)
    endif()
    
endfunction(generator_is_ide)