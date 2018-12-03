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
 * This is a basic generic test runner that runs all registered test suites.
 * In a cmake/ctest scenario, it's probably best to compile this to an object
 * file, then link each individual test class you create to its own executable.
 * That way if one crashes, ctest can report it and continue testing the
 * other tests. The other option is to link all your test classes into one
 * monolithic test excutable. If you do that, the listener I'm using will
 * generate a single test report at the end of the entire test run, but
 * if any test actually crashes, all testing will stop at that point.
 * You also won't have the option to run one single individual test
 * with a monolithic test.
 *
 * This can optionally use my cppunit_contrib listener if you installed
 * it somewhere the build can find it. See
 * https://github.com/FlyingRhenquest/cppunit_contrib
 * If you use it, it'll automatically time all the tests
 * and print a report at the end of the run about what
 * passed and what failed.
 */

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/TestResultCollector.h>
#ifdef HAVE_FR_LISTENER
// Yeah, I know, fuckery...
#include <fr/cppunit_contrib/listener>
#endif
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[])
{
  bool tests_failed = false;
  CppUnit::TestResult controller;

#ifdef HAVE_FR_LISTENER
  fr::cppunit_contrib::listener progress;
  controller.addListener(&progress);
#endif
  
  CppUnit::TextTestRunner runner;
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
  try {
    runner.run(controller);
    CppUnit::TestResultCollector &result = runner.result();
    if(result.testFailuresTotal() > 0) {
      tests_failed = true;
    }

#ifdef HAVE_FR_LISTENER
    std::cout << progress.get_report();
#endif
  } catch (std::exception &e) {
    std::cerr << "ERROR: Uncaught exception while running tests: " << e.what();
    tests_failed = true;
  }
  
  return (int) tests_failed;
}
