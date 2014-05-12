#include <CppUnitTest.h>
#include <exception>
#include <datastore/JsonStorage.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(TestScheme)
	{
	public:
		TEST_METHOD(GivenValidJsonVerifyOneField)
		{
      const char* schemeJson =
        "[                          "
        "  {                        "
        "    \"name\": \"field1\",  "
        "    \"type\": \"text\",    "
        "    \"size\": 128,         "
        "    \"key\": true,         "
        "    \"description\": \"This is a single field\" "
        "  }                        "
        "]                          ";

      try
      {
        DataStore::SchemeJson scheme(schemeJson);
        
        const DataStore::IFieldDescriptorConstListConstPtrH fields = scheme.getFieldDescriptors();
        Assert::AreEqual((size_t)1, fields->size());

        const DataStore::IFieldDescriptor* oneField = fields->cbegin()->get();
        Assert::AreEqual("field1", oneField->getName());
        Assert::AreEqual(DataStore::TypeInfo_String, oneField->getType());
        Assert::AreEqual((size_t)128, oneField->getSize());
        Assert::AreEqual("This is a single field", oneField->getDescription());
      }
      catch (std::exception& ex)
      {
        Logger::WriteMessage(ex.what());
        Assert::Fail(L"Exception");
      }
		}

    TEST_METHOD(GivenMissingArrayJsonVerifyFailure)
    {
      const char* missingArrayJson = "{ \"name\": \"bad\", \"type\": \"text\" }";

      try
      {
        DataStore::SchemeJson scheme(missingArrayJson);
        Assert::Fail(L"Exception expected, but missed");
      }
      catch (std::exception& ex)
      {
        (void)ex;
        // Expected
      }
    }

    TEST_METHOD(GivenAllTypesJsonVerifyTypeInfo)
    {
      const char* schemeJson =
        "[                              "
        "  {                            "
        "    \"name\": \"textField\",   "
        "    \"type\": \"text\",        "
        "    \"size\": 128,             "
        "    \"key\": true,             "
        "    \"description\": \"This is a text field\" "
        "  },                           "
        "  {                            "
        "    \"name\": \"floatField\",  "
        "    \"type\": \"float\",       "
        "    \"size\": 32,              "
        "    \"description\": \"This is a float field\" "
        "  },                           "
        "  {                            "
        "    \"name\": \"dateField\",   "
        "    \"type\": \"date\",        "
        "    \"description\": \"This is a date field\" "
        "  },                           "
        "  {                            "
        "    \"name\": \"timeField\",   "
        "    \"type\": \"time\",        "
        "    \"description\": \"This is a time field\" "
        "  }                            "
        "]                              ";

      try
      {
        DataStore::SchemeJson scheme(schemeJson);
        DataStore::IFieldDescriptorConstListConstPtrH fields = scheme.getFieldDescriptors();

        Assert::AreEqual((size_t)4, fields->size());

        DataStore::IFieldDescriptorConstList::const_iterator field = fields->cbegin();
        Assert::AreEqual(DataStore::TypeInfo_String, (*field)->getType());
        ++field;
        Assert::AreEqual(DataStore::TypeInfo_Float, (*field)->getType());
        ++field;
        Assert::AreEqual(DataStore::TypeInfo_Date, (*field)->getType());
        ++field;
        Assert::AreEqual(DataStore::TypeInfo_Time, (*field)->getType());
      }
      catch (std::exception& ex)
      {
        Logger::WriteMessage(ex.what());
        Assert::Fail(L"Exception");
      }
    }

    TEST_METHOD(GivenInvalidTypeJsonVerifyFailure)
    {
      const char* schemeJson =
        "[                          "
        "  {                        "
        "    \"name\": \"field1\",  "
        "    \"type\": \"invalid\",    "
        "    \"size\": 128,         "
        "    \"description\": \"This is an invalid field\" "
        "  }                        "
        "]                          ";

      try
      {
        DataStore::SchemeJson scheme(schemeJson);
        Assert::Fail(L"Exception expected, but missed");
      }
      catch (std::exception& ex)
      {
        (void)ex;
        // Exception expected
      }
    }
	};
}