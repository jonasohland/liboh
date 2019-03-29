include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/liboh.cmake")


add_executable(TimerExample timer.cpp)
liboh_setup(TimerExample)