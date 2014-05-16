
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

      IRowConstPtrH operator*() const
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
     If select is not specified (NULL), all fields are selected
    */
    ISelectionConstPtrH query(IFieldDescriptorConstListConstPtrH select);

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

