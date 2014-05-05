/**
*/

#include <datastore/Scheme.h>

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

using namespace DataStore;

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
        TypeInfo type = mResource::TypeInfo_Empty;
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
          else
          {
            std::string ex("Invalid Scheme JSON: Unexpected member: ");
            ex += memberName;
            throw std::exception(ex.c_str());
          }
        }

        // Check required members
        if (name.empty() || type == mResource::TypeInfo_Empty)
        {
          throw std::exception("Invalid Scheme JSON: required field missing");
        }

        std::shared_ptr<IFieldDescriptor> fieldDescriptor;
        fieldDescriptor = FieldDescriptorFactory::Create(type, name.c_str(), 
          description.c_str(), size);
        if (!fieldDescriptor)
        {
          std::string ex("Error creating FieldDescriptor for ");
          ex += name;
          throw std::exception(ex.c_str());
        }

        mFields.push_back(fieldDescriptor);
      }
    }

    /** Parse supported data types, return FieldType which corresponds */
    TypeInfo parseTypeName(const char* type)
    {
      std::string typeName(type);
      if (typeName == "text")
        return mResource::TypeInfo_String;
      else if (typeName == "date")
        return mResource::TypeInfo_Date;
      else if (typeName == "float")
        return mResource::TypeInfo_Float;
      else if (typeName == "time")
        return mResource::TypeInfo_Time;
      else
        return mResource::TypeInfo_Empty;
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

    const IScheme::IFieldDescritors& getFieldDescriptors() const
    {
      return mFields;
    }

  private:
    rapidjson::Document mScheme;
    IScheme::IFieldDescritors mFields;
  };
}

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
  const IFieldDescritors& fields = getFieldDescriptors();

  if (fields.size() != headerFieldNames.size())
  {
    return false;
  }

  IFieldDescritors::const_iterator field = fields.begin();
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

const IScheme::IFieldDescritors& SchemeJson::getFieldDescriptors() const
{
  return mImpl->getFieldDescriptors();
}