
#include <datastore/FieldDescriptor.h>

using namespace DataStore;

std::shared_ptr<IFieldValue> TextFieldDescriptor::tryParse(const char* fieldString) const
{
  return NULL;
}

std::shared_ptr<IFieldValue> DateFieldDescriptor::tryParse(const char* fieldString) const
{
  return NULL;
}


std::shared_ptr<IFieldValue> TimeFieldDescriptor::tryParse(const char* fieldString) const
{
  return NULL;
}


std::shared_ptr<IFieldValue> FloatFieldDescriptor::tryParse(const char* fieldString) const
{
  return NULL;
}


std::shared_ptr<IFieldValueDescriptor> FieldDescriptorFactory::Create(FieldType type,
  const char* name, const char* description, size_t size)
{
  switch (type)
  {
  case kFieldType_Text:
    return std::shared_ptr<IFieldValueDescriptor>(new TextFieldDescriptor(name, description, size));
    break;
  case kFieldType_Date:
    return std::shared_ptr<IFieldValueDescriptor>(new DateFieldDescriptor(name, description));
    break;
  case kFieldType_Time:
    return std::shared_ptr<IFieldValueDescriptor>(new TimeFieldDescriptor(name, description));
    break;
  case kFieldType_Float:
    return std::shared_ptr<IFieldValueDescriptor>(new FloatFieldDescriptor(name, description, size));
    break;
  default:
    return NULL;
  }
}