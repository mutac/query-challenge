
#include "CppUnitTest.h"
#include <datastore/Database.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(TestDatabase)
	{
	public:
		
		TEST_METHOD(GivenSameKeyValuesVerifyReplacement)
		{
      const char* schemeJson =
        "[                          "
        "  {                        "
        "    \"name\": \"keyField\",  "
        "    \"type\": \"text\",   "
        "    \"size\": 32,          "
        "    \"key\": true,         "
        "    \"description\": \"This is a key field\" "
        "  },                       "
        "  {                        "
        "    \"name\": \"valueField\",  "
        "    \"type\": \"float\",   "
        "    \"size\": 32,          "
        "    \"key\": false,        "
        "    \"description\": \"This is a value field\" "
        "  }                        "
        "]                          ";

      try
      {
        DataStore::ISchemePtrH scheme(new DataStore::SchemeJson(schemeJson));
        DataStore::Database database(scheme);

        DataStore::IFieldDescriptorConstListConstPtrH fields = scheme->getFieldDescriptors();
        Assert::AreEqual((size_t)2, fields->size());

        bool added = false;
        bool inserted = false;

        //
        // Create a row that has a key field with value 123.0, and insert it
        //
        DataStore::IRowPtrH newRow = database.createRow();

        DataStore::IFieldDescriptorConstPtrH keyField = (*fields)[0];
        DataStore::ValuePtrH keyValue = keyField->fromString("ThisIsAUniqueKeyValue");
        Assert::AreEqual(true, keyField->isKey());

        DataStore::IFieldDescriptorConstPtrH valueField = (*fields)[1];
        DataStore::ValuePtrH valueValue = valueField->fromString("42.0");
        Assert::AreEqual(false, valueField->isKey());

        added = newRow->setValue(keyField, keyValue);
        Assert::AreEqual(true, added);
        added = newRow->setValue(valueField, valueValue);
        Assert::AreEqual(true, added);

        inserted = database.insert(newRow);
        Assert::AreEqual(true, inserted);

        //
        // Insert another row that has the same key value, 
        // but a different value in the non-key field.  The 
        // previously inserted row should be replaced.
        //

        newRow = database.createRow();

        keyValue = keyField->fromString("ThisIsAUniqueKeyValue");
        Assert::AreEqual(true, keyField->isKey());

        valueValue = valueField->fromString("84.84");
        Assert::AreEqual(false, valueField->isKey());

        added = newRow->setValue(keyField, keyValue);
        Assert::AreEqual(true, added);
        added = newRow->setValue(valueField, valueValue);
        Assert::AreEqual(true, added);

        inserted = database.insert(newRow);
        Assert::AreEqual(true, inserted);

        // Need to be able to query the databse back and see if the row
        // was replaced
        Assert::Fail(L"Not fully implemented");
      }
      catch (std::exception& ex)
      {
       Logger::WriteMessage(ex.what());
        Assert::Fail(L"Exception");
      }
		}

	};
}