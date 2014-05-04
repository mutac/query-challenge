
#ifndef __IFIELD_H__
#define __IFIELD_H__

#include <datastore/FieldType.h>
#include <tclap/CmdLine.h>
#include <memory>

namespace DataStore
{
  /**
  */
  class FieldDescriptor
  {
  public:
    FieldDescriptor();
    ~FieldDescriptor();

    const char* getName() const;
    const char* getDescription() const;
    FieldType getType() const;
    size_t getSize() const;
  };

  /**
  */
  struct IField
  {
    virtual const FieldDescriptor& getDescriptor() = 0;
  };

  /**
  */
  struct IFieldParser
  {
    virtual std::shared_ptr<IField> tryParse(const char* fieldString) = 0;
  };
}

#endif