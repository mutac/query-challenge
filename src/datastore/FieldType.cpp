
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

  localtime_s(&timeDetails, &mDate);

  char v[11 + 1];
  sprintf_s(v, sizeof(v), "%04d-%02d-%02d", 
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

  int found = sscanf(str, "%d:%d:%d", &hour, &minute, &seconds);
  if (found == 3)
  {
    struct tm timeDetails = { 0 };
    timeDetails.tm_hour = hour;
    timeDetails.tm_min = minute;
    timeDetails.tm_sec = seconds;

    mTime = mktime(&timeDetails);
    return mTime != -1;
  }
  else
  {
    return false;
  }
}

mStd::mString Time::toString() const
{
  struct tm timeDetails = { 0 };

  localtime_s(&timeDetails, &mTime);

  char v[11 + 1];
  sprintf_s(v, sizeof(v), "%02d:%02d:%02d",
    timeDetails.tm_hour,
    timeDetails.tm_min,
    timeDetails.tm_sec);
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
    return false;
  }

  template<>
  bool type_conversion(const DataStore::Time& from,
    const TypeInfo& toType, Variant* to)
  {
    return false;
  }
}