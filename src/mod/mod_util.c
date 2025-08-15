#include "mod_util.h"


// Dynamic array (C++ vector) implementation

// Resize the vector. Intended for internal use.
u32 _vec_resize(Vector* self, bool shrink)
{
    if (!shrink)
    {
        if (self->capacity >= INT_MAX)
            {
                log_error("\
                    [ERROR]  Why are you trying to assign %i bytes of memory to a single vector \n\
                    [ERROR]  is this. a cry for help \n\
                    [ERROR]  I'm sorry but i'm not going to let you do this to yourself :/\n\
                    ", 
                    self->elementSize * self->numElements
                );
                return VEC_ERR_MAX_CAPACITY;
            }
        else if (self->capacity == 0)
        {
            self->capacity = 1;
        }
        else 
        {
            self->capacity *= 2;
        }
    }
    else
    {
        if (self->capacity == 0)
        {
            return VEC_ERR_EMPTY;
        }
        else if (self->capacity == 1)
        {
            self->capacity = 0;
        }
        else 
        {
            self->capacity *= 2;
        }
    }

    // Allocate new table, initialize to 0, copy and free old table.
    void* new_data = recomp_alloc(self->elementSize * self->capacity);
    Lib_MemSet(new_data, 0, self->elementSize * self->capacity);
    Lib_MemCpy(new_data, self->dataStart, self->elementSize * self->capacity);
    recomp_free(self->dataStart);
    self->dataStart = new_data;

    return VEC_SUCCESS;
}

// Initialize the vector. Allocates 0 bytes of data until element is pushed.
Vector* vector_init(size_t elementSize)
{
    Vector* self = (Vector*)recomp_alloc(sizeof(Vector));

    self->elementSize = elementSize;
    self->numElements = 0;
    self->capacity = 0;
    self->dataStart = recomp_alloc(self->capacity);
}

// Add an element to the end of the vector. Θ(1)
int vec_push_back(Vector* self, void* data)
{
    int rc;

    // Resize vector (or chastise user) if full
    while (self->numElements >= self->capacity)
    {
        if(!(rc = _vec_resize(self, VEC_GROW)) == VEC_SUCCESS)
        {
            return rc;
        }
    }
    
    // Add the element
    Lib_MemCpy(&(self->dataStart[self->elementSize * self->numElements]), data, self->elementSize);

    self->numElements++;

    return rc;
}

// Pop the element with the lowest memory address. Use for queue-like structures.
// Use vec_pop_back where possible, as dynamic array dequeue is Θ(n).
int vec_pop(Vector* self, void* addr)
{
    return vec_pop_at(self, addr, 0);
}

// Erase the element at index idx.
int vec_erase(Vector* self, int idx)
{
    void* dummyAddr = recomp_alloc(self->elementSize);
    vec_pop_at(self, dummyAddr, idx);
    recomp_free(dummyAddr);
}

// Pop the element with the highest memory address. Use for stack-like structures.
// Use this over vec_pop where possible, as dynamic array dequeue is Θ(n).
int vec_pop_back(Vector* self, void* addr)
{
    // If empty, return error.
    if (self->numElements == 0)
    {
        return VEC_ERR_EMPTY;
    }

    // Copy the last element to the output address and zero it out
    Lib_MemCpy(addr, &(self->dataStart[self->elementSize * (self->numElements - 1)]), self->elementSize);
    Lib_MemSet(&(self->dataStart[self->elementSize * (self->numElements - 1)]), self->elementSize);
    self->numElements--;

    int rc;
    while (self->numElements < self->capacity / 2)
    {
        if((rc = _vec_resize(self, VEC_SHRINK)) != VEC_SUCCESS)
        {
            return rc;
        }
    }

    return VEC_SUCCESS;
}

// Pop the element with index idx.
// Use vec_pop_back where position is not important (e.g. random index), as dynamic array mutate at beginning/middle is Θ(n).
int vec_pop_at(Vector* self, void* addr, int idx)
{
    int rc;

    // If empty, return error.
    if (self->numElements == 0)
    {
        return VEC_ERR_EMPTY;
    }
    else
    {
        // Copy the element at index idx to the output address
        Lib_MemCpy(addr, &(self->dataStart[self->elementSize * idx]), self->elementSize);

        // Move all elements from idx back by one, zero out the last element, and decrement the number of elements.
        memmove(&(self->dataStart[idx * self->elementSize]), &(self->dataStart[ (idx + 1) * self->elementSize]), self->numElements - 1);

        (self->numElements)--;

        // Shrink the vector if numElements drops below preceding power of 2.
        while (self->numElements < self->capacity / 2)
        {
            if(!(rc = _vec_resize(self, VEC_SHRINK)) == VEC_SUCCESS)
            {
                return rc;
            }
        }
    }
}

void vec_teardown(Vector* self)
{
    recomp_free(self->dataStart);
}