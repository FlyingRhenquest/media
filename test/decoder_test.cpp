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
 * Some basic tests to make sure the decoder is at least pretending to
 * be working.
 */

#include <boost/log/trivial.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <fr/media/audio_decoder_subscriber>
#include <fr/media/decoder>
#include <fr/media/video_decoder_subscriber>
#include <functional>
#include <memory>

// Now I know what you're thinking. You're thinking if I multiply-inherit
// video and audio subscribers, I'm going to have a conflict in subscribe.
// Well that's not a rabbit hole I'm inclined to go down.

class test_helper : public fr::media::decoder_subscriber_interface {

  void audio_cb(AVFrame *)
  {
    audio_packet_count++;
  }

  void video_cb(AVFrame *)
  {
    video_packet_count++;
  }
  
  class audio_helper : public fr::media::audio_decoder_subscriber
  {
    test_helper *owner;
  public:
    audio_helper(test_helper *owner) : owner(owner)
    {      
    }

    virtual ~audio_helper()
    {
    }

    void audio_available_cb(AVFrame *frame) override
    {
      owner->audio_cb(frame);
    }
    
  };

  class video_helper : public fr::media::video_decoder_subscriber
  {
    test_helper *owner;
  public:
    video_helper(test_helper *owner) : owner(owner)
    {
    }

    virtual ~video_helper()
    {
    }

    void video_available_cb(AVFrame *frame) override
    {
      owner->video_cb(frame);
    }
  };

  // I don't really care about other packets, but I'll count
  // them anyway. I usually expect to get 0.

  void other_available_cb(AVFrame *frame, AVMediaType m)
  {
    other_packet_count++;
  }

  boost::signals2::connection other_subscription;
  
public:

  typedef std::shared_ptr<test_helper> pointer;

  static pointer create()
  {
    return std::make_shared<test_helper>();
  }
  
  audio_helper ahelp;
  video_helper vhelp;
  size_t video_packet_count;
  size_t audio_packet_count;
  size_t other_packet_count;

  test_helper() : ahelp(this), vhelp(this), video_packet_count(0l), audio_packet_count(0l), other_packet_count(0l)
  {
  }

  virtual ~test_helper()
  {
    other_subscription.disconnect();
  }

  void subscribe(fr::media::decoder *that)
  {
    ahelp.subscribe(that);
    vhelp.subscribe(that);
    other_subscription = that->other_available.connect(std::bind(&test_helper::other_available_cb, this, std::placeholders::_1, std::placeholders::_2));
  }
  
};

class decoder_test : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(decoder_test);
  CPPUNIT_TEST(count_packets);
  CPPUNIT_TEST(destroy_listener_before_decoder);
  CPPUNIT_TEST_SUITE_END();

public:

  void count_packets()
  {
    auto helper = test_helper::create();
    // TEST_VIDEO should be defined and passed in by the build
    // system. For the basic one we'll just autogenerate one
    // with ffmpeg so you don't have to worry about it
    auto decoder = fr::media::decoder::create(TEST_VIDEO);
    decoder->add(helper);
    decoder->process();
    decoder->join();
    BOOST_LOG_TRIVIAL(info) << "Done processing. Helper counts:";
    BOOST_LOG_TRIVIAL(info) << "Video packets: " << helper->video_packet_count;
    BOOST_LOG_TRIVIAL(info) << "Audio packets: " << helper->audio_packet_count;
    BOOST_LOG_TRIVIAL(info) << "Other packets: " << helper->other_packet_count;
    // There should be more than 0 audio and video packets in my video
    CPPUNIT_ASSERT(helper->audio_packet_count > 0);
    CPPUNIT_ASSERT(helper->video_packet_count > 0);
  }

  // So what happens if we destroy a listener before we the decoder
  // goes away? Let's try it and see!
  void destroy_listener_before_decoder()
  {
    auto decoder = fr::media::decoder::create(TEST_VIDEO);
    {
      auto helper = test_helper::create();
      decoder->add(helper);
    }

    decoder->process();
    decoder->join();
    // We atent crashed?
    CPPUNIT_ASSERT(true);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(decoder_test);
  
