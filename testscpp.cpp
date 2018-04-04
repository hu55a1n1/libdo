/*
 This file is part of libdo

 Copyright (c) 2018 Shoaib Ahmed

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include "libdo.h"

static int run_after_iterations = 5;

int main() {
    int iterations = 0;
    auto d = do_init();
    if (!d) {
        exit(EXIT_FAILURE);
    }

    auto dw = do_work_when(
            // Cannot capture, use void data ptr instead
            [](void *data) {
                std::cout << "> Work ran after " << *reinterpret_cast<int *>(data) << " iterations" << std::endl;
                return true;
            },
            &iterations,
            [](void *data) {
                return (*reinterpret_cast<int *>(data)) >= run_after_iterations;
            }
    );
    if (!dw) {
        do_destroy(d);
        exit(EXIT_FAILURE);
    }

    std::cout << "--- Work will run after " << run_after_iterations << " iterations ---" << std::endl;
    do_so(d, dw);
    while (do_loop(d)) {
        iterations++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    do_destroy(d);
    exit(EXIT_SUCCESS);
}
