## fr::media

This is a project to demonstrate how to use ffmpeg to get video and audio
frames in c++. At the moment it consists of a decoder and an object to take
video frames from the decoder and convert them to OpenCV Mat objects.

### Motivation

I have an experimental project in mind for later on for which I'll need
to do this. I realize that there are other ways to get Mats from video
(OpenCV has direct support for that,) but I also want to understand
the underlying libraries I'm using.

### Requirements

1. ffmpeg dev libraries
2. OpenCV dev libraries
3. cppunit to run the unit tests
4. Boost::Log, Boost::Signals2
5. (optional) My cppunit_contrib project at https://github.com/FlyingRhenquest/cppunit_contrib. The build will look for it in /usr/local/include or /usr/include and use it if it's there. It times the individual tests but otherwise doesn't buy you that much.

### Building

Make a temporary build directory somewhere. Like /tmp/build_media. Git
clone this somewhere else. Cd to your temporary directory and go
"cmake top_level_media_dir" (For example "cmake ~/sandbox/media").

Cmake will generate a makefile and test video. Then just run make and
ctest --verbose to run the tests.

The library itself is header only. Make install will install it in the
default include directory, under fr/media (Usually
/usr/local/include/fr/media)

### Usage

Create a decoder and add subscribers to it. Once all the subscribers are
added, run decoder->process(). This will kick off a separate thread in
which the video is decoded. Subscribers will be notified when new packets
are available. You can do other processing in your main thread or just
run decoder->join() to wait for the decoder thread to finish.

The only actual subscribers to this right now are frame2cv and the test
helper in the decoder test. frame2cv exposes its own available signal,
which provides an OpenCV Mat of the frame it just received. One of the
frame2cv tests writes a video frame every 20 frames to a png file in
the build directory, using OpenCV's imwrite. The other test times how long
it takes to decode all 300 frames in the test video and computes the
average time it takes to convert a frame (2 ms for the generated test
video, on my system.)

### Todos

At some point I might want to write a test that runs against the test
video a few thousand times just to make sure I'm not leaking memory
somewhere. It's incredibly easy to do that with the libav libs.

At this point it wouldn't be hard to export a python API with
boost::python. If I get a lot of interest, I might write one.
I might write one anyway, as my future project will probably
end up wanting it.