
#include "CppUnitTest.h"
#include <datastore/FieldType.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(TestDate)
	{
	public:
		
		TEST_METHOD(GivenValidStringValidateEncoding)
		{
      DataStore::Date date;

      bool isValid = false;

      isValid = date.fromString("2014-01-01");
      Assert::IsTrue(isValid);
      Assert::IsTrue(date.toString() == "2014-01-01");

      isValid = date.fromString("1970-01-01");
      Assert::IsTrue(isValid);
      Assert::IsTrue(date.toString() == "1970-01-01");

      isValid = date.fromString("1982-08-19");
      Assert::IsTrue(isValid);
      Assert::IsTrue(date.toString() == "1982-08-19");
    }

	};
}