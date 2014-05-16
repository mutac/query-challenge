/** Query a datastore
*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <algorithm>
#include <tclap/CmdLine.h>
#include <datastore/Database.h>
#include <datastore/JsonStorage.h>

bool getStringValuesSeperatedBy(const char delim, 
  const std::string& row, std::vector<std::string>* outFields)
{
  outFields->clear();

  std::istringstream tokenStream(row);
  for (std::string field; std::getline(tokenStream, field, delim);)
  {
    outFields->push_back(field);
  }

  return true;
}

void printRow(const DataStore::IRow* row)
{
  // don't really need to get the fields each time...
  DataStore::IFieldDescriptorConstListConstPtrH fields = row->getFieldDescriptors();
  DataStore::IFieldDescriptorConstList::const_iterator field = fields->cbegin();

  if (!fields->empty())
  {
    while (field != fields->cend())
    {
      DataStore::ValueConstPtrH v = row->getValue(*(field->get()));

      mStd::mString strValue("");
      v->getValue().convertTo(&strValue);

      std::cout << strValue.c_str();

      ++field;
      if (field != fields->cend())
      {
        std::cout << ",";
      }
    }

    std::cout << std::endl;
  }
}

struct fieldNameComparitor : public std::unary_function<DataStore::IFieldDescriptorConstPtrH, bool>
{
  explicit fieldNameComparitor(const std::string expected) :
    mExpected(expected) 
  {
  }

  bool operator() (const DataStore::IFieldDescriptorConstPtrH& arg)
  {
    return mExpected == arg->getName();
  }

  std::string mExpected;
};

int main(int argc, char** argv)
{
  try
  {
    TCLAP::CmdLine cmd("Query tool", ' ');
    TCLAP::ValueArg<std::string> selectArg("s", "select", "Comma separated list of field names to select", false, "", "Field selection");
    TCLAP::ValueArg<std::string> filterArg("f", "filter", "Filter expression in the form FIELDNAME=\"value\", filters selction", false, "", "Filter expression");
    TCLAP::ValueArg<std::string> orderArg("o", "order", "Comma separated list of field names with which to order a selection", false, "", "Order by");
    TCLAP::ValueArg<std::string> datastoreFileArg("d", "db", "JSON database file name to load or create", false, "db.json", "Database file");
    cmd.add(selectArg);
    cmd.add(filterArg);
    cmd.add(orderArg);
    cmd.add(datastoreFileArg);    
    cmd.parse(argc, argv);

    DataStore::DatabasePtrH database = 
      DataStore::DataStorageJson::Load(datastoreFileArg.getValue().c_str());

    //
    // Translate selection (-s) switch into a list of field descriptors
    //

    std::vector<std::string> fieldsToSelect;
    if (selectArg.isSet())
    {
      getStringValuesSeperatedBy(',', selectArg.getValue(), &fieldsToSelect);
    }

    DataStore::IFieldDescriptorConstListConstPtrH allFields = 
      database->getScheme()->getFieldDescriptors();

    DataStore::IFieldDescriptorConstListPtrH selectedFields
      (new DataStore::IFieldDescriptorConstList());

    for (std::vector<std::string>::const_iterator selectedFieldName = fieldsToSelect.cbegin();
      selectedFieldName != fieldsToSelect.cend(); ++selectedFieldName)
    {
      DataStore::IFieldDescriptorConstList::const_iterator found = std::find_if(allFields->cbegin(),
        allFields->cend(), fieldNameComparitor(*selectedFieldName));

      if (found == allFields->cend())
      {
        std::string ex = "Unrecognized field \"" + *selectedFieldName + 
          "\"" + " specified in selection";
        throw std::exception(ex.c_str());
      }

      selectedFields->push_back(*found);
    }

    //
    // Perform query, and print result
    //

    DataStore::ISelectionConstPtrH result = database->query(selectedFields);

    for (DataStore::ISelection::const_iterator row = result->cbegin(); 
      row != result->cend(); ++row)
    {
      printRow((*row).get());
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

