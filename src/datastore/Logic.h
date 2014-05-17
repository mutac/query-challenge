
#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <datastore/Row.h>
#include <datastore/PointerType.h>
#include <vector>

namespace DataStore
{
  /**
   A logical qualifier, used to constrain portions of a query
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

  /**
    Logical expression AST nodes
  */
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
    Forms the root of a logical expression tree that is used to match rows.  
    By default (default ctor), a Predicate matches everything.

    The distinction between an IQualifier and a Predicate is in place to
    facilitate expression optimization (though not implemented)
  */
  class Predicate
  {
  public:
    static const Predicate& AlwaysTrue();

    Predicate() {}
    Predicate(IQualifierPtrH qualifierRoot);
    
    /* default copy ctor okay */
    /* default dtor okay */
   
    virtual bool matches(const IRow& row) const;
  private:
    IQualifierPtrH mRoot;
  };
}

#endif
