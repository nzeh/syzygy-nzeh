// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Test launcher that runs all the tests in a test suite with a large timeout.
//
// By default the unittests run with a timeout of 45 seconds, it's not enough
// for some more intensive tests (like the integration tests). This code is
// pretty much a copy-paste of base/test/run_all_unittests.cc, it just add an
// argument line to the process command line to increase the timeout of the
// unittests.

#include <iostream>
#include "syzygy/refinery/process_state/process_state.h"

struct A {
    int x;
};

struct B {
};

struct C : public A, public B {
    int z;
};

using std::cout;
using std::endl;

using namespace refinery;

int main(int argc, char** argv) {
    C c;
    A *a = &c;
    B *b = &c;

    std::cout << "@C: " << &c << std::endl;
    std::cout << "@A: " << a << std::endl;
    std::cout << "@B: " << b << std::endl;
    std::cout << "@C.x: " << &c.x << std::endl;
    std::cout << "@C.z: " << &c.z << std::endl;
    std::cout << "@A.x: " << &a->x << std::endl;
}
