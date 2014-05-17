
#ifndef __FIELD_DESCRIPTOR_H__
#define __FIELD_DESCRIPTOR_H__

#include <datastore/Value.h>
#include <datastore/FieldType.h>
#include <datastore/PointerType.h>
#include <string>
#include <vector>

namespace DataStore
{
  typedef int FieldId;

  /**
    Provides information about a field, and a method
    for translating a string into a new instance of
    an object that represents a value of its type.
  */
  struct IFieldDescriptor
  {
    virtual const char* getName() const = 0;
    virtual const char* getDescription() const = 0;
    virtual TypeInfo getType() const = 0;
    virtual size_t getSize() const = 0;
    virtual FieldId getId() const = 0;
    virtual bool isKey() const = 0;

    virtual bool operator==(const IFieldDescriptor& other) const = 0;

    inline bool operator!=(const IFieldDescriptor& other) const
    {
      return !(*this == other);
    }

    /** Create a new instance of a Value by parsing a string, 
        NULL if parsing failed 
     */
    virtual std::shared_ptr<Value> fromString(const char* str) const = 0;
  };

  typedef PointerType<IFieldDescriptor>::Shared IFieldDescriptorPtrH;
  typedef PointerType<IFieldDescriptor>::SharedConst IFieldDescriptorConstPtrH;

  typedef std::vector<IFieldDescriptorPtrH> IFieldDescriptorList;
  typedef std::vector<IFieldDescriptorConstPtrH> IFieldDescriptorConstList;

  typedef PointerType<IFieldDescriptorList>::Shared IFieldDescriptorListPtrH;
  typedef PointerType<IFieldDescriptorList>::SharedConst IFieldDescriptorListConstPtrH;

  typedef PointerType<IFieldDescriptorConstList>::Shared IFieldDescriptorConstListPtrH;
  typedef PointerType<IFieldDescriptorConstList>::SharedConst IFieldDescriptorConstListConstPtrH;

  /**
    Given a type, and other necessary information, 
    produces an instance of an IFieldDiscriptor
  */
  class FieldDescriptorFactory
  {
  public:
    static IFieldDescriptorPtrH Create(FieldId id, TypeInfo type,
    const char* name, const char* description, bool isKey, size_t size);

  private:
    FieldDescriptorFactory();
  };

  /**
    Base implementation of a typed FieldDescriptor
  */
  class FieldDescriptorBase : public IFieldDescriptor
  {
  public:
    virtual const char* getName() const
    {
      return mName.c_str();
    }

    virtual const char* getDescription() const
    {
      return mDescrition.c_str();
    }

    virtual TypeInfo getType() const
    {
      return mType;
    }

    virtual size_t getSize() const
    {
      return mSize;
    }

    virtual bool isKey() const
    {
      return mIsKey;
    }

    virtual FieldId getId() const
    {
      return mId;
    }

    virtual bool operator==(const IFieldDescriptor& other) const
    {
      return mId == other.getId();
    }

  protected:
    FieldDescriptorBase(FieldId id, TypeInfo type, const char* name,
      const char* description, bool isKey, size_t size = 0) :
        mId(id), mType(type), mName(name), mDescrition(description),
        mIsKey(isKey), mSize(size)
    {
    }
  
  private:
    std::string mName;
    std::string mDescrition;
    TypeInfo mType;
    size_t mSize;
    FieldId mId;
    bool mIsKey;
  };

  /**
  */
  class TextFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TextFieldDescriptor(FieldId id, const char* name, 
      const char* description, bool isKey, size_t size) :
        FieldDescriptorBase(id, TypeInfo_String, name, description, isKey, size)
    {
    }

    virtual ValuePtrH fromString(const char* str) const;
  };

  /**
  */
  class DateFieldDescriptor : public FieldDescriptorBase
  {
  public:
    DateFieldDescriptor(FieldId id, const char* name, const char* description, bool isKey) :
      FieldDescriptorBase(id, TypeInfo_Date, name, description, isKey, 0)
    {
    }

    virtual ValuePtrH fromString(const char* str) const;
  };

  /**
  */
  class TimeFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TimeFieldDescriptor(FieldId id, const char* name, const char* description, bool isKey) :
      FieldDescriptorBase(id, TypeInfo_Time, name, description, isKey, 0)
    {
    }

    virtual ValuePtrH fromString(const char* str) const;
  };

  /**
  */
  class FloatFieldDescriptor : public FieldDescriptorBase
  {
  public:
    FloatFieldDescriptor(FieldId id, const char* name, 
      const char* description, bool isKey, size_t size) :
      FieldDescriptorBase(id, TypeInfo_Float, name, description, isKey, size)
    {
    }

    virtual ValuePtrH fromString(const char* str) const;
  };
}

#endif

