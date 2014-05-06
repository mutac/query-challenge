
#include "CppUnitTest.h"
#include <datastore/FieldType.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(TestTime)
	{
	public:
		
		TEST_METHOD(GivenValidStringValidateEncoding)
		{
      DataStore::Time time;

      bool isValid = false;

      isValid = time.fromString("00:00:00");
      Assert::IsTrue(isValid);
      Assert::IsTrue(time.toString() == "00:00:00");

      isValid = time.fromString("01:02:03");
      Assert::IsTrue(isValid);
      Assert::IsTrue(time.toString() == "01:02:03");

      isValid = time.fromString("23:59:59");
      Assert::IsTrue(isValid);
      Assert::IsTrue(time.toString() == "23:59:59");
		}

	};
}