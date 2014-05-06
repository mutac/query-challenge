
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <datastore/Scheme.h>
#include <vector>
#include <memory>

namespace DataStore
{
  class Database
  {
  public:
    typedef std::vector<std::shared_ptr<IFieldValue> > IFieldValues;
    
    Database(std::shared_ptr<IScheme> scheme);

    bool insert(const IFieldValues& row);

  private:
    std::shared_ptr<IScheme> mScheme;
  };
}

#endif

