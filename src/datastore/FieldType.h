
#ifndef __FIELD_TYPES_H__
#define __FIELD_TYPES_H__

namespace DataStore
{
  typedef enum
  {
    kFieldType_Unknown,
    kFieldType_Text,
    kFieldType_Float,
    kFieldType_Date,
    kFieldType_Time
  } FieldType;
}

#endif
