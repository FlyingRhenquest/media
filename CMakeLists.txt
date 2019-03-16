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
pkg_check_modules(FFLIBS REQUIRED libavutil libavcodec libavdevice libswscale libavformat libswresample)
pkg_search_module(cppunit REQUIRED cppunit)

# Pocketsphinx for audio if you have it

pkg_search_module(pocketsphinx QUIET pocketsphinx)

find_package(OpenCV REQUIRED)

# Set up boost, which we'll use for a couple of of things

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS log program_options REQUIRED)

# Set up unit tests


if (EXISTS /usr/local/include/fr/cppunit OR EXISTS /usr/include/fr/cppunit)
  set(USE_FR_CPPUNIT TRUE)
else()
  SET(USE_FR_CPPUNIT FALSE)
endif()

set(TEST_DATA_DIR "${CMAKE_BINARY_DIR}/test_data")
file(MAKE_DIRECTORY ${TEST_DATA_DIR})
configure_file(${CMAKE_SOURCE_DIR}/test/data/hello_world.wav ${TEST_DATA_DIR}/hello_world.wav COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/test/data/motion_test.webm ${TEST_DATA_DIR}/motion_test.webm COPYONLY)

if (NOT EXISTS ${TEST_DATA_DIR}/testvideo.webm)
  message("Generating test video")
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/generate_test_video.sh
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Tests

add_executable(decoder_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/decoder_test.cpp)
target_include_directories(decoder_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(decoder_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} Threads::Threads)
target_compile_options(decoder_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER})
target_compile_definitions(decoder_test PRIVATE TEST_VIDEO="${TEST_DATA_DIR}/testvideo.webm")

add_executable(frame2cv_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/frame2cv_test.cpp)
target_include_directories(frame2cv_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(frame2cv_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} Threads::Threads)
target_compile_options(frame2cv_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER} ${OpenCV_CFLAGS_OTHER})
target_compile_definitions(frame2cv_test PRIVATE TEST_VIDEO="${TEST_DATA_DIR}/testvideo.webm")

if (${pocketsphinx_FOUND})
  pkg_get_variable(SPHINX_MODELDIR pocketsphinx modeldir)

  add_executable(sphinx_audio_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/sphinx_audio.cpp)
  target_include_directories(sphinx_audio_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${pocketsphinx_INCLUDE_DIRS})
  target_link_libraries(sphinx_audio_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} ${pocketsphinx_LIBRARIES} Threads::Threads)
  target_compile_options(sphinx_audio_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER} ${OpenCV_CFLAGS_OTHER} ${pocketsphinx_CFLAGS_OTHER})
  target_compile_definitions(sphinx_audio_test PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}" MODELDIR="${SPHINX_MODELDIR}")
endif()
  
add_executable(audio_resampler_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/audio_resampler_test.cpp)
target_include_directories(audio_resampler_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(audio_resampler_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} Threads::Threads)
target_compile_options(audio_resampler_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER})
target_compile_definitions(audio_resampler_test PRIVATE TEST_VIDEO="${TEST_DATA_DIR}/testvideo.webm")


add_executable(static_bg_motion_detector_test ${CMAKE_SOURCE_DIR}/test/test_runner_basic.cpp ${CMAKE_SOURCE_DIR}/test/static_bg_motion_detector_test.cpp)
target_include_directories(static_bg_motion_detector_test PUBLIC ${FFLIBS_INCLUDE_DIRS} ${cppunit_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(static_bg_motion_detector_test PUBLIC ${FFLIBS_LIBRARIES} ${cppunit_LIBRARIES} ${Boost_LIBRARIES} Threads::Threads ${OpenCV_LIBRARIES})
target_compile_options(static_bg_motion_detector_test PUBLIC ${FFLIBS_CFLAGS_OTHER} ${cppunit_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER} ${OpenCV_CFLAGS_OTHER})
target_compile_definitions(static_bg_motion_detector_test PRIVATE TEST_VIDEO="${TEST_DATA_DIR}/motion_test.webm")



Set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
add_compile_definitions(BOOST_LOG_DYN_LINK)

add_compile_definitions(OUTPUT_DIR="${OUTPUT_DIR}")
  
if (USE_FR_CPPUNIT)
  add_compile_definitions(HAVE_FR_LISTENER)
endif()

enable_testing()
add_test(NAME decoder_test COMMAND decoder_test)
add_test(NAME frame2cv_test COMMAND frame2cv_test)
add_test(NAME audio_resampler_test COMMAND audio_resampler_test)
add_test(NAME static_bg_motion_detector_test COMMAND static_bg_motion_detector_test)

if (pocketsphinx_FOUND)
  add_test(NAME sphinx_audio_test COMMAND sphinx_audio_test)
  list(APPEND INSTALL_LIST ${INCLUDE_DIR}/sphinx_audio)
endif()



# Set up installer
include(GNUInstallDirs)
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/fr/media")

set(INSTALL_LIST
  ${INCLUDE_DIR}/audio_decoder_subscriber
  ${INCLUDE_DIR}/audio_resampler
  ${INCLUDE_DIR}/decoder
  ${INCLUDE_DIR}/decoder_interface
  ${INCLUDE_DIR}/decoder_subscriber_interface
  ${INCLUDE_DIR}/frame2cv
  ${INCLUDE_DIR}/video_decoder_subscriber
  )

install(FILES
  ${INSTALL_LIST}
  DESTINATION
  include/fr/media
  )

if (pocketsphinx_FOUND)
  message("Using sphinx model dir: ${SPHINX_MODELDIR}")
endif()


#
# Examples
#

option(BUILD_EXAMPLES "Build examples" ON)

if (BUILD_EXAMPLES)

  # brisque video assumes you installed the brisque library from my repo
  # in /usr/local
  add_executable(brisque_video ${CMAKE_SOURCE_DIR}/examples/brisque_video.cpp)
  target_include_directories(brisque_video PRIVATE ${FFLIBS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(brisque_video PRIVATE ${FFLIBS_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} brisque_iqa svm)
  target_compile_options(brisque_video PRIVATE ${FFLIBS_CFLAGS_OTHER} ${Boost_CFLAGS_OTHER} ${OpenCV_CFLAGS_OTHER})

endif()
