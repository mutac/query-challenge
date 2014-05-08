
#ifndef __FIELD_DESCRIPTOR_H__
#define __FIELD_DESCRIPTOR_H__

#include <datastore/Value.h>
#include <datastore/FieldType.h>
#include <string>
#include <memory>
#include <vector>

namespace DataStore
{
  /**
  */
  struct IFieldDescriptor
  {
    virtual const char* getName() const = 0;
    virtual const char* getDescription() const = 0;
    virtual TypeInfo getType() const = 0;
    virtual size_t getSize() const = 0;
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

  /**
  */
  typedef std::vector<std::shared_ptr<IFieldDescriptor> > IFieldDescriptors;

  /**
  */
  class FieldDescriptorFactory
  {
  public:
    static std::shared_ptr<IFieldDescriptor> Create(TypeInfo type,
    const char* name, const char* description, bool isKey, size_t size);

  private:
    FieldDescriptorFactory();
  };

  /**
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

    virtual bool operator==(const IFieldDescriptor& other) const
    {
      return mType == other.getType() &&
        mName == other.getName() &&
        mSize == other.getSize() &&
        mIsKey == other.isKey();
    }

  protected:
    FieldDescriptorBase(TypeInfo type, const char* name,
      const char* description, bool isKey, size_t size = 0) :
        mType(type), mName(name), mDescrition(description),
        mIsKey(isKey), mSize(size)
    {
    }
  
  private:
    std::string mName;
    std::string mDescrition;
    TypeInfo mType;
    size_t mSize;
    bool mIsKey;
  };

  /**
  */
  class TextFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TextFieldDescriptor(const char* name, 
      const char* description, bool isKey, size_t size) :
        FieldDescriptorBase(TypeInfo_String, name, description, isKey, size)
    {
    }

    virtual std::shared_ptr<Value> fromString(const char* str) const;
  };

  /**
  */
  class DateFieldDescriptor : public FieldDescriptorBase
  {
  public:
    DateFieldDescriptor(const char* name, const char* description, bool isKey) :
      FieldDescriptorBase(TypeInfo_Date, name, description, isKey, 0)
    {
    }

    virtual std::shared_ptr<Value> fromString(const char* str) const;
  };

  /**
  */
  class TimeFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TimeFieldDescriptor(const char* name, const char* description, bool isKey) :
      FieldDescriptorBase(TypeInfo_Time, name, description, isKey, 0)
    {
    }

    virtual std::shared_ptr<Value> fromString(const char* str) const;
  };

  /**
  */
  class FloatFieldDescriptor : public FieldDescriptorBase
  {
  public:
    FloatFieldDescriptor(const char* name, 
      const char* description, bool isKey, size_t size) :
      FieldDescriptorBase(TypeInfo_Float, name, description, isKey, size)
    {
    }

    virtual std::shared_ptr<Value> fromString(const char* str) const;
  };
}

#endif

