
#include <datastore/Database.h>
#include <datastore/Logic.h>
#include <hash_map>

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
    Bread-dead storage...
  */
  class DatabaseInMemory
  {
  public:

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

    DatabaseInMemory()
    {
    }

    RowIdentifier lookupRow(const Predicate& pred) const
    {
      // TODO: Fix RowIdentifier.. too weird
      for (RowIdentifier::IdType id = 0; id < mRows.size(); ++id)
      {
        if (pred.matches(*mRows[id]))
        {
          return RowIdentifier(id);
        }

        ++id;
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
}

Database::Database(ISchemeConstPtrH scheme) :
  mScheme(scheme),
  mMemory(new DatabaseInMemory())
{
  mFields = mScheme->getFieldDescriptors();
  mKeyFields = mScheme->getKeyFieldDescriptors();
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

bool Database::insert(IRowConstPtrH row)
{  
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
  if (!found.empty())
  {
    return mMemory->replace(found, row);
  }
  else
  {
    return mMemory->insert(row);
  }
}


