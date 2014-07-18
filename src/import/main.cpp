/** Imports a bar separated text file into a datastore.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <tclap/CmdLine.h>
#include <datastore/Database.h>
#include <datastore/JsonStorage.h>

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
    TCLAP::CmdLine cmd("Datastore Importer. Reads bar-delimited input and appends records to datastore.\nCreate a new database:\n>  Import.exe -c Scheme.json -d mydb.json\nImport data into your database:\n>  cat Example1.txt | Import.exe -d mydb.json\n(or)\n>  Import.exe -d mydb.json -i Example1.txt", ' ');
    TCLAP::ValueArg<std::string> createUsingSchemeArg("c", "create", "Create a new data store using a JSON scheme file, and exit", false, "Scheme.json", "JSON scheme file");
    TCLAP::ValueArg<std::string> importFileArg("i", "import", "Bar-delimited input file name.  If none specified, reads from STDIN", false, "", "Bar delmited input file");
    TCLAP::ValueArg<std::string> datastoreFileArg("d", "db", "JSON database file name to load or create", true, "db.json", "Database file");
    cmd.add(createUsingSchemeArg);
    cmd.add(importFileArg);
    cmd.add(datastoreFileArg);
    cmd.parse(argc, argv);

    //
    // Load or create the database
    //

    bool isInCreateMode = createUsingSchemeArg.isSet();

    DataStore::DatabasePtrH database;
    if (isInCreateMode)
    {
      // Create a new db
      database = DataStore::DataStorageJson::Create(
        createUsingSchemeArg.getValue().c_str(),
        datastoreFileArg.getValue().c_str());

      std::cout << "Database \"" << datastoreFileArg.getValue()
        << "\" created" << std::endl;

      // and exit
      return 0;
    }
    else
    {
      database = DataStore::DataStorageJson::Load(datastoreFileArg.getValue().c_str());
    }

    // Read from stdin by default, unless '-i' arg is specified
    std::ifstream inputFile;
    std::istream* input = &std::cin;

    if (importFileArg.isSet())
    {
      inputFile.open(importFileArg.getValue().c_str(), std::fstream::in);
      if (!inputFile.is_open())
      {
        std::string ex = "Unable to open input file \"" +
          importFileArg.getValue() + "\"";
        throw std::runtime_error(ex);
      }
      input = &inputFile;
    }

    //
    // Use the FieldDescriptors to decide how to parse the input rows.
    //

    DataStore::ISchemeConstPtrH scheme(database->getScheme());
    DataStore::IFieldDescriptorConstListConstPtrH fieldDescriptors = 
      scheme->getFieldDescriptors();

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
      throw std::runtime_error("Header does not match scheme");
    }

    int replacedCount = 0;
    int insertedCount = 0;

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
          throw std::runtime_error(str);
        }

        // Parse each value in row.

        DataStore::IRowPtrH row = database->createRow();

        DataStore::IFieldDescriptorConstList::const_iterator fieldDescriptor =
          fieldDescriptors->cbegin();
        std::vector<std::string>::const_iterator stringValue = stringValues.cbegin();

        while (stringValue != stringValues.cend() && fieldDescriptor != fieldDescriptors->cend())
        {
          DataStore::ValuePtrH value =
            (*fieldDescriptor)->fromString(stringValue->c_str());

          if (!value)
          {
            std::stringstream ex;
            ex << "Malformed value in field \"" << (*fieldDescriptor)->getName()
              << "\" : \"" << *stringValue << "\", at line " << line;
            std::string str = ex.str();
            throw std::runtime_error(str);
          }

          if (!row->setValue(*(fieldDescriptor->get()), value))
          {
            std::stringstream ex;
            ex << "Unable to add value to row at field \"" << (*fieldDescriptor)->getName()
              << "\" with value \"" << *stringValue << "\", at line" << line;
            std::string str = ex.str();
            throw std::runtime_error(str);
          }

          ++stringValue;
          ++fieldDescriptor;
        }

        DataStore::Database::InsertionResult insertionResult;
        if (!database->insert(row, &insertionResult))
        {
          std::stringstream ex;
          ex << "Error inserting row at line " << line;
          std::string str = ex.str();
          throw std::runtime_error(str);
        }

        if (insertionResult == DataStore::Database::eInsertionResult_Inserted)
          ++insertedCount;
        else if (insertionResult == DataStore::Database::eInsertionResult_Replaced)
          ++replacedCount;

        ++line;
      }
    }

    std::cout << "Inserted " << insertedCount << " new rows" << std::endl;
    std::cout << "Replaced " << replacedCount << " existing rows" << std::endl;
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
