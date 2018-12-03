cmake_minimum_required(VERSION 3.13)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(fr_media CXX)
set(CMAKE_CXX_STANDARD 14)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/include)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ffmpeg generates pkg-config files, so we'll use those
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFLIBS REQUIRED libavutil libavcodec libavdevice libswscale libavformat)
pkg_search_module(cppunit REQUIRED cppunit)

find_package(OpenCV REQUIRED)

# Set up boost, which we'll use for a couple of of things

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS log REQUIRED)

# Set up unit tests

if (NOT EXISTS ${CMAKE_BINARY_DIR}/testvideo.webm)
  message("Generating test video")
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/generate_test_video.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

if (EXISTS /usr/local/include/fr/cppunit OR EXISTS /usr/include/fr/cppunit)
  set(USE_FR_CPPUNIT TRUE)
else()
  SET(USE_FR_CPPUNIT FALSE)
endif()

add_executable(decoder_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/decoder_test.cpp)
target_include_directories(decoder_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(decoder_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} Threads::Threads)
target_compile_options(decoder_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER})

add_executable(frame2cv_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/frame2cv_test.cpp)
target_include_directories(frame2cv_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(frame2cv_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} Threads::Threads)
target_compile_options(frame2cv_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER} ${OpenCV_CFLAGS_OTHER})

add_compile_definitions(TEST_VIDEO="${CMAKE_BINARY_DIR}/testvideo.webm")
set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
add_compile_definitions(BOOST_LOG_DYN_LINK)

add_compile_definitions(OUTPUT_DIR="${OUTPUT_DIR}")
  
if (USE_FR_CPPUNIT)
  add_compile_definitions(HAVE_FR_LISTENER)
endif()

enable_testing()
add_test(NAME decoder_test COMMAND decoder_test)
add_test(NAME frame2cv_test COMMAND frame2cv_test)

# Set up installer
include(GNUInstallDirs)
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/fr/media")
install(FILES
  ${INCLUDE_DIR}/audio_decoder_subscriber
  ${INCLUDE_DIR}/decoder_subscriber_interface
  ${INCLUDE_DIR}/decoder
  ${INCLUDE_DIR}/video_decoder_subscriber
  ${INCLUDE_DIR}/audio_decoder_subscriber
  ${INCLUDE_DIR}/frame2cv
  DESTINATION
  include/fr/media
  )
