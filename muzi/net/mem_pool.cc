#include "mem_pool.h"

#include <cstdlib>
#include <stdint.h>

#define ALIGIN (sizeof(unsigned long))
#define ptr_aligin(p) (((uintptr_t)(p) + (ALIGIN - 1)) & ~(ALIGIN - 1))

namespace muzi
{
void *FixedMemPool::Alloc()
{
    void *ret;
    if (idle_nodes_ != nullptr)
    {
        ret = idle_nodes_;
        idle_nodes_ = idle_nodes_->next;
        return ret;
    }

    if (chunks_ == nullptr && AllocChunk() == nullptr) return nullptr;

    ret = (void *)ptr_aligin(chunks_->last);

    if (chunks_->end - (char *)ret < block_size_)
    {
        AllocChunk();
    }

    chunks_->last = (char *)ret + block_size_;
    return ret;
}

void FixedMemPool::Release(void *ptr)
{
    if (ptr == nullptr) return;

    ((Node *)ptr)->next = idle_nodes_;
    idle_nodes_ = (Node *)ptr;
}

void *FixedMemPool::AllocChunk()
{
    Chunk *chunk = (Chunk *)malloc(chunk_size_);
    if (chunk == nullptr) return nullptr;
    chunk->last = chunk->data;
    chunk->end = (char *)chunk + chunk_size_;
    chunk->next = nullptr;

    if (chunks_ == nullptr)
        chunks_ = chunk;
    else
    {
        chunk->next = chunks_;
        chunks_ = chunk;
    }
    return chunk;
}

void FixedMemPool::Clear()
{
    while (!chunks_)
    {
        auto temp = chunks_;
        free(temp);
        chunks_ = chunks_->next;
    }
}

}   // namesapce muzi
