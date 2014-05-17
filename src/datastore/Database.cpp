
#include <datastore/Database.h>
#include <datastore/Logic.h>
#include <algorithm>

using namespace DataStore;

namespace DataStore
{
  /**
  */
  class RowIdentifier
  {
    enum { kEmpty = -1 };

  public:
    typedef size_t IdType;

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

    inline bool empty() const
    {
      return mId == (IdType)kEmpty;
    }

    inline const IdType& getId() const
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
  public:
    /**
      stl algorithm compatible comparison for sorting a row by a subset of fields
      in ascending order.  The order in which the fields appear in the descriptor
      list imply sorting priority.
    */
    struct IRowOrderByFieldsAscending
    {
      IRowOrderByFieldsAscending(const IFieldDescriptorConstList* byFields) :
        mCompareField(byFields)
      {
      }

      // going through the shared pointer here will make this slower
      bool operator() (IRowConstPtrH& left, IRowConstPtrH& right)
      {
        for (IFieldDescriptorConstList::const_iterator field = mCompareField->cbegin();
          field != mCompareField->cend(); ++field)
        {
          const ValueConstPtrH leftValue = left->getValue(*field->get());
          const ValueConstPtrH rightValue = right->getValue(*field->get());

          // If a field is less than the other, and all previously compared fields
          // are equal, then the whole row is considered 'less than' an the other.

          if (*leftValue < *rightValue)
            return true;
          if (*leftValue != *rightValue)
            return false;
        }

        return false;
      }

    private:
      const IFieldDescriptorConstList* mCompareField;
    };

    /**
      A bit of an assumption here is that the IFieldDescriptor id can be
      used as an index into a vector of values (i.e. the id monotonically
      increases from zero for each field in the scheme).  There should 
      reall be some kind of contract between a scheme and database with 
      which to establish this.

      This represents a COMPLETE row.
    */
    class Row : public IRow
    {
    public:
      Row(IFieldDescriptorConstListConstPtrH fields) :
        mFields(fields)
      {
        // Reserve space for a complete row.
        mRow.resize(fields->size());
      }

      ValueConstPtrH getValue(const IFieldDescriptor& field) const
      {
        if ((size_t)field.getId() < mRow.size())
          return mRow[field.getId()];
        else
          return NULL;
      }

      IFieldDescriptorConstListConstPtrH getFieldDescriptors() const
      {
        return mFields;
      }

      bool setValue(IFieldDescriptorConstPtrH field, 
        ValuePtrH value)
      {
        if ((size_t)field->getId() < mRow.size())
        {
          mRow[field->getId()] = value;
          return true;
        }
        else
        {
          return false;
        }
      }

    private:
      std::vector<ValuePtrH> mRow;
      // Kinda lame to have each row hang on to a ref to the fields,
      // but it's convenient, as it allows a row to be passed in
      // and out of a database while keeping the type information 
      // close at hand.
      IFieldDescriptorConstListConstPtrH mFields;
    };

    /**
    Represents a read-only subset of fields within a row
    */
    class RowSelection : public IRow
    {
    public:
      RowSelection(IFieldDescriptorConstListConstPtrH selectedFields,
        IRowConstPtrH row) :
        mSelectedFields(selectedFields),
        mRow(row)
      {
      }

      ValueConstPtrH getValue(const IFieldDescriptor& field) const
      {
        if (!isSelected(field))
          return NULL;

        return mRow->getValue(field);
      }

      IFieldDescriptorConstListConstPtrH getFieldDescriptors() const
      {
        return mSelectedFields;
      }

      bool setValue(IFieldDescriptorConstPtrH field,
        ValuePtrH value)
      {
        return false; // read only
      }

    private:
      bool isSelected(const IFieldDescriptor& field) const
      {
        for (IFieldDescriptorConstList::const_iterator selectedField = mSelectedFields->cbegin();
          selectedField != mSelectedFields->cend(); ++selectedField)
        {
          if (**selectedField == field)
          {
            return true;
          }
        }

        return false;
      }

      IRowConstPtrH mRow;
      IFieldDescriptorConstListConstPtrH mSelectedFields;
    };

    typedef PointerType<RowSelection>::Shared RowSelectionPtrH;
    typedef PointerType<RowSelection>::SharedConst RowSelectionConstPtrH;

    /**
    */
    class Selection : public ISelection
    {
    public:
      Selection(IFieldDescriptorConstListConstPtrH selectedFields) :
        mSelectedFields(selectedFields)
      {
      }

      IFieldDescriptorConstListConstPtrH getFieldDescriptors() const
      {
        return mSelectedFields;
      }

      IRowConstPtrH operator[](size_t idx) const
      {
        return mSelectedRows[idx];
      }

      size_t size() const
      {
        return mSelectedRows.size();
      }

      void addRow(IRowConstPtrH row)
      {
        RowSelectionConstPtrH selectedRow(new RowSelection(mSelectedFields, row));
        mSelectedRows.push_back(selectedRow);
      }

      void orderBy(const IFieldDescriptorConstList* orderByFields)
      {
        if (orderByFields)
        {
          std::sort(mSelectedRows.begin(),
            mSelectedRows.end(),
            IRowOrderByFieldsAscending(orderByFields));
        }
      }

    private:
      IFieldDescriptorConstListConstPtrH mSelectedFields;
      std::vector<IRowConstPtrH> mSelectedRows;
    };

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    DatabaseInMemory()
    {
    }

    RowIdentifier lookupRow(const Predicate& pred) const
    {
      // RowIdentifier is kind of weird here...
      for (RowIdentifier::IdType id = 0; id < mRows.size(); ++id)
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
      if (id.getId() < mRows.size())
      {
        mRows[id.getId()] = row;
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

    ISelectionConstPtrH query(
      IFieldDescriptorConstListConstPtrH select,
      const Predicate* filterConstraint,
      IFieldDescriptorConstListConstPtrH orderBy)
    {
      Selection* selection = new Selection(select);

      for (std::vector<IRowConstPtrH>::const_iterator row = mRows.cbegin();
        row != mRows.cend(); ++row)
      {
        if (filterConstraint->matches(*(row->get())))
        {
          selection->addRow(*row);
        }
      }

      // For simplicity, delegate the sorting
      // to the selection object.
      if (orderBy)
      {
        selection->orderBy(orderBy.get());
      }

      return ISelectionConstPtrH(selection);
    }

  private:
    std::vector<IRowConstPtrH> mRows;
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
  IRowPtrH newRow(new DatabaseInMemory::Row(mFields));
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

bool Database::insert(IRowConstPtrH row, Result* pResult)
{  
  Result result = eResult_Unknown;
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
    result = eResult_Inserted;
    status = mMemory->insert(row);
  }
  else
  {
    result = eResult_Replaced;
    status = mMemory->replace(found, row);
  }

  if (pResult != NULL)
  {
    *pResult = result;
  }
  return status;
}

ISelectionConstPtrH Database::query(
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
