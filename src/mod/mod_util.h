#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include <string.h>
#include <limits.h>

#ifndef MOD_UTIL_H
#define MOD_UTIL_H

#define log_critical(...) if (logLevel  >= LOG_CRITICAL) {recomp_printf(__VA_ARGS__);}
#define log_error(...) if (logLevel  >= LOG_ERROR) {recomp_printf(__VA_ARGS__);}
#define log_warning(...) if (logLevel  >= LOG_WARNING) {recomp_printf(__VA_ARGS__);}
#define log_info(...) if (logLevel  >= LOG_INFO) {recomp_printf(__VA_ARGS__);}
#define log_debug(...) if (logLevel  >= LOG_DEBUG) {recomp_printf(__VA_ARGS__);}

enum log_level_t
{
    LOG_NOTHING,
    LOG_CRITICAL,
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
};

extern int logLevel;


// Dynamic array (C++ vector) implementation

// Specific use case: 

// Structs & enums
typedef struct Vector_t 
{
    size_t elementSize;
    int numElements;
    int capacity;
    void* dataStart;
} Vector;

typedef enum VecError_t
{
    VEC_SUCCESS,
    VEC_ERR_EMPTY,
    VEC_ERR_MAX_CAPACITY = INT_MAX
} VecError;

typedef enum VecResize_t
{
    VEC_SHRINK = false,
    VEC_GROW = true
} VecResize;

// Functions
// Internal
int _vec_resize(Vector* self, bool shrink);

// Public
Vector* vector_init(size_t elementSize);
int vec_push_back(Vector* self, void* data);
int vec_erase(Vector* self, int idx);
int vec_pop(Vector* self, void* addr);
int vec_pop_at(Vector* self, void* addr, int idx);
void vec_teardown(Vector* self);

#endif