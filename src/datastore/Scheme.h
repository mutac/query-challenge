/**
*/

#ifndef __SCHEME_H__
#define __SCHEME_H__

#include <datastore/FieldDescriptor.h>
#include <memory>
#include <vector>

/**
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

namespace DataStore
{
  class SchemeImpl;

  class Scheme
  {
  public:
    typedef std::vector<std::shared_ptr<IFieldValueDescriptor> > FieldDescritors;

    /** JSON scheme descriptor file */
    Scheme(const char* schemeDescriptorFile);

    /** Returns true if header field names match scheme */
    bool validateHeader(const std::vector<std::string>& headerFieldNames) const;

    /** Return fields of scheme */
    const FieldDescritors& getFieldDescriptors() const;

  private:
    std::shared_ptr<SchemeImpl> mImpl;
  };
}

#endif

