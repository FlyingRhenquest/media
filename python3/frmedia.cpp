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
 * This is a python interface for this media library. It uses pybind11 to
 * define a python interface for all the media objects.
 */

#include <pybind11/pybind11.h>
#include <fr/media/decoder>
#include <fr/media/decoder_interface>
#include <fr/media/decoder_subscriber_interface>

PYBIND11_MODULE(fr_media, m) {

  pybind11::class_<fr::media::decoder_interface>(m, "decoder_interface")
    .def(pybind11::init<>())
    .def("add", (void (fr::media::decoder_interface::*)(fr::media::decoder_subscriber_interface &)) &fr::media::decoder_interface::add, "Add a subscriber");
  
  pybind11::class_<fr::media::decoder, fr::media::decoder_interface>(m, "decoder")
    .def(pybind11::init<std::string , std::string>(), pybind11::arg("filename"), pybind11::arg("format_name") = "" )
    // Join will get called in its destructor, but you can use it if you want to wait in your main thread
    .def("join", &fr::media::decoder::join)
    .def("process", &fr::media::decoder::process)
    .def("shutdown", &fr::media::decoder::shutdown);

}
