#include "../BmUtils/bmUtils.h"

#include <vector>
#include <random>
#include <algorithm>

int main(int argc, const char * argv[])
{
	bm::Assert(false, "false");
	bm::Assert(true, "true");

	char* dummy = new char();
	
	std::vector<void*> allocated;

	bm::PoolAllocator<16, 1024> blockAllocator;
	for ( int i = 0; i < 1026; ++i)
		allocated.push_back(blockAllocator.allocate());

	auto engine = std::default_random_engine();
	std::shuffle(allocated.begin(), allocated.end(), engine);

	for ( auto ptr : allocated)
		blockAllocator.free(ptr);

	bm::PoolAllocator<16, 1024> blockAllocator2;
	void* ptr = blockAllocator.allocate();
	void* ptr2 = blockAllocator2.allocate();
	blockAllocator2.free(ptr);
	blockAllocator.free(ptr2);
	blockAllocator.free(ptr);
	blockAllocator2.free(ptr2);
	blockAllocator.free(dummy);

	delete dummy;

	unsigned int iIndex = bm::TypeIndex::Get<int>();
	unsigned int fIndex = bm::TypeIndex::Get<float>();
	unsigned int dIndex = bm::TypeIndex::Get<double>();
	unsigned int aIndex = bm::TypeIndex::Get<bm::AtomicSpinLock>();

	unsigned int aIndex2 = bm::TypeIndex::Get<bm::AtomicSpinLock>();
	unsigned int dIndex2 = bm::TypeIndex::Get<double>();
	unsigned int fIndex2 = bm::TypeIndex::Get<float>();
	unsigned int iIndex2 = bm::TypeIndex::Get<int>();
	
	return 0;
}