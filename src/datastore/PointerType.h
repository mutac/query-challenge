
#ifndef __POINTER_TYPE_H__
#define __POINTER_TYPE_H__

#include <memory>

namespace DataStore
{
  template <typename T>
  struct PointerType
  {
    typedef std::shared_ptr<T> Shared;
    typedef std::shared_ptr<const T> SharedConst;
    typedef std::unique_ptr<T> Unique;
  };
}

#endif