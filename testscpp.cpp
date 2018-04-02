//
// Created by Shoaib Ahmed on 3/30/18.
//
#include <vector>
#include "libdo.h"

static int runs;

bool cb(void *param) {
    (void) param;
    return true;
}

bool predicate(void *param) {
    (void) param;
    return (runs++ % 2) == 0;
}

int main() {
    auto d = do_init();
    auto dw = do_work_when(cb, nullptr, predicate);
    do_so(d, dw);
    while (do_loop(d));
    return 0;
}
