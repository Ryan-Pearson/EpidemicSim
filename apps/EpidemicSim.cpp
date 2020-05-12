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

      struct CumulativeInfo
      {
         double e_x = 0.0;
         double e_x_sq = 0.0;
         int numAdded = 0;
      };

      struct MonteCarloSirdInfo
      {
         CumulativeInfo s;
         CumulativeInfo i;
         CumulativeInfo r;
         CumulativeInfo d;
      };

      std::vector<MonteCarloSirdInfo> sirdByDay;
      sirdByDay.reserve(100);

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
            if (day >= sirdByDay.size())
            {
               sirdByDay.emplace_back();
            }

            auto& curDay = sirdByDay[day];

            curDay.s.e_x = (curDay.s.e_x * curDay.s.numAdded + static_cast<double>(stats.m_numSusceptible))
               / (curDay.s.numAdded + 1);
            curDay.s.e_x_sq = (curDay.s.e_x_sq * curDay.s.numAdded
                                 + static_cast<double>(stats.m_numSusceptible * stats.m_numSusceptible))
               / (curDay.s.numAdded + 1);
            ++curDay.s.numAdded;

            curDay.i.e_x = (curDay.i.e_x * curDay.i.numAdded + static_cast<double>(stats.m_numInfectious))
               / (curDay.i.numAdded + 1);
            curDay.i.e_x_sq = (curDay.i.e_x_sq * curDay.i.numAdded
                                 + static_cast<double>(stats.m_numInfectious * stats.m_numInfectious))
               / (curDay.i.numAdded + 1);
            ++curDay.i.numAdded;

            curDay.r.e_x =
               (curDay.r.e_x * curDay.r.numAdded + static_cast<double>(stats.m_numRecovered)) / (curDay.r.numAdded + 1);
            curDay.r.e_x_sq =
               (curDay.r.e_x_sq * curDay.r.numAdded + static_cast<double>(stats.m_numRecovered * stats.m_numRecovered))
               / (curDay.r.numAdded + 1);
            ++curDay.r.numAdded;

            curDay.d.e_x =
               (curDay.d.e_x * curDay.d.numAdded + static_cast<double>(stats.m_numDeceased)) / (curDay.d.numAdded + 1);
            curDay.d.e_x_sq =
               (curDay.d.e_x_sq * curDay.d.numAdded + static_cast<double>(stats.m_numDeceased * stats.m_numDeceased))
               / (curDay.d.numAdded + 1);
            ++curDay.d.numAdded;

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

      std::ofstream sirdLog(OUTPUT_ROOT + fmt::format("/sirdLog_overall.csv"));
      sirdLog << "Day,Susceptible_Mean,Susceptible_Std,Infectious_Mean,Infectious_Std,Recovered_Mean,Recovered_Std,"
                 "Deceased_Mean,Deceased_Std,Infectious_Mean,Infectious_Low,Infectious_High,Susceptible+Infectious_"
                 "Mean,Susceptible+Infectious_Low,Susceptible+Infectious_High,Susceptible+Infectious+Recovered_Mean,"
                 "Susceptible+Infectious+Recovered_Low,Susceptible+Infectious+Recovered_High,Susceptible+Infectious+"
                 "Recovered+Deceased_Mean,Susceptible+Infectious+Recovered+Deceased_Low,Susceptible+Infectious+"
                 "Recovered+Deceased_High\n";

      for (int i = 0; i < sirdByDay.size(); ++i)
      {
         const auto& data = sirdByDay[i];

         const double e_s = data.s.e_x;
         const double e_i = data.i.e_x;
         const double e_r = data.r.e_x;
         const double e_d = data.d.e_x;

         const double std_s = std::sqrt(data.s.e_x_sq - data.s.e_x * data.s.e_x);
         const double std_i = std::sqrt(data.i.e_x_sq - data.i.e_x * data.i.e_x);
         const double std_r = std::sqrt(data.r.e_x_sq - data.r.e_x * data.r.e_x);
         const double std_d = std::sqrt(data.d.e_x_sq - data.d.e_x * data.d.e_x);

         const double a_e = e_i;
         const double a_s = std_i;
         const double a_l = a_e - a_s;
         const double a_h = a_e + a_s;

         const double b_e = e_i + e_s;
         const double b_s = std::sqrt(std_i * std_i + std_s * std_s);
         const double b_l = b_e - b_s;
         const double b_h = b_e + b_s;

         const double c_e = e_i + e_s + e_r;
         const double c_s = std::sqrt(std_i * std_i + std_s * std_s + std_r * std_r);
         const double c_l = c_e - c_s;
         const double c_h = c_e + c_s;

         const double d_e = e_i + e_s + e_r + e_d;
         const double d_s = std::sqrt(std_i * std_i + std_s * std_s + std_r * std_r + std_d * std_d);
         const double d_l = d_e - d_s;
         const double d_h = d_e + d_s;

         sirdLog << i << e_s << ',' << std_s << ',' << e_i << ',' << std_i << ',' << e_r << ',' << std_r << ',' << e_d
                 << ',' << std_d << ',' << a_e << ',' << a_l << ',' << a_h << ',' << b_e << ',' << b_l << ',' << b_h
                 << ',' << c_e << ',' << c_l << ',' << c_h << ',' << d_e << ',' << d_l << ',' << d_h << '\n';
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