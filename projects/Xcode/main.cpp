//
//  main.cpp
//  BmUtils/Volumes/JetDrive/Downloads
//
//  Created by Bruce McNeish on 01/06/2017.
//  Copyright © 2017 Bruce McNeish. All rights reserved.
//

#include "../../BmUtils/bmUtils.h"

#include <vector>
#include <random>
#include <algorithm>

int main(int argc, const char * argv[])
{
    bm::Assert(false, "false\n");
    bm::Assert(true, "true\n");
    
    char* dummy = new char();
    
    std::vector<void*> allocated;
    
    bm::PoolAllocator<16, 1024> blockAllocator;
    for ( int i = 0; i < 1026; ++i)
        allocated.push_back(blockAllocator.allocate());
    
    auto engine = std::default_random_engine();
    std::shuffle(allocated.begin(), allocated.end(), engine);
    
    for (auto ptr : allocated)
        blockAllocator.free(ptr);
    
    bm::PoolAllocator<16, 1024> blockAllocator2;
    void* ptr = blockAllocator.allocate();
    void* ptr2 = blockAllocator2.allocate();
    blockAllocator2.free(ptr);
    blockAllocator.free(ptr2);
    blockAllocator.free(ptr);
    blockAllocator.free(ptr);
    blockAllocator2.free(ptr2);
    blockAllocator2.free(ptr2);
    blockAllocator.free(dummy);
    
    delete dummy;
    
    return 0;
}
