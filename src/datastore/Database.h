
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <datastore/Scheme.h>
#include <datastore/PointerType.h>
#include <vector>

namespace DataStore
{
  class DatabaseInMemory;
  typedef PointerType<DatabaseInMemory>::Shared DatabaseInMemoryPtrH;

  class DatabaseOnDiskMemory;
  typedef PointerType<DatabaseOnDiskMemory>::Shared DatabaseOnDiskMemoryPtrH;

  /**
  */
  struct IRow
  {
    virtual ValueConstPtrH getValue(const IFieldDescriptor& field) const = 0;
    virtual IFieldDescriptorConstListConstPtrH getFieldDescriptors() const = 0;

    // The field is passed in a shared ptr so that the impl can hang on to it
    // but that might not actually ever be necessary..  Consider changing to
    // const ref like everything else.
    virtual bool setValue(IFieldDescriptorConstPtrH field, 
      ValuePtrH value) = 0;
  };

  typedef PointerType<IRow>::Shared IRowPtrH;
  typedef PointerType<IRow>::SharedConst IRowConstPtrH;

  /**
  */
  class Database
  {
  public:
    Database(ISchemePtrH scheme);

    IRowPtrH createRow() const;

    bool insert(IRowConstPtrH row);

  private:
    ISchemePtrH mScheme;
    DatabaseInMemoryPtrH mMemory;
    DatabaseOnDiskMemoryPtrH mDisk;
    IFieldDescriptorConstListConstPtrH mFields;
  };
}

#endif

