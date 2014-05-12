
#include <datastore/JsonStorage.h>
#include <datastore/Database.h>

// Throw exceptions rather than assert on JSON parsing failures:
#define RAPIDJSON_ASSERT(x)         \
do {                                \
  if (!(x))                         \
    throw std::exception(#x);       \
} while (0)

#include <rapidjson/document.h>
#include <rapidjson/filestream.h>
#include <rapidjson/prettywriter.h>
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

    SchemeJsonImpl() :
      mFields(new IFieldDescriptorConstList()),
      mKeyFields(new IFieldDescriptorConstList())
    {
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
            type = TypeFromString(member->value.GetString());
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

    template <class Allocator>
    void writeScheme(rapidjson::Value* root, Allocator& allocator) const
    {
      rapidjson::Value& fieldArray = root->SetArray();

      for (IFieldDescriptorConstList::const_iterator field = mFields->begin();
        field != mFields->end(); ++field)
      {
        // Wow...

        rapidjson::Value name;
        rapidjson::Value type;
        rapidjson::Value size;
        rapidjson::Value description;
        rapidjson::Value key;

        name.SetString((*field)->getName());
        type.SetString(TypeToString((*field)->getType()));
        size.SetInt((*field)->getSize());
        description.SetString((*field)->getDescription());
        key.SetBool((*field)->isKey());

        rapidjson::Value newFieldObject(rapidjson::kObjectType);
        newFieldObject.AddMember("name", name, allocator);
        newFieldObject.AddMember("type", type, allocator);
        newFieldObject.AddMember("size", size, allocator);
        newFieldObject.AddMember("description", description, allocator);
        newFieldObject.AddMember("key", key, allocator);

        fieldArray.PushBack(newFieldObject, allocator);
      }
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
    /** Parse supported data types, return TypeInfo which corresponds */
    static TypeInfo TypeFromString(const char* type)
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

    /** Return string representation of a TypeInfo */
    static const char* TypeToString(TypeInfo type)
    {
      if (type == DataStore::TypeInfo_String)
        return "text";
      else if (type == DataStore::TypeInfo_Date)
        return "date";
      else if (type == DataStore::TypeInfo_Float)
        return "float";
      else if (type == DataStore::TypeInfo_Time)
        return "time";
      else
        return "null";
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
      loadDatastore();
    }

    DataStorageJsonImpl(SchemeJsonConstPtrH scheme, FILE* newDbFile) :
      mScheme(scheme),
      mDatabaseFile(newDbFile),
      mRowDataRoot(NULL)
    {
    }

    /**
    */
    SchemeJsonConstPtrH getScheme() const
    {
      return mScheme;
    }

    /**
    */
    void load(Database* database)
    {
      // If the row root is not set, the datastore must be new
      if (mRowDataRoot != NULL)
        loadRows(mRowDataRoot, database);
    }

    void beginPersist()
    {
      mRowDataRoot = NULL;

      rapidjson::Value& dbObject = mDatabase.SetObject();

      rapidjson::Value schemeObject;
      mScheme->getImpl()->writeScheme(&schemeObject, mDatabase.GetAllocator());
      dbObject.AddMember("scheme", schemeObject, mDatabase.GetAllocator());

      rapidjson::Value rowsObject(rapidjson::kArrayType);
      dbObject.AddMember("rows", rowsObject, mDatabase.GetAllocator());
      mRowDataRoot = &rowsObject;
    }

    void persistRow(const IRow* row)
    {
      if (mRowDataRoot != NULL)
      {
        rapidjson::Value jsonRow(rapidjson::kArrayType);

        IFieldDescriptorConstListConstPtrH fields = row->getFieldDescriptors();
        for (IFieldDescriptorConstList::const_iterator field = fields->cbegin();
          field != fields->cend(); ++field)
        {
          rapidjson::Value jsonVal;

          ValueConstPtrH rowValue = row->getValue(*(field->get()));

          mStd::mString strValue;
          rowValue->getValue().convertTo(&strValue);
          jsonVal.SetString(strValue.c_str());

          jsonRow.PushBack(jsonVal, mDatabase.GetAllocator());
        }

        mRowDataRoot->PushBack(jsonRow, mDatabase.GetAllocator());
      }
    }

    void endPersist()
    {
      rapidjson::PrettyWriter<rapidjson::FileStream> writer(mDatabaseFile);
      mDatabase.Accept(writer);
    }

  private:

    /**
    */
    void loadDatastore()
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

      for (rapidjson::Value::MemberIterator dbMember = mDatabase.MemberBegin();
        dbMember != mDatabase.MemberEnd(); ++dbMember)
      {
        if (strcmp(dbMember->name.GetString(), "scheme") == 0)
        {
          mScheme = SchemeJsonPtrH(new SchemeJson());
          mScheme->getImpl()->readScheme(&dbMember->value);
          foundScheme = true;
        }
        else if (strcmp(dbMember->name.GetString(), "rows") == 0)
        {
          // Loading rows is dictated by the database instance, cache
          // the location of the rows for later.
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

    /**
    */
    void loadRows(const rapidjson::Value* root, Database* database)
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

        IRowPtrH newRow = database->createRow();

        size_t fieldIdx = 0;
        for (rapidjson::Value::ConstValueIterator jsonValue = row->Begin();
          jsonValue != row->End(); ++jsonValue)
        {
          IFieldDescriptorConstPtrH field = (*fieldDescriptors)[fieldIdx];
          ValuePtrH value = field->fromString(jsonValue->GetString());
          newRow->setValue(field, value);
        }

        if (!database->insert(newRow))
        {
          throw std::exception("Invalid Database JSON: corrupt row");
        }
      }
    }

    rapidjson::Document mDatabase;
    rapidjson::FileStream mDatabaseFile;

    rapidjson::Value* mRowDataRoot;

    SchemeJsonConstPtrH mScheme;
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

SchemeJson::SchemeJson()
: mImpl(new SchemeJsonImpl())
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

SchemeJsonImpl* SchemeJson::getImpl() const
{
  return mImpl.get();
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

void DataStorageJson::load(Database* database)
{
  mImpl->load(database);
}

void DataStorageJson::beginPersist()
{
  mImpl->beginPersist();
}

void DataStorageJson::persistRow(const IRow* row)
{
  mImpl->persistRow(row);
}

void DataStorageJson::endPersist()
{
  mImpl->endPersist();
}

