
#ifndef __JSON_STORAGE_H__
#define __JSON_STORAGE_H__

#include <datastore/Scheme.h>
#include <datastore/DataStorage.h>
#include <datastore/Database.h>
#include <Resource/mString.h>

namespace DataStore
{
  /**
  IScheme implementation, reads from a JSON scheme encoding

  Example:
  @vertabim
  [
  {
  "name": "STB",
  "type": "text",
  "size": 64,
  "key": true,
  "description": "The set top box id on which the media asset was viewed"
  },
  {
  "name": "TITLE",
  "type": "text",
  "size": 64,
  "description": "The title of the media asset"
  },
  {
  "name": "PROVIDER",
  "type": "text",
  "size": 64,
  "description": "The distributor of the media asset"
  },
  {
  "name": "DATE",
  "type": "date",
  "description": "The local date on which the content was leased by through the STB"
  },
  {
  "name": "REV",
  "type": "float",
  "size": 32,
  "description": "The price incurred by the STB to lease the asset. (Price in US dollars and cents)"
  },
  {
  "name": "VIEW_TIME",
  "type": "time",
  "description": "The amount of time the STB played the asset"
  }
  ]
  */
  class SchemeJsonImpl;
  typedef PointerType<SchemeJsonImpl>::Shared SchemeJsonImplPtrH;

  class SchemeJson : public IScheme
  {
  public:
    /** JSON scheme descriptor file */
    SchemeJson(FILE* schemeFile);

    /** JSON scheme descriptor json text */
    SchemeJson(const char* schemeJson);

    /** @hidden - internal use only */
    SchemeJson(const void* root);

    bool allFieldsPresent(const std::vector<std::string>& fieldNames) const;
    bool allFieldsPresent(const IFieldDescriptorList& fields) const;

    IFieldDescriptorConstListConstPtrH getFieldDescriptors() const;
    IFieldDescriptorConstListConstPtrH getKeyFieldDescriptors() const;

  private:
    SchemeJsonImplPtrH mImpl;
  };

  typedef PointerType<SchemeJson>::Shared SchemeJsonPtrH;
  typedef PointerType<SchemeJson>::SharedConst SchemeJsonConstPtrH;

  /**
  */
  class DataStorageJsonImpl;
  typedef PointerType<DataStorageJsonImpl>::Shared DataStorageJsonImplPtrH;
  
  /**
  {
    "scheme": [ //scheme object (above) ],
    "rows": [
      [value1, value2, value3, ...],
      ...
    ]
  }
  */
  class DataStorageJson : public IDataStorage
  {
  public:
    static DatabasePtrH Load(FILE* existingDbFile);
    static DatabasePtrH Create(FILE* schemeJson, FILE* newdbFile);
    static DatabasePtrH Create(SchemeJsonConstPtrH scheme, FILE* newDbFile);

    ISchemeConstPtrH getScheme();

  private:
    /** Load an existing datastorage */
    DataStorageJson(FILE* dbFile);

    /** Create a new datastorage, given a scheme */
    DataStorageJson(SchemeJsonConstPtrH scheme, FILE* newDbFile);

    DataStorageJsonImplPtrH mImpl;
  };
}

#endif