Collection of simple to use C++ header only utilities.

1. Fixed Sized Block Alocator - BM::BlockAllocator<kBlockSize, kBlockCount>
   Very simple premise.  The util allocates kBlockCount blocks of size kBlockSize.  When it runs out, the allocate member function returns nullptr to indicate failure.
   
2. Atomic Spin Lock - BM::AtomicSpinLock
   Simple mutex replacement class that uses atomic<bool> and compare_and_exchange to provide a lock-free mutex.
   
More to come.
