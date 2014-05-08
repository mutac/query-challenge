
#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <datastore/FieldValue.h>

namespace DataStore
{
  /**
  */
  struct IQualifier
  {
    virtual bool matches(const FieldValue& fieldValue) const = 0;
  };

  /**
  */
  typedef std::vector<std::shared_ptr<IQualifier> > IQualifiers;

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

      void with(std::shared_ptr<IQualifier> qualifier);
      bool matches(const FieldValue& value) const;

    private:
      IQualifiers mQualifiers;
    };

    /**
    */
    class Exact : public IQualifier
    {
    public:
      Exact(std::shared_ptr<IFieldDescriptor> field,
        std::shared_ptr<Value> expected) :
        mField(field), mExpected(expected)
      {
      }

      bool matches(const FieldValue& value) const;

    private:
      std::shared_ptr<IFieldDescriptor> mField;
      std::shared_ptr<Value> mExpected;
    };
  }

  /**
  */
  class Predicate
  {
  public:
    Predicate(std::shared_ptr<IQualifier> qualifierRoot) :
      mRoot(qualifierRoot)
    {
    }

    bool matches(const FieldValue& values) const;

  private:
    std::shared_ptr<IQualifier> mRoot;
  };
}

#endif
