
#include <datastore/FieldDescriptor.h>

using namespace DataStore;

std::shared_ptr<IFieldValue> TextFieldDescriptor::deserialize(const char* fieldString) const
{
  return NULL;
}

std::shared_ptr<IFieldValue> DateFieldDescriptor::deserialize(const char* fieldString) const
{
  return NULL;
}

std::shared_ptr<IFieldValue> TimeFieldDescriptor::deserialize(const char* fieldString) const
{
  return NULL;
}

std::shared_ptr<IFieldValue> FloatFieldDescriptor::deserialize(const char* fieldString) const
{
  return NULL;
}

std::shared_ptr<IFieldDescriptor> FieldDescriptorFactory::Create(FieldType type,
  const char* name, const char* description, size_t size)
{
  switch (type)
  {
  case kFieldType_Text:
    return std::shared_ptr<IFieldDescriptor>(new TextFieldDescriptor(name, description, size));
    break;
  case kFieldType_Date:
    return std::shared_ptr<IFieldDescriptor>(new DateFieldDescriptor(name, description));
    break;
  case kFieldType_Time:
    return std::shared_ptr<IFieldDescriptor>(new TimeFieldDescriptor(name, description));
    break;
  case kFieldType_Float:
    return std::shared_ptr<IFieldDescriptor>(new FloatFieldDescriptor(name, description, size));
    break;
  default:
    return NULL;
  }
}