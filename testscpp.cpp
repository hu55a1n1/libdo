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
