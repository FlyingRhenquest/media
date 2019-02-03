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
 * This example requires 
 * https://github.com/FlyingRhenquest/No-Reference-Image-Quality-Assessment-using-BRISQUE-Model
 * to be built and installed somewhere. It is a simple example of how to run
 * the brisque image quality assessor on each individual video frame
 * in your video. It will simply output the frame number and the
 * video quality value reported by Brisque in that frame.
 *
 * * Requires libsvm
 */

#include <brisque/brisque.h>
#include <chrono>
#include <functional>
#include <libsvm/svm.h>
#include <fr/media/decoder>
#include <fr/media/frame2cv>

void brisque_frame_callback(cv::Mat frame, svm_model *model, long *framecount)
{
  (*framecount)++;
  float score = jd_brisque::computescore(frame, model);
  std::cout << "Frame " << (*framecount) << " score " << score << std::endl;
}

void print_help(char *arg0)
{
  std::cout << "Usage: " << arg0 << " video_file" << std::endl << std::endl;
  std::cout << "Will print the brisque quality assessment of each frame in the video file." << std::endl;
  std::cout << "video_file can be any file or URL that ffplay can play." << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
  long framecount = 0l;
  svm_model *model = jd_brisque::load_model("/usr/local/share/brisque/allmodel");
  if (argc < 2) {
    print_help(argv[0]);
  } else {
    try {
      // I'm assuming you installed the brisque library in
      // /usr/local. If you didn't, you'll have to change the
      // model location and probably the cmake file to find
      // the library (I could just pass it in the args, but
      // this is just an example. So I'm not.)

      if (nullptr == model) {
	throw std::logic_error("Unable to find brisque model.");
      }
      std::cout << "Opening " << std::string(argv[1]) << std::endl;
      fr::media::decoder::pointer video = fr::media::decoder::create(argv[1]);
      fr::media::frame2cv::pointer converter = fr::media::frame2cv::create(); // convert frames to mats
      video->add(converter);
      converter->available.connect(std::bind(&brisque_frame_callback, std::placeholders::_1, model, &framecount));

      std::chrono::high_resolution_clock::time_point run_start = std::chrono::high_resolution_clock::now();
      video->process();
      video->join();

      std::chrono::high_resolution_clock::time_point run_end = std::chrono::high_resolution_clock::now();
      size_t total_millis = std::chrono::duration_cast<std::chrono::milliseconds>(run_end - run_start).count();
      std::cout << "Processed " << framecount << " frames in " << total_millis << " milliseconds" << std::endl;
      size_t millis_per_frame = total_millis / framecount;
      std::cout << "Total ms per frame: " << millis_per_frame <<  std::endl;
      // This will leak if I throw, so don't write your code like this.
        
    } catch (std::exception &e) {
      std::cout << "Error running example: " << e.what();
    }
    if (nullptr != model) {
      svm_free_and_destroy_model(&model);
    }
  }
  
}
