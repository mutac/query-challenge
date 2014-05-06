
#ifndef __FIELD_DESCRIPTOR_H__
#define __FIELD_DESCRIPTOR_H__

#include <datastore/FieldValue.h>
#include <datastore/FieldType.h>
#include <string>
#include <memory>

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

    // For simplicity serialization & deserialization can only have
    // one encoding: a string.  This could be generalized to work
    // with any encoding.
    virtual std::shared_ptr<IFieldValue> deserialize(const char* bytes, size_t size) const = 0;
  };

  /**
  */
  class FieldDescriptorFactory
  {
  public:
    static std::shared_ptr<IFieldDescriptor> Create(TypeInfo type,
    const char* name, const char* description, size_t size);

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

  protected:
    FieldDescriptorBase(TypeInfo type, const char* name,
      const char* description, size_t size = 0) :
      mType(type), mName(name), mDescrition(description), mSize(size)
    {
    }
  
  private:
    std::string mName;
    std::string mDescrition;
    TypeInfo mType;
    size_t mSize;
  };

  /**
  */
  class TextFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TextFieldDescriptor(const char* name, const char* description, size_t size) :
      FieldDescriptorBase(TypeInfo_String, name, description, size)
    {
    }

    virtual std::shared_ptr<IFieldValue> deserialize(const char* bytes, size_t size) const;
  };

  /**
  */
  class DateFieldDescriptor : public FieldDescriptorBase
  {
  public:
    DateFieldDescriptor(const char* name, const char* description) :
      FieldDescriptorBase(TypeInfo_Date, name, description, 0)
    {
    }

    virtual std::shared_ptr<IFieldValue> deserialize(const char* bytes, size_t size) const;
  };

  /**
  */
  class TimeFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TimeFieldDescriptor(const char* name, const char* description) :
      FieldDescriptorBase(TypeInfo_Time, name, description, 0)
    {
    }

    virtual std::shared_ptr<IFieldValue> deserialize(const char* bytes, size_t size) const;
  };

  /**
  */
  class FloatFieldDescriptor : public FieldDescriptorBase
  {
  public:
    FloatFieldDescriptor(const char* name, const char* description, size_t size) :
      FieldDescriptorBase(TypeInfo_Float, name, description, size)
    {
    }

    virtual std::shared_ptr<IFieldValue> deserialize(const char* bytes, size_t size) const;
  };
}

#endif

