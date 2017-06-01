#include "BmBlockAllocator.h"

#include <vector>
#include <random>
#include <algorithm>

int main(int argc, char argv[])
{
	char* dummy = new char();
	
	std::vector<void*> allocated;

	BM::BlockAllocator<16, 1024> blockAllocator;
	for ( int i = 0; i < 1026; ++i)
		allocated.push_back(blockAllocator.allocate());

	auto engine = std::default_random_engine();
	std::shuffle(allocated.begin(), allocated.end(), engine);

	for ( auto ptr : allocated)
		blockAllocator.free(ptr);

	BM::BlockAllocator<16, 1024> blockAllocator2;
	void* ptr = blockAllocator.allocate();
	void* ptr2 = blockAllocator2.allocate();
	blockAllocator2.free(ptr);
	blockAllocator.free(ptr2);
	blockAllocator.free(ptr);
	blockAllocator2.free(ptr2);
	blockAllocator.free(dummy);

	delete dummy;

	return 0;
}