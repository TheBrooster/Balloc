Collection of simple to use C++ header only utilities.

1. Fixed Sized Block Alocator - bm::BlockAllocator<kBlockSize, kBlockCount> - The util allocates kBlockCount blocks of size kBlockSize.  Use the allocate() member function to get a block, free(void* ptr) to return the block to the pool.  When all blocks are used, allocate() returns nullptr.  Does not grow.
   
2. Atomic Spin Lock - bm::AtomicSpinLock - Mutex replacement class that uses atomic<bool> and compare_and_exchange to provide a lock-free mutex.
   
3. Compile-time and Runtime CRC generator - bm::CSTR_TO_CRC("CompileTimeExample") - bm::stringToCrc(&aString)

More to come.