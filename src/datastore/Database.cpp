
#include <datastore/Database.h>
#include <datastore/Logic.h>

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
   Brain-dead storage strategy... A list
  */
  class DatabaseInMemoryStorge
  {
  public:
    DatabaseInMemoryStorge()
    {
    }

    RowIdentifier lookupRow(const Predicate& pred) const
    {
      RowIdentifier::IdType id = 0;

      /*
      for (std::vector<Values>::const_iterator row = mRows.cbegin();
        row != mRows.cend(); ++row)
      {
        if (pred.matches(*row))
        {
          return RowIdentifier(id);
        }

        ++id;
      }
      */

      return RowIdentifier::Empty();
    }

    bool replace(const RowIdentifier& id, const Values& row)
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

    bool insert(const Values& row)
    {
      mRows.push_back(row);
      return true;
    }

  private:
    std::vector<Values> mRows;
  };
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

Database::Database(std::shared_ptr<IScheme> scheme) :
  mScheme(scheme),
  mMemory(new DatabaseInMemoryStorge())
{
  mFields = mScheme->getFieldDescriptors();
}

bool Database::insert(const FieldValues& row)
{
  if (!mScheme->allFieldsPresent(row))
  {
    return false;
  }

  //
  // From the row, select only the key fields
  //

  FieldValues keyValues;
  Selection::SelectFields(mScheme->getKeyFieldDescriptors(), 
    row, &keyValues);
  if (keyValues.empty())
  {
    return false;
  }

  //
  // Create constraint that will match iff all key fields are exact
  //

  Logic::And and;
  for (FieldValues::const_iterator key = keyValues.cbegin(); 
    key != keyValues.cend(); ++key)
  {
    std::shared_ptr<IQualifier> keyField(
      new Logic::Exact(key->getFieldDescriptor(), key->getValue()));
    and.with(keyField);
  }

  /*
  Predicate uniqueByKey(and);

  RowIdentifier found = mMemory->lookupRow(uniqueByKey);
  if (found.empty())
  {
    return mMemory->replace(found, row);
  }
  else
  {
    return mMemory->insert(row);
  }
  */

  return false;
}


