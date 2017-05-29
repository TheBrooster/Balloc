#include "BmBalloc.h"

#include <vector>
#include <random>
#include <algorithm>

int main(int argc, char argv[])
{
	char* dummy = new char();
	
	std::vector<void*> allocated;

	BM::Balloc<16, 1024> blockAllocator;
	for ( int i = 0; i < 1026; ++i)
		allocated.push_back(blockAllocator.Allocate());

	auto engine = std::default_random_engine();
	std::shuffle(allocated.begin(), allocated.end(), engine);

	for ( auto ptr : allocated)
		blockAllocator.Free(ptr);

	BM::Balloc<16, 1024> blockAllocator2;
	void* ptr = blockAllocator.Allocate();
	void* ptr2 = blockAllocator2.Allocate();
	blockAllocator2.Free(ptr);
	blockAllocator.Free(ptr2);
	blockAllocator.Free(ptr);
	blockAllocator2.Free(ptr2);
	blockAllocator.Free(dummy);

	delete dummy;

	return 0;
}