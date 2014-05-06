
#include <datastore/FieldDescriptor.h>
#include <datastore/FieldType.h>

using namespace DataStore;

std::shared_ptr<IFieldValue> TextFieldDescriptor::fromString(const char* str) const
{
  return std::shared_ptr<IFieldValue>(new FieldValue((char*)str));
}

std::shared_ptr<IFieldValue> DateFieldDescriptor::fromString(const char* str) const
{
  DataStore::Date date;
  if (date.fromString(str))
  {
    return std::shared_ptr<IFieldValue>(new FieldValue(date));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<IFieldValue> TimeFieldDescriptor::fromString(const char* str) const
{
  // 
  // Assume that encoding is plain text:
  //

  DataStore::Time time;
  if (time.fromString(str))
  {
    return std::shared_ptr<IFieldValue>(new FieldValue(time));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<IFieldValue> FloatFieldDescriptor::fromString(const char* str) const
{
  mStd::Variant v(str);
  if (v.changeType(TypeInfo_Float))
  {
    return std::shared_ptr<IFieldValue>(new FieldValue(v));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<IFieldDescriptor> FieldDescriptorFactory::Create(TypeInfo type,
  const char* name, const char* description, size_t size)
{
  if (type == TypeInfo_String)
    return std::shared_ptr<IFieldDescriptor>(new TextFieldDescriptor(name, description, size));
  else if (type == TypeInfo_Date)
    return std::shared_ptr<IFieldDescriptor>(new DateFieldDescriptor(name, description));
  else if (type == TypeInfo_Time)
    return std::shared_ptr<IFieldDescriptor>(new TimeFieldDescriptor(name, description));
  else if (type == TypeInfo_Float)
    return std::shared_ptr<IFieldDescriptor>(new FloatFieldDescriptor(name, description, size));
  else
    return NULL;
}