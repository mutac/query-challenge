
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <datastore/Scheme.h>
#include <datastore/DataStorage.h>
#include <Resource/mString.h>
#include <vector>

namespace DataStore
{
  class DatabaseInMemory;
  typedef PointerType<DatabaseInMemory>::Shared DatabaseInMemoryPtrH;

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
    Database(IDataStoragePtrH storage);

    // Create an in-memory only database
    Database(ISchemeConstPtrH scheme);

    ~Database();

    IRowPtrH createRow() const;
    bool insert(IRowConstPtrH row);

    ISchemeConstPtrH getScheme() const;

    void persist();

  private:
    ISchemeConstPtrH mScheme;
    IDataStoragePtrH mStorage;
    DatabaseInMemoryPtrH mMemory;
    IFieldDescriptorConstListConstPtrH mFields;
    IFieldDescriptorConstListConstPtrH mKeyFields;
  };

  typedef PointerType<Database>::Shared DatabasePtrH;
}

#endif

