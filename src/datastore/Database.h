
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <datastore/Scheme.h>
#include <datastore/DataStorage.h>
#include <datastore/Row.h>
#include <datastore/Logic.h>

namespace DataStore
{
  class DatabaseInMemory;
  typedef PointerType<DatabaseInMemory>::Shared DatabaseInMemoryPtrH;

  /**
   A selection of particular fields and rows from a database.
   Outard appearance is similar to std::vector
  */
  struct ISelection
  {
    /**
    */
    class const_iterator
    {
    public:
      const_iterator() :
        mIdx((size_t)-1)
      {
      }

      const_iterator(size_t startIdx, const ISelection* selection) :
        mIdx(startIdx), mSelection(selection)
      {
      }

      // default dtor, copy-ctor okay

      const_iterator& operator++()
      {
        ++mIdx;
        return *this;
      }

      bool operator==(const const_iterator& other) const
      {
        return mSelection == other.mSelection && mIdx == other.mIdx;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(mSelection == other.mSelection && mIdx == other.mIdx);
      }

      const IRowConstPtrH operator*() const
      {
        return (*mSelection)[mIdx];
      }

    private:
      size_t mIdx;
      const ISelection* mSelection;
    };

    ///////////////////////////////////////////////////////////////////////

    virtual IFieldDescriptorConstListConstPtrH getFieldDescriptors() const = 0;
    virtual IRowConstPtrH operator[](size_t idx) const = 0;
    virtual size_t size() const = 0;

    inline const_iterator cbegin() const
    {
      return const_iterator(0, this);
    }

    inline const_iterator cend() const
    {
      return const_iterator(size(), this);
    }
  };

  typedef PointerType<ISelection>::Shared ISelectionPtrH;
  typedef PointerType<ISelection>::SharedConst ISelectionConstPtrH;

  /**
  */
  class Database
  {
  public:
    /**
      The result of an operation on a Database
    */
    typedef enum
    {
      eResult_Unknown,
      eResult_Replaced,
      eResult_Inserted
    } Result;

    /**
      Creates a database with a storage-back
    */
    Database(IDataStoragePtrH storage);

    /** Create an in-memory only database 
    */
    Database(ISchemeConstPtrH scheme);

    ~Database();

    /** Create a new empty row that can be inserted into the database
    */
    IRowPtrH createRow() const;

    /** Insert row into database.  A duplicate row will be replaced
    */
    bool insert(IRowConstPtrH row, Result* pResult = NULL);

    /**
     If select is not specified (NULL), all fields are selected.
     If filterConstraint is not specified (NULL), all rows are selected.
     if orderBy is not specified (NULL), the order is undefined.
    */
    ISelectionConstPtrH query(
      IFieldDescriptorConstListConstPtrH select,
      const Predicate* filterConstraint,
      IFieldDescriptorConstListConstPtrH orderBy);

    /**
    */
    ISchemeConstPtrH getScheme() const;

    /** Persist in-memory portion of databas 
    */
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

