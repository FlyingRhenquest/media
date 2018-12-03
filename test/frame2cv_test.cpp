/**
 * Copyright 2018 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * Test frame2cv. Largely a sanity test to make sure real images are coming
 * from frame2cv (and indirectly, decoder)
 */

#include <chrono>
#include <cppunit/extensions/HelperMacros.h>
#include <fr/media/decoder>
#include <fr/media/frame2cv>
#include <functional>
#include <memory>
#include <opencv2/opencv.hpp>

class frame2cv_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(frame2cv_test);
  CPPUNIT_TEST(save_images_test);
  CPPUNIT_TEST(timing_test);
  CPPUNIT_TEST_SUITE_END();

  size_t frame_counter;

  void cv_frame_available_cb(cv::Mat frame)
  {
    frame_counter++;
    // Save a frame every 20
    if (frame_counter % 20 == 0) {     
      std::string filename(OUTPUT_DIR "/frame_"); // Yes you CAN do this.
      filename.append(std::to_string(frame_counter));
      filename.append(".png");
      BOOST_LOG_TRIVIAL(info) << "Writing image " << filename;
      cv::imwrite(filename, frame);
    }
  }

  void timing_available_cb(cv::Mat frame)
  {
    // Just count the mat, so we can work out about how
    // long the conversion takes
    frame_counter++;
  }
  
public:

  void save_images_test()
  {
    frame_counter = 0l;
    auto decoder = fr::media::decoder::create(TEST_VIDEO);
    auto converter = fr::media::frame2cv::create();
    decoder->add(converter);
    // Currently I don't have an add in frame2cv
    converter->available.connect(std::bind(&frame2cv_test::cv_frame_available_cb, this, std::placeholders::_1));
    decoder->process();
    decoder->join();
    CPPUNIT_ASSERT(frame_counter > 0);
  }

  // This test doesn't do anything with the mat, so we can work out about how long each
  // frame takes to get this far

  void timing_test()
  {
    frame_counter = 0;
    auto decoder = fr::media::decoder::create(TEST_VIDEO);
    auto converter = fr::media::frame2cv::create();
    decoder->add(converter);
    converter->available.connect(std::bind(&frame2cv_test::timing_available_cb, this, std::placeholders::_1));
    // You can use auto here, if you want to.
    std::chrono::high_resolution_clock::time_point test_start = std::chrono::high_resolution_clock::now();
    decoder->process();
    decoder->join();
    CPPUNIT_ASSERT(frame_counter > 0);
    std::chrono::high_resolution_clock::time_point test_end = std::chrono::high_resolution_clock::now();
    size_t total_millis = std::chrono::duration_cast<std::chrono::milliseconds>(test_end - test_start).count();
    size_t millis_per_frame = total_millis / frame_counter;
    BOOST_LOG_TRIVIAL(info) << "Processed " << frame_counter << " frames in " << total_millis << " ms";
    BOOST_LOG_TRIVIAL(info) << "Total ms per frame: " << millis_per_frame;
  }
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(frame2cv_test);
