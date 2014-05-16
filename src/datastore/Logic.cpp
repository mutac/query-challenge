
#include <datastore/Logic.h>

using namespace DataStore;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

Predicate::Predicate(IQualifierPtrH qualifierRoot) :
  mRoot(qualifierRoot)
{
}

bool Predicate::matches(const IRow& row) const
{
  if (mRoot)
    return mRoot->matches(row);
  else
    return true;
}

const Predicate& Predicate::AlwaysTrue()
{
  static Predicate always(NULL);
  return always;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void Logic::And::with(IQualifierPtrH qualifier)
{
  mQualifiers.push_back(qualifier);
}

bool Logic::And::matches(const IRow& row) const
{
  for (IQualifierList::const_iterator qual = mQualifiers.cbegin();
    qual != mQualifiers.cend(); ++qual)
  {
    if (!(*qual)->matches(row))
      return false;
  }

  return true;
}

void Logic::And::getFieldDescriptors(IFieldDescriptorConstList* outFieldDescriptors) const
{
  for (IQualifierList::const_iterator qual = mQualifiers.cbegin();
    qual != mQualifiers.cend(); ++qual)
  {
    (*qual)->getFieldDescriptors(outFieldDescriptors);
  }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool Logic::Exact::matches(const IRow& row) const
{
  ValueConstPtrH value = row.getValue(*mExpectedField);
  if (value)
  {
    return *value == *mExpectedValue;
  }
  else
  {
    return false;
  }
}

void Logic::Exact::getFieldDescriptors(IFieldDescriptorConstList* outFieldDescriptors) const
{
  outFieldDescriptors->push_back(mExpectedField);
}