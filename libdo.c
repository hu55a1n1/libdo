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
#include <stdlib.h> /* malloc, realloc, free */
#include <stdint.h> /* SIZE_MAX */
#include "libdo.h"
#include "vector.h"

#undef malloc
#undef realloc
#undef free

static do_malloc_func do_malloc = malloc;
static do_realloc_func do_realloc = realloc;
static do_free_func do_free = free;

static void do_remove(struct do_doer *doer, struct do_work *work);

union predicate {
    bool *p;
    returns_true_func fn;
    time_t tm;
};

enum predicate_type {
    DO_PREDICATE_PTR,
    DO_PREDICATE_FUNC,
    DO_PREDICATE_TIME
};

struct predicate_container {
    enum predicate_type pt;
    union predicate predicate;
};

struct do_work {
    size_t prio;
    struct predicate_container pc;
    work_func work_fn;
    void *data;
};

struct do_doer {
    bool sorted;
    struct do_work **vector;
};


/* Doer */
struct do_doer *do_init() {
    struct do_doer *d = (struct do_doer *) do_malloc(sizeof(*d));
    if (d) {
        d->sorted = true;
        d->vector = NULL;
    }
    return d;
}

void do_destroy(struct do_doer *doer) {
    struct do_work **it;
    if (!doer) {
        return;
    }
    for (it = vector_begin(doer->vector); it != vector_end(doer->vector); it++) {
        do_work_destroy(*it);
    }
    vector_free(doer->vector);
    do_free(doer);
}


void do_set_prio_changed(struct do_doer *doer) {
    if (doer) {
        doer->sorted = false;
    }
}


/* Work */
struct do_work *do_work_init() {
    struct do_work *work = (struct do_work *) do_malloc(sizeof(*work));
    if (work) {
        work->prio = SIZE_MAX;
        work->pc.pt = DO_PREDICATE_PTR;
        work->pc.predicate.p = NULL;
        work->work_fn = NULL;
        work->data = NULL;
    }
    return work;
}

void do_work_destroy(struct do_work *work) {
    do_free(work);
}

void do_work_set_work_func(struct do_work *work, work_func work_fn) {
    if (work) {
        work->work_fn = work_fn;
    }
}

void do_work_set_data(struct do_work *work, void *data) {
    if (work) {
        work->data = data;
    }
}

void do_work_set_prio(struct do_work *work, size_t prio) {
    if (work) {
        if (prio < 1) {
            prio = 1;
        }
        work->prio = prio;
    }
}

void do_work_set_predicate_ptr(struct do_work *work, bool *predicate_p) {
    if (work && predicate_p) {
        work->pc.pt = DO_PREDICATE_PTR;
        work->pc.predicate.p = predicate_p;
    }
}

void do_work_set_predicate_ptr_null(struct do_work *work) {
    if (work) {
        work->pc.pt = DO_PREDICATE_PTR;
        work->pc.predicate.p = NULL;
    }
}

void do_work_set_predicate_func(struct do_work *work, returns_true_func predicate_fn) {
    if (work) {
        work->pc.pt = DO_PREDICATE_FUNC;
        work->pc.predicate.fn = predicate_fn;
    }
}

void do_work_set_predicate_time(struct do_work *work, time_t predicate_tm) {
    if (work) {
        work->pc.pt = DO_PREDICATE_TIME;
        work->pc.predicate.tm = predicate_tm;
    }
}


/* Convenience initializers */
struct do_work *do_work_if(work_func work_fn, void *data, bool *predicate_p) {
    struct do_work *work = do_work_init();
    if (work) {
        do_work_set_work_func(work, work_fn);
        do_work_set_data(work, data);
        do_work_set_predicate_ptr(work, predicate_p);
        return work;
    }
    return NULL;
}

struct do_work *do_work_when(work_func work_fn, void *data, returns_true_func predicate_fn) {
    struct do_work *work = do_work_init();
    if (work) {
        do_work_set_work_func(work, work_fn);
        do_work_set_data(work, data);
        do_work_set_predicate_func(work, predicate_fn);
        return work;
    }
    return NULL;
}

struct do_work *do_work_after(work_func work_fn, void *data, time_t tm) {
    struct do_work *work = do_work_init();
    if (work) {
        do_work_set_work_func(work, work_fn);
        do_work_set_data(work, data);
        do_work_set_predicate_time(work, tm);
        return work;
    }
    return NULL;
}

void do_sort(struct do_doer *doer) {
    size_t sz, i, j;
    if (!doer) {
        return;
    }
    sz = vector_size(doer->vector);
    for (i = 1; i < sz; ++i) {
        for (j = 0; j < sz - 1; ++j) {
            if (doer->vector[j]->prio > doer->vector[i]->prio) {
                vector_swap(doer->vector, j, i, struct do_work *);
            }
        }
    }
}


/* Lifecycle */
size_t do_loop(struct do_doer *doer) {
    struct do_work **it, **itbegin, **itend;
    size_t i = 0, oldsz = 0;
    time_t now_tm = time(NULL);
    if (!doer) {
        return 0;
    }
    if (!doer->sorted) {
        do_sort(doer);
        doer->sorted = true;
    }
    itbegin = vector_begin(doer->vector);
    itend = vector_end(doer->vector);
    oldsz = vector_size(doer->vector);
    for (it = itbegin; it != itend; it++, i++) {
        bool is_tbd = false;
        switch ((*it)->pc.pt) {
            case DO_PREDICATE_PTR:
                if ((*it)->pc.predicate.p) {
                    is_tbd = *((*it)->pc.predicate.p);
                } else {
                    do_not_do(doer, *it);
                }
                break;
            case DO_PREDICATE_FUNC:
                is_tbd = (*it)->pc.predicate.fn((*it)->data);
                break;
            case DO_PREDICATE_TIME:
                is_tbd = (now_tm >= (*it)->pc.predicate.tm);
                break;
        }

        if (is_tbd) {
            if ((*it)->work_fn && (*it)->work_fn((*it)->data)) {
                if (vector_size(doer->vector) > oldsz) {
                    /* Adjust iterators */
                    size_t j = i;
                    it = vector_begin(doer->vector);
                    while (j--) it++;
                    itend = vector_begin(doer->vector);
                    j = oldsz;
                    while (j--) itend++;
                }
                do_not_do(doer, *it);
            }
        }
    }
    for (it = vector_begin(doer->vector); it != vector_end(doer->vector);) {
        if ((*it)->pc.pt == DO_PREDICATE_PTR && !(*it)->pc.predicate.p) {
            do_remove(doer, *it);
            continue;
        }
        it++;
    }
    return vector_size(doer->vector);
}

bool do_so(struct do_doer *doer, struct do_work *work) {
    if (doer && work) {
        size_t oldsz = vector_size(doer->vector);
        vector_push_back(doer->vector, work, struct do_work *);
        do_set_prio_changed(doer);
        if (vector_size(doer->vector) > oldsz) {
            return true;
        }
    }
    return false;
}

bool expire_work(void *work) {
    if (work) {
        do_work_set_predicate_ptr_null((struct do_work *) work);
    }
    return true;
}

bool do_so_until(struct do_doer *doer, struct do_work *work, time_t expiry_tm) {
    struct do_work *expirer;
    if (!doer || !work) {
        return false;
    }
    expirer = do_work_after(expire_work, work, expiry_tm);
    if (expirer) {
        expirer->prio = 0;
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
    return false;
}

void do_not_do(struct do_doer *doer, struct do_work *work) {
    (void) doer;
    do_work_set_predicate_ptr_null(work);
}

void do_remove(struct do_doer *doer, struct do_work *work) {
    struct do_work **it;
    size_t i;
    if (!doer || !work) {
        return;
    }
    i = 0;
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
