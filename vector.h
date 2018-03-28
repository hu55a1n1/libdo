/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Evan Teran
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include <stddef.h> /* for size_t */
#include <assert.h> /* for assert */

/**
 * @brief vector_set_capacity - For internal use, sets the capacity variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_capacity(vec, size) \
do { \
    if(vec) { \
        ((size_t *)(vec))[-1] = (size); \
    } \
} while(0)

/**
 * @brief vector_set_size - For internal use, sets the size variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_size(vec, size) \
do { \
    if(vec) { \
        ((size_t *)(vec))[-2] = (size); \
    } \
} while(0)

/**
 * @brief vector_capacity - gets the current capacity of the vector
 * @param vec - the vector
 * @return the capacity as a size_t
 */
#define vector_capacity(vec) \
    ((vec) ? ((size_t *)(vec))[-1] : (size_t)0)

/**
 * @brief vector_size - gets the current size of the vector
 * @param vec - the vector
 * @return the size as a size_t
 */
#define vector_size(vec) \
    ((vec) ? ((size_t *)(vec))[-2] : (size_t)0)

/**
 * @brief vector_empty - returns non-zero if the vector is empty
 * @param vec - the vector
 * @return non-zero if empty, zero if non-empty
 */
#define vector_empty(vec) \
    (vector_size(vec) == 0)

/**
 * @brief vector_grow - For internal use, ensures that the vector is at least <count> elements big
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_grow(vec, count) \
do { \
    if(!(vec)) { \
        size_t *_p_ = do_malloc((count) * sizeof(*(vec)) + (sizeof(size_t) * 2)); \
        assert(_p_); \
        (vec) = (void *)(&_p_[2]); \
        vector_set_capacity((vec), (count)); \
        vector_set_size((vec), 0); \
    } else { \
        size_t *_p1_ = &((size_t *)(vec))[-2]; \
        size_t *_p2_ = do_realloc(_p1_, ((count) * sizeof(*(vec))+ (sizeof(size_t) * 2))); \
        assert(_p2_); \
        (vec) = (void *)(&_p2_[2]); \
        vector_set_capacity((vec), (count)); \
    } \
} while(0)

/**
 * @brief vector_pop_back - removes the last element from the vector
 * @param vec - the vector
 * @return void
 */
#define vector_pop_back(vec) \
do { \
    vector_set_size((vec), vector_size(vec) - 1); \
} while(0)

/**
 * @brief vector_erase - removes the element at index i from the vector
 * @param vec - the vector
 * @param i - index of element to remove
 * @return void
 */
#define vector_erase(vec, i) \
do { \
    if (vec) { \
        size_t _sz_ = vector_size(vec); \
        if ((i) < _sz_) { \
            vector_set_size((vec), _sz_ - 1); \
            size_t x; \
            for (x = i; x < (_sz_ - 1); ++x) { \
                (vec)[x] = (vec)[x + 1]; \
            } \
            (i)--; \
        } \
    } \
} while(0)

/**
 * @brief vector_free - frees all memory associated with the vector
 * @param vec - the vector
 * @return void
 */
#define vector_free(vec) \
do { \
    if(vec) { \
        size_t *p1 = &((size_t *)(vec))[-2]; \
        do_free(p1); \
    } \
} while(0)

/**
 * @brief vector_swap - swaps element at indices i and j
 * @param vec - the vector
 * @param i - index of element to be swapped
 * @param j - index of element to swap with
 * @return void
 */
#define SWAP(x, y, T) do { T SWAP = x; (x) = y; (y) = SWAP; } while (0)
#define vector_swap(vec, i, j, T) \
do { \
    if(vec) { \
        size_t _sz_ = vector_size(vec); \
        if((i) < _sz_ && (j) < _sz_) { \
            SWAP((vec)[i], (vec)[j], T); \
        } \
    } \
} while(0)


/**
 * @brief vector_begin - returns an iterator to first element of the vector
 * @param vec - the vector
 * @return a pointer to the first element
 */
#define vector_begin(vec) \
    (vec)

/**
 * @brief vector_end - returns an iterator to one past the last element of the vector
 * @param vec - the vector
 * @return a pointer to one past the last element
 */
#define vector_end(vec) \
    &((vec)[vector_size(vec)])


/**
 * @brief vector_push_back - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#ifdef LOGARITHMIC_GROWTH

#define vector_push_back(vec, value) \
do { \
    size_t _cap_ = vector_capacity(vec); \
    if(_cap_ <= vector_size(vec)) { \
        vector_grow((vec), !_cap_ ? _cap_ + 1 : _cap_ * 2); \
    } \
    vec[vector_size(vec)] = (value); \
    vector_set_size((vec), vector_size(vec) + 1); \
} while(0)

#else

#define vector_push_back(vec, value) \
do { \
    size_t _cap_ = vector_capacity(vec); \
    if(_cap_ <= vector_size(vec)) { \
        vector_grow((vec), _cap_ + 1); \
    } \
    (vec)[vector_size(vec)] = (value); \
    vector_set_size((vec), vector_size(vec) + 1); \
} while(0)

#endif

#endif
