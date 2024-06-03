

#ifndef DOKO_DARRAY_H
#define DOKO_DARRAY_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define BINARY_SEARCH_INSERT_INDEX(arr, size, target, index)                   \
    do {                                                                       \
        size_t low = 0, high = (size_t)(size) - 1, mid = 0;                    \
        while (low <= high) {                                                  \
            mid = low + (high - low) / 2;                                      \
            if ((arr)[mid] == (target)) {                                      \
                break;                                                         \
            }                                                                  \
            if ((arr)[mid] < (target)) {                                       \
                low = mid + 1;                                                 \
            } else {                                                           \
                if (mid == 0)                                                  \
                    break;                                                     \
                high = mid - 1;                                                \
            }                                                                  \
        }                                                                      \
        (index) = mid;                                                         \
    } while (0)

#define DARRAY_DEF(name, type)                                                 \
    typedef struct name {                                                      \
            type   buffer;                                                     \
            size_t length;                                                     \
            size_t size;                                                       \
    } name##_t;

#define DARRAY_FREE(darray)                                                    \
    do {                                                                       \
        if ((darray).buffer != NULL) {                                         \
            free((darray).buffer);                                             \
            (darray).buffer = NULL;                                            \
            (darray).size   = 0;                                               \
            (darray).length = 0;                                               \
        }                                                                      \
    } while (0)

#define DARRAY_INIT(darray, size_initial)                                      \
    do {                                                                       \
        (darray).size   = 0;                                                   \
        (darray).length = (size_initial);                                      \
        (darray).buffer = calloc(sizeof((darray).buffer[0]), (size_initial));  \
        if ((darray).buffer == NULL) {                                         \
            fprintf(                                                           \
                stderr, "Cannot malloc dynamic array: %s", strerror(errno)     \
            );                                                                 \
            exit(1);                                                           \
        }                                                                      \
    } while (0);

#define DARRAY_GROW_SIZE_TO(darray, size_)                                     \
    do {                                                                       \
        if ((size_) > (darray).length) {                                       \
            if ((darray).length == 0)                                          \
                (darray).length = 32;                                          \
            while ((size_) > (darray).length)                                  \
                (darray).length *= 2;                                          \
            (darray).buffer = realloc(                                         \
                (darray).buffer, sizeof((darray).buffer[0]) * (darray).length  \
            );                                                                 \
            if ((darray).buffer == NULL) {                                     \
                fprintf(                                                       \
                    stderr, "Cannot realloc dynamic array: %s",                \
                    strerror(errno)                                            \
                );                                                             \
                exit(1);                                                       \
            }                                                                  \
            /* you have to zero out when using realloc */                      \
            memset(                                                            \
                (darray).buffer + (darray).size, 0,                            \
                (sizeof((darray).buffer[0])) *                                 \
                    ((darray).length - (darray.size))                          \
            );                                                                 \
            (darray).size = (size_);                                           \
        }                                                                      \
    } while (0)

#define DARRAY_APPEND(darray, item)                                            \
    do {                                                                       \
        if ((darray).size >= (darray).length) {                                \
            if ((darray).length == 0)                                          \
                (darray).length = 32;                                          \
            (darray).length *= 2;                                              \
            (darray).buffer = realloc(                                         \
                (darray).buffer, sizeof((darray).buffer[0]) * (darray).length  \
            );                                                                 \
            if ((darray).buffer == NULL) {                                     \
                fprintf(                                                       \
                    stderr, "Cannot realloc dynamic array: %s",                \
                    strerror(errno)                                            \
                );                                                             \
                exit(1);                                                       \
            }                                                                  \
            /* you have to zero out when using realloc */                      \
            memset(                                                            \
                (darray).buffer + (darray).size, 0,                            \
                (sizeof((darray).buffer[0])) *                                 \
                    ((darray).length - (darray.size))                          \
            );                                                                 \
        }                                                                      \
        (darray).buffer[(darray).size++] = (item);                             \
    } while (0)

#define DARRAY_FOR_EACH(darray, iter)                                          \
    for (size_t(iter) = 0; (iter) < (darray).size; ++(iter))

#define DARRAY_FOR_EACH_I(darray, iter)                                        \
    for (; (iter) < (darray).size; ++(iter))

#define DARRAY_FOR_I(darray, iter, until)                                      \
    for (; (iter) < (darray).size && (iter) < (until); ++(iter))

#endif
