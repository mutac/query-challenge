/** Imports a bar separated text file into a datastore.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <tclap/CmdLine.h>
#include <datastore/IField.h>
#include <datastore/Scheme.h>

static const char kFieldDelimiter = '|';

int main(int argc, char** argv)
{
  try
  {
    TCLAP::CmdLine cmd("Datastore Importer. Reads bar-delimited input and appends records to datastore.", ' ');
    TCLAP::ValueArg<std::string> schemeArg("s", "scheme", "JSON Datastore scheme filename", false, "Scheme.json", "JSON scheme filename");

    cmd.add(schemeArg);
    cmd.parse(argc, argv);

    //
    //
    //

    DataStore::Scheme scheme(schemeArg.getValue().c_str());

    //
    // Read records from stdin
    //

    std::string header;
    std::cin >> header;

    for (std::string row; std::getline(std::cin, row);)
    {
      std::istringstream tokenStream(row);
      for (std::string field; std::getline(tokenStream, field, kFieldDelimiter);)
      {
        std::cout << field << "   ";
      }
      std::cout << std::endl;
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