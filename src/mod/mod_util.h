#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "recompui.h"

#include <string.h>

#ifndef MOD_UTIL_H
#define MOD_UTIL_H

#define log_critical(...) if (logLevel  >= LOG_CRITICAL) {recomp_printf(__VA_ARGS__);}
#define log_error(...) if (logLevel  >= LOG_ERROR) {recomp_printf(__VA_ARGS__);}
#define log_warning(...) if (logLevel  >= LOG_WARNING) {recomp_printf(__VA_ARGS__);}
#define log_info(...) if (logLevel  >= LOG_INFO) {recomp_printf(__VA_ARGS__);}
#define log_debug(...) if (logLevel  >= LOG_DEBUG) {recomp_printf(__VA_ARGS__);}

#define STOP_EXECUTION(...) bool printed = false; while (true) { if (!printed) log_critical(__VA_ARGS__); printed = true; }

void print_bytes(void* addr, int n);

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

// Structs & enums
typedef struct Vector_t 
{
    size_t elementSize;
    u32 numElements;
    u32 capacity;
    void* dataStart;
} Vector;

typedef enum VecError_t
{
    VEC_SUCCESS,
    VEC_ERR_EMPTY,
    VEC_ERR_MAX_CAPACITY = UINT32_MAX
} VecError;

typedef enum VecResize_t
{
    VEC_SHRINK = false,
    VEC_GROW = true
} VecResize;

// Functions
// Internal
u32 _vec_resize(Vector* self, bool shrink);

// Public
Vector* vec_init(size_t elementSize);
u32 vec_push_back(Vector* self, void* data);
u32 vec_erase(Vector* self, int idx);
u32 vec_pop(Vector* self, void* addr);
u32 vec_pop_at(Vector* self, void* addr, int idx);

void vec_errmsg(int err);
void vec_printData(Vector* self);

void vec_teardown(Vector* self);

// Specific use case: Category Sequences
Vector* CAT_SEQ_INIT(int seqIdArray[], size_t size);

#endif