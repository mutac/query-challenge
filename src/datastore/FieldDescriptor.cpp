
#include <datastore/FieldDescriptor.h>
#include <datastore/FieldType.h>

using namespace DataStore;

ValuePtrH TextFieldDescriptor::fromString(const char* str) const
{
  // BUG: (char*)str constructor does not internally allocate
  // memory to hold onto the string
  mStd::mString s(str);
  return ValuePtrH(new Value(s));
}

ValuePtrH DateFieldDescriptor::fromString(const char* str) const
{
  DataStore::Date date;
  if (date.fromString(str))
  {
    return ValuePtrH(new Value(date));
  }
  else
  {
    return NULL;
  }
}

ValuePtrH TimeFieldDescriptor::fromString(const char* str) const
{
  DataStore::Time time;
  if (time.fromString(str))
  {
    return ValuePtrH(new Value(time));
  }
  else
  {
    return NULL;
  }
}

ValuePtrH FloatFieldDescriptor::fromString(const char* str) const
{
  mStd::Variant v(str);
  if (v.changeType(TypeInfo_Float))
  {
    return ValuePtrH(new Value(v));
  }
  else
  {
    return NULL;
  }
}

IFieldDescriptorPtrH FieldDescriptorFactory::Create(FieldId id, TypeInfo type,
  const char* name, const char* description, bool isKey, size_t size)
{
  if (type == TypeInfo_String)
    return IFieldDescriptorPtrH(new TextFieldDescriptor(id, name, description, isKey, size));
  else if (type == TypeInfo_Date)
    return IFieldDescriptorPtrH(new DateFieldDescriptor(id, name, description, isKey));
  else if (type == TypeInfo_Time)
    return IFieldDescriptorPtrH(new TimeFieldDescriptor(id, name, description, isKey));
  else if (type == TypeInfo_Float)
    return IFieldDescriptorPtrH(new FloatFieldDescriptor(id, name, description, isKey, size));
  else
    return NULL;
}