// STL
#include <iostream>

// Epidemic
#include "Epidemic/XmlReader.h"

void handle_eptr(std::exception_ptr eptr)
{
   try
   {
      if (eptr)
      {
         std::rethrow_exception(eptr);
      }
   }
   catch (const std::exception& e)
   {
      std::cout << "Caught exception \"" << e.what() << "\"\n";
   }
}

int main()
{
   std::exception_ptr eptr;
   try
   {
      const std::string fullFilePath = "/Users/pearsrs1/EpidemicSim/test_input.xml";
      const auto worldConfiguration = Epidemic::get_world_config_from_xml(fullFilePath);
   }
   catch (...)
   {
      eptr = std::current_exception();
   }
   handle_eptr(eptr);
   return EXIT_SUCCESS;
}