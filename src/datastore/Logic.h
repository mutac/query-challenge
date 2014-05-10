
#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <datastore/Database.h>
#include <datastore/PointerType.h>
#include <vector>

namespace DataStore
{
  /**
  */
  struct IQualifier
  {
    virtual bool matches(const IRow& row) const = 0;
    virtual void getFieldDescriptors(IFieldDescriptorConstList* outFieldDescriptors) const = 0;
  };

  typedef PointerType<IQualifier>::Shared IQualifierPtrH;
  typedef PointerType<IQualifier>::SharedConst IQualifierConstPtrH;
  typedef std::vector<IQualifierPtrH> IQualifierList;
  typedef PointerType<IQualifierList>::Shared IQualifierListPtrH;

  namespace Logic
  {
    /**
    */
    class And : public IQualifier
    {
    public:
      And()
      {
      }

      void with(IQualifierPtrH qualifier);
      bool matches(const IRow& row) const;
      void getFieldDescriptors(IFieldDescriptorConstList* outFieldDescriptors) const;

    private:
      IQualifierList mQualifiers;
    };

    /**
    */
    class Exact : public IQualifier
    {
    public:
      Exact(IFieldDescriptorConstPtrH expectedField, ValueConstPtrH value) :
        mExpectedField(expectedField), mExpectedValue(value)
      {
      }

      bool matches(const IRow& row) const;
      void getFieldDescriptors(IFieldDescriptorConstList* outFieldDescriptors) const;

    private:
      IFieldDescriptorConstPtrH mExpectedField;
      ValueConstPtrH mExpectedValue;
    };
  }

  /**
  */
  class Predicate
  {
  public:
    Predicate(IQualifierPtrH qualifierRoot);

    bool matches(const IRow& row) const;

  private:
    void compile();

    IQualifierPtrH mRoot;
    IFieldDescriptorConstListPtrH mSearchFields;
  };
}

#endif
