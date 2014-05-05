
#ifndef __IFieldValue_H__
#define __IFieldValue_H__

#include <resource/Variant.h>

namespace DataStore
{
  /**
  */
  struct IFieldValue
  {
    virtual const mResource::Variant& getValue() const = 0;
  };

  /**
  */
  class FieldValue : public IFieldValue
  {
  public:
    FieldValue(const mResource::Variant& v) :
      mValue(v)
    {
    }

    const mResource::Variant& getValue() const
    {
      return mValue;
    }

  private:
    mResource::Variant mValue;
  };
}

#endif