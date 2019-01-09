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
 * Verify the audio resampler can resample audio.
 */

#include <boost/log/trivial.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <fr/media/decoder>
#include <fr/media/audio_decoder_subscriber>
#include <fr/media/audio_resampler>
#include <memory>
#include <iostream>
#include <fstream>


// Receives and counts output frames
class test_receiver : public fr::media::audio_decoder_subscriber {

public:

  typedef std::shared_ptr<test_receiver> pointer;
  
  int frames_received;
  size_t samples_received;
  int64_t last_pts; // Saves last PTS seen, from which we can determine length of the sample
  ushort min_sample, max_sample;

  test_receiver() : frames_received(0), samples_received(0l), last_pts(0l), min_sample(65535), max_sample(0)
  {
  }

  virtual ~test_receiver()
  {
  }

  
  static pointer create()
  {
    return std::make_shared<test_receiver>();
  }

  
  
  void audio_available_cb(AVFrame *frame) override
  {
    frames_received++;
    last_pts = frame->pts;
    samples_received += frame->nb_samples;

    ushort *data = (ushort *) frame->data;
    for (int i = 0 ; i < frame->nb_samples; ++i) {
      if (data[i] < min_sample) {
	min_sample = data[i];
	BOOST_LOG_TRIVIAL(debug) << "New min sample: " << (int) min_sample;
      }
      if (data[i] > max_sample) {
	max_sample = data[i];
	BOOST_LOG_TRIVIAL(debug) << "New max sample: " << (int) max_sample;
      }
    }
    
    CPPUNIT_ASSERT(frame->sample_rate > 0);
    
  }
  
};

class audio_resampler_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(audio_resampler_test);
  CPPUNIT_TEST(basic_resample_test);
  CPPUNIT_TEST_SUITE_END();

public:

  // Use our already-created video sample and resample to
  // 16K 16 bit PCM mono (Which coincidentally is what sphinx wants
  // for its acoustic model.)
  // We're counting some things but if we can make it through
  // this test without segfaulting, we probably did OK.
  
  void basic_resample_test()
  {
    auto decoder = fr::media::decoder::create(TEST_VIDEO);
    auto resampler = fr::media::audio_resampler::create(AV_CH_LAYOUT_MONO, (AVSampleFormat) AV_SAMPLE_FMT_S16, 16000);
    auto receiver = test_receiver::create();

    decoder->add(resampler);
    resampler->add(receiver);
    decoder->process();
    decoder->join();
    
    CPPUNIT_ASSERT(receiver->frames_received > 0);
    CPPUNIT_ASSERT(receiver->samples_received > 0);
    // The length of our audio sample was 10 seconds
    BOOST_LOG_TRIVIAL(debug) << "Received " << receiver->samples_received << " samples";
    CPPUNIT_ASSERT(receiver->samples_received > 0);
    BOOST_LOG_TRIVIAL(debug) << "Max sample: " << receiver->max_sample;
    CPPUNIT_ASSERT(receiver->max_sample != 0);
      
  }
    
  
};

CPPUNIT_TEST_SUITE_REGISTRATION(audio_resampler_test);
