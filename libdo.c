/*
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
#include <stdlib.h> /* malloc, realloc, free */
#include "libdo.h"
#include "vector.h"

#undef malloc
#undef realloc
#undef free

static do_malloc_func do_malloc = malloc;
static do_realloc_func do_realloc = realloc;
static do_free_func do_free = free;


union predicate {
    bool *p;
    returns_true_func fn;
#ifndef DO_SANS_TIME
    time_t tm;
#endif
};

enum predicate_type {
    DO_PREDICATE_PTR,
    DO_PREDICATE_FUNC,
#ifndef DO_SANS_TIME
    DO_PREDICATE_TIME,
#endif
};

struct predicate_container {
    enum predicate_type pt;
    union predicate predicate;
};

struct do_work {
#ifndef DO_SANS_PRIO
    size_t prio;
#endif
    struct predicate_container pc;
    work_func work;
    void *data;
};

struct do_doer {
#ifndef DO_SANS_PRIO
    bool sorted;
#endif
#ifndef DO_SANS_TIME
    time_t now;
#endif
    struct do_work **vector;
};


/* Doer */
struct do_doer *do_init() {
    struct do_doer *d = do_malloc(sizeof(*d));
    if (d) {
#ifndef DO_SANS_PRIO
        d->sorted = true;
#endif
#ifndef DO_SANS_TIME
        d->now = 0;
#endif
        d->vector = NULL;
    }
    return d;
}

void do_destroy(struct do_doer *doer) {
    if (!doer)
        return;

    struct do_work **it;
    for (it = vector_begin(doer->vector); it != vector_end(doer->vector); it++) {
        do_work_destroy(*it);
    }
    vector_free(doer->vector);
    do_free(doer);
}

#ifndef DO_SANS_PRIO

void do_set_prio_changed(struct do_doer *doer) {
    if (doer) {
        doer->sorted = false;
    }
}

#endif


/* Work */
struct do_work *do_work_init() {
    struct do_work *dw = do_malloc(sizeof(*dw));
    if (dw) {
#ifndef DO_SANS_PRIO
        dw->prio = true;
#endif
        dw->pc.pt = DO_PREDICATE_PTR;
        dw->pc.predicate.p = NULL;
        dw->work = NULL;
        dw->data = NULL;
    }
    return dw;
}

void do_work_destroy(struct do_work *work) {
    do_free(work);
}

void do_work_set_work_func(struct do_work *work, work_func work_fn) {
    if (work) {
        work->work = work_fn;
    }
}

void do_work_set_data(struct do_work *work, void *data) {
    if (work) {
        work->data = data;
    }
}

#ifndef DO_SANS_PRIO

void do_work_set_prio(struct do_work *work, size_t prio) {
    if (work) {
        work->prio = prio;
    }
}

#endif

void do_work_set_predicate_ptr(struct do_work *work, bool *predicate_p) {
    if (work) {
        work->pc.pt = DO_PREDICATE_PTR;
        work->pc.predicate.p = predicate_p;
    }
}

void do_work_set_predicate_func(struct do_work *work, returns_true_func predicate_fn) {
    if (work) {
        work->pc.pt = DO_PREDICATE_FUNC;
        work->pc.predicate.fn = predicate_fn;
    }
}

#ifndef DO_SANS_TIME

void do_work_set_predicate_time(struct do_work *work, time_t predicate_tm) {
    if (work) {
        work->pc.pt = DO_PREDICATE_TIME;
        work->pc.predicate.tm = predicate_tm;
    }
}

#endif


/* Convenience initializers */
struct do_work *do_work_if(work_func work_fn, void *data, bool *predicate_p) {
    struct do_work *dw = do_work_init();
    if (dw) {
        do_work_set_work_func(dw, work_fn);
        do_work_set_data(dw, data);
        do_work_set_predicate_ptr(dw, predicate_p);
        return dw;
    }
    return NULL;
}

struct do_work *do_work_when(work_func work_fn, void *data, returns_true_func predicate_fn) {
    struct do_work *dw = do_work_init();
    if (dw) {
        do_work_set_work_func(dw, work_fn);
        do_work_set_data(dw, data);
        do_work_set_predicate_func(dw, predicate_fn);
        return dw;
    }
    return NULL;
}

#ifndef DO_SANS_TIME

struct do_work *do_work_after(work_func work_fn, void *data, time_t tm) {
    struct do_work *dw = do_work_init();
    if (dw) {
        do_work_set_work_func(dw, work_fn);
        do_work_set_data(dw, data);
        do_work_set_predicate_time(dw, tm);
        return dw;
    }
    return NULL;
}

#endif

#ifndef DO_SANS_PRIO

void do_sort(struct do_doer *doer) {
    if (!doer)
        return;

    size_t sz = vector_size(doer->vector);
    for (size_t i = 1; i < sz; ++i) {
        for (size_t j = 0; j < sz - 1; ++j) {
            if (doer->vector[j]->prio > doer->vector[i]->prio)
                vector_swap(doer->vector, j, i, struct do_work *);
        }
    }
}

#endif


/* Lifecycle */
size_t do_loop(struct do_doer *doer) {
    if (!doer)
        return 0;

#ifndef DO_SANS_PRIO
    if (!doer->sorted) {
        do_sort(doer);
        doer->sorted = true;
    }
#endif
#ifndef DO_SANS_TIME
    time(&(doer->now));
#endif
    struct do_work **it;
    size_t i = 0;
    for (it = vector_begin(doer->vector); it != vector_end(doer->vector); i++) {
        bool erased = false;
        if ((*it)->work) {
            bool is_tbd = false;
            switch ((*it)->pc.pt) {
                case DO_PREDICATE_PTR:
                    if ((*it)->pc.predicate.p) {
                        is_tbd = *((*it)->pc.predicate.p);
                    } else {
                        do_not_do(doer, *it);
                        continue;
                    }
                    break;
                case DO_PREDICATE_FUNC:
                    is_tbd = (*it)->pc.predicate.fn((*it)->data);
                    break;
#ifndef DO_SANS_TIME
                case DO_PREDICATE_TIME:
                    is_tbd = (doer->now >= (*it)->pc.predicate.tm);
                    break;
#endif
            }

            if (is_tbd) {
                bool ret = (*it)->work((*it)->data);
                if (ret) {
                    do_not_do(doer, *it);
                    erased = true;
                }
            }
        }
        if (!erased)
            it++;
    }

    return vector_size(doer->vector);
}

bool do_so(struct do_doer *doer, struct do_work *work) {
    if (doer && work) {
        size_t oldsz = vector_size(doer->vector);
        vector_push_back(doer->vector, work);
#ifndef DO_SANS_PRIO
        do_set_prio_changed(doer);
#endif
        if (vector_size(doer->vector) > oldsz) {
            return true;
        }
    }
    return false;
}

#ifndef DO_SANS_TIME

bool expire_work(void *work) {
    if (work) {
        do_work_set_predicate_ptr(work, NULL);
    }
    return true;
}

bool do_so_until(struct do_doer *doer, struct do_work *work, time_t expiry_tm) {
    if (doer && work) {
        struct do_work *expirer = do_work_after(expire_work, work, expiry_tm);
        if (expirer) {
            if (do_so(doer, expirer)) {
                if (do_so(doer, work)) {
                    return true;
                } else {
                    do_work_destroy(work);
                    do_not_do(doer, expirer);
                }
            } else {
                do_work_destroy(expirer);
            }
        }
    }
    return false;
}

#endif

void do_not_do(struct do_doer *doer, struct do_work *work) {
    if (!doer || !work)
        return;

    struct do_work **it;
    size_t i = 0;
    for (it = vector_begin(doer->vector); it != vector_end(doer->vector); i++, it++) {
        if (work == *it) {
            vector_erase(doer->vector, i);
            do_work_destroy(work);
            return;
        }
    }
}


/* Fine tuning */
void do_set_dyn_mem_func(do_malloc_func malloc_func, do_realloc_func realloc_func, do_free_func free_func) {
    do_malloc = malloc_func;
    do_realloc = realloc_func;
    do_free = free_func;
}
