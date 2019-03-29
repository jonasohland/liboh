include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/liboh.cmake")


add_executable(TimerExample timer.cpp)
liboh_setup(TimerExample)

add_executable(SimpleTimer simple_timer.cpp)
liboh_setup(SimpleTimer)

add_executable(SimpleRepeatTimer simple_repeat_timer.cpp)
liboh_setup(SimpleRepeatTimer)