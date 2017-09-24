#include <SDKDDKVer.h>
#include "CppUnitTest.h"

#include "../../../BmUtils/bmCRC.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{		
	TEST_CLASS(crcTest)
	{
	public:
		
		TEST_METHOD(crcTestMethod)
		{
			Assert::IsTrue(bm::CSTR_TO_CRC("testString") == bm::stringToCrc("testString"));
		}
	};
}