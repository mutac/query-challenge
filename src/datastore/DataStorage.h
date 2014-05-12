
#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <datastore/Scheme.h>

namespace DataStore
{
  class Database;
  struct IRow;

  struct IDataStorage
  {
    virtual ISchemeConstPtrH getScheme() = 0;
    virtual void load(Database* database) = 0;

    virtual void beginPersist() = 0;
    virtual void persistRow(const IRow* row) = 0;
    virtual void endPersist() = 0;
  };

  typedef PointerType<IDataStorage>::Shared IDataStoragePtrH;
}

#endif
