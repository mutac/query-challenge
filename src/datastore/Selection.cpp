
#include <datastore/Selection.h>

using namespace DataStore;

/*
bool Selection::SelectFields(const IFieldDescriptors& fieldDescriptors,
  const FieldValues& inFieldValues,
  FieldValues* outSelection)
{
  // woooooo

  for (FieldValues::const_iterator value = inFieldValues.cbegin(); 
    value != inFieldValues.cend(); ++value)
  {
    for (IFieldDescriptors::const_iterator field = fieldDescriptors.cbegin();
      field != fieldDescriptors.cend(); ++field)
    {
      if (**field == *(value->getFieldDescriptor()))
      {
        outSelection->push_back(*value);
        break;
      }
    }
  }

  return !outSelection->empty();
}

bool Selection::SelectField(const IFieldDescriptor& fieldDescriptor,
  const FieldValues& inFieldValues, FieldValue* outValue)
{
  // weeeeeeeee

  for (FieldValues::const_iterator value = inFieldValues.cbegin();
    value != inFieldValues.cend(); ++value)
  {
    if (fieldDescriptor == *(value->getFieldDescriptor()))
    {
      *outValue = *value;
      return true;
    }
  }

  return false;
}
*/