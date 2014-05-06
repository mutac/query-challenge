
#include <datastore/FieldDescriptor.h>
#include <datastore/FieldType.h>

using namespace DataStore;

std::shared_ptr<IFieldValue> TextFieldDescriptor::deserialize(const char* bytes, size_t size) const
{
  //
  // Assume that encoding is plain text:
  //
  // No deserialization necessary, just
  // store the string.
  //

  return std::shared_ptr<IFieldValue>(new FieldValue((char*)bytes));
}

std::shared_ptr<IFieldValue> DateFieldDescriptor::deserialize(const char* bytes, size_t size) const
{
  // 
  // Assume that encoding is plain text:
  //

  DataStore::Date date;
  if (date.fromString(bytes))
  {
    return std::shared_ptr<IFieldValue>(new FieldValue(date));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<IFieldValue> TimeFieldDescriptor::deserialize(const char* bytes, size_t size) const
{
  // 
  // Assume that encoding is plain text:
  //

  DataStore::Time time;
  if (time.fromString(bytes))
  {
    return std::shared_ptr<IFieldValue>(new FieldValue(time));
  }
  else
  {
    return NULL;
  }
}

std::shared_ptr<IFieldValue> FloatFieldDescriptor::deserialize(const char* bytes, size_t size) const
{
  // 
  // Assume that encoding is plain text:
  // Use variant string-to-float conversion to deserialize
  //

  mStd::Variant v(bytes);
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