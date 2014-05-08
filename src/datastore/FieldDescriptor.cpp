
#include <datastore/FieldDescriptor.h>
#include <datastore/FieldType.h>

using namespace DataStore;

std::shared_ptr<Value> TextFieldDescriptor::fromString(const char* str) const
{
  return std::shared_ptr<Value>(new Value((char*)str));
}

std::shared_ptr<Value> DateFieldDescriptor::fromString(const char* str) const
{
  DataStore::Date date;
  if (date.fromString(str))
  {
    return std::shared_ptr<Value>(new Value(date));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<Value> TimeFieldDescriptor::fromString(const char* str) const
{
  DataStore::Time time;
  if (time.fromString(str))
  {
    return std::shared_ptr<Value>(new Value(time));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<Value> FloatFieldDescriptor::fromString(const char* str) const
{
  mStd::Variant v(str);
  if (v.changeType(TypeInfo_Float))
  {
    return std::shared_ptr<Value>(new Value(v));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<IFieldDescriptor> FieldDescriptorFactory::Create(TypeInfo type,
  const char* name, const char* description, bool isKey, size_t size)
{
  if (type == TypeInfo_String)
    return std::shared_ptr<IFieldDescriptor>(new TextFieldDescriptor(name, description, isKey, size));
  else if (type == TypeInfo_Date)
    return std::shared_ptr<IFieldDescriptor>(new DateFieldDescriptor(name, description, isKey));
  else if (type == TypeInfo_Time)
    return std::shared_ptr<IFieldDescriptor>(new TimeFieldDescriptor(name, description, isKey));
  else if (type == TypeInfo_Float)
    return std::shared_ptr<IFieldDescriptor>(new FloatFieldDescriptor(name, description, isKey, size));
  else
    return NULL;
}