/** Imports a bar separated text file into a datastore.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <tclap/CmdLine.h>
#include <datastore/Database.h>

static const char kFieldDelimiter = '|';

bool getStringValues(const std::string& row, std::vector<std::string>* outFields)
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

    FILE* schemeFile = fopen(schemeArg.getValue().c_str(), "r");
    if (!schemeFile)
    {
      throw std::exception("Unable to open scheme file");
    }

    //
    // Parse the scheme, and get the FieldDescriptors that will
    // describe how to parse the input rows.
    //

    std::shared_ptr<DataStore::IScheme> scheme(new DataStore::SchemeJson(schemeFile));
    DataStore::Database database(scheme);

    DataStore::IFieldDescriptorConstListConstPtrH fieldDescriptors = scheme->getFieldDescriptors();

    //
    // Read records from input, parse, and validate that they match scheme
    //

    size_t line = 1;

    std::string header;
    *input >> header;
    line++;

    std::vector<std::string> headerFieldNames;
    getStringValues(header, &headerFieldNames);
    if (!scheme->allFieldsPresent(headerFieldNames))
    {
      throw std::exception("Header does not match scheme");
    }

    for (std::string row; std::getline(*input, row);)
    {
      if (row.size() > 0)
      {
        std::vector<std::string> stringValues;
        getStringValues(row, &stringValues);

        // Make sure that the row has the right number of fields
        if (stringValues.size() != fieldDescriptors->size())
        {
          std::stringstream ex;
          ex << "Row is missing a field, at line: " << line;
          std::string str = ex.str();
          throw std::exception(str.c_str());
        }

        // Parse each value in row.

        std::shared_ptr<DataStore::IRow> row = database.createRow();
        
        DataStore::IFieldDescriptorConstList::const_iterator fieldDescriptor = 
          fieldDescriptors->cbegin();
        std::vector<std::string>::const_iterator stringValue = stringValues.cbegin();

        while (stringValue != stringValues.cend() && fieldDescriptor != fieldDescriptors->cend())
        {
          std::shared_ptr<DataStore::Value> value = 
            (*fieldDescriptor)->fromString(stringValue->c_str());
 
          if (!value)
          {
            std::stringstream ex;
            ex << "Malformed value in field \"" << (*fieldDescriptor)->getName() 
              << "\" : \"" << *stringValue << "\", at line " << line;
            std::string str = ex.str();
            throw std::exception(str.c_str());
          }

          if (!row->setValue(*fieldDescriptor, value))
          {
            std::stringstream ex;
            ex << "Unable to add value to row at field \"" << (*fieldDescriptor)->getName()
              << "\" with value \"" << *stringValue << "\", at line" << line;
            std::string str = ex.str();
            throw std::exception(str.c_str());
          }

          ++stringValue;
          ++fieldDescriptor;
        }

        if (!database.insert(row))
        {
          std::stringstream ex;
          ex << "Error inserting row at line " << line;
          std::string str = ex.str();
          throw std::exception(str.c_str());
        }

        ++line;
      }
    }
  }
  catch (TCLAP::ArgException &e)
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  catch (std::exception& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    return 1;
  }

	return 0;
}