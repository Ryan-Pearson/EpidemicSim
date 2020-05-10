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
      const std::string fullFilePath = "/Users/pearsrs1/EpidemicSim/test_input_small.xml";
      const auto worldConfiguration = Epidemic::get_world_config_from_xml(fullFilePath);
      auto world = Epidemic::build_world(worldConfiguration);

      const auto logStats = [](const Epidemic::Timestep time, const Epidemic::World::SIRD_Levels& stats) {
         std::cout << fmt::format("Timestep {}: S[{}] I[{}] R[{}] D[{}]\n", time, stats.m_numSusceptible,
            stats.m_numInfectious, stats.m_numRecovered, stats.m_numDeceased);
      };

      Epidemic::Timestep curTime = 0;
      auto worldStats = world.get_cur_sird_levels();
      int numInfectious = worldStats.m_numInfectious;
      logStats(curTime, worldStats);

      while (numInfectious > 0)
      {
         std::tie(curTime, worldStats) = world.run_timestep();
         if ((curTime % (1440 * 60)) == 0)
         {
            logStats(curTime, worldStats);
         }
         numInfectious = worldStats.m_numInfectious;
      }

      logStats(curTime, worldStats);
      std::cout << "Simulation complete" << std::endl;
   }
   catch (...)
   {
      eptr = std::current_exception();
   }
   handle_eptr(eptr);
   return EXIT_SUCCESS;
}