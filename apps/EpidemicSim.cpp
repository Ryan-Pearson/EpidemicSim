// STL
#include <iostream>

// FMT
#include <fmt/format.h>

// Epidemic
#include "Epidemic/WorldBuilder.h"
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
      auto world = Epidemic::build_world(worldConfiguration);

      int numInfectious = world.get_cur_sird_levels().m_numInfectious;
      while (numInfectious > 0)
      {
         const auto worldStats = world.run_timestep();

         if ((worldStats.first % 86400) == 0)
         {
            std::cout << fmt::format("Timestep {}: S[{}] I[{}] R[{}] D[{}]\n", worldStats.first,
               worldStats.second.m_numSusceptible, worldStats.second.m_numInfectious, worldStats.second.m_numRecovered,
               worldStats.second.m_numDeceased);
         }

         numInfectious = worldStats.second.m_numInfectious;
      }
      std::cout << "Simulation complete" << std::endl;
   }
   catch (...)
   {
      eptr = std::current_exception();
   }
   handle_eptr(eptr);
   return EXIT_SUCCESS;
}