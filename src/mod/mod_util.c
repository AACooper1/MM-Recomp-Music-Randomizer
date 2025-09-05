#include "mod_util.h"

void print_bytes(void* addr, int n)
{
        recomp_printf("Data starting from %p is:\n\n", addr);
        recomp_printf("\t\t00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n\n");

        uintptr_t addrInt = (uintptr_t) addr;
        recomp_printf("%08x\t", addrInt);

        for (int i = 0; i < n; i++)
        {
            if (i % 16 == 0 && i != 0) 
            {
                recomp_printf("\n%08x\t", addrInt + i);
            }
            
            recomp_printf("%02x ", *(unsigned char*)(addr + i));
        }
        recomp_printf("\n");
}


// Dynamic array (C++ vector) implementation

// Specific use case: Category Sequences
Vector* CAT_SEQ_INIT(int seqIdArray[], size_t size, Vector** SeqCatArray,  int seqCatIdx)
{
    Vector* seqVec = vec_init(sizeof(int)); 

    int rc;

    for (size_t i = 0; i < size; i++) 
    { 
        if( (rc = vec_push_back(seqVec, &(seqIdArray[i]))) != VEC_SUCCESS )
        {
            vec_errmsg(rc);
        }
        if( (rc = vec_push_back(SeqCatArray[seqIdArray[i]], &seqCatIdx)) != VEC_SUCCESS )
        {
            vec_errmsg(rc);
        }
    }

    return seqVec;
}

// Resize the vector. Intended for internal use.
u32 _vec_resize(Vector* self, bool grow_or_shrink)
{
    if (grow_or_shrink)
    {
        if (self->capacity / self->elementSize >= UINT32_MAX)
        {
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
            self->capacity /= 2;
        }
    }

    // Allocate new table, initialize to 0, copy and free old table.
    void* new_data = recomp_alloc(self->elementSize * self->capacity);
    Lib_MemSet(new_data, 0, self->elementSize * self->capacity);
    if (self->capacity > 1)
    {
        Lib_MemCpy(new_data, self->dataStart, self->elementSize * self->capacity);
        recomp_free(self->dataStart);
    }
    self->dataStart = new_data;

    return VEC_SUCCESS;
}

// Initialize the vector. Allocates 0 bytes of data until element is pushed.
Vector* vec_init(size_t elementSize)
{
    Vector* self = (Vector*)recomp_alloc(sizeof(Vector));

    self->elementSize = elementSize;
    self->numElements = 0;
    self->capacity = 0;
    self->dataStart = recomp_alloc(self->capacity);

    return self;
}

// Add an element to the end of the vector. Θ(1)
u32 vec_push_back(Vector* self, void* data)
{
    int rc;

    // Resize vector (or chastise user) if full
    while (self->numElements >= self->capacity)
    {
        if((rc = _vec_resize(self, VEC_GROW)) != VEC_SUCCESS)
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
u32 vec_pop(Vector* self, void* addr)
{
    return vec_pop_at(self, addr, 0);
}

// Erase the element at index idx.
u32 vec_erase(Vector* self, int idx)
{
    void* dummyAddr = recomp_alloc(self->elementSize);
    u32 rc = vec_pop_at(self, dummyAddr, idx);
    recomp_free(dummyAddr);

    return rc;
}

// Pop the element with the highest memory address. Use for stack-like structures.
// Use this over vec_pop where possible, as dynamic array stack pop is Θ(1).
u32 vec_pop_back(Vector* self, void* addr)
{
    // If empty, return error.
    if (self->numElements == 0)
    {
        return VEC_ERR_EMPTY;
    }

    // Copy the last element to the output address and zero it out
    Lib_MemCpy(addr, &(self->dataStart[self->elementSize * (self->numElements - 1)]), self->elementSize);
    Lib_MemSet(&(self->dataStart[self->elementSize * (self->numElements - 1)]), 0, self->elementSize);
    self->numElements--;

    int rc;
    if (self->numElements < self->capacity / 2)
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
u32 vec_pop_at(Vector* self, void* addr, int idx)
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
        if (self->numElements < self->capacity / 2)
        {
            if((rc = _vec_resize(self, VEC_SHRINK)) != VEC_SUCCESS)
            {
                return rc;
            }
        }

        return VEC_SUCCESS;
    }
}

u32 vec_at(Vector* self, size_t idx, void* addr)
{
    if (idx >= self->numElements || idx < 0)
    {
        return VEC_OUT_OF_RANGE;
    }
    
    Lib_MemCpy(addr, self->dataStart + (self->elementSize * idx), self->elementSize);

    return VEC_SUCCESS;
}

// Concatenate a vector onto tgt. Does not destroy src, remember to free when you are done.
u32 vec_concat(Vector* tgt, Vector* src)
{
    if (src->elementSize != tgt->elementSize)
    {
        return VEC_ERR_DIFFERENT_DATA_TYPES;
    }

    while (tgt->capacity < src->numElements + tgt->numElements)
    {
        _vec_resize(tgt, VEC_GROW);
    }

    if (src->numElements > 0)
    {
        Lib_MemCpy(tgt->dataStart + (tgt->numElements * tgt->elementSize), src->dataStart, src->numElements * src->elementSize);
    }
    else
    {
        log_warning("[Warning] src vector had no elements, tgt will remain unmodified.\n")
    }

    tgt->numElements += src->numElements;

    return VEC_SUCCESS;
}

// Randomize a vector in-place using the Fisher-Yates Algorithm.
u32 vec_randomize(Vector* self)
{
    if (self->numElements == 0)
    {
        return VEC_ERR_EMPTY;
    }
    else if (self->numElements == 1)
    {
        return VEC_SUCCESS;
    }
    else
    {
        void* tempData = recomp_alloc(self->elementSize);

        for (int i = self->numElements - 1; i > 0; i--)
        {
            u32 rand_idx = Rand_Next() % i;
            Lib_MemCpy(tempData, self->dataStart + (i * self->elementSize), self->elementSize);
            memmove(self->dataStart + (i * self->elementSize), self->dataStart + (rand_idx * self->elementSize), self->elementSize);
            Lib_MemCpy(self->dataStart + (rand_idx * self->elementSize), tempData, self->elementSize);
        }
        
        recomp_free(tempData);
    }

    return VEC_SUCCESS;
}

// Prints info about the vector, followed by the data it points to, in chunks of size elementSize.
void vec_printData(Vector* self)
{
    recomp_printf("\n");
    recomp_printf("Vector at address %p:\n", self);
    recomp_printf("\t   elementSize: %i\n", self->elementSize);
    recomp_printf("\t   numElements: %i\n", self->numElements);
    recomp_printf("\t   capacity: %i\n", self->capacity);
    recomp_printf("\t   Data stored at address %p:\n", self->dataStart);
    for (u32 i = 0; i < self->numElements * self->elementSize; i++)
    {
        if(i % (4 * self->elementSize)  == 0)
        {
            recomp_printf("\t");
        }
        if(i % self->elementSize == 0)
        {
            recomp_printf("\t0x");
        }
        recomp_printf("%02x", *(unsigned char*)(self->dataStart + i));
        recomp_printf("");
        if(i % (4 * self->elementSize) == (4 * self->elementSize) - 1)
        {
            recomp_printf("\n");
        }
    }
    recomp_printf("\n");
}

void vec_errmsg(int err)
{
    switch (err)
    {
        case VEC_SUCCESS:
            log_error("Operation completed successfully.\n");
            break;

        case VEC_ERR_EMPTY:
            log_error("Vector is empty.\n");
            break;

        case VEC_ERR_MAX_CAPACITY:
            log_error("\
                [ERROR]  Why are you trying to assign %i bytes of memory to a single vector \n\
                [ERROR]  is this. a cry for help \n\
                [ERROR]  I'm sorry but i'm not going to let you do this to yourself :/\n\
                ", 
                err
            );
            break;
        case VEC_ERR_DIFFERENT_DATA_TYPES:
            log_error("vec1 and vec2 mush have the same data sizes.\n");
            break;
        }
}

// Deconstructor for vectors. Frees the memory at its data address and then frees the memory of its struct.
void vec_teardown(Vector* self)
{
    if (self->numElements > 0)
    {
        recomp_free(self->dataStart);
    }
    recomp_free(self);
}