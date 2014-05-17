
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
  struct IQueryResult
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

      const_iterator(size_t startIdx, const IQueryResult* selection) :
        mIdx(startIdx), mResult(selection)
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
        return mResult == other.mResult && mIdx == other.mIdx;
      }

      bool operator!=(const const_iterator& other) const
      {
        return !(mResult == other.mResult && mIdx == other.mIdx);
      }

      const IRowConstPtrH operator*() const
      {
        return (*mResult)[mIdx];
      }

    private:
      size_t mIdx;
      const IQueryResult* mResult;
    };

    ///////////////////////////////////////////////////////////////////////

    /** Returns fields that selected within result */
    virtual IFieldDescriptorConstListConstPtrH getFieldDescriptors() const = 0;

    /** Return the ith result */
    virtual IRowConstPtrH operator[](size_t idx) const = 0;

    /** Returns total number of results */
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

  typedef PointerType<IQueryResult>::Shared IQueryResultPtrH;
  typedef PointerType<IQueryResult>::SharedConst IQueryResultConstPtrH;

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
      eInsertionResult_Unknown,
      eInsertionResult_Replaced,
      eInsertionResult_Inserted
    } InsertionResult;

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
    bool insert(IRowConstPtrH row, InsertionResult* pResult = NULL);

    /**
     If select is not specified (NULL), all fields are selected.
     If filterConstraint is not specified (NULL), all rows are selected.
     if orderBy is not specified (NULL), the order is undefined.
    */
    IQueryResultConstPtrH query(
      IFieldDescriptorConstListConstPtrH select = NULL,
      const Predicate* filterConstraint = NULL,
      IFieldDescriptorConstListConstPtrH orderBy = NULL);

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

