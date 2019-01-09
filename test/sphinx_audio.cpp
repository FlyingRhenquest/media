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
 * See if sphinx can recognize hello world from my cheesy little wav.
 */

#include <chrono>
#include <cppunit/extensions/HelperMacros.h>
#include <fr/media/decoder>
#include <fr/media/sphinx_audio>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

class sphinx_audio_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(sphinx_audio_test);
  CPPUNIT_TEST(test_hello_world);
  CPPUNIT_TEST_SUITE_END();

  void sphinx_audio_cb(std::string word, float64 confidence, std::vector<std::string> *word_vector, std::vector<float64> *confidence_vector)
  {
    BOOST_LOG_TRIVIAL(debug) << "audio CB -- words: \"" << word << "\" confidence " << confidence;
    word_vector->push_back(word);
    confidence_vector->push_back(confidence);
  }

public:

  // This is a pretty bad example for sphinx, but it does kind of work. That's about all the time I want
  // to spend on it.
  
  void test_hello_world()
  {
    std::vector<std::string> word_vector;
    std::vector<float64> confidence_vector;
    auto decoder = fr::media::decoder::create(TEST_DATA_DIR "/hello_world.wav");
    auto audio_processor = fr::media::sphinx_audio::create(std::string(MODELDIR "/en-us/en-us"),
							   std::string(MODELDIR "/en-us/en-us.lm.bin"),
							   std::string(MODELDIR "/en-us/cmudict-en-us.dict"));
    decoder->add(audio_processor);
    // Bind to test callback
    audio_processor->available.connect(std::bind(&sphinx_audio_test::sphinx_audio_cb, this, std::placeholders::_1, std::placeholders::_2, &word_vector, &confidence_vector));
    decoder->process();

    // Shut down the sphinx process in a few seconds
    std::this_thread::sleep_for(std::chrono::seconds(2));
    audio_processor->shutdown();
    decoder->shutdown();
    decoder->join();

    CPPUNIT_ASSERT(word_vector.size() == 1);
    CPPUNIT_ASSERT(word_vector.front() == std::string("hello world"));
  }

  
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(sphinx_audio_test);
