
#include <datastore/Database.h>
#include <datastore/Logic.h>
#include <algorithm>

using namespace DataStore;

namespace DataStore
{
  /**
    Identifies a row within the in-memory database, It's really just
    a size_t...
  */
  class RowIdentifier
  {
    enum { kEmpty = -1 };
    typedef size_t IdType;
  public:

    inline static RowIdentifier Empty()
    {
      return RowIdentifier();
    }

    inline RowIdentifier() :
      mId((IdType)kEmpty)
    {
    }

    inline RowIdentifier(const IdType& id) :
      mId(id)
    {
    }

    inline RowIdentifier& operator++()
    {
      ++mId;
      return *this;
    }

    inline bool operator<(const IdType& id) const
    {
      return mId < id;
    }

    inline bool empty() const
    {
      return mId == (IdType)kEmpty;
    }

    inline const IdType& getId() const
    {
      return mId;
    }

    inline operator IdType() const
    {
      return mId;
    }

  private:
    IdType mId;
  };

  /**
    Bread-dead storage, stores entire database in memory using
    vectors of rows.
  */
  class DatabaseInMemory
  {
  protected:
    typedef std::vector<IRowConstPtrH> IRowConstList;
    typedef PointerType<IRowConstList>::Shared IRowConstListPtrH;
    typedef PointerType<IRowConstList>::SharedConst IRowConstListConstPtrH;

  public:
    /**
      stl algorithm compatible comparison for sorting a row by a subset of fields
      in ascending order.  The order in which the fields appear in the descriptor
      list imply sorting priority.
    */
    struct IRowOrderByFieldsAscending
    {
      IRowOrderByFieldsAscending(const IFieldDescriptorConstList& byFields) :
        mCompareField(byFields)
      {
      }

      // going through the shared pointer here will make this slower
      bool operator() (IRowConstPtrH& left, IRowConstPtrH& right)
      {
        for (IFieldDescriptorConstList::const_iterator field = mCompareField.cbegin();
          field != mCompareField.cend(); ++field)
        {
          const ValueConstPtrH leftValue = left->getValue(*field->get());
          const ValueConstPtrH rightValue = right->getValue(*field->get());

          // All previous fields are equal, and this one is less than the other
          if (*leftValue < *rightValue)
            return true;

          // If not equal, don't continue looking at other fields
          if (*leftValue != *rightValue)
            return false;
        }

        return false;
      }

    private:
      const IFieldDescriptorConstList& mCompareField;
    };

    /**
      A bit of an assumption here is that the IFieldDescriptor id can be
      used as an index into a vector of values (i.e. the id monotonically
      increases from zero for each field in the scheme).  There should 
      reall be some kind of contract between a scheme and database with 
      which to establish this.
    */
    class Row : public IRow
    {
    public:
      Row(const IFieldDescriptorConstList& fields)
      {
        // Reserve space for a complete row.
        mRow.resize(fields.size());
      }

      ValueConstPtrH getValue(const IFieldDescriptor& field) const
      {
        if ((size_t)field.getId() < mRow.size())
          return mRow[field.getId()];
        else
          return NULL;
      }

      bool setValue(const IFieldDescriptor& field, 
        ValuePtrH value)
      {
        if ((size_t)field.getId() < mRow.size())
        {
          mRow[field.getId()] = value;
          return true;
        }
        else
        {
          return false;
        }
      }

    private:
      std::vector<ValuePtrH> mRow;
    };

    /**
      A thin container that stores a reference to a subset of rows from
      the database, and a reference to a subset of fields.  By the time
      a row subset makes it into a Result, it has already been ordered.
    */
    class Result : public IQueryResult
    {
    public:
      Result(IFieldDescriptorConstListConstPtrH selectedFields,
        IRowConstListConstPtrH selectedRows) :
        mSelectedFields(selectedFields),
        mSelectedRows(selectedRows)
      {
      }

      IFieldDescriptorConstListConstPtrH getFieldDescriptors() const
      {
        return mSelectedFields;
      }

      IRowConstPtrH operator[](size_t idx) const
      {
        return (*mSelectedRows)[idx];
      }

      size_t size() const
      {
        return mSelectedRows->size();
      }

    private:
      IFieldDescriptorConstListConstPtrH mSelectedFields;
      IRowConstListConstPtrH mSelectedRows;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    DatabaseInMemory()
    {
    }

    RowIdentifier lookupRow(const Predicate& pred) const
    {
      for (RowIdentifier id(0); id < mRows.size(); ++id)
      {
        if (pred.matches(*mRows[id]))
        {
          return RowIdentifier(id);
        }
      }

      return RowIdentifier::Empty();
    }

    bool replace(const RowIdentifier& id, IRowConstPtrH row)
    {
      if (id < mRows.size())
      {
        mRows[id] = row;
        return true;
      }
      else
      {
        return false;
      }
    }

    bool insert(IRowConstPtrH row)
    {
      mRows.push_back(row);
      return true;
    }

    void persist(IDataStorage* storage)
    {
      for (std::vector<IRowConstPtrH>::const_iterator row = mRows.cbegin();
        row != mRows.cend(); ++row)
      {
        storage->persistRow(row->get());
      }
    }

    IQueryResultConstPtrH query(
      IFieldDescriptorConstListConstPtrH selectFields,
      const Predicate* filterConstraint,
      IFieldDescriptorConstListConstPtrH orderBy)
    {
      IRowConstListPtrH selectedRows(new IRowConstList());

      for (std::vector<IRowConstPtrH>::const_iterator row = mRows.cbegin();
        row != mRows.cend(); ++row)
      {
        if (filterConstraint->matches(*(row->get())))
        {
          selectedRows->push_back(*row);
        }
      }

      if (orderBy)
      {
        std::sort(selectedRows->begin(),
          selectedRows->end(),
          IRowOrderByFieldsAscending(*(orderBy.get())));
      }

      return IQueryResultConstPtrH(
        new Result(selectFields, selectedRows));
    }

  private:
    IRowConstList mRows;
  };
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

Database::Database(IDataStoragePtrH storage) :
  mStorage(storage),
  mScheme(storage->getScheme()),
  mMemory(new DatabaseInMemory())
{
  mFields = mScheme->getFieldDescriptors();
  mKeyFields = mScheme->getKeyFieldDescriptors();

  storage->load(this);
}

Database::Database(ISchemeConstPtrH scheme) :
  mScheme(scheme),
  mMemory(new DatabaseInMemory())
{
  mFields = mScheme->getFieldDescriptors();
  mKeyFields = mScheme->getKeyFieldDescriptors();
}

Database::~Database()
{
  persist();
}

ISchemeConstPtrH Database::getScheme() const
{
  return mScheme;
}

IRowPtrH Database::createRow() const
{
  IRowPtrH newRow(new DatabaseInMemory::Row(*(mFields.get())));
  return newRow;
}

void Database::persist()
{
  if (mStorage)
  {
    mStorage->beginPersist();
    mMemory->persist(mStorage.get());
    mStorage->endPersist();
  }
}

bool Database::insert(IRowConstPtrH row, InsertionResult* pResult)
{  
  InsertionResult result = eInsertionResult_Unknown;
  bool status = false;

  //
  // Create constraint that will match iff all key fields are exact
  //

  Logic::And* and = new Logic::And();
  for (IFieldDescriptorConstList::const_iterator keyField = mKeyFields->cbegin();
    keyField != mKeyFields->cend(); ++keyField)
  {
    IFieldDescriptorConstPtrH field = *keyField;

    IQualifierPtrH exactKeyValue(new Logic::Exact(field, row->getValue(*field)));
    and->with(exactKeyValue);
  }

  IQualifierPtrH exactKeys(and);
  Predicate matchExactKeys(exactKeys);

  RowIdentifier found = mMemory->lookupRow(matchExactKeys);
  if (found.empty())
  {
    result = eInsertionResult_Inserted;
    status = mMemory->insert(row);
  }
  else
  {
    result = eInsertionResult_Replaced;
    status = mMemory->replace(found, row);
  }

  if (pResult != NULL)
  {
    *pResult = result;
  }
  return status;
}

IQueryResultConstPtrH Database::query(
  IFieldDescriptorConstListConstPtrH selectFields,
  const Predicate* filterConstraint,
  IFieldDescriptorConstListConstPtrH orderBy)
{
  IFieldDescriptorConstListConstPtrH select = selectFields;
  if (!select || select->empty())
  {
    select = mScheme->getFieldDescriptors();
  }

  const Predicate* filter = &Predicate::AlwaysTrue();
  if (filterConstraint != NULL)
  {
    filter = filterConstraint;
  }

  return mMemory->query(select, filter, orderBy);
}
