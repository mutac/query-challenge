/**
*/

#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <datastore/FieldDescriptor.h>
#include <datastore/FieldValue.h>
#include <memory>
#include <vector>

namespace DataStore
{

  /**
  */
  struct IScheme
  {
    /** Returns true if header field names match scheme */
    virtual bool allFieldsPresent(const std::vector<std::string>& headerFieldNames) const = 0;
    
    /** Returns true if values comprises the complete set of type-correct values */
    virtual bool allFieldsPresent(const FieldValues& values) const = 0;

    /** Return fields of scheme */
    virtual const IFieldDescriptors& getFieldDescriptors() const = 0;

    /** Return only the key fields */
    virtual const IFieldDescriptors& getKeyFieldDescriptors() const = 0;
  };

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
  class SchemeJson : public IScheme
  {
  public:
    SchemeJson(FILE* schemeFile);

    /** JSON scheme descriptor json text */
    SchemeJson(const char* schemeJson);

    bool allFieldsPresent(const std::vector<std::string>& fieldNames) const;
    bool allFieldsPresent(const FieldValues& values) const;

    const IFieldDescriptors& getFieldDescriptors() const;
    const IFieldDescriptors& getKeyFieldDescriptors() const;

  private:
    std::shared_ptr<SchemeJsonImpl> mImpl;
  };
}

#endif

