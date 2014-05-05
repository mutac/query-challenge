/** Imports a bar separated text file into a datastore.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <tclap/CmdLine.h>
#include <datastore/FieldValue.h>
#include <datastore/Scheme.h>

static const char kFieldDelimiter = '|';

bool getValues(const std::string& row, std::vector<std::string>* outFields)
{
  outFields->clear();

  std::istringstream tokenStream(row);
  for (std::string field; std::getline(tokenStream, field, kFieldDelimiter);)
  {
    outFields->push_back(field);
  }

  return true;
}

int main(int argc, char** argv)
{
  try
  {
    TCLAP::CmdLine cmd("Datastore Importer. Reads bar-delimited input and appends records to datastore.", ' ');
    TCLAP::ValueArg<std::string> schemeArg("s", "scheme", "JSON Datastore scheme filename", false, "Scheme.json", "JSON scheme filename");
    TCLAP::ValueArg<std::string> importFile("i", "import", "Bar-delimited input file name", false, "", "Bar-delmiited input file");
    cmd.add(schemeArg);
    cmd.add(importFile);
    cmd.parse(argc, argv);

    std::ifstream inputFile;
    std::istream* input = &std::cin;

    if (!importFile.getValue().empty())
    {
      inputFile.open(importFile.getValue().c_str(), std::fstream::in);
      input = &inputFile;
    }

    //
    //
    //

    FILE* schemeFile = fopen(schemeArg.getValue().c_str(), "r");
    if (!schemeFile)
    {
      throw std::exception("Unable to open scheme file");
    }

    DataStore::SchemeJson scheme(schemeFile);
    DataStore::IScheme::IFieldDescritors fieldDescriptors = scheme.getFieldDescriptors();

    //
    // Read records from input, parse, and validate that it matches scheme
    //

    size_t line = 1;

    std::string header;
    *input >> header;
    line++;

    std::vector<std::string> headerFieldNames;
    getValues(header, &headerFieldNames);
    if (!scheme.validateHeader(headerFieldNames))
    {
      throw std::exception("Header does not match scheme");
    }

    for (std::string row; std::getline(*input, row);)
    {
      if (row.size() > 0)
      {
        std::vector<std::string> values;
        getValues(row, &values);

        // Make sure that the row has the right number of fields
        if (values.size() != fieldDescriptors.size())
        {
          std::stringstream ex;
          ex << "Row is missing a field, at line: " << line;
          std::string str = ex.str();
          throw std::exception(str.c_str());
        }

        // Parse each value in row.  Iterators... blech
        DataStore::IScheme::IFieldDescritors::const_iterator fieldDesriptor = fieldDescriptors.cbegin();
        for (std::vector<std::string>::const_iterator value = values.cbegin();
          value != values.cend() && fieldDesriptor != fieldDescriptors.cend();
          ++value, ++fieldDesriptor)
        {
          std::shared_ptr<DataStore::IFieldValue> fieldValue = 
            (*fieldDesriptor)->deserialize(value->c_str(), value->length());

          if (!fieldValue)
          {
            std::stringstream ex;
            ex << "Malformed value in Field: \"" << (*fieldDesriptor)->getName() <<
              "\" : \"" << *value << "\", at line " << line;
            std::string str = ex.str();
            throw std::exception(str.c_str());
          }
        }

        line++;
      }
    }
  }
  catch (TCLAP::ArgException &e)
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }
  catch (std::exception& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
  }

	return 0;
}