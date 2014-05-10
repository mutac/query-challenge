
#ifndef __VALUE_H__
#define __VALUE_H__

#include <resource/Variant.h>
#include <datastore/PointerType.h>

namespace DataStore
{
  /**
  */
  class Value
  {
  public:
    Value(const mStd::Variant& v) :
      mValue(v)
    {
    }

    template<typename T>
    Value(const T& v) :
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

  typedef PointerType<Value>::Shared ValuePtrH;
  typedef PointerType<Value>::SharedConst ValueConstPtrH;
}

#endif

