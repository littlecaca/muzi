#ifndef MUZI_BASE_MEM_POOL_H_
#define MUZI_BASE_MEM_POOL_H_

#include <cstdint>
#include <noncopyable.h>

#define BLOCKSIZE 4096
#define CHUNKSIZE 49152 // 4096 * 12

namespace muzi
{

class FixedMemPool : muzi::noncopyable
{
public:
    FixedMemPool() : chunk_size_(CHUNKSIZE), block_size_(BLOCKSIZE),
        chunks_(nullptr), idle_nodes_(nullptr)
    {
        AllocChunk();
    }

    void *Alloc();
    void Release(void *ptr);

    ~FixedMemPool() noexcept
    {
        Clear();
    }

private:
    void *AllocChunk();
    void Clear();

private:
    struct Chunk
    {
        Chunk *next;
        char *last;
        char *end;
        char data[];
    };

    struct Node
    {
        Node *next;
    };

    uint32_t chunk_size_;
    uint32_t block_size_;

    Chunk *chunks_;
    Node *idle_nodes_;
};

}   // namespace muzi

#endif  // MUZI_BASE_MEM_POOL_H_
