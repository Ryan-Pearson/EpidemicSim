#include "Epidemic/World.h"

// STL
#include <algorithm>

namespace Epidemic {

struct update_SIR
{
   World::SIRD_Levels& m_toUpdate;
   void operator()(const SirdState::Susceptible&) const { ++m_toUpdate.m_numSusceptible; }
   void operator()(const SirdState::Infectious&) const { ++m_toUpdate.m_numInfectious; }
   void operator()(const SirdState::Recovered&) const { ++m_toUpdate.m_numRecovered; }
   void operator()(const SirdState::Deceased&) const { ++m_toUpdate.m_numDeceased; }
};

static World::SIRD_Levels get_updated_levels(const std::vector<Agent>& agents)
{
   World::SIRD_Levels ret;

   auto visitor = update_SIR {ret};

   for (const auto& agent : agents)
   {
      std::visit(visitor, agent.get_SIRD_state());
   }

   return ret;
}

World::World(
   std::vector<Community> communities, std::vector<Agent> agents, const Timestep numberOfInfectionTimestepsRemaining) :
   m_numberOfInfectionTimestepsRemaining(numberOfInfectionTimestepsRemaining),
   m_communities(std::move(communities)),
   m_agents(std::move(agents))
{
   m_curLevels = get_updated_levels(m_agents);
}

std::pair<Timestep, World::SIRD_Levels> World::run_timestep()
{
   ++m_curTimestep;
   m_numberOfInfectionTimestepsRemaining -= m_curLevels.m_numInfectious;

   // Update agent SIRD status and move them
   for (auto& agent : m_agents)
   {
      agent.update_agent_state(m_curTimestep);
      agent.move_agent(m_curTimestep);
   }

   // Check for nearby agents to infect
   m_cache_agentsToInfect.clear();
   for (auto& agent : m_agents)
   {
      const auto agentsToInfect = agent.get_nearby_agents_to_infect();
      m_cache_agentsToInfect.insert(m_cache_agentsToInfect.end(), agentsToInfect.cbegin(), agentsToInfect.cend());
   }

   std::sort(m_cache_agentsToInfect.begin(), m_cache_agentsToInfect.end());
   m_cache_agentsToInfect.erase(
      std::unique(m_cache_agentsToInfect.begin(), m_cache_agentsToInfect.end()), m_cache_agentsToInfect.end());

   for (const auto agentToInfect : m_cache_agentsToInfect)
   {
      const auto numberOfDaysInfected = m_agents[agentToInfect].attempt_infection(m_curTimestep);
      if (numberOfDaysInfected)
      {
         m_numberOfInfectionTimestepsRemaining += numberOfDaysInfected.value();
         ++numberInfectedThisDay;
      }
   }

   m_curLevels = get_updated_levels(m_agents);

   if ((m_curTimestep % TIMESTEP_PER_DAY) == 0)
   {
      const double daysRemaining =
         static_cast<double>(m_numberOfInfectionTimestepsRemaining) / static_cast<double>(TIMESTEP_PER_DAY);
      const double expectedTotalInfectedStart = daysRemaining
         + static_cast<double>(m_curLevels.m_numInfectious) * static_cast<double>(m_curTimestep / TIMESTEP_PER_DAY);
      const double expectedTotalInfectedConverged = daysRemaining * 2.0;
      const double expectedTotalInfectedDays = std::min(expectedTotalInfectedStart, expectedTotalInfectedConverged);
      const double boundedNumInfected = static_cast<double>(std::max(size_t(1), m_curLevels.m_numInfectious));
      const double expectedInfectedDaysPerAgent = expectedTotalInfectedDays / boundedNumInfected;
      const double infectionsPerPerson = static_cast<double>(numberInfectedThisDay) / boundedNumInfected;
      m_curRLevel = expectedInfectedDaysPerAgent * infectionsPerPerson;
      numberInfectedThisDay = 0;
   }

   return std::make_pair(m_curTimestep, m_curLevels);
}

} // namespace Epidemic