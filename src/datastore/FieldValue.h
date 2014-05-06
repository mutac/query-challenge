
#ifndef __IFieldValue_H__
#define __IFieldValue_H__

#include <resource/Variant.h>

namespace DataStore
{
  /**
  */
  struct IFieldValue
  {
    virtual const mStd::Variant& getValue() const = 0;
  };

  /**
  */
  class FieldValue : public IFieldValue
  {
  public:
    FieldValue(const mStd::Variant& v) :
      mValue(v)
    {
    }

    /** Short-cut:  Variant must be assinable to T */
    template<typename T>
    FieldValue(const T& v) :
      mValue(v)
    {
    }

    const mStd::Variant& getValue() const
    {
      return mValue;
    }

  private:
    mStd::Variant mValue;
  };
}

#endif