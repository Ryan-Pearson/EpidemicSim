#include "Epidemic/Agent.h"

namespace Epidemic {

static AgentId uniqueAgentId = -1;
static AgentType uniqueAgentType = -1;
std::unordered_map<std::string, AgentType> Agent::s_typeByName;

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

} // namespace Epidemic