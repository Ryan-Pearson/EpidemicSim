//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_WORLD_H
#define EPIDEMIC_SIM_WORLD_H

// Epidemic
#include "Agent.h"

namespace Epidemic {

class World
{
public:
   struct SIRD_Levels
   {
      size_t m_numSusceptible = 0;
      size_t m_numInfectious = 0;
      size_t m_numRecovered = 0;
      size_t m_numDeceased = 0;
   };

   explicit World(
      std::vector<Community> communities, std::vector<Agent> agents, Timestep numberOfInfectionTimestepsRemaining);

   [[nodiscard]] Timestep get_cur_timestep() const noexcept { return m_curTimestep; }
   [[nodiscard]] SIRD_Levels get_cur_sird_levels() && noexcept { return m_curLevels; }
   [[nodiscard]] const SIRD_Levels& get_cur_sird_levels() const& noexcept { return m_curLevels; }
   [[nodiscard]] const Agent& get_agent(const AgentId agentId) const& noexcept { return m_agents[static_cast<size_t>(agentId)]; }
   [[nodiscard]] double get_cur_R_level() const noexcept { return m_curRLevel; }

   std::pair<Timestep, SIRD_Levels> run_timestep();

private:
   Timestep m_curTimestep = 0;
   Timestep m_numberOfInfectionTimestepsRemaining;
   SIRD_Levels m_curLevels;
   int numberInfectedThisDay = 0;
   double m_curRLevel = 0.0;
   std::vector<Community> m_communities;
   std::vector<Agent> m_agents;

   std::vector<AgentId> m_cache_agentsToInfect;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_WORLD_H