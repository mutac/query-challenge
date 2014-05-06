
#include <datastore/Database.h>

using namespace DataStore;

Database::Database(std::shared_ptr<IScheme> scheme) :
  mScheme(scheme)
{
}

bool Database::insert(const Database::IFieldValues& values)
{
  return false;
}


