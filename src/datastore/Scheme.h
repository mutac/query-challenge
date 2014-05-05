/**
*/

#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <datastore/FieldDescriptor.h>
#include <memory>
#include <vector>

namespace DataStore
{

  /**
  */
  struct IScheme
  {
    typedef std::vector<std::shared_ptr<IFieldDescriptor> > IFieldDescritors;

    /** Returns true if header field names match scheme */
    bool validateHeader(const std::vector<std::string>& headerFieldNames) const;

    /** Return fields of scheme */
    const IFieldDescritors& getFieldDescriptors() const;
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

    /** Returns true if header field names match scheme */
    bool validateHeader(const std::vector<std::string>& headerFieldNames) const;

    /** Return fields of scheme */
    const IFieldDescritors& getFieldDescriptors() const;

  private:
    std::shared_ptr<SchemeJsonImpl> mImpl;
  };
}

#endif

