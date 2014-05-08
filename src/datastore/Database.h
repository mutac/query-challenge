
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <datastore/Scheme.h>
#include <datastore/Selection.h>
#include <vector>
#include <memory>

namespace DataStore
{
  class DatabaseInMemoryStorge;

  class Database
  {
  public:
    
    Database(std::shared_ptr<IScheme> scheme);

    bool insert(const FieldValues& row);

  private:
    std::shared_ptr<IScheme> mScheme;
    std::shared_ptr<DatabaseInMemoryStorge> mMemory;
    IFieldDescriptors mFields;
  };
}

#endif

