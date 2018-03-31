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
#ifndef LIB_DO_H
#define LIB_DO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>   /* size_t */

#if defined(__STDC__)
# define C89
# if defined(__STDC_VERSION__)
#  define C90
#  if (__STDC_VERSION__ >= 199409L)
#   define C94
#  endif /* C94 */
#  if (__STDC_VERSION__ >= 199901L)
#   define C99
#  endif /* C99 */
# endif /* C90 */
#endif /* C89 */

#if defined(C99)

# include <stdbool.h>

#else
typedef enum { false = 0, true = !false } bool;
#endif

#ifndef DO_SANS_TIME

# include <time.h>

#endif

/* Types */
typedef void *(*do_malloc_func)(size_t);

typedef void *(*do_realloc_func)(void *, size_t);

typedef void (*do_free_func)(void *);

typedef bool (*work_func)(void *);

typedef bool (*returns_true_func)(void *);


/* Opaque structs */
struct do_doer;

struct do_work;


/* Doer */
struct do_doer *do_init();

void do_destroy(struct do_doer *doer);

#ifndef DO_SANS_PRIO

void do_set_prio_changed(struct do_doer *doer);

#endif


/* Work */
struct do_work *do_work_init();

void do_work_destroy(struct do_work *work);

void do_work_set_work_func(struct do_work *work, work_func work_fn);

void do_work_set_data(struct do_work *work, void *data);

#ifndef DO_SANS_PRIO

void do_work_set_prio(struct do_work *work, size_t prio);

#endif

void do_work_set_predicate_ptr(struct do_work *work, bool *predicate_p);

void do_work_set_predicate_func(struct do_work *work, returns_true_func predicate_fn);

#ifndef DO_SANS_TIME

void do_work_set_predicate_time(struct do_work *work, time_t predicate_tm);

#endif

/* Convenience initializers */
struct do_work *do_work_if(work_func work_fn, void *data, bool *predicate_p);

struct do_work *do_work_when(work_func work_fn, void *data, returns_true_func predicate_fn);

#ifndef DO_SANS_TIME

struct do_work *do_work_after(work_func work_fn, void *data, time_t tm);

#endif


/* Lifecycle */
size_t do_loop(struct do_doer *doer);

bool do_so(struct do_doer *doer, struct do_work *work);

#ifndef DO_SANS_TIME

bool do_so_until(struct do_doer *doer, struct do_work *work, time_t expiry_tm);

#endif

void do_not_do(struct do_doer *doer, struct do_work *work);


/* Fine tuning */
void do_set_dyn_mem_func(do_malloc_func malloc_func, do_realloc_func realloc_func, do_free_func free_func);

#ifdef __cplusplus
}
#endif

#endif /* LIB_DO_H */
