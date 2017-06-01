Collection of simple to use C++ header only utilities.

1. Fixed Sized Block Alocator - BM::BlockAllocator<kBlockSize, kBlockCount> - The util allocates kBlockCount blocks of size kBlockSize.  Use the allocate() member function to get a block, free(void* ptr) to return the block to the pool.  When all blocks are used, allocate() returns nullptr.  Does not grow.
   
2. Atomic Spin Lock - BM::AtomicSpinLock - Mutex replacement class that uses atomic<bool> and compare_and_exchange to provide a lock-free mutex.
   
More to come.
