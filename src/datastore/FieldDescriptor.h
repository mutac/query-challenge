
#ifndef __IFIELD_DESCRIPTOR_H__
#define __IFIELD_DESCRIPTOR_H__

#include <datastore/IField.h>
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
    virtual FieldType getType() const = 0;
    virtual size_t getSize() const = 0;
    /*
    virtual std::shared_ptr<IField> tryParse(const char* fieldString) const = 0;
    */
    virtual std::shared_ptr<IField> tryParse(const char* fieldString) const = 0;
  };

  /**
  */
  class FieldDescriptorFactory
  {
  public:
    static std::shared_ptr<IFieldDescriptor> Create(FieldType type,
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

    virtual FieldType getType() const
    {
      return mType;
    }

    virtual size_t getSize() const
    {
      return mSize;
    }

  protected:
    FieldDescriptorBase(FieldType type, const char* name,
      const char* description, size_t size = 0) :
      mType(type), mName(name), mDescrition(description), mSize(size)
    {
    }
  
  private:
    std::string mName;
    std::string mDescrition;
    FieldType mType;
    size_t mSize;
  };

  /**
  */
  class TextFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TextFieldDescriptor(const char* name, const char* description, size_t size) :
      FieldDescriptorBase(kFieldType_Text, name, description, size)
    {
    }

    virtual std::shared_ptr<IField> tryParse(const char* fieldString) const;
  };

  /**
  */
  class DateFieldDescriptor : public FieldDescriptorBase
  {
  public:
    DateFieldDescriptor(const char* name, const char* description) :
      FieldDescriptorBase(kFieldType_Date, name, description, 0)
    {
    }

    virtual std::shared_ptr<IField> tryParse(const char* fieldString) const;
  };

  /**
  */
  class TimeFieldDescriptor : public FieldDescriptorBase
  {
  public:
    TimeFieldDescriptor(const char* name, const char* description) :
      FieldDescriptorBase(kFieldType_Date, name, description, 0)
    {
    }

    virtual std::shared_ptr<IField> tryParse(const char* fieldString) const;
  };

  /**
  */
  class FloatFieldDescriptor : public FieldDescriptorBase
  {
  public:
    FloatFieldDescriptor(const char* name, const char* description, size_t size) :
      FieldDescriptorBase(kFieldType_Date, name, description, size)
    {
    }

    virtual std::shared_ptr<IField> tryParse(const char* fieldString) const;
  };
}

#endif

