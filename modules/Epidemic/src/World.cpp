#include "Epidemic/World.h"

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

   auto visitor = update_SIR{ret};

   for (const auto& agent : agents)
   {
      std::visit(visitor, agent.get_SIRD_state());
   }

   return ret;
}

World::World(std::vector<Community> communities, std::vector<Agent> agents) :
   m_communities(std::move(communities)), m_agents(std::move(agents))
{
   m_curLevels = get_updated_levels(m_agents);
}

std::pair<Timestep, World::SIRD_Levels> World::run_timestep()
{
   ++m_curTimestep;

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
      m_agents[agentToInfect].attempt_infection(m_curTimestep);
   }

   m_curLevels = get_updated_levels(m_agents);

   return std::make_pair(m_curTimestep, m_curLevels);
}

} // namespace Epidemic