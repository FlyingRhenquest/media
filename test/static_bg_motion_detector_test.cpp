/**
 * Copyright 2019 Bruce Ide
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
 * Test static_bg_motion_detector
 *
 */

#include <chrono>
#include <cppunit/extensions/HelperMacros.h>
#include <fr/media/decoder>
#include <fr/media/frame2cv>
#include <fr/media/motion_detector>
#include <memory>
#include <string>

class static_bg_motion_detector_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(static_bg_motion_detector_test);
  CPPUNIT_TEST(basic_motion_test);
  CPPUNIT_TEST_SUITE_END();

  // Do something with callback data. While the test technically only
  // cares that motion was detected, I want to save detected motion
  // every 5 frames or so

  void motion_detected(cv::Mat frame, size_t frame_time, std::vector<std::vector<cv::Point>> contours, size_t &last_saved, bool &motion_detected)
  {
    // First things first
    motion_detected = true;
    BOOST_LOG_TRIVIAL(debug) << "Motion detected at video frame " << frame_time;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (frame_time - last_saved > 4) {
      last_saved = frame_time;
      std::string base_filename("frame_");
      base_filename.append(std::to_string(frame_time));
      base_filename.append(".png");
      // Draw bounding rects around all contours
      for (auto contour : contours) {
	cv::Rect bbox = cv::boundingRect(contour);
	// Draw green rectangle
	cv::rectangle(frame, bbox, cv::Scalar(0, 2550, 0));	
      }

      cv::imwrite( base_filename, frame);
      
    }
  }
  
public:

  void basic_motion_test()
  {
    bool motion_detected = false;
    // I'm going to need to record a short test video in a minute...
    auto decoder = fr::media::decoder::create(TEST_VIDEO);
    auto converter = fr::media::frame2cv::create();
    size_t counter = 0l; // Frame counter
    decoder->add(converter);
    converter->available.connect([&counter](cv::Mat frame) { counter++; });
    
    auto detector = fr::media::static_bg_motion_detector::create();
    detector->subscribe(converter);

    std::chrono::steady_clock::time_point test_start = std::chrono::steady_clock::now();
    size_t last_save = 0;

    detector->available.connect([this, &motion_detected, &last_save](cv::Mat frame, size_t frameno, std::vector<std::vector<cv::Point>> contours) { this->motion_detected(frame, frameno, contours, last_save, motion_detected); }); 
    
    decoder->process();
    decoder->join();

    // Do some accounting
    std::chrono::steady_clock::time_point test_end = std::chrono::steady_clock::now();
    size_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(test_end - test_start).count();
    BOOST_LOG_TRIVIAL(info) << "Processed " << counter << " frames in " << ms << " ms";
    BOOST_LOG_TRIVIAL(info) << "Per frame processing time was " << (ms / counter) << " ms";
    CPPUNIT_ASSERT(motion_detected);
  }
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(static_bg_motion_detector_test);
