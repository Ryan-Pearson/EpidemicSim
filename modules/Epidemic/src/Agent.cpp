#include "Epidemic/Agent.h"

namespace Epidemic {

static AgentId uniqueAgentId = -1;
static AgentType uniqueAgentType = -1;
std::unordered_map<std::string, AgentType> Agent::s_typeByName;

constexpr double MAX_INFECTION_RADIUS = 10.0;

Agent::Agent(const std::string& agentName, Building* curBuilding, const Building::Position& curPosition) :
   m_id(++uniqueAgentId),
   m_type(get_agent_type_by_name(agentName)),
   m_curBuilding(curBuilding),
   m_curPosition(curPosition)
{
   (void)m_curPosition;
   (void)m_curBuilding;
}

AgentType Agent::get_agent_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueAgentType);
      it = emplace_ret.first;
   }
   return it->second;
}

std::vector<AgentId> Agent::infect_nearby_agents() const
{
   std::vector<AgentId> ret;

   const auto nearbyAgentsAndDistance = m_curBuilding->get_nearby_agents(MAX_INFECTION_RADIUS, m_id);
   for(const auto& curAgentAndDistance : nearbyAgentsAndDistance)
   {
      const AgentId agentToAttemptToInfect = curAgentAndDistance.first;
      const Building::Distance distanceToAgent = curAgentAndDistance.second;

      // TODO: Make better
      static std::normal_distribution infectionDist(0.0, 2.0);
      const double draw = infectionDist(Statistics::GLOBAL_RANDOM_ENGINE);
      const bool infectionSuccessful = std::abs(draw) > distanceToAgent;

      if (infectionSuccessful)
      {
         ret.emplace_back(agentToAttemptToInfect);
      }
   }

   return ret;
}

} // namespace Epidemic