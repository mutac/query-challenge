/**
*/

#include <datastore/Scheme.h>

// Throw exceptions rather than assert on JSON parsing failures:
#define RAPIDJSON_ASSERT(x)             \
  do {                                  \
    if (!(x))                           \
      throw std::exception(#x);         \
  } while(0)

#include <rapidjson/document.h>
#include <rapidjson/filestream.h>
#include <fstream>
#include <streambuf>
#include <string>

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

namespace DataStore
{
  /**
    Hide implementation details from client code--avoid including rapidjson publically.
  */
  class SchemeJsonImpl
  {
  public:
    SchemeJsonImpl(const char* schemeJson)
    {
      mScheme.Parse<0>(schemeJson);
      setScheme();
    }

    SchemeJsonImpl(FILE* schemeFile)
    {
      rapidjson::FileStream schemeStream(schemeFile);
      mScheme.ParseStream<0>(schemeStream);

      setScheme();
    }

    /** Build scheme from JSON, or throw if format is incorrect */
    void setScheme()
    {
      mFields.clear();

      if (!mScheme.IsArray())
      {
        throw std::exception("Invalid Scheme JSON: expected array");
      }

      for (rapidjson::Value::ConstValueIterator field = mScheme.Begin();
        field != mScheme.End(); ++field)
      {
        if (field->GetType() != rapidjson::kObjectType)
        {
          std::string ex = "Invalid Scheme JSON: Expected object, found ";
          ex += valueType(field->GetType());
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

        std::shared_ptr<IFieldDescriptor> fieldDescriptor;
        fieldDescriptor = FieldDescriptorFactory::Create(type, name.c_str(), 
          description.c_str(), isKey, size);
        if (!fieldDescriptor)
        {
          std::string ex("Error creating FieldDescriptor for ");
          ex += name;
          throw std::exception(ex.c_str());
        }

        mFields.push_back(fieldDescriptor);
      }

      throwOnInvalidConstraints();
    }

    const IScheme::IFieldDescriptors& getFieldDescriptors() const
    {
      return mFields;
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

    /** Given a rapidjson type, return a string describing it.  For error reporting... */
    const char* valueType(rapidjson::Type type)
    {
      static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
      if (((int)type) < 0 || ((int)type) > (sizeof(kTypeNames) / sizeof(char*)))
      {
        return "Unknown";
      }

      return kTypeNames[(int)type];
    }

    /** Throw an exception if one of a few constrainst are not met */
    void throwOnInvalidConstraints() const
    {
      bool hasOneKey = false;

      for (IScheme::IFieldDescriptors::const_iterator field = mFields.begin();
        field != mFields.end(); ++field)
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
      }

      if (!hasOneKey)
      {
        throw std::exception("Unmet Scheme Constraints: At least one field must be a 'key'");
      }
    }

    rapidjson::Document mScheme;
    IScheme::IFieldDescriptors mFields;
  };
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
using namespace DataStore;

SchemeJson::SchemeJson(FILE* schemeFile) 
  : mImpl(new SchemeJsonImpl(schemeFile))
{
}

SchemeJson::SchemeJson(const char* schemeJson) 
  : mImpl(new SchemeJsonImpl(schemeJson))
{
}

bool SchemeJson::validateHeader(const std::vector<std::string>& headerFieldNames) const
{
  const IFieldDescriptors& fields = getFieldDescriptors();

  if (fields.size() != headerFieldNames.size())
  {
    return false;
  }

  IFieldDescriptors::const_iterator field = fields.begin();
  std::vector<std::string>::const_iterator headerFieldName = headerFieldNames.cbegin();

  while (field != fields.end() && headerFieldName != headerFieldNames.cend())
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

const IScheme::IFieldDescriptors& SchemeJson::getFieldDescriptors() const
{
  return mImpl->getFieldDescriptors();
}