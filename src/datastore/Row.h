
#ifndef __ROW_H__
#define __ROW_H__

#include <datastore/FieldDescriptor.h>
#include <datastore/Value.h>
#include <datastore/PointerType.h>

namespace DataStore
{
  /**
    Represents a row of values within a database.
  */
  struct IRow
  {
    virtual ValueConstPtrH getValue(const IFieldDescriptor& field) const = 0;
    virtual bool setValue(const IFieldDescriptor& field, ValuePtrH value) = 0;
  };

  typedef PointerType<IRow>::Shared IRowPtrH;
  typedef PointerType<IRow>::SharedConst IRowConstPtrH;
}

#endif
