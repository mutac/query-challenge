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

/**
*/
struct fieldNameComparator :
  public std::unary_function<DataStore::IFieldDescriptorConstPtrH, bool>
{
  explicit fieldNameComparator(const char* expected) :
  mExpected(expected)
  {
  }

  bool operator() (const DataStore::IFieldDescriptorConstPtrH& arg)
  {
    return mExpected == arg->getName();
  }

  std::string mExpected;
};

/**
*/
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

/**
*/
DataStore::IFieldDescriptorConstPtrH findFieldByName(const char* fieldName,
  const DataStore::IFieldDescriptorConstList* fields)
{
  DataStore::IFieldDescriptorConstList::const_iterator found = std::find_if(fields->cbegin(),
    fields->cend(), fieldNameComparator(fieldName));

  if (found != fields->cend())
  {
    return *found;
  }
  else
  {
    return NULL;
  }
}

/**
*/
DataStore::IFieldDescriptorConstListPtrH parseSelectExpression(const std::string& expression,
  const DataStore::IFieldDescriptorConstList* fields)
{

  std::vector<std::string> fieldsToSelect;
  getStringValuesSeperatedBy(',', expression, &fieldsToSelect);

  DataStore::IFieldDescriptorConstListPtrH selectedFields
    (new DataStore::IFieldDescriptorConstList());

  for (std::vector<std::string>::const_iterator selectedFieldName = fieldsToSelect.cbegin();
    selectedFieldName != fieldsToSelect.cend(); ++selectedFieldName)
  {
    DataStore::IFieldDescriptorConstPtrH selectedField =
      findFieldByName(selectedFieldName->c_str(), fields);

    if (!selectedField)
    {
      std::string ex = "Unrecognized field \"" + *selectedFieldName +
        "\"" + " specified in selection";
      throw std::exception(ex.c_str());
    }

    selectedFields->push_back(selectedField);
  }

  return selectedFields;
}

/**
*/
DataStore::IQualifierPtrH parseFilterExpression(const std::string& expression, 
  const DataStore::IFieldDescriptorConstList* fields)
{
  //
  // For now, only support FIELD="value"
  //

  // Hack some reasonably large values
  char fieldName[64];
  char expectedValue[1024];

  int parsedItems = sscanf(expression.c_str(), "%64[^=]=%1024s", fieldName, 
    expectedValue);
  if (parsedItems != 2)
  {
    throw std::exception("Syntax error in filter expression");
  }

  DataStore::IFieldDescriptorConstPtrH field = findFieldByName(fieldName, fields);
  if (!field)
  {
    std::string ex = "Unrecognized field \"";
    ex += fieldName;
    ex += "\" specified in filter expression";
    throw std::exception(ex.c_str());
  }

  DataStore::ValuePtrH desiredValue = field->fromString(expectedValue);
  if (!desiredValue)
  {
    std::string ex = "Syntax error in filter expression, value format is incorrect";
    throw std::exception(ex.c_str());
  }

  DataStore::IQualifierPtrH exactMatch(new DataStore::Logic::Exact(field, desiredValue));
  return exactMatch;
}

/**
*/
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

/**
*/
int main(int argc, char** argv)
{
  try
  {
    TCLAP::CmdLine cmd("Query tool", ' ');
    TCLAP::ValueArg<std::string> selectArg("s", "select", "Comma separated list of field names to select, if omitted, all fields are selected", false, "", "Field selection");
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

    DataStore::IFieldDescriptorConstListConstPtrH allFields =
      database->getScheme()->getFieldDescriptors();

    //
    // Translate selection (-s) switch into a list of field descriptors
    //

    DataStore::IFieldDescriptorConstListPtrH selectedFields;
    if (selectArg.isSet())
    {
      selectedFields = parseSelectExpression(selectArg.getValue(), 
        allFields.get());
    }

    //
    // Parse filter expression
    //

    DataStore::Predicate filter = DataStore::Predicate::AlwaysTrue();
    if (filterArg.isSet())
    {
      DataStore::IQualifierPtrH filterAst = 
        parseFilterExpression(filterArg.getValue(), allFields.get());
      filter = DataStore::Predicate(filterAst);
    }

    //
    // Perform query, and print result
    //

    DataStore::ISelectionConstPtrH result = database->query(selectedFields, &filter);

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

