
#include <datastore/Logic.h>

using namespace DataStore;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool Predicate::matches(const FieldValue& values) const
{
  return mRoot->matches(values);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void Logic::And::with(std::shared_ptr<IQualifier> qualifier)
{
  mQualifiers.push_back(qualifier);
}

bool Logic::And::matches(const FieldValue& value) const
{
  bool match = false;
  for (IQualifiers::const_iterator qual = mQualifiers.cbegin();
    qual != mQualifiers.cend(); ++qual)
  {
    if (!(*qual)->matches(value))
    {
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool Logic::Exact::matches(const FieldValue& field) const
{
  return false;
}