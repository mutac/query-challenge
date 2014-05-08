
#ifndef __FieldValue_H__
#define __FieldValue_H__

#include <datastore/Value.h>
#include <datastore/FieldDescriptor.h>
#include <vector>

namespace DataStore
{
  /**
  */
  class FieldValue
  {
  public:
    FieldValue(std::shared_ptr<IFieldDescriptor> fieldDescriptor,
      std::shared_ptr<Value> value) :
        mValue(value), mFieldDescriptor(fieldDescriptor)
    {
      mAssert(value);
      mAssert(fieldDescriptor);
    }

    const std::shared_ptr<Value> getValue() const
    {
      return mValue;
    }

    const std::shared_ptr<IFieldDescriptor> getFieldDescriptor() const
    {
      return mFieldDescriptor;
    }

  private:
    std::shared_ptr<Value> mValue;
    std::shared_ptr<IFieldDescriptor> mFieldDescriptor;
  };

  /**
  */
  typedef std::vector<FieldValue> FieldValues;
}

#endif