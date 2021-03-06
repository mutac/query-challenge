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
struct fieldNameCompareEquals :
  public std::unary_function<DataStore::IFieldDescriptorConstPtrH, bool>
{
  explicit fieldNameCompareEquals(const char* expected) :
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
bool getStringValuesSeparatedBy(const char delim, 
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
  const DataStore::IFieldDescriptorConstList& fields)
{
  DataStore::IFieldDescriptorConstList::const_iterator found = std::find_if(fields.cbegin(),
    fields.cend(), fieldNameCompareEquals(fieldName));

  if (found != fields.cend())
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
DataStore::IFieldDescriptorConstListPtrH parseFieldNameList(const std::string& expression,
  const DataStore::IFieldDescriptorConstList& fields)
{

  std::vector<std::string> fieldsToSelect;
  getStringValuesSeparatedBy(',', expression, &fieldsToSelect);

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
        "\"" + " specified in list";
      throw std::runtime_error(ex);
    }

    selectedFields->push_back(selectedField);
  }

  return selectedFields;
}

/**
*/
DataStore::IQualifierPtrH parseFilterExpression(const std::string& expression, 
  const DataStore::IFieldDescriptorConstList& fields)
{
  //
  // For now, only support FIELD="value"
  //

  // Hack some reasonably large values and use sscanf as a quick & dirty parser
  char fieldName[64];
  char expectedValue[1024];

  // Doesn't really work for all types of data, but should work for example
  // data sets.
  int parsedItems = sscanf(expression.c_str(), "%64[^=]=%1024[^\n]", fieldName, 
    expectedValue);
  if (parsedItems != 2)
  {
    throw std::runtime_error("Syntax error in filter expression");
  }

  DataStore::IFieldDescriptorConstPtrH field = findFieldByName(fieldName, fields);
  if (!field)
  {
    std::string ex = "Unrecognized field \"";
    ex += fieldName;
    ex += "\" specified in filter expression";
    throw std::runtime_error(ex);
  }

  DataStore::ValuePtrH desiredValue = field->fromString(expectedValue);
  if (!desiredValue)
  {
    std::string ex = "Syntax error in filter expression, value format is incorrect";
    throw std::runtime_error(ex);
  }

  DataStore::IQualifierPtrH exactMatch(new DataStore::Logic::Exact(field, desiredValue));
  return exactMatch;
}

/**
*/
void printFields(const DataStore::IFieldDescriptorConstList& fields)
{
  DataStore::IFieldDescriptorConstList::const_iterator field = fields.cbegin();

  if (!fields.empty())
  {
    while (field != fields.cend())
    {
      std::cout << (*field)->getName();

      ++field;
      if (field != fields.cend())
      {
        std::cout << ",";
      }
    }

    std::cout << std::endl;
  }
}

/**
*/
void printResult(const DataStore::IQueryResult& result)
{
  DataStore::IFieldDescriptorConstListConstPtrH fields = result.getFieldDescriptors();

  for (DataStore::IQueryResult::const_iterator row = result.cbegin();
    row != result.cend(); ++row)
  {
    DataStore::IFieldDescriptorConstList::const_iterator field = fields->cbegin();

    if (!fields->empty())
    {
      while (field != fields->cend())
      {
        DataStore::ValueConstPtrH v = (*row)->getValue(*(field->get()));

        mStd::mString strValue("");
        if (v)
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
}

/**
*/
int main(int argc, char** argv)
{
  try
  {
    TCLAP::CmdLine cmd("Query tool", ' ');
    TCLAP::SwitchArg showArg("", "show", "Show fields and exit", false);
    TCLAP::ValueArg<std::string> selectArg("s", "select", "Comma separated list of field names to select, if omitted, all fields are selected", false, "", "Field selection");
    TCLAP::ValueArg<std::string> filterArg("f", "filter", "Filter expression in the form FIELDNAME=\"value\", filters selction", false, "", "Filter expression");
    TCLAP::ValueArg<std::string> orderArg("o", "order", "Comma separated list of field names with which to order a selection", false, "", "Order by");
    TCLAP::ValueArg<std::string> datastoreFileArg("d", "db", "JSON database file name to load or create", false, "db.json", "Database file");
    cmd.add(showArg);
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
    // Show fields and exit?
    //
    if (showArg.isSet())
    {
      printFields(*(allFields.get()));
      return 0;
    }

    //
    // Parse selection (-s) into a list of field descriptors
    //

    DataStore::IFieldDescriptorConstListPtrH selectedFields;
    if (selectArg.isSet())
    {
      selectedFields = parseFieldNameList(selectArg.getValue(),
        *(allFields.get()));
    }

    //
    // Parse filter (-f) expression into a logical expression AST
    //

    DataStore::Predicate filter = DataStore::Predicate::AlwaysTrue();
    if (filterArg.isSet())
    {
      DataStore::IQualifierPtrH filterAst =
        parseFilterExpression(filterArg.getValue(), *(allFields.get()));
      filter = DataStore::Predicate(filterAst);
    }

    //
    // Parse order (-o) in to list of field descriptors, order is
    // always ascending.
    //

    DataStore::IFieldDescriptorConstListPtrH orderByFields;
    if (orderArg.isSet())
    {
      orderByFields = parseFieldNameList(orderArg.getValue(),
        *(allFields.get()));
    }

    //
    // Perform query, and print result
    //

    DataStore::IQueryResultConstPtrH result =
      database->query(selectedFields, &filter, orderByFields);
    printResult(*(result.get()));
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

