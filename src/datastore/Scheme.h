/**
*/

#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <datastore/FieldDescriptor.h>
#include <datastore/PointerType.h>
#include <vector>

namespace DataStore
{
  /**
  */
  struct IScheme
  {
    /** Returns true if header field names match scheme */
    virtual bool allFieldsPresent(const std::vector<std::string>& headerFieldNames) const = 0;
    
    /** Returns true if values comprises the complete set of type-correct values */
    virtual bool allFieldsPresent(const IFieldDescriptorList& fields) const = 0;

    /** Return fields of scheme */
    virtual IFieldDescriptorConstListConstPtrH getFieldDescriptors() const = 0;

    /** Return only the key fields */
    virtual IFieldDescriptorConstListConstPtrH getKeyFieldDescriptors() const = 0;
  };

  typedef PointerType<IScheme>::Shared ISchemePtrH;
  typedef PointerType<IScheme>::SharedConst ISchemeConstPtrH;
}

#endif

