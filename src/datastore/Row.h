
#ifndef __ROW_H__
#define __ROW_H__

#include <datastore/FieldDescriptor.h>
#include <datastore/Value.h>
#include <datastore/PointerType.h>

namespace DataStore
{
  /**
  */
  struct IRow
  {
    virtual ValueConstPtrH getValue(const IFieldDescriptor& field) const = 0;
    virtual IFieldDescriptorConstListConstPtrH getFieldDescriptors() const = 0;

    // The field is passed in a shared ptr so that the impl can hang on to it
    // but that might not actually ever be necessary..  Consider changing to
    // const ref like everything else.
    virtual bool setValue(IFieldDescriptorConstPtrH field,
      ValuePtrH value) = 0;
  };

  typedef PointerType<IRow>::Shared IRowPtrH;
  typedef PointerType<IRow>::SharedConst IRowConstPtrH;
}

#endif
