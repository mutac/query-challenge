
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

      isValid = time.fromString("00:00");
      Assert::IsTrue(isValid);
      Assert::IsTrue(time.toString() == "00:0000");

      isValid = time.fromString("01:02");
      Assert::IsTrue(isValid);
      Assert::IsTrue(time.toString() == "01:0002");

      isValid = time.fromString("23:124");
      Assert::IsTrue(isValid);
      Assert::IsTrue(time.toString() == "23:0124");
		}

	};
}