
#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <datastore/Scheme.h>

namespace DataStore
{
  struct IDataStorage
  {
    virtual ISchemeConstPtrH getScheme() = 0;
  };

  typedef PointerType<IDataStorage>::Shared IDataStoragePtrH;
}

#endif
