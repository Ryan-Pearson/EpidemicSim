// STL
#include <fstream>
#include <iostream>

// FMT
#include <fmt/format.h>

// Epidemic
#include "Epidemic/WorldBuilder.h"
#include "Epidemic/XmlReader.h"

static const std::string SOURCE_DIR = BASE_DIR;
static const std::string OUTPUT_ROOT = BASE_DIR + std::string("/output");
constexpr std::optional<Epidemic::AgentId> AGENT_TO_LOG_DIAGNOSTICS = std::nullopt;

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
      const std::string fullFilePath = SOURCE_DIR + "/test_input_small.xml";
      const auto worldConfiguration = Epidemic::get_world_config_from_xml(fullFilePath);

      for (int i = 0; i < worldConfiguration.m_numMonteCarloRuns; ++i)
      {
         auto world = Epidemic::build_world(worldConfiguration);

         std::ofstream buildingLog;
         std::ofstream positionLog;
         std::ofstream sirdLog(OUTPUT_ROOT + fmt::format("/sirdLog_{}.csv", i));
         sirdLog << "Day,R_est,Susceptible,Infectious,Recovered,Deceased,Infectious,Susceptible+Infectious,"
                    "Susceptible+Infectious+Recovered,Susceptible+Infectious+Recovered+Deceased\n";

         if (AGENT_TO_LOG_DIAGNOSTICS)
         {
            buildingLog = std::ofstream(OUTPUT_ROOT + fmt::format("/buildingLog_{}.csv", i));
            buildingLog << "Day,Hour,Building\n";

            positionLog = std::ofstream(OUTPUT_ROOT + fmt::format("/positionLog_{}.csv", i));
            buildingLog << "Timestep,Building,X,Y\n";
         }

         const auto logStats = [&](const int day, const double curR, const Epidemic::World::SIRD_Levels& stats) {
            std::cout << fmt::format("Day {}: S[{}] I[{}] R[{}] D[{}] R[{}]\n", day, stats.m_numSusceptible,
               stats.m_numInfectious, stats.m_numRecovered, stats.m_numDeceased, curR);
            sirdLog << day << ',' << curR << ',' << stats.m_numSusceptible << ',' << stats.m_numInfectious << ','
                    << stats.m_numRecovered << ',' << stats.m_numDeceased << ',' << stats.m_numInfectious << ','
                    << stats.m_numSusceptible + stats.m_numInfectious << ','
                    << stats.m_numSusceptible + stats.m_numInfectious + stats.m_numRecovered << ','
                    << stats.m_numSusceptible + stats.m_numInfectious + stats.m_numRecovered + stats.m_numDeceased
                    << '\n';
         };

         Epidemic::Timestep curTime = 0;
         auto worldStats = world.get_cur_sird_levels();
         int numInfectious = worldStats.m_numInfectious;
         logStats(curTime / Epidemic::TIMESTEP_PER_DAY, world.get_cur_R_level(), worldStats);

         while (numInfectious > 0)
         {
            std::tie(curTime, worldStats) = world.run_timestep();
            if ((curTime % Epidemic::TIMESTEP_PER_DAY) == 0)
            {
               logStats(curTime / Epidemic::TIMESTEP_PER_DAY, world.get_cur_R_level(), worldStats);
            }
            numInfectious = worldStats.m_numInfectious;

            if (AGENT_TO_LOG_DIAGNOSTICS)
            {
               const auto& agent = world.get_agent(AGENT_TO_LOG_DIAGNOSTICS.value());
               const auto buildingId = agent.get_current_building_id();
               const auto position = agent.get_current_position();
               positionLog << curTime << ',' << buildingId << ',' << position.m_x << ',' << position.m_y << '\n';

               if ((curTime % Epidemic::TIMESTEP_PER_HOUR) == 0)
               {
                  const auto day = curTime / Epidemic::TIMESTEP_PER_DAY;
                  const auto hour = (curTime / Epidemic::TIMESTEP_PER_HOUR) % 24;
                  buildingLog << day << ',' << hour << ',' << buildingId << '\n';
               }
            }
         }

         logStats(curTime / Epidemic::TIMESTEP_PER_DAY, world.get_cur_R_level(), worldStats);
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