
#include <datastore/Logic.h>

using namespace DataStore;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

Predicate::Predicate(IQualifierPtrH qualifierRoot) :
  mSearchFields(new IFieldDescriptorConstList()),
  mRoot(qualifierRoot)
{
  compile();
}

void Predicate::compile()
{
  // Collect the fields that need to be searched
  mSearchFields->clear();
  mRoot->getFieldDescriptors(mSearchFields.get());
}

bool Predicate::matches(const IRow& row) const
{
  return mRoot->matches(row);
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