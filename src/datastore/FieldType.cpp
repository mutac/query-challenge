
#include <datastore/FieldType.h>
#include <string.h>
#include <time.h>

using namespace DataStore;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool Date::fromString(const char* str)
{
  int year = 0;
  int month = 0;
  int day = 0;  

  int found = sscanf(str, "%d-%d-%d", &year, &month, &day);
  if (found == 3)
  {
    struct tm timeDetails = { 0 };
    timeDetails.tm_year = year - 1900;
    timeDetails.tm_mon = month - 1;
    timeDetails.tm_mday = day;

    mDate = mktime(&timeDetails);
    return mDate != -1;
  }
  else
  {
    return false;
  }
}

mStd::mString Date::toString() const
{
  struct tm timeDetails = { 0 };

#ifdef _MSC_VER
  localtime_s(&timeDetails, &mDate);
#else
  localtime_r(&mDate, &timeDetails);
#endif

  char v[11 + 1];

#ifdef _MSC_VER
  sprintf_s(
#else
  snprintf(
#endif
    v, sizeof(v), "%04d-%02d-%02d",
    timeDetails.tm_year + 1900,
    timeDetails.tm_mon + 1,
    timeDetails.tm_mday);
  return mStd::mString(v);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool Time::fromString(const char* str)
{
  int hour = 0;
  int minute = 0;
  int seconds = 0;

  int found = sscanf(str, "%d:%d", &hour, &seconds);
  if (found == 2)
  {
    mTime = (hour << 12) | seconds;
    return seconds < 3600;
  }
  else
  {
    return false;
  }
}

mStd::mString Time::toString() const
{
  char v[11 + 1];
#ifdef _MSC_VER
  sprintf_s(
#else
  snprintf(
#endif
    v, sizeof(v), "%02d:%04d",
    (mTime >> 12), (mTime & 0xFFF));
  return mStd::mString(v);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
namespace mStd
{
  //
  // Define necessary fiddly bits to allow Date and Time classes
  // can be stored in a Variant.
  //

  template<>
  bool type_conversion(const DataStore::Date& from,
    const TypeInfo& toType, Variant* to)
  {
    if (toType == DataStore::TypeInfo_String)
    {
      *to = from.toString();
      return true;
    }
    else
    {
      return false;
    }
  }

  template<>
  bool type_conversion(const DataStore::Time& from,
    const TypeInfo& toType, Variant* to)
  {
    if (toType == DataStore::TypeInfo_String)
    {
      *to = from.toString();
      return true;
    }
    else
    {
      return false;
    }
  }
}
