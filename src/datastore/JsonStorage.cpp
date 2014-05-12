
#include <datastore/JsonStorage.h>

// Throw exceptions rather than assert on JSON parsing failures:
#define RAPIDJSON_ASSERT(x)         \
do {                                \
  if (!(x))                         \
    throw std::exception(#x);       \
} while (0)

#include <rapidjson/document.h>
#include <rapidjson/filestream.h>
#include <fstream>
#include <streambuf>
#include <string>

using namespace DataStore;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

namespace DataStore
{
  /** Given a rapidjson type, return a string describing it.  For error reporting... */
  const char* jsonTypeToString(rapidjson::Type type)
  {
    static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
    if (((int)type) < 0 || ((int)type) > (sizeof(kTypeNames) / sizeof(char*)))
    {
      return "Unknown";
    }

    return kTypeNames[(int)type];
  }

  /////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////

  /**
  Hide implementation details from client code--avoid including rapidjson publically.
  */
  class SchemeJsonImpl
  {
  public:
    SchemeJsonImpl(const char* schemeJson) :
      mFields(new IFieldDescriptorConstList()),
      mKeyFields(new IFieldDescriptorConstList())
    {
      rapidjson::Document scheme;
      scheme.Parse<0>(schemeJson);
      readScheme(&scheme);
    }

    SchemeJsonImpl(FILE* schemeFile) :
      mFields(new IFieldDescriptorConstList()),
      mKeyFields(new IFieldDescriptorConstList())
    {
      rapidjson::FileStream schemeStream(schemeFile);
      rapidjson::Document scheme;
      scheme.ParseStream<0>(schemeStream);
      readScheme(&scheme);
    }

    SchemeJsonImpl(const rapidjson::Value* schemeJsonRoot)
    {
      readScheme(schemeJsonRoot);
    }

    /** Build scheme from JSON, or throw if format is incorrect */
    void readScheme(const rapidjson::Value* root)
    {
      mFields->clear();
      mKeyFields->clear();

      if (!root->IsArray())
      {
        std::string ex = "Invalid Scheme JSON: expected array, found ";
        ex += jsonTypeToString(root->GetType());
        throw std::exception(ex.c_str());
      }

      FieldId fieldId = 0;

      for (rapidjson::Value::ConstValueIterator field = root->Begin();
        field != root->End(); ++field, ++fieldId)
      {
        if (field->GetType() != rapidjson::kObjectType)
        {
          std::string ex = "Invalid Scheme JSON: Expected object, found ";
          ex += jsonTypeToString(field->GetType());
          throw std::exception(ex.c_str());
        }

        //
        // Expected members:
        //
        // name
        // type
        // size
        // description
        //

        std::string name;
        std::string description;
        TypeInfo type = DataStore::TypeInfo_Empty;
        bool isKey = false;
        size_t size = 0;

        for (rapidjson::Value::ConstMemberIterator member = field->MemberBegin();
          member != field->MemberEnd(); ++member)
        {
          std::string memberName(member->name.GetString());

          if (memberName == "name")
          {
            name = member->value.GetString();
          }
          else if (memberName == "type")
          {
            type = parseTypeName(member->value.GetString());
          }
          else if (memberName == "size")
          {
            size = (size_t)member->value.GetInt64();
          }
          else if (memberName == "description")
          {
            description = member->value.GetString();
          }
          else if (memberName == "key")
          {
            isKey = member->value.GetBool();
          }
          else
          {
            std::string ex("Invalid Scheme JSON: Unexpected member: ");
            ex += memberName;
            throw std::exception(ex.c_str());
          }
        }

        IFieldDescriptorPtrH fieldDescriptor;
        fieldDescriptor = FieldDescriptorFactory::Create(fieldId, type, name.c_str(),
          description.c_str(), isKey, size);
        if (!fieldDescriptor)
        {
          std::string ex("Internal error creating FieldDescriptor for ");
          ex += name;
          throw std::exception(ex.c_str());
        }

        if (fieldDescriptor->isKey())
        {
          mKeyFields->push_back(fieldDescriptor);
        }

        mFields->push_back(fieldDescriptor);
      }

      throwOnInvalidConstraints();
    }

    IFieldDescriptorConstListConstPtrH getFieldDescriptors() const
    {
      return mFields;
    }

    IFieldDescriptorConstListConstPtrH getKeyFieldDescriptors() const
    {
      return mKeyFields;
    }

  private:
    /** Parse supported data types, return FieldType which corresponds */
    TypeInfo parseTypeName(const char* type)
    {
      std::string typeName(type);
      if (typeName == "text")
        return DataStore::TypeInfo_String;
      else if (typeName == "date")
        return DataStore::TypeInfo_Date;
      else if (typeName == "float")
        return DataStore::TypeInfo_Float;
      else if (typeName == "time")
        return DataStore::TypeInfo_Time;
      else
        return DataStore::TypeInfo_Empty;
    }

    /** Throw an exception if one of a few constrainst are not met */
    void throwOnInvalidConstraints() const
    {
      bool hasOneKey = false;

      for (IFieldDescriptorConstList::const_iterator field = mFields->begin();
        field != mFields->end(); ++field)
      {
        if (strlen((*field)->getName()) == 0)
        {
          throw std::exception("Unmet Scheme Constraints: 'name' member is required");
        }
        if ((*field)->getType() == DataStore::TypeInfo_Empty)
        {
          std::string ex("Unmet Scheme Constraints: Invalid 'type' in ");
          ex += (*field)->getName();
          throw std::exception();
        }
        if ((*field)->isKey())
        {
          hasOneKey = true;
        }

        // Names should be unique too probably...
      }

      if (!hasOneKey)
      {
        throw std::exception("Unmet Scheme Constraints: At least one field must be a 'key'");
      }
    }

    IFieldDescriptorConstListPtrH mFields;
    IFieldDescriptorConstListPtrH mKeyFields;
  };

  /////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////

  /**
  */
  class DataStorageJsonImpl
  {
  public:
    DataStorageJsonImpl(FILE* dbFile) :
      mDatabaseFile(dbFile),
      mRowDataRoot(NULL)
    {
      readDatastore();
    }

    DataStorageJsonImpl(SchemeJsonConstPtrH scheme, FILE* newDbFile) :
      mScheme(scheme),
      mDatabaseFile(newDbFile),
      mRowDataRoot(NULL)
    {
    }

    SchemeJsonConstPtrH getScheme() const
    {
      return mScheme;
    }

  private:

    /**
    */
    void readDatastore()
    {
      mDatabase.ParseStream<0>(mDatabaseFile);

      if (!mDatabase.IsObject())
      {
        std::string ex = "Invalid Database JSON: expected array, found ";
        ex += jsonTypeToString(mDatabase.GetType());
        throw std::exception(ex.c_str());
      }

      bool foundScheme = false;
      bool foundRows = false;

      for (rapidjson::Value::ConstMemberIterator dbMember = mDatabase.MemberBegin();
        dbMember != mDatabase.MemberEnd(); ++dbMember)
      {
        if (dbMember->name.GetString() == "scheme")
        {
          mScheme = SchemeJsonPtrH(new SchemeJson((void*)&dbMember->value));
          foundScheme = true;
        }
        else if (dbMember->name.GetString() == "rows")
        {
          mRowDataRoot = &dbMember->value;
          foundRows = true;
        }
      }

      if (!foundScheme)
      {
        throw std::exception("Invalid Database JSON: \"scheme\" member not found");
      }
      if (!foundRows)
      {
        throw std::exception("Invalid Database JSON: \"rows\" member not found");
      }
    }

    void readRows(const rapidjson::Value* root)
    {
      // Scheme must have already been parsed by this point
      mDebugAssert(mScheme);

      IFieldDescriptorConstListConstPtrH fieldDescriptors = 
        mScheme->getFieldDescriptors();

      if (!root->IsArray())
      {
        std::string ex = "Invalid Database JSON: expected array, found ";
        ex += jsonTypeToString(root->GetType());
        throw std::exception(ex.c_str());
      }

      //
      // Rows layed out as an array of arrays, with the fields sorted
      // in order according to the scheme.
      //
      for (rapidjson::Value::ConstValueIterator row = root->Begin(); 
        row != root->End(); ++row)
      {
        if (!row->IsArray())
        {
          std::string ex = "Invalid Database JSON: expected array, found ";
          ex += jsonTypeToString(row->GetType());
          throw std::exception(ex.c_str());
        } 

        int fieldIdx = 0;
        for (rapidjson::Value::ConstValueIterator field = row->Begin();
          field != row->End(); ++field)
        {
          ValueConstPtrH value = 
            (*fieldDescriptors)[fieldIdx]->fromString(field->GetString());
          ++fieldIdx;
        }
      }
    }

    rapidjson::FileStream mDatabaseFile;
    SchemeJsonConstPtrH mScheme;
    rapidjson::Document mDatabase;
    const rapidjson::Value* mRowDataRoot;
  };
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

SchemeJson::SchemeJson(FILE* schemeFile)
: mImpl(new SchemeJsonImpl(schemeFile))
{
}

SchemeJson::SchemeJson(const char* schemeJson)
: mImpl(new SchemeJsonImpl(schemeJson))
{
}

SchemeJson::SchemeJson(const void* root)
: mImpl(new SchemeJsonImpl(reinterpret_cast<const rapidjson::Value*>(root)))
{
}

bool SchemeJson::allFieldsPresent(const std::vector<std::string>& headerFieldNames) const
{
  IFieldDescriptorConstListConstPtrH fields = getFieldDescriptors();

  if (fields->size() != headerFieldNames.size())
  {
    return false;
  }

  IFieldDescriptorConstList::const_iterator field = fields->begin();
  std::vector<std::string>::const_iterator headerFieldName = headerFieldNames.cbegin();

  while (field != fields->end() && headerFieldName != headerFieldNames.cend())
  {
    if (*headerFieldName != (*field)->getName())
    {
      return false;
    }

    ++field;
    ++headerFieldName;
  }

  return true;
}

bool SchemeJson::allFieldsPresent(const IFieldDescriptorList& fields) const
{
  // Does not currently check types...
  IFieldDescriptorConstListConstPtrH expectedFields = getFieldDescriptors();
  return expectedFields->size() == fields.size();
}

IFieldDescriptorConstListConstPtrH SchemeJson::getFieldDescriptors() const
{
  return mImpl->getFieldDescriptors();
}

IFieldDescriptorConstListConstPtrH SchemeJson::getKeyFieldDescriptors() const
{
  return mImpl->getKeyFieldDescriptors();
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

DatabasePtrH DataStorageJson::Load(FILE* dbFile)
{
  IDataStoragePtrH existingStoragePtrH(new DataStorageJson(dbFile));
  DatabasePtrH db(new Database(existingStoragePtrH));
  return db;
}

DatabasePtrH DataStorageJson::Create(SchemeJsonConstPtrH scheme, FILE* emptyDbFile)
{
  IDataStoragePtrH newStoragePtrH(new DataStorageJson(scheme, emptyDbFile));
  DatabasePtrH newDb(new Database(newStoragePtrH));
  return newDb;
}

DatabasePtrH DataStorageJson::Create(FILE* schemeFile, FILE* emptyDbFile)
{
  SchemeJsonPtrH scheme(new SchemeJson(schemeFile));
  IDataStoragePtrH newStoragePtrH(new DataStorageJson(scheme, emptyDbFile));
  DatabasePtrH newDb(new Database(newStoragePtrH));
  return newDb;
}

DataStorageJson::DataStorageJson(FILE* dbFile) :
  mImpl(new DataStorageJsonImpl(dbFile))
{
}

DataStorageJson::DataStorageJson(SchemeJsonConstPtrH scheme, FILE* newDbFile) :
  mImpl(new DataStorageJsonImpl(scheme, newDbFile))
{
}

ISchemeConstPtrH DataStorageJson::getScheme()
{
  return mImpl->getScheme();
}



