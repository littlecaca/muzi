#ifndef MUZI_BASE_MEM_POOL_H_
#define MUZI_BASE_MEM_POOL_H_

#include <cstdint>
#include <noncopyable.h>

#define PAGESIZE 4096


class FixedMemPool : muzi::noncopyable
{
public:
    FixedMemPool() : chunk_size_(PAGESIZE - 1){}


private:
    uint32_t chunk_size_;
    uint32_t block_size_;
};

#endif  // MUZI_BASE_MEM_POOL_H_
