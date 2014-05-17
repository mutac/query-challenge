
#ifndef __JSON_STORAGE_H__
#define __JSON_STORAGE_H__

#include <datastore/Scheme.h>
#include <datastore/DataStorage.h>
#include <datastore/Database.h>
#include <Resource/mString.h>

namespace DataStore
{
  /**
    @hidden
    Hide json encoding/decoding details from client
  */
  class SchemeJsonImpl;
  typedef PointerType<SchemeJsonImpl>::Shared SchemeJsonImplPtrH;

  /**
  IScheme implementation, reads from a JSON scheme encoding:

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
  class SchemeJson : public IScheme
  {
  public:
    /** JSON scheme descriptor file */
    SchemeJson(FILE* schemeFile);

    /** JSON scheme descriptor json text */
    SchemeJson(const char* schemeJson);

    SchemeJson();

    bool allFieldsPresent(const std::vector<std::string>& fieldNames) const;
    bool allFieldsPresent(const IFieldDescriptorList& fields) const;

    IFieldDescriptorConstListConstPtrH getFieldDescriptors() const;
    IFieldDescriptorConstListConstPtrH getKeyFieldDescriptors() const;

    /** @hidden - internal use only */
    SchemeJsonImpl* getImpl() const;

  private:
    SchemeJsonImplPtrH mImpl;
  };

  typedef PointerType<SchemeJson>::Shared SchemeJsonPtrH;
  typedef PointerType<SchemeJson>::SharedConst SchemeJsonConstPtrH;

  /**
    @hidden
    Hide json encoding/deconding details from client
  */
  class DataStorageJsonImpl;
  typedef PointerType<DataStorageJsonImpl>::Shared DataStorageJsonImplPtrH;
  
  /**
  An IDataStorage implementation that stores a scheme and value-row information
  in a json encoded file.  It is expected that all rows will be loaded into
  memory at once, and all rows will be written back to it at once.  i.e.
  it does not support partial loading and repeated access.

  @verbatim
  {
    "scheme": [ //scheme object (above) ],
    "rows": [
      [value1, value2, value3, ...],
      ...
    ]
  }
  @endverbatim
  */
  class DataStorageJson : public IDataStorage
  {
  public:
    static DatabasePtrH Load(const char* existingDbFilename);
    static DatabasePtrH Create(const char* schemeFilename, const char* newDbFilename);
    static DatabasePtrH Create(SchemeJsonConstPtrH scheme, const char* newDbFilename);

    ISchemeConstPtrH getScheme();

    void load(Database* database);

    void beginPersist();
    void persistRow(const IRow* row);
    void endPersist();

  private:
    /** Load an existing datastorage */
    DataStorageJson(const char* existingDbFilename);


    /** Create a new datastorage, given a scheme */
    DataStorageJson(SchemeJsonConstPtrH scheme,
      const char* newDbFilename);

    DataStorageJsonImplPtrH mImpl;
  };
}

#endif