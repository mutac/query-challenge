
#ifndef __FIELD_TYPES_H__
#define __FIELD_TYPES_H__

#include <Resource/TypeInfo.h>
#include <Resource/Variant.h>
#include <Resource/mString.h>
#include <time.h>
#include <inttypes.h>

namespace DataStore
{
  //
  // These are the supported types of the datastore
  //

  // Reuse pre-defined types
  using mStd::TypeInfo;
  using mStd::TypeInfo_Empty;
  using mStd::TypeInfo_String;
  using mStd::TypeInfo_Float;

  // Begin user types at 20
  static const TypeInfo TypeInfo_Date = 20;
  static const TypeInfo TypeInfo_Time = 21;

  /**
    Date type: YYYY-MM-DD
  */
  class Date
  {
  public:
    Date() :
      mDate(0)
    {
    }

    bool fromString(const char* str);
    mStd::mString toString() const;

    bool operator==(const Date& other) const
    {
      return other.mDate == mDate;
    }
    bool operator!=(const Date& other) const
    {
      return other.mDate != mDate;
    }

  private:
    // Could use smaller type
    time_t mDate;
  };

  /**
    Time type: HH:SSSS
  */
  class Time
  {
  public:
    Time() :
      mTime(0)
    {
    }

    bool fromString(const char* str);
    mStd::mString toString() const;

    const bool operator==(const Time& other) const
    {
      return other.mTime == mTime;
    }
    const bool operator!=(const Time& other) const
    {
      return other.mTime != mTime;
    }

  private:
    uint32_t mTime;
  };
}

namespace mStd
{
  //
  // Define necessary fiddly bits to allow Date and Time classes
  // can be stored in a Variant.
  //

  template<>
  inline const TypeInfo& type_of<DataStore::Date>()
  {
    return DataStore::TypeInfo_Date;
  }

  template<>
  bool type_conversion(const DataStore::Date& from, 
    const TypeInfo& toType, Variant* to);

  template<>
  inline const TypeInfo& type_of<DataStore::Time>()
  {
    return DataStore::TypeInfo_Time;
  }

  template<>
  bool type_conversion(const DataStore::Time& from,
    const TypeInfo& toType, Variant* to);
}

#endif
